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

#include <xio/event/event_data.h>
#include <xio/event/waker.h>

namespace xio {
    turbo::Status empty_handler_callback(FileHandle handle, EventData *data, turbo::Time cur) {
        TURBO_UNUSED(handle);
        TURBO_UNUSED(data);
        TURBO_UNUSED(cur);
        return turbo::OkStatus();
    }

    bool EventData::event_data_is_waker() const {
        return (event_status & EventDataStatus::kEventWaker) != EventDataStatus::kEventNone;
    }

    bool EventData::event_data_is_in_loop() const {
        return (event_status & EventDataStatus::kEventInPoller) != EventDataStatus::kEventNone;
    }

    void EventData::set_event_data_waker() {
        event_status |= EventDataStatus::kEventWaker;
    }

    void EventData::set_event_data_not_waker() {
        static constexpr int kNotWakerMask = ~static_cast<int>(EventDataStatus::kEventWaker);

        event_status = static_cast<EventDataStatus>(static_cast<int32_t>(event_status) & kNotWakerMask);
    }

    void EventData::set_in_loop() {
        event_status |= EventDataStatus::kEventInPoller;
    }

    void EventData::set_out_loop() {
        static constexpr int kNotInLoopMask = ~static_cast<int>(EventDataStatus::kEventInPoller);

        event_status = static_cast<EventDataStatus>(static_cast<int32_t>(event_status) & kNotInLoopMask);
    }

} // namespace xio
