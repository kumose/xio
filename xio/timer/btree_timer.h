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
#include <memory>

#include <fermat/container/intrusive_list.h>
#include <turbo/container/btree_map.h>
#include <turbo/memory/resource_pool.h>
#include <turbo/times/time.h>
#include <xio/timer/timer.h>

namespace xio {

    struct BtreeTimerTask : public TimerTask {
        int64_t due_ms{0};
    };

    /// Timer scheduler keyed by millisecond expire time (btree_map<ms, TimerList>).
    class BtreeTimer : public TimerBase {
    public:
        BtreeTimer();

        ~BtreeTimer() override;

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

        std::string_view name() const override { return "BtreeTimer"; }

    private:
        using TimerList = fermat::IntrusiveList<BtreeTimerTask>;
        using TimerMap = turbo::btree_map<int64_t, std::unique_ptr<TimerList>>;

        turbo::Duration _default_poll_timeout;
        turbo::Time _start_time;

        size_t _count{0};

        TimerMap _map;
        TimerList _timeouts_to_run;

        turbo::Time _next_trigger_at;
        bool _destroying{false};

        static uint64_t make_timer_id(turbo::ResourceId<BtreeTimerTask> slot, uint32_t version);

        static turbo::ResourceId<BtreeTimerTask> slot_of_timer_id(uint64_t id);

        static uint32_t version_of_timer_id(uint64_t id);

        static void return_task_resource(uint64_t timer_id);

        int64_t calc_due_ms(turbo::Time time) const;

        turbo::Time compute_next_trigger_at(turbo::Time now) const;

        static void unlink_task(BtreeTimerTask *task);

        void unlink_from_map(BtreeTimerTask *task);

        bool should_enqueue_immediately(const BtreeTimerTask *task, turbo::Time now) const;

        void insert_timeouts_to_run(BtreeTimerTask *task);

        void cancel_task(BtreeTimerTask *task, uint64_t id);

        uint64_t schedule_task(BtreeTimerTask *task, turbo::ResourceId<BtreeTimerTask> slot_id);

        void process_expired(turbo::Time now);

        void purge_all();

        void purge_all(data_releaser free_param);

        TimerList &list_at(int64_t due_ms);
    };

}  // namespace xio
