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

#include <turbo/base/macros.h>
#if defined(OS_LINUX)

#include <liburing.h>

#include <xio/event/poller.h>

namespace xio {

    class UringPoller : public Poller {
    public:
        UringPoller();

        ~UringPoller() override;

        turbo::Status initialize(const PollerConfig &pc) override;

        turbo::Result<turbo::Time> poll(turbo::Duration dur,
                                        fermat::Vector<EventData *> &datas) override;

        turbo::Status handle_event(EventData * TURBO_RESTRICT data, EventType enable_event,
                                   EventType disable_event, turbo::Time cur) override;

        turbo::Status remove_event(EventData * TURBO_RESTRICT data, turbo::Time cur) override;

        bool supports_disk_async() const override { return true; }

        std::string_view name() const override { return "io_uring"; }

    private:
        turbo::Status submit_queue();

        turbo::Status apply_mask_change(EventData *data, int32_t old_mask, int32_t new_mask);

        static unsigned poll_mask_from_active(int32_t mask);

        static int32_t poll_events_to_mask(int poll_events);

        static void merge_ready(EventData *data, int32_t ready,
                                fermat::Vector<EventData *> &datas);

        struct io_uring _ring{};
        bool _initialized{false};
    };

}  // namespace xio

#endif  // defined(OS_LINUX)