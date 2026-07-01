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

#include <xio/timer/wheel_timer.h>
#include <algorithm>
#include <turbo/memory/resource_pool.h>

namespace xio {

    void WheelTimer::return_task_resource(uint64_t timer_id) {
        if (timer_id == kInvalidTimerId) {
            return;
        }
        turbo::return_resource(slot_of_timer_id(timer_id));
    }

    WheelTimer::WheelTimer()
            : _tick_interval(turbo::Duration::milliseconds(1)),
              _default_poll_timeout(turbo::Duration::milliseconds(100)),
              _start_time(turbo::Time::current_time()),
              _buckets(kWheelSlotCount),
              _bitmap(kBitmapWords, 0),
              _next_trigger_at(turbo::Time::current_time() + turbo::Duration::milliseconds(100)) {}

    WheelTimer::~WheelTimer() {
        _destroying = true;
        purge_all();
    }

    void WheelTimer::purge_all() { cancel_all(); }

    void WheelTimer::cancel_all() {
        for (size_t i = 0; i < kWheelSlotCount; ++i) {
            TimerList &list = _buckets[i];
            while (!list.empty()) {
                WheelTimerTask &t = list.front();
                void *param = t.arg;
                const uint64_t id = t.timer_id;
                list.pop_front();
                return_task_resource(id);
            }
        }
        while (!_timeouts_to_run.empty()) {
            WheelTimerTask &t = _timeouts_to_run.front();
            void *param = t.arg;
            const uint64_t id = t.timer_id;
            _timeouts_to_run.pop_front();
            return_task_resource(id);
        }
        _count = 0;
    }


    void WheelTimer::cancel_all(data_releaser free_param) {
        for (size_t i = 0; i < kWheelSlotCount; ++i) {
            TimerList &list = _buckets[i];
            while (!list.empty()) {
                WheelTimerTask &t = list.front();
                void *param = t.arg;
                const uint64_t id = t.timer_id;
                list.pop_front();
                return_task_resource(id);
                free_param(param);
            }
        }
        while (!_timeouts_to_run.empty()) {
            WheelTimerTask &t = _timeouts_to_run.front();
            void *param = t.arg;
            const uint64_t id = t.timer_id;
            _timeouts_to_run.pop_front();
            return_task_resource(id);
            free_param(param);
        }
        _count = 0;
    }

    size_t WheelTimer::slot_index(int level, unsigned tick) {
        return static_cast<size_t>(level) * kWheelSize + (tick & kWheelMask);
    }

    WheelTimer::TimerList &WheelTimer::bucket(int level, unsigned tick) {
        return _buckets[slot_index(level, tick)];
    }

    const WheelTimer::TimerList &WheelTimer::bucket(int level, unsigned tick) const {
        return _buckets[slot_index(level, tick)];
    }

    void WheelTimer::set_bitmap_bit(unsigned tick) { _bitmap[tick >> 6] |= (1ull << (tick & 63)); }

    void WheelTimer::clear_bitmap_bit(unsigned tick) { _bitmap[tick >> 6] &= ~(1ull << (tick & 63)); }

    uint64_t WheelTimer::make_timer_id(turbo::ResourceId<WheelTimerTask> slot, uint32_t version) {
        return (static_cast<uint64_t>(version) << 32) | slot.value;
    }

    turbo::ResourceId<WheelTimerTask> WheelTimer::slot_of_timer_id(uint64_t id) {
        return turbo::ResourceId<WheelTimerTask>{static_cast<uint32_t>(id & 0xfffffffful)};
    }

    uint32_t WheelTimer::version_of_timer_id(uint64_t id) { return static_cast<uint32_t>(id >> 32); }

    int64_t WheelTimer::calc_next_tick(turbo::Time now) const {
        const int64_t ms = turbo::Duration::to_milliseconds(now - _start_time);
        if (ms < 0) {
            return 0;
        }
        const int64_t tick_ms = turbo::Duration::to_milliseconds(_tick_interval);
        if (tick_ms <= 0) {
            return 0;
        }
        return ms / tick_ms;
    }

    int64_t WheelTimer::time_to_wheel_ticks(turbo::Duration d) const {
        const int64_t ms = turbo::Duration::to_milliseconds(d);
        const int64_t tick_ms = turbo::Duration::to_milliseconds(_tick_interval);
        if (tick_ms <= 0) {
            return 0;
        }
        return ms / tick_ms;
    }

    int64_t WheelTimer::ticks_until_next(int64_t next_tick) const {
        auto const start = static_cast<unsigned>(next_tick & kWheelMask);
        if (start != 0) {
            for (unsigned i = start; i < kWheelSize; ++i) {
                if (!bucket(0, i).empty()) {
                    return static_cast<int64_t>(i - start + 1);
                }
            }
            return static_cast<int64_t>(kWheelSize - ((next_tick - 1) & kWheelMask));
        }
        if (!bucket(0, 0).empty()) {
            return 1;
        }
        for (unsigned i = 1; i < kWheelSize; ++i) {
            if (!bucket(0, i).empty()) {
                return static_cast<int64_t>(i + 1);
            }
        }
        return static_cast<int64_t>(kWheelSize);
    }

    turbo::Time WheelTimer::compute_next_trigger_at(turbo::Time now) const {
        if (_count == 0) {
            return now + _default_poll_timeout;
        }
        turbo::Time next = now + _default_poll_timeout;
        const int64_t next_tick = calc_next_tick(now);
        const int64_t ticks = ticks_until_next(next_tick);
        const int64_t fire_tick = next_tick + ticks - 1;
        next = _start_time + _tick_interval * fire_tick;
        if (!_timeouts_to_run.empty()) {
            const turbo::Time queue_earliest = _timeouts_to_run.front().expire_time;
            if (queue_earliest < next) {
                next = queue_earliest;
            }
        }
        return next;
    }

    void WheelTimer::schedule_timeout_impl(WheelTimerTask *task, int64_t due_tick,
                                           int64_t next_tick_to_process, int64_t next_tick) {
        int64_t diff = due_tick - next_tick_to_process;
        TimerList *list = nullptr;

        if (diff < 0) {
            list = &bucket(0, static_cast<unsigned>(next_tick & kWheelMask));
            set_bitmap_bit(static_cast<unsigned>(next_tick & kWheelMask));
            task->bucket = static_cast<int32_t>(next_tick & kWheelMask);
        } else if (diff < static_cast<int64_t>(kWheelSize)) {
            const unsigned slot = static_cast<unsigned>(due_tick & kWheelMask);
            list = &bucket(0, slot);
            set_bitmap_bit(slot);
            task->bucket = static_cast<int32_t>(slot);
        } else if (diff < (1ll << (2 * kWheelBits))) {
            list = &bucket(1, static_cast<unsigned>((due_tick >> kWheelBits) & kWheelMask));
            task->bucket = -1;
        } else if (diff < (1ll << (3 * kWheelBits))) {
            list = &bucket(2, static_cast<unsigned>((due_tick >> (2 * kWheelBits)) & kWheelMask));
            task->bucket = -1;
        } else {
            if (diff > static_cast<int64_t>(kLargestSlot)) {
                due_tick = static_cast<int64_t>(kLargestSlot) + next_tick_to_process;
            }
            list = &bucket(3, static_cast<unsigned>((due_tick >> (3 * kWheelBits)) & kWheelMask));
            task->bucket = -1;
        }

        task->due_tick = due_tick;
        task->timer = this;
        list->push_back(*task);
    }

    bool WheelTimer::cascade_timers(int level, unsigned tick, turbo::Time now) {
        TimerList tmp;
        tmp.swap(bucket(level, tick));
        const int64_t next_tick = calc_next_tick(now);
        while (!tmp.empty()) {
            WheelTimerTask &t = tmp.front();
            tmp.pop_front();
            int64_t remaining_ms = turbo::Duration::to_milliseconds(t.expire_time - now);
            if (remaining_ms < 0) {
                remaining_ms = 0;
            }
            const int64_t due =
                    next_tick + time_to_wheel_ticks(turbo::Duration::milliseconds(remaining_ms));
            t.due_tick = due;
            if (should_enqueue_immediately(&t, now)) {
                t.timer = nullptr;
                t.bucket = -1;
                insert_timeouts_to_run(&t);
            } else {
                schedule_timeout_impl(&t, due, _expire_tick, next_tick);
            }
        }
        return tick == 0;
    }

    void WheelTimer::unlink_task(WheelTimerTask *task) {
        if (task->mpNext != nullptr || task->mpPrev != nullptr) {
            TimerList::remove(*task);
        }
    }

    void WheelTimer::unlink_from_wheel(WheelTimerTask *task) {
        if (task->timer != this) {
            return;
        }
        unlink_task(task);
        if (task->bucket >= 0 && bucket(0, static_cast<unsigned>(task->bucket)).empty()) {
            clear_bitmap_bit(static_cast<unsigned>(task->bucket));
        }
        task->timer = nullptr;
        task->bucket = -1;
    }

    bool WheelTimer::should_enqueue_immediately(const WheelTimerTask *task, turbo::Time now) const {
        if (calc_next_tick(task->expire_time) <= calc_next_tick(now)) {
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

    // _timeouts_to_run: ascending expire_time (full order, not only front).
    void WheelTimer::insert_timeouts_to_run(WheelTimerTask *task) {
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
            WheelTimerTask &cur = *it;
            if (task->expire_time < cur.expire_time) {
                _timeouts_to_run.insert(it, *task);
                return;
            }
        }
        _timeouts_to_run.push_back(*task);
    }

    void WheelTimer::cancel_task(WheelTimerTask *task, uint64_t id) {
        const turbo::Time canceled_expire = task->expire_time;
        bool removed = false;
        if (task->timer == this) {
            unlink_from_wheel(task);
            removed = true;
        } else if (task->mpNext != nullptr || task->mpPrev != nullptr) {
            unlink_task(task);
            removed = true;
        }
        if (!removed) {
            // Callback is running: task already popped from _timeouts_to_run.
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

    uint64_t WheelTimer::schedule_task(WheelTimerTask *task, turbo::ResourceId<WheelTimerTask> slot_id) {
        const turbo::Time now = turbo::Time::current_time();
        if (task->timer == this) {
            unlink_from_wheel(task);
            if (_count > 0) {
                --_count;
            }
        } else if (task->mpNext != nullptr || task->mpPrev != nullptr) {
            unlink_task(task);
        }

        task->version += 2;
        const uint64_t id = make_timer_id(slot_id, task->version);
        task->timer_id = id;

        const int64_t due_tick = calc_next_tick(task->expire_time);
        task->due_tick = due_tick;
        const bool immediate = should_enqueue_immediately(task, now);
        if (immediate) {
            task->timer = nullptr;
            task->bucket = -1;
            insert_timeouts_to_run(task);
        } else {
            const int64_t next_tick = calc_next_tick(now);
            const int64_t base_tick = std::min(_expire_tick, next_tick);
            schedule_timeout_impl(task, due_tick, base_tick, next_tick);
        }
        ++_count;
        if (immediate || task->expire_time < _next_trigger_at) {
            _next_trigger_at = compute_next_trigger_at(now);
        }
        return id;
    }

    void WheelTimer::process_expired(turbo::Time now) {
        const int64_t next_tick = calc_next_tick(now);

        while (_expire_tick <= next_tick) {
            const int idx = static_cast<int>(_expire_tick & kWheelMask);
            if (idx == 0) {
                if (cascade_timers(1, static_cast<unsigned>((_expire_tick >> kWheelBits) & kWheelMask),
                                   now) &&
                    cascade_timers(2,
                                   static_cast<unsigned>((_expire_tick >> (2 * kWheelBits)) &
                                                         kWheelMask),
                                   now) &&
                    cascade_timers(3,
                                   static_cast<unsigned>((_expire_tick >> (3 * kWheelBits)) &
                                                         kWheelMask),
                                   now)) {
                }
            }

            TimerList *cbs = &bucket(0, static_cast<unsigned>(idx));
            while (!cbs->empty()) {
                WheelTimerTask &t = cbs->front();
                cbs->pop_front();
                t.timer = nullptr;
                t.bucket = -1;
                insert_timeouts_to_run(&t);
            }

            clear_bitmap_bit(static_cast<unsigned>(idx));
            if (_expire_tick == next_tick) {
                ++_expire_tick;
                break;
            }
            ++_expire_tick;
        }

        while (!_timeouts_to_run.empty()) {
            WheelTimerTask &t = _timeouts_to_run.front();
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
            t.bucket = -1;

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

            cb(arg, t.expire_time,now);

            if (t.timer_id != id || t.version != id_version) {
                return_task_resource(id);
                continue;
            }

            t.version += 2;
            t.timer_id = kInvalidTimerId;
            return_task_resource(id);

            if (repeating && period > turbo::Duration::zero()) {
                expire = expire + period;
                turbo::ResourceId<WheelTimerTask> slot_id;
                auto *task = turbo::get_resource<WheelTimerTask>(&slot_id);
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

    turbo::Time WheelTimer::next_timeout() const { return _next_trigger_at; }

    void WheelTimer::on_trigger(turbo::Time stamp) {
        process_expired(stamp);
        _next_trigger_at = compute_next_trigger_at(stamp);
    }

    uint64_t WheelTimer::run_at(turbo::Time expire_time, timer_callback callback, void *param) {
        turbo::ResourceId<WheelTimerTask> slot_id;
        auto *task = turbo::get_resource<WheelTimerTask>(&slot_id);
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

    uint64_t WheelTimer::run_after(turbo::Duration duration, timer_callback callback, void *param) {
        const turbo::Time now = turbo::Time::current_time();
        return run_at(now + duration, callback, param);
    }

    uint64_t WheelTimer::run_every(turbo::Duration duration, timer_callback callback, void *param) {
        const turbo::Time now = turbo::Time::current_time();
        return run_every(now + duration, duration, callback, param);
    }

    uint64_t WheelTimer::run_every(turbo::Time expire_time, turbo::Duration duration,
                                   timer_callback callback, void *param) {
        turbo::ResourceId<WheelTimerTask> slot_id;
        auto *task = turbo::get_resource<WheelTimerTask>(&slot_id);
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

    void WheelTimer::cancel(uint64_t id) {
        if (id == kInvalidTimerId) {
            return;
        }
        const turbo::ResourceId<WheelTimerTask> slot_id = slot_of_timer_id(id);
        WheelTimerTask *task = turbo::address_resource(slot_id);
        if (task == nullptr) {
            return;
        }
        if (version_of_timer_id(id) != task->version) {
            return;
        }
        cancel_task(task, id);
    }

}  // namespace xio
