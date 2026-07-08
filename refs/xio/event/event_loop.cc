// Copyright (C) 2026 Kumo inc. and its affiliates. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#include <xio/event/event_loop.h>
#include <new>
#include <thread>
#include <xio/event/poller.h>
#include <turbo/memory/object_pool.h>
#include <turbo/memory/object_pool_inl.h>
#include <xio/event/thread_id.h>

namespace xio {
    namespace {
        thread_local turbo::Time tls_loop_now{};

        turbo::Time loop_now() {
            return tls_loop_now;
        }
    } // namespace

    thread_local EventLoop *EventLoop::tls_event_loop = nullptr;

    void EventLoop::run_at_post_trampoline(void *arg) {
        auto *ctx = static_cast<TimerPostCtx *>(arg);
        auto id = ctx->loop->run_at_in_loop(ctx->expire_time, ctx->callback, ctx->param);
        ctx->promise.set_value(std::move(id));
    }

    void EventLoop::cancel_timer_post_trampoline(void *arg) {
        auto *ctx = static_cast<TimerPostCtx *>(arg);
        ctx->loop->cancel_timer_in_loop(ctx->timer_id);
        ctx->promise.set_value(0);
    }

    void EventLoop::run_stop_token(void *arg) {
        auto *ctx = static_cast<StopToken *>(arg);
        ctx->promise.set_value(0);
    }

    void EventLoop::handle_events_post_trampoline(void *arg) {
        auto *ctx = static_cast<HandleEventsPostCtx *>(arg);
        auto st = ctx->loop->handle_events_in_loop(ctx->data, ctx->enable_event, ctx->disable_event, loop_now());
        ctx->promise.set_value(std::move(st));
    }


    void EventLoop::remove_events_post_trampoline(void *arg) {
        auto *ctx = static_cast<HandleEventsPostCtx *>(arg);
        auto st = ctx->loop->remove_events_in_loop(ctx->data, loop_now());
        ctx->promise.set_value(std::move(st));
    }

    void EventLoop::release_post_task(AsyncTask &task) {
        task.free_func(task.arg);
        task.arg = nullptr;
        task.func = nullptr;
        task.free_func = empty_data_releaser;
    }

    void EventLoop::run_post_task(AsyncTask &task) {
        if (task.func != nullptr) {
            task.func(task.arg);
        }
        release_post_task(task);
    }

    void EventLoop::clear_post_tasks() {
        fermat::Vector<AsyncTask> pending;
        pending.swap(_local_tasks);
        {
            std::unique_lock lock(_task_mutex);
            for (size_t i = 0; i < _remote_tasks.size(); ++i) {
                pending.push_back(_remote_tasks[i]);
            }
            _remote_tasks.clear();
        }
        for (size_t i = 0; i < pending.size(); ++i) {
            release_post_task(pending[i]);
        }
    }

    EventLoop::~EventLoop() {
        DKCHECK(!_thread.joinable());
        clear_resource();
    }

    bool EventLoop::is_in_loop_thread() const { return tls_event_loop == this; }

    bool EventLoop::supports_disk_async() const {
        return _poller != nullptr && _poller->supports_disk_async();
    }

    std::string_view EventLoop::backend() const {
        if (_poller == nullptr) {
            return std::string_view{};
        }
        return _poller->name();
    }

    turbo::Status EventLoop::start(const EventLoopOption &option) {
        if (_loop_status != EventLoopStatus::kEventLoopNone) {
            return _status;
        }

        std::unique_lock<std::mutex> lock(_lifecycle_mutex);
        if (_loop_status != EventLoopStatus::kEventLoopNone) {
            return _status;
        }
        _option = option;

        if (option.disk_io) {
            _poller = Poller::make_disk_async_poller();
        } else {
            _poller = Poller::make_poller();
        }
        if (_poller == nullptr) {
            return turbo::unimplemented_error("EventLoop: no poller on this platform");
        }


        turbo::Status st = _poller->initialize(option.poller_config);
        if (!st.ok()) {
            _poller.reset();
            return st;
        }

        st = _waker.open();
        if (!st.ok()) {
            _poller.reset();
            return st;
        }

        st = _waker.register_with(&_waker.event(), this, _poller.get(), turbo::Time::current_time());
        if (!st.ok()) {
            _waker.close();
            _poller.reset();
            return st;
        }
        if (option.wheel_timer) {
            _timer = TimerBase::create_wheel_timer();
        } else {
            _timer = TimerBase::create_btree_timer();
        }

        _thread = std::thread([this]() { loop_main(); });

        _lifecycle_cv.wait(lock, [this] { return _loop_status != EventLoopStatus::kEventLoopNone; });
        if (!_status.ok()) {
            _thread.join();
            clear_resource();
        }
        return _status;
    }

    void EventLoop::stop() {
        if (!_running.load() || _loop_status != EventLoopStatus::kEventLoopRunning) {
            return;
        }
        sync();
        _running.store(false);
        wakeup();
        {
            std::unique_lock<std::mutex> lock(_lifecycle_mutex);
            _lifecycle_cv.wait(lock, [this]() { return _loop_status == EventLoopStatus::kEventLoopStopped; });
        }
        _thread.join();
    }

    void EventLoop::sync() {
        if (is_in_loop_thread()) {
            return;
        }
        StopToken token;
        auto f = token.promise.get_future();
        post_task(run_stop_token, &token);
        f.wait();
    }

    void EventLoop::loop_main() {
        tls_event_loop = this;
        _loop_thread_id = ThreadId::get_id();
        if (this->_option.initialize_tasks.empty()) {
            _status = turbo::OkStatus();
        } else {
            for (auto &it: this->_option.initialize_tasks) {
                _status = it.func(it.arg);
                if (!_status.ok()) {
                    break;
                }
            }
        }

        if (_status.ok()) {
            std::unique_lock<std::mutex> lock(_lifecycle_mutex);
            this->_loop_status = EventLoopStatus::kEventLoopRunning;
            _lifecycle_cv.notify_all();
        } else {
            std::unique_lock<std::mutex> lock(_lifecycle_mutex);
            this->_loop_status = EventLoopStatus::kEventLoopStopped;
            _lifecycle_cv.notify_all();
            return;
        }
        /////////////////////////////////////////////////////////////////////////////
        /// loop

        while (_running.load(std::memory_order_acquire)) {
            /// first run post
            drain_post_tasks();
            /// run persist task
            for (auto &it: _loop_tasks) {
                it.func(it.arg);
            }
            tls_loop_now = turbo::Time::current_time();
            run_once();
        }
        /////////////////////////////////////////////////////////////////////////
        ///stopping
        this->_status = turbo::failed_precondition_error("finalized");
        this->_loop_status = EventLoopStatus::kEventLoopStoping;
        {
            for (auto it = _loop_exit_tasks.rbegin(); it != _loop_exit_tasks.rend(); ++it) {
                it->func(it->arg);
            }
        }

        std::lock_guard<std::mutex> lock(_lifecycle_mutex);
        _lifecycle_cv.notify_all();
        this->_loop_status = EventLoopStatus::kEventLoopStopped;
    }

    void EventLoop::run_once() {
        fermat::Vector<EventData *> ready;
        auto rs = _poller->poll(poll_timeout(tls_loop_now), ready);
        if (!rs.ok()) {
            KLOG_EVERY_MIN(ERROR) << rs.status().message();
            return;
        }
        tls_loop_now = rs.value_or_die();
        dispatch_ready(ready);
        _timer->on_trigger(tls_loop_now);
    }

    void EventLoop::drain_post_tasks() {
        fermat::Vector<AsyncTask> local_batch;
        local_batch.swap(_local_tasks);

        fermat::Vector<AsyncTask> remote_batch;
        {
            std::unique_lock lock(_task_mutex);
            remote_batch.swap(_remote_tasks);
        }

        for (size_t i = 0; i < remote_batch.size(); ++i) {
            run_post_task(remote_batch[i]);
        }
        for (size_t i = 0; i < local_batch.size(); ++i) {
            run_post_task(local_batch[i]);
        }
    }

    turbo::Duration EventLoop::poll_timeout(turbo::Time now) const {
        const turbo::Time next = _timer->next_timeout();
        turbo::Duration dur = next - now;
        static const turbo::Duration max_wait = turbo::Duration::seconds(30);
        if (dur > max_wait) {
            dur = max_wait;
        }
        return dur;
    }

    void EventLoop::dispatch_ready(const fermat::Vector<EventData *> &ready) {
        const turbo::Time cur = loop_now();
        for (size_t i = 0; i < ready.size(); ++i) {
            EventData *data = ready[i];
            if (data->event_data_is_waker()) {
                (void) event_data_on_waker_ready(data->handle, data, cur);
                continue;
            }
            const int32_t ev = static_cast<int32_t>(data->triggered);
            if ((ev & static_cast<int32_t>(EventType::kEventRead)) != 0) {
                (void) data->read_callback(data->handle, data, cur);
            }
            if ((ev & static_cast<int32_t>(EventType::kEventWrite)) != 0) {
                (void) data->write_callback(data->handle, data, cur);
            }
            if ((ev & static_cast<int32_t>(EventType::kEventError)) != 0) {
                (void) data->error_callback(data->handle, data, cur);
            }
            if ((ev & static_cast<int32_t>(EventType::kEventHangUp)) != 0) {
                (void) data->error_callback(data->handle, data, cur);
            }
            if ((ev & static_cast<int32_t>(EventType::kEventReadHup)) != 0) {
                (void) data->read_callback(data->handle, data, cur);
            }
        }
    }

    void EventLoop::post_loop_task(task_func func, void *arg) {
        AsyncTask task;
        task.func = func;
        task.arg = arg;
        post_loop_task(task);
    }

    void EventLoop::post_loop_task(AsyncTask task) {
        if (is_in_loop_thread()) {
            _loop_tasks.push_back(task);
            return;
        }
        AsyncTask t;
        auto ctx = turbo::get_object<PostTaskCtx>();
        ctx->task = task;
        ctx->loop = this;
        t.arg = ctx;
        t.func = post_loop_task_impl;
        t.free_func = post_task_ctx_free;
        post_task(t);
    }

    void EventLoop::post_task_ctx_free(void *arg) {
        auto ptr = static_cast<PostTaskCtx *>(arg);
        turbo::return_object<PostTaskCtx>(ptr);
    }

    void EventLoop::post_loop_task_impl(void *arg) {
        auto ptr = static_cast<PostTaskCtx *>(arg);
        ptr->loop->_loop_tasks.push_back(ptr->task);
    }

    void EventLoop::cancel_loop_task_impl(void *arg) {
        auto ptr = static_cast<PostTaskCtx *>(arg);
        auto loop = ptr->loop;
        auto task = ptr->task;
        fermat::Vector<AsyncTask> tmp;
        tmp.reserve(loop->_loop_tasks.size());
        for (auto &it: loop->_loop_tasks) {
            if (it.func == task.func && it.arg == task.arg && it.free_func == task.free_func) {
                continue;
            }
            tmp.push_back(it);
        }
        loop->_loop_tasks.swap(tmp);
    }

    void EventLoop::post_exit_task_impl(void *arg) {
        auto ptr = static_cast<PostTaskCtx *>(arg);
        ptr->loop->_loop_exit_tasks.push_back(ptr->task);
    }

    void EventLoop::cancel_exit_task_impl(void *arg) {
        auto ptr = static_cast<PostTaskCtx *>(arg);
        auto loop = ptr->loop;
        auto task = ptr->task;
        fermat::Vector<AsyncTask> tmp;
        tmp.reserve(loop->_loop_exit_tasks.size());
        for (auto &it: loop->_loop_exit_tasks) {
            if (it.func == task.func && it.arg == task.arg && it.free_func == task.free_func) {
                continue;
            }
            tmp.push_back(it);
        }
        loop->_loop_exit_tasks.swap(tmp);
    }


    void EventLoop::cancel_loop_task(task_func func, void *arg) {
        AsyncTask task;
        task.func = func;
        task.arg = arg;
        cancel_loop_task(task);
    }

    void EventLoop::cancel_loop_task(AsyncTask task) {
        if (is_in_loop_thread()) {
            fermat::Vector<AsyncTask> tmp;
            tmp.reserve(_loop_tasks.size());
            for (auto &it: _loop_tasks) {
                if (it.func == task.func && it.arg == task.arg && it.free_func == task.free_func) {
                    continue;
                }
                tmp.push_back(it);
            }
            _loop_tasks.swap(tmp);
            return;
        }
        AsyncTask t;
        auto ctx = turbo::get_object<PostTaskCtx>();
        ctx->task = task;
        ctx->loop = this;
        t.arg = ctx;
        t.func = cancel_loop_task_impl;
        t.free_func = post_task_ctx_free;
        post_task(t);
    }

    void EventLoop::post_exit_task(task_func func, void *arg) {
        AsyncTask task;
        task.func = func;
        task.arg = arg;
        post_exit_task(task);
    }

    void EventLoop::post_exit_task(AsyncTask task) {
        if (is_in_loop_thread()) {
            _loop_exit_tasks.push_back(task);
            return;
        }
        AsyncTask t;
        auto ctx = turbo::get_object<PostTaskCtx>();
        ctx->task = task;
        ctx->loop = this;
        t.arg = ctx;
        t.func = post_exit_task_impl;
        t.free_func = post_task_ctx_free;
        post_task(t);
    }

    void EventLoop::cancel_exit_task(task_func func, void *arg) {
        AsyncTask task;
        task.func = func;
        task.arg = arg;
        cancel_exit_task(task);
    }

    void EventLoop::cancel_exit_task(AsyncTask task) {
        if (is_in_loop_thread()) {
            fermat::Vector<AsyncTask> tmp;
            tmp.reserve(_loop_exit_tasks.size());
            for (auto &it: _loop_exit_tasks) {
                if (it.func == task.func && it.arg == task.arg && it.free_func == task.free_func) {
                    continue;
                }
                tmp.push_back(it);
            }
            _loop_exit_tasks.swap(tmp);
            return;
        }
        AsyncTask t;
        auto ctx = turbo::get_object<PostTaskCtx>();
        ctx->task = task;
        ctx->loop = this;
        t.arg = ctx;
        t.func = cancel_exit_task_impl;
        t.free_func = post_task_ctx_free;
        post_task(t);
    }

    void EventLoop::post_task(task_func func, void *arg) { post_task_impl(AsyncTask{arg, func}); }

    void EventLoop::post_task(AsyncTask task) { post_task_impl(task); }


    void EventLoop::post_task(turbo::span<AsyncTask> tasks) {
        if (is_in_loop_thread()) {
            _local_tasks.reserve(_local_tasks.size() + tasks.size());
            for (const auto &task: tasks) {
                _local_tasks.push_back(task);
            }
            return;
        }
        {
            std::unique_lock lock(_task_mutex);
            _remote_tasks.reserve(_remote_tasks.size() + tasks.size());
            for (const auto &task: tasks) {
                _remote_tasks.push_back(task);
            }
        }
        wakeup();
    }

    void EventLoop::post_task_in_loop(turbo::span<AsyncTask> tasks) {
        DKCHECK(is_in_loop_thread());
        _local_tasks.reserve(_local_tasks.size() + tasks.size());
        for (const auto &task: tasks) {
            _local_tasks.push_back(task);
        }
    }

    void EventLoop::post_task_impl(AsyncTask task) {
        if (is_in_loop_thread()) {
            _local_tasks.push_back(task);
            return;
        }
        {
            std::unique_lock lock(_task_mutex);
            _remote_tasks.push_back(task);
        }
        wakeup();
    }

    void EventLoop::post_task_in_loop(task_func func, void *arg) {
        post_task_in_loop(AsyncTask{arg, func});
    }

    void EventLoop::post_task_in_loop(AsyncTask task) {
        DKCHECK(is_in_loop_thread());
        _local_tasks.push_back(task);
    }

    void EventLoop::clear_timers() {
        if (_timer) {
            _timer->cancel_all();
        }
    }

    void EventLoop::wakeup() {
        if (is_in_loop_thread() || _loop_status != EventLoopStatus::kEventLoopRunning) {
            return;
        }
        _waker.signal();
    }

    turbo::Status EventLoop::handle_events(EventData *data, EventType enable_event, EventType disable_event) {
        if (is_in_loop_thread()) {
            return handle_events_in_loop(data, enable_event, disable_event, loop_now());
        }

        HandleEventsPostCtx ctx;
        ctx.loop = this;
        ctx.data = data;
        ctx.enable_event = enable_event;
        ctx.disable_event = disable_event;
        std::future<turbo::Status> future = ctx.promise.get_future();
        post_task_impl(AsyncTask{&ctx, handle_events_post_trampoline});
        return future.get();
    }

    turbo::Status EventLoop::remove_events(EventData * TURBO_RESTRICT data) {
        if (is_in_loop_thread()) {
            return remove_events_in_loop(data, loop_now());
        }

        HandleEventsPostCtx ctx;
        ctx.loop = this;
        ctx.data = data;
        std::future<turbo::Status> future = ctx.promise.get_future();
        post_task_impl(AsyncTask{&ctx, remove_events_post_trampoline});
        return future.get();
    }

    turbo::Status EventLoop::handle_events_in_loop(EventData *data, EventType enable_event, EventType disable_event,
                                                   turbo::Time cur) {
        DKCHECK(is_in_loop_thread());
        TURBO_RETURN_NOT_OK(_poller->handle_event(data, enable_event, disable_event, cur));
        return turbo::OkStatus();
    }

    turbo::Status EventLoop::remove_events_in_loop(EventData *data, turbo::Time cur) {
        DKCHECK(is_in_loop_thread());
        turbo::Status st = _poller->remove_event(data, cur);
        data->set_out_loop();
        return st;
    }

    turbo::Status EventLoop::handle_events_impl(EventData *data, EventType event_type,
                                                turbo::Time cur) {
        return enable_events_in_loop(data, event_type, cur);
    }

    void EventLoop::clear_resource() {
        for (auto &it: _loop_tasks) {
            it.free_func(it.arg);
        }
        _loop_tasks.clear();

        for (auto &it: _loop_exit_tasks) {
            it.free_func(it.arg);
        }
        _loop_exit_tasks.clear();
        clear_post_tasks();
        clear_timers();
        _waker.close();
        _poller.reset();
    }

    uint64_t EventLoop::run_at(turbo::Time expire_time, timer_callback callback, void *param) {
        if (!is_in_loop_thread()) {
            TimerPostCtx ctx;
            ctx.loop = this;
            ctx.expire_time = expire_time;
            ctx.callback = callback;
            ctx.param = param;
            auto f = ctx.promise.get_future();
            post_task(AsyncTask{&ctx, run_at_post_trampoline});
            return f.get();
        }
        return run_at_in_loop(expire_time, callback, param);
    }

    uint64_t EventLoop::run_after(turbo::Duration duration, timer_callback callback, void *param) {
        auto t = turbo::Time::current_time() + duration;
        return run_at(t, callback, param);
    }
    void EventLoop::cancel_timer(uint64_t timer_id) {
        if (!is_in_loop_thread()) {
            TimerPostCtx ctx;
            ctx.loop = this;
            ctx.timer_id = timer_id;
            auto f = ctx.promise.get_future();
            post_task(AsyncTask{&ctx, cancel_timer_post_trampoline});
            f.get();
            return;
        }
        cancel_timer_in_loop(timer_id);
    }

    uint64_t EventLoop::run_at_in_loop(turbo::Time expire_time, timer_callback callback, void *param) {
        DKCHECK(is_in_loop_thread());
        return _timer->run_at(expire_time, callback, param);
    }

    uint64_t EventLoop::run_after_in_loop(turbo::Duration duration, timer_callback callback, void *param) {
        DKCHECK(is_in_loop_thread());
        return _timer->run_after(duration, callback, param);
    }

    void EventLoop::cancel_timer_in_loop(uint64_t timer_id) {
        _timer->cancel(timer_id);
    }
} // namespace xio
