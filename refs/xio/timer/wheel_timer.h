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

#pragma once

#include <cstddef>
#include <cstdint>

#include <fermat/container/intrusive_list.h>
#include <fermat/container/vector.h>
#include <turbo/memory/resource_pool.h>
#include <turbo/times/time.h>
#include <xio/timer/timer.h>

namespace xio {
    struct WheelTimerTask : public TimerTask {
        int64_t due_tick{0};
        int32_t bucket{-1};
    };
    /// Folly HHWheelTimer: 4 levels x 256 slots, flat _buckets[kWheelSlotCount].
    class WheelTimer : public TimerBase {
    public:
        WheelTimer();

        ~WheelTimer() override;

        uint64_t run_at(turbo::Time expire_time, timer_callback callback, void *param) override;

        uint64_t run_after(turbo::Duration duration, timer_callback callback, void *param) override;

        uint64_t run_every(turbo::Duration duration, timer_callback callback, void *param) override;

        uint64_t run_every(turbo::Time expire_time, turbo::Duration duration, timer_callback callback,
                           void *param) override;

        turbo::Time next_timeout() const override;

        void on_trigger(turbo::Time stamp) override;

        void cancel(uint64_t id) override;

        void cancel_all() override;

        void cancel_all(data_releaser free_param) override;

        std::string_view name() const override {
            return "WheelTimer";
        }

    private:
        static constexpr int kWheelLevels = 4;
        static constexpr int kWheelBits = 8;
        static constexpr unsigned kWheelSize = 1u << kWheelBits;
        static constexpr unsigned kWheelMask = kWheelSize - 1;
        static constexpr size_t kWheelSlotCount =
                static_cast<size_t>(kWheelLevels) * kWheelSize;  // 1024
        static constexpr size_t kBitmapWords = (kWheelSize + 63) / 64;           // 4
        static constexpr uint32_t kLargestSlot = 0xffffffffUL;

        using TimerList = fermat::IntrusiveList<WheelTimerTask>;

        turbo::Duration _tick_interval;
        turbo::Duration _default_poll_timeout;
        turbo::Time _start_time;

        int64_t _expire_tick{1};
        size_t _count{0};

        fermat::Vector<TimerList> _buckets;       // size kWheelSlotCount == 1024
        fermat::Vector<uint64_t> _bitmap;         // size kBitmapWords == 4 (level-0 only)
        // Ready-to-fire tasks, sorted by expire_time ascending.
        TimerList _timeouts_to_run;

        turbo::Time _next_trigger_at;
        bool _destroying{false};

        static size_t slot_index(int level, unsigned tick);

        TimerList &bucket(int level, unsigned tick);

        const TimerList &bucket(int level, unsigned tick) const;

        void set_bitmap_bit(unsigned tick);

        void clear_bitmap_bit(unsigned tick);

        static uint64_t make_timer_id(turbo::ResourceId<WheelTimerTask> slot, uint32_t version);

        static turbo::ResourceId<WheelTimerTask> slot_of_timer_id(uint64_t id);

        static uint32_t version_of_timer_id(uint64_t id);

        static void return_task_resource(uint64_t timer_id);

        int64_t calc_next_tick(turbo::Time now) const;

        int64_t time_to_wheel_ticks(turbo::Duration d) const;

        int64_t ticks_until_next(int64_t next_tick) const;

        turbo::Time compute_next_trigger_at(turbo::Time now) const;

        void schedule_timeout_impl(WheelTimerTask *task, int64_t due_tick, int64_t next_tick_to_process,
                                   int64_t next_tick);

        bool cascade_timers(int level, unsigned tick, turbo::Time now);

        static void unlink_task(WheelTimerTask *task);

        void cancel_task(WheelTimerTask *task, uint64_t id);

        void unlink_from_wheel(WheelTimerTask *task);

        bool should_enqueue_immediately(const WheelTimerTask *task, turbo::Time now) const;

        void insert_timeouts_to_run(WheelTimerTask *task);

        uint64_t schedule_task(WheelTimerTask *task, turbo::ResourceId<WheelTimerTask> slot_id);

        void process_expired(turbo::Time now);

        void purge_all();
    };
} // namespace xio
