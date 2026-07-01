#include <xio/io/pair_ware.h>
#include <turbo/base/fd_utility.h>
#include <turbo/utility/status.h>
#include <turbo/memory/object_pool.h>
#include <cerrno>

#if defined(OS_WIN)
#include <winsock2.h>
#define ERRNO WSAGetLastError()
#else
#include <unistd.h>
#define ERRNO errno
#endif

namespace xio {
    void PairWare::bind(FileHandle read_fd, FileHandle write_fd, EventLoop *loop) {
        _read_fd = read_fd;
        _write_fd = write_fd;
        _event_loop = loop;

        _read_event_data.handle = read_fd;
        _read_event_data.data = this;
        _read_event_data.read_callback = PairWare::on_read_call;
        _read_event_data.error_callback = PairWare::on_error_call;

        _write_event_data.handle = write_fd;
        _write_event_data.data = this;
        _write_event_data.write_callback = PairWare::on_write_call;
        _write_event_data.error_callback = PairWare::on_error_call;
    }

    void PairWare::read_timeout_at(turbo::Time time) {
        DKCHECK(_event_loop->is_in_loop_thread());
        cancel_read_timer();
        _read_timeout = time;
        _read_timer_id = _event_loop->run_at(_read_timeout, read_timer_callback, this);
    }

    void PairWare::cancel_read_timer() {
        DKCHECK(_event_loop->is_in_loop_thread());
        if (_read_timer_id != TimerBase::kInvalidTimerId) {
            _event_loop->cancel_timer(_read_timer_id);
            _read_timer_id = TimerBase::kInvalidTimerId;
        }
    }

    void PairWare::write_timeout_at(turbo::Time time) {
        DKCHECK(_event_loop->is_in_loop_thread());
        cancel_write_timer();
        _write_timeout = time;
        _write_timer_id = _event_loop->run_at(_write_timeout, write_timer_callback, this);
    }

    void PairWare::cancel_write_timer() {
        DKCHECK(_event_loop->is_in_loop_thread());
        if (_write_timer_id != TimerBase::kInvalidTimerId) {
            _event_loop->cancel_timer(_write_timer_id);
            _write_timer_id = TimerBase::kInvalidTimerId;
        }
    }

    void PairWare::attach_impl() {
        if (_enable_read_flag) {
            (void) enable_read();
        } else {
            (void) disable_read();
        }
        if (_enable_write_flag) {
            (void) enable_write();
        } else {
            (void) disable_write();
        }
    }

    void PairWare::attach_trampoline(void *arg) {
        static_cast<PairWare *>(arg)->attach_impl();
    }

    void PairWare::emit(bool enable_read, bool enable_write) {
        _enable_read_flag = enable_read;
        _enable_write_flag = enable_write;
        if (_event_loop->is_in_loop_thread()) {
            attach_impl();
        } else {
            _event_loop->post_task(attach_trampoline, this);
        }
    }

    void PairWare::detach_trampoline(void *arg) {
        static_cast<PairWare *>(arg)->detach_impl();
    }

    void PairWare::run_async_trampoline(void *arg) {
        auto *self = static_cast<TaskProxy *>(arg);
        self->func(self->p, self->arg);
    }

    void PairWare::run_sync_trampoline(void *arg) {
        auto *self = static_cast<SyncTaskProxy *>(arg);
        self->func(self->p, self->arg);
        self->promise.set_value(0);
    }

    void PairWare::run_proxy_free(void *arg) {
        turbo::return_object<TaskProxy>(reinterpret_cast<TaskProxy *>(arg));
    }

    void PairWare::detach_impl() {
        cancel_all_timer();
        (void) disable_all();
        _enable_read_flag = false;
        _enable_write_flag = false;
    }

    void PairWare::detach() {
        if (_event_loop->is_in_loop_thread()) {
            detach_impl();
        } else {
            _event_loop->post_task(detach_trampoline, this);
        }
    }

    void PairWare::run_async(PairFactor func, void *arg) {
        if (_event_loop->is_in_loop_thread()) {
            func(this, arg);
            return;
        }
        auto *ptr = turbo::get_object<TaskProxy>();
        ptr->arg = arg;
        ptr->p = this;
        ptr->func = func;
        AsyncTask task;
        task.arg = ptr;
        task.func = run_async_trampoline;
        task.free_func = run_proxy_free;
        _event_loop->post_task(task);
    }

    void PairWare::run_sync(PairFactor func, void *arg) {
        if (_event_loop->is_in_loop_thread()) {
            func(this, arg);
            return;
        }
        SyncTaskProxy proxy;
        proxy.arg = arg;
        proxy.p = this;
        proxy.func = func;
        auto f = proxy.promise.get_future();
        _event_loop->post_task(run_sync_trampoline, &proxy);
        f.wait();
    }

    turbo::Status PairWare::enable_read() {
        DKCHECK(_event_loop->is_in_loop_thread());
        return _event_loop->enable_events_in_loop(&_read_event_data, EventType::kEventRead,
                                                  turbo::Time::current_time());
    }

    turbo::Status PairWare::enable_write() {
        DKCHECK(_event_loop->is_in_loop_thread());
        return _event_loop->enable_events_in_loop(&_write_event_data, EventType::kEventWrite,
                                                  turbo::Time::current_time());
    }

    turbo::Status PairWare::disable_read() {
        DKCHECK(_event_loop->is_in_loop_thread());
        return _event_loop->disable_events_in_loop(&_read_event_data, EventType::kEventRead,
                                                   turbo::Time::current_time());
    }

    turbo::Status PairWare::disable_write() {
        DKCHECK(_event_loop->is_in_loop_thread());
        return _event_loop->disable_events_in_loop(&_write_event_data, EventType::kEventWrite,
                                                   turbo::Time::current_time());
    }

    turbo::Status PairWare::disable_all() {
        DKCHECK(_event_loop->is_in_loop_thread());
        auto st = _event_loop->remove_events_in_loop(&_read_event_data, turbo::Time::current_time());
        if (!st.ok()) {
            return st;
        }
        return _event_loop->remove_events_in_loop(&_write_event_data, turbo::Time::current_time());
    }

    void PairWare::read_timer_callback(void *arg, turbo::Time dl, turbo::Time curr) {
        auto *self = static_cast<PairWare *>(arg);
        self->_read_timer_id = TimerBase::kInvalidTimerId;
        (void) self->on_read_timeout(dl, curr);
    }

    void PairWare::write_timer_callback(void *arg, turbo::Time dl, turbo::Time curr) {
        auto *self = static_cast<PairWare *>(arg);
        self->_write_timer_id = TimerBase::kInvalidTimerId;
        (void) self->on_write_timeout(dl, curr);
    }

    void PairWare::resume_read_callback(void *arg, turbo::Time, turbo::Time) {
        auto *self = static_cast<PairWare *>(arg);
        self->_resume_read_timer_id = TimerBase::kInvalidTimerId;
        (void) self->enable_read();
    }

    void PairWare::resume_write_callback(void *arg, turbo::Time, turbo::Time) {
        auto *self = static_cast<PairWare *>(arg);
        self->_resume_write_timer_id = TimerBase::kInvalidTimerId;
        (void) self->enable_write();
    }

    turbo::Status PairWare::resume_read_at(turbo::Time time) {
        DKCHECK(_event_loop->is_in_loop_thread());
        cancel_resume_read_timer();
        if (time <= turbo::Time::current_time()) {
            return enable_read();
        }
        _resume_read_time = time;
        _resume_read_timer_id = _event_loop->run_at(time, resume_read_callback, this);
        return turbo::OkStatus();
    }

    void PairWare::cancel_resume_read_timer() {
        DKCHECK(_event_loop->is_in_loop_thread());
        if (_resume_read_timer_id != TimerBase::kInvalidTimerId) {
            _event_loop->cancel_timer(_resume_read_timer_id);
            _resume_read_timer_id = TimerBase::kInvalidTimerId;
        }
    }

    turbo::Status PairWare::resume_write_at(turbo::Time time) {
        DKCHECK(_event_loop->is_in_loop_thread());
        cancel_resume_write_timer();
        if (time <= turbo::Time::current_time()) {
            return enable_write();
        }
        _resume_write_time = time;
        _resume_write_timer_id = _event_loop->run_at(time, resume_write_callback, this);
        return turbo::OkStatus();
    }

    void PairWare::cancel_resume_write_timer() {
        DKCHECK(_event_loop->is_in_loop_thread());
        if (_resume_write_timer_id != TimerBase::kInvalidTimerId) {
            _event_loop->cancel_timer(_resume_write_timer_id);
            _resume_write_timer_id = TimerBase::kInvalidTimerId;
        }
    }

    void PairWare::cancel_resume_all_timer() {
        cancel_resume_read_timer();
        cancel_resume_write_timer();
    }

    turbo::Status PairWare::resume_all_at(turbo::Time time) {
        auto st = resume_read_at(time);
        if (!st.ok()) {
            return st;
        }
        return resume_write_at(time);
    }

    void PairWare::cancel_all_timer() {
        cancel_read_timer();
        cancel_write_timer();
        cancel_resume_all_timer();
    }

    turbo::Status PairWare::make_nonblocking() {
        if (_read_fd != kInvalidFileHandle && turbo::make_non_blocking(_read_fd) != 0) {
            return turbo::internal_error("Failed to set non-blocking on read fd", errno);
        }
        if (_write_fd != kInvalidFileHandle && turbo::make_non_blocking(_write_fd) != 0) {
            return turbo::internal_error("Failed to set non-blocking on write fd", errno);
        }
        return turbo::OkStatus();
    }

    void PairWare::close() {
        if (_read_fd != kInvalidFileHandle) {
#if defined(OS_WIN)
            ::closesocket(_read_fd);
#else
            ::close(_read_fd);
#endif
            _read_fd = kInvalidFileHandle;
        }
        if (_write_fd != kInvalidFileHandle && _write_fd != _read_fd) {
#if defined(OS_WIN)
            ::closesocket(_write_fd);
#else
            ::close(_write_fd);
#endif
            _write_fd = kInvalidFileHandle;
        }
    }

    turbo::Status PairWare::on_read_call(FileHandle, EventData *data, turbo::Time cur) {
        return static_cast<PairWare *>(data->data)->on_read(cur);
    }

    turbo::Status PairWare::on_write_call(FileHandle, EventData *data, turbo::Time cur) {
        return static_cast<PairWare *>(data->data)->on_write(cur);
    }

    turbo::Status PairWare::on_error_call(FileHandle, EventData *data, turbo::Time cur) {
        return static_cast<PairWare *>(data->data)->on_error(cur);
    }
} // namespace xio
