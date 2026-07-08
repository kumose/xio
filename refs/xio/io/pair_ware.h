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

#include <xio/io/fwd.h>
#include <xio/event/fwd.h>
#include <xio/event/event_loop.h>
#include <xio/event/event_data.h>
#include <xio/timer/timer.h>
#include <fermat/container/cord_buffer.h>

#include <future>

#ifndef DEFAULT_BUFFER_BLOCK
#define DEFAULT_BUFFER_BLOCK (16 * 1024)
#endif

#ifndef DEFAULT_BUFFER_ALIGNMENT
#define DEFAULT_BUFFER_ALIGNMENT 64
#endif

namespace xio {
    class PairWare;
    using PairFactor = void (*)(PairWare *, void *arg);

    /// Base class for a pair of file descriptors (socketpair, pipe) with event-driven management.
    /// Binds two fds: one for reading, one for writing.
    ///
    /// **Threading model**
    /// - **Thread-safe:** `emit`, `detach`, `run_async`, `run_sync`.
    /// - **EventLoop thread only:** `enable_read` / `enable_write` / `disable_*`, and all timer
    ///   APIs. Not thread-safe. From another thread, compose these inside a `run_async` or
    ///   `run_sync` callback.
    /// - **Fd helpers** (`make_nonblocking`, `close`): before poller registration (no `enable_*` /
    ///   `emit`), may be called from any thread when the **caller guarantees** exclusive fd
    ///   access; after registration, use the EventLoop thread or `run_async` / `run_sync`.
    class PairWare {
    public:
        PairWare() = default;

        virtual ~PairWare() = default;

        PairWare(const PairWare &) = delete;

        PairWare &operator=(const PairWare &) = delete;

        /// Binds a pair of file descriptors (read_fd, write_fd) to the EventLoop.
        /// Does not register fds with the poller; registration happens on first `enable_*` / `emit`.
        /// @param read_fd File descriptor used for read events.
        /// @param write_fd File descriptor used for write events.
        /// @param loop The EventLoop that will drive I/O events.
        /// @attention May be called off-loop during setup if the caller serializes fd access until
        /// hand-off to the loop. After poller registration, prefer the EventLoop thread.
        void bind(FileHandle read_fd, FileHandle write_fd, EventLoop *loop);

        /// Atomically enables or disables read and write event interests.
        /// Thread-safe. If called from outside the loop thread, the operation is posted
        /// asynchronously to the loop thread.
        /// @param enable_read True to enable read events on `read_fd`, false to disable.
        /// @param enable_write True to enable write events on `write_fd`, false to disable.
        void emit(bool enable_read, bool enable_write);

        /// Unregisters both fds and clears enable flags.
        /// Thread-safe. If called from outside the loop thread, the operation is posted
        /// asynchronously to the loop thread.
        void detach();

        /// Posts a callback to run on the EventLoop thread without blocking the caller.
        /// Thread-safe. Use this to compose in-loop operations (e.g. `enable_read`,
        /// `read_timeout_at`) from other threads.
        /// @param func Callback invoked on the loop thread; first argument is `this`.
        /// @param arg User argument passed to `func`.
        void run_async(PairFactor func, void *arg);

        /// Posts a callback to run on the EventLoop thread and blocks until it completes.
        /// Thread-safe. Use this when the caller needs to observe in-loop side effects
        /// before continuing.
        /// @param func Callback invoked on the loop thread; first argument is `this`.
        /// @param arg User argument passed to `func`.
        void run_sync(PairFactor func, void *arg);

        /// Pure virtual callback invoked when the read fd becomes readable.
        virtual turbo::Status on_read(turbo::Time cur) = 0;

        /// Pure virtual callback invoked when the write fd becomes writable.
        virtual turbo::Status on_write(turbo::Time cur) = 0;

        /// Pure virtual callback invoked when an error occurs on either fd.
        virtual turbo::Status on_error(turbo::Time cur) = 0;

        /// Pure virtual callback invoked when a read deadline expires.
        virtual turbo::Status on_read_timeout(turbo::Time expire, turbo::Time cur) = 0;

        virtual turbo::Status on_write_timeout(turbo::Time expire, turbo::Time cur) = 0;

        /// Enables read events on `read_fd` (I/O interest only; does not arm a deadline).
        /// @attention Must be called from the event loop thread. Not thread-safe.
        turbo::Status enable_read();

        /// Enables write events on `write_fd` (I/O interest only; does not arm a deadline).
        /// @attention Must be called from the event loop thread. Not thread-safe.
        turbo::Status enable_write();

        /// Disables read events on `read_fd`.
        /// @attention Must be called from the event loop thread. Not thread-safe.
        turbo::Status disable_read();

        /// Disables write events on `write_fd`.
        /// @attention Must be called from the event loop thread. Not thread-safe.
        turbo::Status disable_write();

        /// Removes both fds from the poller.
        /// @attention Must be called from the event loop thread. Not thread-safe.
        turbo::Status disable_all();

        /////////////////////////////////////////////////////////////////////////////////////////////////
        /// timer region
        ///
        /// Business-level deadlines and rate-limit resumes. Separate from `enable_read` /
        /// `enable_write`. All timer APIs below are in-loop only.

        /// Arms an absolute read deadline on the read direction.
        /// @param time Absolute expiration time.
        /// @attention Must be called from the event loop thread. Not thread-safe.
        void read_timeout_at(turbo::Time time);

        /// Cancels the active read deadline timer, if any.
        /// @attention Must be called from the event loop thread. Not thread-safe.
        void cancel_read_timer();

        /// Arms an absolute write deadline on the write direction.
        /// @param time Absolute expiration time.
        /// @attention Must be called from the event loop thread. Not thread-safe.
        void write_timeout_at(turbo::Time time);

        /// Cancels the active write deadline timer, if any.
        /// @attention Must be called from the event loop thread. Not thread-safe.
        void cancel_write_timer();

        /// Schedules re-enabling of read events at an absolute time (e.g. rate limiting).
        /// @param time Absolute time to call `enable_read`.
        /// @attention Must be called from the event loop thread. Not thread-safe.
        turbo::Status resume_read_at(turbo::Time time);

        /// Cancels a pending read resume timer, if any.
        /// @attention Must be called from the event loop thread. Not thread-safe.
        void cancel_resume_read_timer();

        /// Schedules re-enabling of write events at an absolute time.
        /// @param time Absolute time to call `enable_write`.
        /// @attention Must be called from the event loop thread. Not thread-safe.
        turbo::Status resume_write_at(turbo::Time time);

        /// Cancels a pending write resume timer, if any.
        /// @attention Must be called from the event loop thread. Not thread-safe.
        void cancel_resume_write_timer();

        /// Schedules re-enabling of both directions at the same absolute time.
        /// @param time Absolute time to resume read and write.
        /// @attention Must be called from the event loop thread. Not thread-safe.
        turbo::Status resume_all_at(turbo::Time time);

        /// Cancels pending read and write resume timers.
        /// @attention Must be called from the event loop thread. Not thread-safe.
        void cancel_resume_all_timer();

        /// Cancels all deadline and resume timers.
        /// @attention Must be called from the event loop thread. Not thread-safe.
        void cancel_all_timer();

        /// Returns the absolute read deadline last passed to `read_timeout_at`.
        /// @attention Must be called from the event loop thread. Not thread-safe.
        turbo::Time get_read_timeout_time() const { return _read_timeout; }

        /// Returns the EventLoop timer id for the active read deadline, or invalid if none.
        /// @attention Must be called from the event loop thread. Not thread-safe.
        uint64_t get_read_timer_id() const { return _read_timer_id; }

        /// Returns the absolute write deadline last passed to `write_timeout_at`.
        /// @attention Must be called from the event loop thread. Not thread-safe.
        turbo::Time get_write_timeout_time() const { return _write_timeout; }

        /// Returns the EventLoop timer id for the active write deadline, or invalid if none.
        /// @attention Must be called from the event loop thread. Not thread-safe.
        uint64_t get_write_timer_id() const { return _write_timer_id; }

        /// Returns the absolute time last passed to `resume_read_at`.
        /// @attention Must be called from the event loop thread. Not thread-safe.
        turbo::Time get_resume_read_time() const { return _resume_read_time; }

        /// Returns the EventLoop timer id for a pending read resume, or invalid if none.
        /// @attention Must be called from the event loop thread. Not thread-safe.
        uint64_t get_resume_read_timer_id() const { return _resume_read_timer_id; }

        /// Returns the absolute time last passed to `resume_write_at`.
        /// @attention Must be called from the event loop thread. Not thread-safe.
        turbo::Time get_resume_write_time() const { return _resume_write_time; }

        /// Returns the EventLoop timer id for a pending write resume, or invalid if none.
        /// @attention Must be called from the event loop thread. Not thread-safe.
        uint64_t get_resume_write_timer_id() const { return _resume_write_timer_id; }

        // -------------------------------------------------------------------------
        // Fd operations
        // -------------------------------------------------------------------------
        // Before poller registration: callable from any thread when the caller holds exclusive
        // access to the fds. After `enable_*` / `emit`: EventLoop thread only, or post via
        // `run_async` / `run_sync`.

        /// Makes both file descriptors non-blocking.
        /// @attention See fd-operation threading rules in the section comment above.
        turbo::Status make_nonblocking();

        /// Closes both file descriptors.
        /// @attention See fd-operation threading rules in the section comment above.
        void close();

    protected:
        static turbo::Status on_read_call(FileHandle handle, EventData *data, turbo::Time cur);

        static turbo::Status on_write_call(FileHandle handle, EventData *data, turbo::Time cur);

        static turbo::Status on_error_call(FileHandle handle, EventData *data, turbo::Time cur);

        void attach_impl();

        void detach_impl();

        struct TaskProxy {
            PairWare *p{nullptr};
            void *arg{nullptr};
            PairFactor func{nullptr};
        };

        struct SyncTaskProxy : public TaskProxy {
            std::promise<int> promise;
        };

        static void attach_trampoline(void *arg);

        static void detach_trampoline(void *arg);

        static void run_async_trampoline(void *arg);

        static void run_sync_trampoline(void *arg);

        static void run_proxy_free(void *arg);

        static void read_timer_callback(void *arg, turbo::Time dl, turbo::Time curr);

        static void write_timer_callback(void *arg, turbo::Time dl, turbo::Time curr);

        static void resume_read_callback(void *arg, turbo::Time, turbo::Time);

        static void resume_write_callback(void *arg, turbo::Time, turbo::Time);

        EventData _read_event_data;
        EventData _write_event_data;
        FileHandle _read_fd{kInvalidFileHandle};
        FileHandle _write_fd{kInvalidFileHandle};
        EventLoop *_event_loop{nullptr};

        bool _enable_read_flag{false};
        bool _enable_write_flag{false};

        turbo::Status _status;

        turbo::Time _read_timeout{turbo::Time::future_infinite()};
        turbo::Time _write_timeout{turbo::Time::future_infinite()};

        uint64_t _read_timer_id{TimerBase::kInvalidTimerId};
        uint64_t _write_timer_id{TimerBase::kInvalidTimerId};

        turbo::Time _resume_read_time{turbo::Time::future_infinite()};
        turbo::Time _resume_write_time{turbo::Time::future_infinite()};

        uint64_t _resume_read_timer_id{TimerBase::kInvalidTimerId};
        uint64_t _resume_write_timer_id{TimerBase::kInvalidTimerId};
    };
} // namespace xio
