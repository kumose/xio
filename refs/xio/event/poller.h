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

#include <fermat/container/vector.h>
#include <xio/event/event.h>
#include <xio/event/fwd.h>
#include <turbo/utility/status.h>
#include <xio/event/event_data.h>

namespace xio {
    /// Abstract interface for platform-specific event polling (epoll, kqueue, etc.).
    ///
    /// All methods of Poller must be called only from the thread that owns the
    /// associated EventLoop (i.e., from within the loop's run loop). The design
    /// guarantees this single-threaded access pattern, so no internal locking
    /// is needed.
    ///
    /// **Important:** All methods that accept an `EventData*` parameter assume
    /// the pointer is non-null. No null checks are performed internally.
    /// The caller must ensure that every `EventData*` passed to these functions
    /// points to a valid, properly constructed `EventData` object.
    ///
    /// Regarding registration state: the poller does not require an EventData
    /// to be "already registered" before calling enable/disable or handle_event.
    /// Implementations may automatically register the EventData on the first
    /// enable operation, or may return a specific error code; the abstract
    /// interface leaves this flexibility. Similarly, remove_event is safe to call
    /// even if the EventData is not currently registered (may be a no-op).
    class Poller {
    public:
        virtual ~Poller() = default;

        /// Initializes the poller instance with the given configuration.
        ///
        /// This method must be called once before any other Poller operations.
        /// It is invoked by EventLoop::start() on the loop's thread.
        ///
        /// @param pc Configuration parameters for the poller (e.g., max events, flags).
        /// @return OkStatus on success, or an error status if initialization fails.
        virtual turbo::Status initialize(const PollerConfig &pc) = 0;

        /// Waits for I/O events on registered file descriptors.
        ///
        /// The function blocks for at most `dur` duration, or indefinitely if
        /// `dur` is negative. When events occur, the corresponding EventData
        /// objects are appended to `datas` (which is cleared before use).
        /// The current time is returned via `turbo::Result<turbo::Time>`.
        ///
        /// @param dur Maximum time to block (negative means no timeout).
        /// @param datas Output vector that will contain ready EventData entries.
        /// @return The current time (as of after the poll) on success, or an error.
        virtual turbo::Result<turbo::Time> poll(turbo::Duration dur, fermat::Vector<EventData *> &datas) = 0;

        /// Atomically updates the event mask for a given EventData.
        ///
        /// Enables events in `enable_event` and disables events in `disable_event`
        /// in a single system call (e.g., epoll_ctl with EPOLL_CTL_MOD).
        /// This is the most efficient way to change event interests.
        ///
        /// If the EventData has not yet been registered with the poller,
        /// the implementation may automatically register it (treating disable_event as a no-op)
        /// or return an error indicating that explicit registration is required.
        ///
        /// @param data The target EventData. Must be non-null.
        /// @param enable_event Bitmask of events to add.
        /// @param disable_event Bitmask of events to remove.
        /// @return OkStatus on success, or an error.
        /// @pre data != nullptr
        virtual turbo::Status handle_event(EventData * TURBO_RESTRICT data, EventType enable_event,
                                           EventType disable_event , turbo::Time cur) = 0;

        /// Enables one or more event types for an EventData.
        ///
        /// Equivalent to `handle_event(data, event, EventType::kEventNone)`.
        /// If the EventData has not yet been registered, the implementation may
        /// register it with the specified events.
        ///
        /// @param data The target EventData. Must be non-null.
        /// @param event The event type(s) to enable.
        /// @return OkStatus on success, or an error.
        /// @pre data != nullptr
        turbo::Status enable_event(EventData * TURBO_RESTRICT data, EventType event, turbo::Time cur) {
            return handle_event(data, event, EventType::kEventNone, cur);
        }

        /// Disables one or more event types for an EventData.
        ///
        /// Equivalent to `handle_event(data, EventType::kEventNone, event)`.
        /// If the EventData is not currently registered, the call may be a no-op
        /// or return an error (implementation-defined).
        ///
        /// @param data The target EventData. Must be non-null.
        /// @param event The event type(s) to disable.
        /// @return OkStatus on success, or an error.
        /// @pre data != nullptr
        turbo::Status disable_event(EventData *TURBO_RESTRICT data, EventType event, turbo::Time cur) {
            return handle_event(data, EventType::kEventNone, event, cur);
        }

        /// Removes an EventData from the poller completely.
        ///
        /// After removal, the EventData is no longer monitored. The EventData
        /// may be reused later with a different loop or re-added.
        /// If the EventData is not currently registered, the call is typically a no-op.
        ///
        /// @param data The EventData to remove. Must be non-null.
        /// @return OkStatus on success, or an error.
        /// @pre data != nullptr
        virtual turbo::Status remove_event(EventData * TURBO_RESTRICT data, turbo::Time cur) = 0;

        /// Indicates whether the poller supports asynchronous disk I/O (AIO).
        /// This flag is used when the EventLoop is started with `supports_disk_async = true`.
        ///
        /// @return true if the poller can handle disk I/O asynchronously.
        virtual bool supports_disk_async() const = 0;

        /// Returns a human-readable name of the underlying poller backend
        /// (e.g., "epoll", "kqueue", "poll", "select").
        ///
        /// @return A string view naming the poller implementation.
        virtual std::string_view name() const = 0;

        /// Factory method to create the default poller (optimized for network sockets).
        static std::unique_ptr<Poller> make_poller();

        /// Factory method to create a poller that supports disk asynchronous I/O.
        static std::unique_ptr<Poller> make_disk_async_poller();

    protected:
        /// The EventLoop that owns this Poller. Set by EventLoop during construction.
        EventLoop *_loop{nullptr};
    };
} // namespace xio
