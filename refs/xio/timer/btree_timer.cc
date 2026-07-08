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

#include <xio/timer/btree_timer.h>

#include <turbo/memory/resource_pool.h>

namespace xio {

    void BtreeTimer::return_task_resource(uint64_t timer_id) {
        if (timer_id == kInvalidTimerId) {
            return;
        }
        turbo::return_resource(slot_of_timer_id(timer_id));
    }

    BtreeTimer::BtreeTimer()
            : _default_poll_timeout(turbo::Duration::milliseconds(100)),
              _start_time(turbo::Time::current_time()),
              _next_trigger_at(turbo::Time::current_time() + turbo::Duration::milliseconds(100)) {}

    BtreeTimer::~BtreeTimer() {
        _destroying = true;
        purge_all();
    }

    BtreeTimer::TimerList &BtreeTimer::list_at(int64_t due_ms) {
        auto it = _map.find(due_ms);
        if (it == _map.end()) {
            it = _map.emplace(due_ms, std::make_unique<TimerList>()).first;
        }
        return *it->second;
    }

    void BtreeTimer::purge_all() {
        for (auto it = _map.begin(); it != _map.end();) {
            TimerList &list = *it->second;
            while (!list.empty()) {
                BtreeTimerTask &t = list.front();
                list.pop_front();
                return_task_resource(t.timer_id);
            }
            it = _map.erase(it);
        }
        while (!_timeouts_to_run.empty()) {
            BtreeTimerTask &t = _timeouts_to_run.front();
            _timeouts_to_run.pop_front();
            return_task_resource(t.timer_id);
        }
        _count = 0;
    }

    void BtreeTimer::purge_all(data_releaser free_param) {
        for (auto it = _map.begin(); it != _map.end();) {
            TimerList &list = *it->second;
            while (!list.empty()) {
                BtreeTimerTask &t = list.front();
                void *param = t.arg;
                list.pop_front();
                return_task_resource(t.timer_id);
                free_param(param);
            }
            it = _map.erase(it);
        }
        while (!_timeouts_to_run.empty()) {
            BtreeTimerTask &t = _timeouts_to_run.front();
            void *param = t.arg;
            _timeouts_to_run.pop_front();
            return_task_resource(t.timer_id);
            free_param(param);
        }
        _count = 0;
    }

    uint64_t BtreeTimer::make_timer_id(turbo::ResourceId<BtreeTimerTask> slot, uint32_t version) {
        return (static_cast<uint64_t>(version) << 32) | slot.value;
    }

    turbo::ResourceId<BtreeTimerTask> BtreeTimer::slot_of_timer_id(uint64_t id) {
        return turbo::ResourceId<BtreeTimerTask>{static_cast<uint32_t>(id & 0xfffffffful)};
    }

    uint32_t BtreeTimer::version_of_timer_id(uint64_t id) {
        return static_cast<uint32_t>(id >> 32);
    }

    int64_t BtreeTimer::calc_due_ms(turbo::Time time) const {
        const int64_t ms = turbo::Duration::to_milliseconds(time - _start_time);
        return ms < 0 ? 0 : ms;
    }

    turbo::Time BtreeTimer::compute_next_trigger_at(turbo::Time now) const {
        if (_count == 0) {
            return now + _default_poll_timeout;
        }
        turbo::Time next = now + _default_poll_timeout;
        if (!_map.empty()) {
            const turbo::Time map_next =
                    _start_time + turbo::Duration::milliseconds(_map.begin()->first);
            if (map_next < next) {
                next = map_next;
            }
        }
        if (!_timeouts_to_run.empty()) {
            const turbo::Time queue_earliest = _timeouts_to_run.front().expire_time;
            if (queue_earliest < next) {
                next = queue_earliest;
            }
        }
        return next;
    }

    void BtreeTimer::unlink_task(BtreeTimerTask *task) {
        if (task->mpNext != nullptr || task->mpPrev != nullptr) {
            TimerList::remove(*task);
        }
    }

    void BtreeTimer::unlink_from_map(BtreeTimerTask *task) {
        if (task->timer != this) {
            return;
        }
        const int64_t due_ms = task->due_ms;
        unlink_task(task);
        auto it = _map.find(due_ms);
        if (it != _map.end() && it->second->empty()) {
            _map.erase(it);
        }
        task->timer = nullptr;
        task->due_ms = 0;
    }

    bool BtreeTimer::should_enqueue_immediately(const BtreeTimerTask *task, turbo::Time now) const {
        if (calc_due_ms(task->expire_time) <= calc_due_ms(now)) {
            return true;
        }
        if (_timeouts_to_run.empty()) {
            return false;
        }
        const turbo::Time front_expire = _timeouts_to_run.front().expire_time;
        if (front_expire > now) {
            return false;
        }
        return task->expire_time <= front_expire;
    }

    void BtreeTimer::insert_timeouts_to_run(BtreeTimerTask *task) {
        if (_timeouts_to_run.empty()) {
            _timeouts_to_run.push_back(*task);
            return;
        }
        if (task->expire_time >= _timeouts_to_run.back().expire_time) {
            _timeouts_to_run.push_back(*task);
            return;
        }
        if (task->expire_time < _timeouts_to_run.front().expire_time) {
            _timeouts_to_run.push_front(*task);
            return;
        }
        for (TimerList::iterator it = _timeouts_to_run.begin(); it != _timeouts_to_run.end(); ++it) {
            BtreeTimerTask &cur = *it;
            if (task->expire_time < cur.expire_time) {
                _timeouts_to_run.insert(it, *task);
                return;
            }
        }
        _timeouts_to_run.push_back(*task);
    }

    void BtreeTimer::cancel_task(BtreeTimerTask *task, uint64_t id) {
        const turbo::Time canceled_expire = task->expire_time;
        bool removed = false;
        if (task->timer == this) {
            unlink_from_map(task);
            removed = true;
        } else if (task->mpNext != nullptr || task->mpPrev != nullptr) {
            unlink_task(task);
            removed = true;
        }
        if (!removed) {
            if (id == kInvalidTimerId || task->timer_id != id ||
                version_of_timer_id(id) != task->version) {
                return;
            }
            task->version += 2;
            task->timer_id = kInvalidTimerId;
            const turbo::Time now = turbo::Time::current_time();
            if (_count == 0) {
                _next_trigger_at = now + _default_poll_timeout;
            } else if (canceled_expire <= _next_trigger_at) {
                _next_trigger_at = compute_next_trigger_at(now);
            }
            return;
        }
        if (_count > 0) {
            --_count;
        }
        task->version += 2;
        task->timer_id = kInvalidTimerId;
        return_task_resource(id);
        const turbo::Time now = turbo::Time::current_time();
        if (_count == 0) {
            _next_trigger_at = now + _default_poll_timeout;
        } else if (canceled_expire <= _next_trigger_at) {
            _next_trigger_at = compute_next_trigger_at(now);
        }
    }

    uint64_t BtreeTimer::schedule_task(BtreeTimerTask *task,
                                       turbo::ResourceId<BtreeTimerTask> slot_id) {
        const turbo::Time now = turbo::Time::current_time();
        if (task->timer == this) {
            unlink_from_map(task);
            if (_count > 0) {
                --_count;
            }
        } else if (task->mpNext != nullptr || task->mpPrev != nullptr) {
            unlink_task(task);
        }

        task->version += 2;
        const uint64_t id = make_timer_id(slot_id, task->version);
        task->timer_id = id;

        const int64_t due_ms = calc_due_ms(task->expire_time);
        task->due_ms = due_ms;
        const bool immediate = should_enqueue_immediately(task, now);
        if (immediate) {
            task->timer = nullptr;
            insert_timeouts_to_run(task);
        } else {
            task->timer = this;
            list_at(due_ms).push_back(*task);
        }
        ++_count;
        if (immediate || task->expire_time < _next_trigger_at) {
            _next_trigger_at = compute_next_trigger_at(now);
        }
        return id;
    }

    void BtreeTimer::process_expired(turbo::Time now) {
        const int64_t now_ms = calc_due_ms(now);

        while (!_map.empty()) {
            auto it = _map.begin();
            if (it->first > now_ms) {
                break;
            }
            TimerList ready;
            ready.swap(*it->second);
            _map.erase(it);
            while (!ready.empty()) {
                BtreeTimerTask &t = ready.front();
                ready.pop_front();
                t.timer = nullptr;
                t.due_ms = 0;
                insert_timeouts_to_run(&t);
            }
        }

        while (!_timeouts_to_run.empty()) {
            BtreeTimerTask &t = _timeouts_to_run.front();
            _timeouts_to_run.pop_front();

            const uint64_t id = t.timer_id;
            const uint32_t id_version = version_of_timer_id(id);
            if (t.version != id_version) {
                continue;
            }
            if (_count > 0) {
                --_count;
            }
            t.timer = nullptr;
            t.due_ms = 0;

            timer_callback cb = t.callback;
            void *arg = t.arg;
            const bool repeating = (t.status != 0);
            turbo::Duration period = t.duration;
            turbo::Time expire = t.expire_time;

            if (_destroying) {
                t.version += 2;
                t.timer_id = kInvalidTimerId;
                return_task_resource(id);
                continue;
            }

            if (cb != nullptr) {
                cb(arg, expire, now);   // 修正回调签名
            }

            if (t.timer_id != id || t.version != id_version) {
                return_task_resource(id);
                continue;
            }

            t.version += 2;
            t.timer_id = kInvalidTimerId;
            return_task_resource(id);

            if (repeating && period > turbo::Duration::zero()) {
                expire = expire + period;
                turbo::ResourceId<BtreeTimerTask> slot_id;
                BtreeTimerTask *task = turbo::get_resource<BtreeTimerTask>(&slot_id);
                if (task != nullptr) {
                    task->arg = arg;
                    task->callback = cb;
                    task->expire_time = expire;
                    task->duration = period;
                    task->status = 1u;
                    schedule_task(task, slot_id);
                }
            }
        }
    }

    turbo::Time BtreeTimer::next_timeout() const {
        return _next_trigger_at;
    }

    void BtreeTimer::on_trigger(turbo::Time stamp) {
        process_expired(stamp);
        _next_trigger_at = compute_next_trigger_at(stamp);
    }

    uint64_t BtreeTimer::run_at(turbo::Time expire_time, timer_callback callback, void *param) {
        turbo::ResourceId<BtreeTimerTask> slot_id;
        BtreeTimerTask *task = turbo::get_resource<BtreeTimerTask>(&slot_id);
        if (task == nullptr) {
            return kInvalidTimerId;
        }
        task->arg = param;
        task->callback = callback;
        task->expire_time = expire_time;
        task->duration = turbo::Duration::zero();
        task->status = 0u;
        return schedule_task(task, slot_id);
    }

    uint64_t BtreeTimer::run_after(turbo::Duration duration, timer_callback callback, void *param) {
        const turbo::Time now = turbo::Time::current_time();
        return run_at(now + duration, callback, param);
    }

    uint64_t BtreeTimer::run_every(turbo::Duration duration, timer_callback callback, void *param) {
        const turbo::Time now = turbo::Time::current_time();
        return run_every(now + duration, duration, callback, param);
    }

    uint64_t BtreeTimer::run_every(turbo::Time expire_time, turbo::Duration duration,
                                   timer_callback callback, void *param) {
        turbo::ResourceId<BtreeTimerTask> slot_id;
        BtreeTimerTask *task = turbo::get_resource<BtreeTimerTask>(&slot_id);
        if (task == nullptr) {
            return kInvalidTimerId;
        }
        task->arg = param;
        task->callback = callback;
        task->expire_time = expire_time;
        task->duration = duration;
        task->status = 1u;
        return schedule_task(task, slot_id);
    }

    void BtreeTimer::cancel(uint64_t id) {
        if (id == kInvalidTimerId) {
            return;
        }
        const turbo::ResourceId<BtreeTimerTask> slot_id = slot_of_timer_id(id);
        BtreeTimerTask *task = turbo::address_resource(slot_id);
        if (task == nullptr) {
            return;
        }
        if (version_of_timer_id(id) != task->version) {
            return;
        }
        cancel_task(task, id);
    }

    void BtreeTimer::cancel_all() {
        purge_all();
    }

    void BtreeTimer::cancel_all(data_releaser free_param) {
        purge_all(free_param);
    }

}  // namespace xio
