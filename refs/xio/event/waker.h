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

#include <atomic>

#include <xio/event/event_data.h>
#include <xio/event/fwd.h>
#include <turbo/utility/status.h>

namespace xio {
    /// Coalesced cross-thread notifier for EventLoop (eventfd / pipe / socket).
    class Waker {
    public:
        Waker();

        ~Waker();

        Waker(const Waker &) = delete;

        Waker &operator=(const Waker &) = delete;

        turbo::Status open();

        void close();

        bool is_open() const { return _read_handle != kInvalidFileHandle; }
        FileHandle read_handle() const { return _read_handle; }

        /// Bind |data| via event_data_setup_waker() and register READ with |poller|.
        /// @param cur Current time (from EventLoop) - required by Poller::enable_event.
        turbo::Status register_with(EventData *data, EventLoop *loop, Poller *poller, turbo::Time cur);

        /// Unregister from poller (no cur needed, internal uses current_time).
        void unregister(EventData *data, Poller *poller);

        /// Signal the loop at most once until the loop calls consume() / on_waker_ready.
        void signal();

        /// Drain kernel notification state; loop thread only.
        turbo::Status consume();

        EventData &event() { return _event; }
        const EventData &event() const { return _event; }

    private:
        void platform_open();

        void platform_close();

        void platform_signal();

        void platform_drain();

        EventData _event;
        FileHandle _read_handle{kInvalidFileHandle};
        FileHandle _write_handle{kInvalidFileHandle};
        std::atomic<bool> _wake_pending{false};
        Poller *_poller{nullptr};
        EventData *_registered_data{nullptr};
#if defined(OS_WIN)
        FileHandle _listen_sock{kInvalidFileHandle};
#endif
    };

    /// Configure an EventData to be used with a Waker.
    /// Sets up the handle, loop, read_callback, and user data pointer.
    turbo::Status event_data_on_waker_ready(FileHandle handle, EventData *data, turbo::Time cur);

    turbo::Status event_data_setup_waker(EventData *data, Waker *waker, EventLoop *loop);
} // namespace xio
