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

#include <xio/event/fwd.h>
#include <xio/event/event.h>
#include <turbo/utility/status.h>
#include <xio/timer/timer.h>

namespace xio {
    class Waker;

    /// Callback for I/O events (read, write, error) on a file handle.
    using handler_callback = turbo::Status (*)(FileHandle handle, EventData *data, turbo::Time cur);

    /// Default no-op handler callback for I/O events.
    turbo::Status empty_handler_callback(FileHandle handle, EventData *data, turbo::Time cur);

    /// Default no-op data releaser.
    void empty_data_releaser(void *data);

    /// Bitmask flags used in EventData::event_status.
    enum class EventDataStatus : int32_t {
        kEventNone = 0,
        kEventInPoller = 1 << 0, ///< EventData is currently registered in a poller.
        kEventWaker = 1 << 1, ///< EventData is associated with a Waker.
    };

    // Bitwise operators for EventDataStatus (allows combining flags with |, &= etc.)
    inline EventDataStatus operator|=(EventDataStatus &lhs, EventDataStatus rhs) {
        lhs = static_cast<EventDataStatus>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
        return lhs;
    }

    inline EventDataStatus operator|(EventDataStatus lhs, EventDataStatus rhs) {
        return static_cast<EventDataStatus>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
    }

    inline EventDataStatus operator&=(EventDataStatus &lhs, EventDataStatus rhs) {
        lhs = static_cast<EventDataStatus>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
        return lhs;
    }

    inline EventDataStatus operator&(EventDataStatus lhs, EventDataStatus rhs) {
        return static_cast<EventDataStatus>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
    }

    /// Lightweight I/O event control block.
    ///
    /// EventData carries only the metadata necessary for event dispatching
    /// within an EventLoop. It does not embed any business logic or protocol
    /// state. User-defined data is attached via the opaque `data` pointer
    /// and its associated `data_deleter`.
    ///
    /// All callbacks are guaranteed to be non-null throughout the lifetime
    /// of the object, which allows the event loop to call them unconditionally
    /// without branches.
    ///
    /// Thread safety: all operations on an EventData must occur on the
    /// thread that owns its associated EventLoop. Cross-loop operations
    /// require queuing to the target loop's thread.
    class EventData {
    public:
        EventData() = default;

        /// Destroys the EventData and releases the user-attached data.
        ///
        /// The release is performed by calling data_deleter(data), which
        /// defaults to a no-op. If the user has set a custom deleter, it
        /// will be invoked exactly once.
        ~EventData() {
            data_deleter(data);
        }

        /// File descriptor or socket handle monitored by the event loop.
        ///
        /// Set to kInvalidFileHandle initially. Must be a valid, non-blocking
        /// handle before being registered with the loop.
        FileHandle handle{kInvalidFileHandle};

        /// Opaque user data associated with this event.
        ///
        /// Lifetime is managed by the user through the `data_deleter` member.
        /// The loop itself never interprets or owns this pointer. It is safe
        /// to access from any callback on the loop's thread.
        void *data{nullptr};

        /// Callback invoked when the file handle becomes readable.
        ///
        /// Defaults to an empty no-op. The loop always calls this when
        /// a read event is delivered, without checking for null.
        handler_callback read_callback{empty_handler_callback};

        /// Callback invoked when the file handle becomes writable.
        ///
        /// Defaults to an empty no-op. The loop always calls this when
        /// a write event is delivered, without checking for null.
        handler_callback write_callback{empty_handler_callback};

        /// Callback invoked when an error is detected on the file handle.
        ///
        /// This covers asynchronous errors (e.g., EPOLLERR). The default
        /// is a no-op, but users should typically set this to handle
        /// connection failures gracefully.
        handler_callback error_callback{empty_handler_callback};

        /// Custom deleter for the user-attached `data` pointer.
        ///
        /// Called exactly once when the EventData is destroyed. Defaults
        /// to a no-op that does nothing. Set this if `data` owns resources
        /// that must be released (e.g., a connection object allocated
        /// with `new`).
        data_releaser data_deleter{empty_data_releaser};

        /// Bitmask of events currently registered with the underlying
        /// polling mechanism (e.g., epoll/kqueue).
        ///
        /// This is used internally by the loop to know what changes need
        /// to be made on the next epoll_ctl / kevent call. It is not
        /// intended for direct user manipulation.
        int32_t active_event{0};

        /// Internal status flags used by the event loop.
        ///
        /// Bits may indicate whether the EventData is paused, closed,
        /// scheduled for removal, etc. The exact definitions are private
        /// to the loop implementation.
        EventDataStatus event_status{EventDataStatus::kEventNone};

        /// The event type that triggered the current callback (read, write, error...).
        /// Used internally by poller and event loop. Users should treat it as read-only.
        EventType triggered{EventType::kEventNone};

        /// Marks the given EventData as belonging to a Waker.
        ///
        /// This function sets the EventDataStatus::kEventWaker flag on the data.
        /// It must only be called on an EventData that is already associated with
        /// a Waker (e.g., after event_data_setup_waker). The caller guarantees
        /// that the pointer is non-null and that the EventData is not concurrently
        /// accessed from another thread.
        ///
        /// @param data Non-null pointer to the EventData to be marked as waker.
        void set_event_data_waker();

        void set_event_data_not_waker();

        /// Marks the given EventData as currently registered in an EventLoop.
        ///
        /// This function sets the EventDataStatus::kEventInPoller flag on the data.
        /// It should be called after the EventData has been successfully added
        /// to the underlying poller. The caller guarantees that the pointer is
        /// non-null and that the flag is set only while the EventData remains
        /// registered.
        ///
        /// @param data Non-null pointer to the EventData that is now in the loop.
        void set_in_loop();

        void set_out_loop();


        /// Check whether an EventData is associated with a Waker.
        /// @param data Non-null pointer to the EventData.
        /// @return true if the event data is a waker, false otherwise.
        bool event_data_is_waker() const;

        /// Check whether an EventData is currently registered in an EventLoop.
        /// @param data Non-null pointer to the EventData.
        /// @return true if the event data is currently registered in a loop.
        bool event_data_is_in_loop() const;
    };
} // namespace xio
