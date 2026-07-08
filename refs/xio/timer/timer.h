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
#include <xio/event/fwd.h>
#include <fermat/container/intrusive_list.h>
#include <fermat/container/vector.h>
#include <turbo/memory/resource_pool.h>
#include <turbo/times/time.h>

namespace xio {
    typedef void (*timer_callback)(void *param, turbo::Time expire_time, turbo::Time cur);

    class TimerBase;

    struct TimerTask : public fermat::IntrusiveListNode {
        void *arg{nullptr};
        timer_callback callback{nullptr};
        turbo::Time expire_time;
        turbo::Duration duration;
        uint32_t version{0};
        uint32_t status{0};
        uint64_t timer_id{0};
        TimerBase *timer{nullptr};
    };

    /// TimerBase interface.
    class TimerBase {
    public:
        static constexpr uint64_t kInvalidTimerId = 0;

        virtual ~TimerBase() = default;

        virtual uint64_t run_at(turbo::Time expire_time, timer_callback callback, void *param = nullptr) = 0;

        virtual uint64_t run_after(turbo::Duration duration, timer_callback callback, void *param = nullptr) = 0;

        virtual uint64_t run_every(turbo::Duration duration, timer_callback callback, void *param = nullptr) = 0;

        virtual uint64_t run_every(turbo::Time expire_time, turbo::Duration duration, timer_callback callback,
                                   void *param = nullptr) = 0;

        virtual turbo::Time next_timeout() const = 0;

        virtual void on_trigger(turbo::Time stamp) = 0;

        virtual void cancel(uint64_t id) = 0;

        virtual void cancel_all() = 0;

        virtual void cancel_all(data_releaser free_param) = 0;

        virtual std::string_view name() const = 0;

        static std::unique_ptr<TimerBase> create_wheel_timer();

        static std::unique_ptr<TimerBase> create_btree_timer();
    };
} // namespace xio
