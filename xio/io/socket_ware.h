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

/// Platform-specific socket headers.
#if defined(OS_WIN)
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif


namespace xio {
    class SocketWare;
    using SocketFactor = void (*)(SocketWare *, void * arg);

    /// Base class for socket-based I/O handling with event-driven buffer management.
    ///
    /// **Threading model**
    /// - **Thread-safe:** `emit`, `detach`, `run_async`, `run_sync`.
    /// - **EventLoop thread only:** `enable_read` / `enable_write` / `disable_*`, and all timer
    ///   APIs (`read_timeout_at`, `resume_*_at`, getters, etc.). Not thread-safe. From another
    ///   thread, compose these inside a `run_async` or `run_sync` callback.
    /// - **Fd / TCP syscall wrappers** (`make_nonblocking`, `setsockopt`, `shutdown`, `close`, …):
    ///   Before the fd is registered with the poller (no `enable_*` / `emit` yet), may be called
    ///   from any thread if the **caller guarantees** exclusive, safe access to the fd (typical
    ///   setup path: create socket off-loop, set options, then hand off to the loop). Once
    ///   registered, use the EventLoop thread or `run_async` / `run_sync`.
    class SocketWare {
    public:
        SocketWare() = default;

        virtual ~SocketWare() = default;

        SocketWare(const SocketWare &) = delete;

        SocketWare &operator=(const SocketWare &) = delete;

        /// Binds the socket to a file descriptor and associates it with an EventLoop.
        /// Does not register the fd with the poller; registration happens on first `enable_*` / `emit`.
        /// @param fd The file descriptor of the socket.
        /// @param loop The EventLoop that will drive I/O events.
        /// @attention May be called off-loop during setup if the caller serializes fd access until
        /// hand-off to the loop. After poller registration, prefer the EventLoop thread.
        void bind(FileHandle fd, EventLoop *loop);

        /// Atomically enables or disables read and write event interests.
        /// Thread-safe. If called from outside the loop thread, the operation is posted
        /// asynchronously to the loop thread.
        /// @param enable_read True to enable read events, false to disable.
        /// @param enable_write True to enable write events, false to disable.
        void emit(bool enable_read, bool enable_write);

        /// Unregisters all events and clears enable flags.
        /// Thread-safe. If called from outside the loop thread, the operation is posted
        /// asynchronously to the loop thread.
        void detach();

        /// Posts a callback to run on the EventLoop thread without blocking the caller.
        /// Thread-safe. Use this to compose in-loop operations (e.g. `enable_read`,
        /// `read_timeout_at`) from other threads.
        /// @param func Callback invoked on the loop thread; first argument is `this`.
        /// @param arg User argument passed to `func`.
        void run_async(SocketFactor func, void *arg);

        /// Posts a callback to run on the EventLoop thread and blocks until it completes.
        /// Thread-safe. Use this when the caller needs to observe in-loop side effects
        /// before continuing.
        /// @param func Callback invoked on the loop thread; first argument is `this`.
        /// @param arg User argument passed to `func`.
        void run_sync(SocketFactor func, void *arg);

        /// Pure virtual callback invoked when the socket becomes readable.
        virtual turbo::Status on_read(turbo::Time cur) = 0;

        /// Pure virtual callback invoked when the socket becomes writable.
        virtual turbo::Status on_write(turbo::Time cur) = 0;

        /// Pure virtual callback invoked when an error occurs on the socket.
        virtual turbo::Status on_error(turbo::Time cur) = 0;

        /// Pure virtual callback invoked when a timeout expires.
        virtual turbo::Status on_read_timeout(turbo::Time expire, turbo::Time cur) = 0;

        virtual turbo::Status on_write_timeout(turbo::Time expire, turbo::Time cur) = 0;

        /// Enables read events (I/O interest only; does not arm a deadline).
        /// @attention Must be called from the event loop thread. Not thread-safe.
        turbo::Status enable_read();

        /// Enables write events (I/O interest only; does not arm a deadline).
        /// @attention Must be called from the event loop thread. Not thread-safe.
        turbo::Status enable_write();

        /// Disables read events.
        /// @attention Must be called from the event loop thread. Not thread-safe.
        turbo::Status disable_read();

        /// Disables write events.
        /// @attention Must be called from the event loop thread. Not thread-safe.
        turbo::Status disable_write();

        /// Removes the socket from the poller (read and write).
        /// @attention Must be called from the event loop thread. Not thread-safe.
        turbo::Status disable_all();

        /////////////////////////////////////////////////////////////////////////////////////////////////
        /// timer region
        ///
        /// Business-level deadlines and rate-limit resumes. Separate from `enable_read` /
        /// `enable_write`. All timer APIs below are in-loop only.

        /// Arms an absolute read deadline. May be called once and covers subsequent re-enables
        /// until cancelled or fired.
        /// @param time Absolute expiration time.
        /// @attention Must be called from the event loop thread. Not thread-safe.
        void read_timeout_at(turbo::Time time);

        /// Cancels the active read deadline timer, if any.
        /// @attention Must be called from the event loop thread. Not thread-safe.
        void cancel_read_timer();

        /// Arms an absolute write deadline.
        /// @param time Absolute expiration time.
        /// @attention Must be called from the event loop thread. Not thread-safe.
        void write_timeout_at(turbo::Time time);

        /// Cancels the active write deadline timer, if any.
        /// @attention Must be called from the event loop thread. Not thread-safe.
        void cancel_write_timer();

        /// Schedules re-enabling of read events at an absolute time (e.g. rate limiting).
        /// Does not change the read deadline set by `read_timeout_at`.
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

        /// Schedules re-enabling of both read and write at the same absolute time.
        /// @param time Absolute time to resume both directions.
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
        // TCP socket operations (system call wrappers)
        // -------------------------------------------------------------------------
        // Plain fd/syscall helpers. Before poller registration: callable from any thread when
        // the caller holds exclusive access to the fd (e.g. freshly created socket, options
        // applied before `emit`). After `enable_*` / `emit`: EventLoop thread only, or post
        // via `run_async` / `run_sync`.

        /// Shuts down the read half of the connection.
        /// @attention See fd-operation threading rules in the section comment above.
        void shutdown_read();

        /// Shuts down the write half of the connection.
        /// @attention See fd-operation threading rules in the section comment above.
        void shutdown_write();

        /// Shuts down both halves of the connection.
        /// @attention See fd-operation threading rules in the section comment above.
        void shutdown_all();

        /// Closes the underlying socket fd.
        /// @attention See fd-operation threading rules in the section comment above.
        void close();

        /// Makes the underlying file descriptor non-blocking.
        /// @attention See fd-operation threading rules in the section comment above.
        turbo::Status make_nonblocking();

        /// Sets the size of the socket send buffer (SO_SNDBUF).
        /// @attention See fd-operation threading rules in the section comment above.
        turbo::Status set_send_buffer_size(int size);

        /// Gets the current send buffer size.
        /// @attention See fd-operation threading rules in the section comment above.
        turbo::Result<int> get_send_buffer_size() const;

        /// Sets the size of the socket receive buffer (SO_RCVBUF).
        /// @attention See fd-operation threading rules in the section comment above.
        turbo::Status set_recv_buffer_size(int size);

        /// Gets the current receive buffer size.
        /// @attention See fd-operation threading rules in the section comment above.
        turbo::Result<int> get_recv_buffer_size() const;

        /// Enables or disables TCP_NODELAY (disable Nagle's algorithm).
        /// @attention See fd-operation threading rules in the section comment above.
        turbo::Status set_tcp_nodelay(bool on = true);

        /// Gets current TCP_NODELAY setting.
        /// @attention See fd-operation threading rules in the section comment above.
        turbo::Result<bool> get_tcp_nodelay() const;

        /// Enables or disables SO_REUSEADDR.
        /// @attention See fd-operation threading rules in the section comment above.
        turbo::Status set_reuse_addr(bool on = true);

        /// Gets current SO_REUSEADDR setting.
        /// @attention See fd-operation threading rules in the section comment above.
        turbo::Result<bool> get_reuse_addr() const;

        /// Enables or disables SO_REUSEPORT (Linux only).
        /// @attention See fd-operation threading rules in the section comment above.
        turbo::Status set_reuse_port(bool on = true);

        /// Gets current SO_REUSEPORT setting.
        /// @attention See fd-operation threading rules in the section comment above.
        turbo::Result<bool> get_reuse_port() const;

        /// Enables or disables SO_KEEPALIVE.
        /// @attention See fd-operation threading rules in the section comment above.
        turbo::Status set_keep_alive(bool on = true);

        /// Gets current SO_KEEPALIVE setting.
        /// @attention See fd-operation threading rules in the section comment above.
        turbo::Result<bool> get_keep_alive() const;

        /// Sets TCP keep-alive idle time (seconds) before sending probes.
        /// @attention See fd-operation threading rules in the section comment above.
        turbo::Status set_tcp_keep_idle(int seconds);

        /// Sets TCP keep-alive probe interval (seconds).
        /// @attention See fd-operation threading rules in the section comment above.
        turbo::Status set_tcp_keep_interval(int seconds);

        /// Sets TCP keep-alive probe count before declaring connection dead.
        /// @attention See fd-operation threading rules in the section comment above.
        turbo::Status set_tcp_keep_count(int count);

        /// Enables or disables SO_LINGER.
        /// @attention See fd-operation threading rules in the section comment above.
        turbo::Status set_linger(bool on, int seconds);

        /// Gets current SO_LINGER setting.
        /// @attention See fd-operation threading rules in the section comment above.
        turbo::Result<std::pair<bool, int> > get_linger() const;

        /// Enables or disables SO_OOBINLINE (out-of-band data inline).
        /// @attention See fd-operation threading rules in the section comment above.
        turbo::Status set_oob_inline(bool on = true);

        /// Enables or disables IP_TOS (Type of Service) for IPv4.
        /// @attention See fd-operation threading rules in the section comment above.
        turbo::Status set_ip_tos(int tos);

        /// Gets current IP_TOS value.
        /// @attention See fd-operation threading rules in the section comment above.
        turbo::Result<int> get_ip_tos() const;

        /// Enables or disables IP_TTL (Time To Live) for IPv4.
        /// @attention See fd-operation threading rules in the section comment above.
        turbo::Status set_ip_ttl(int ttl);

        /// Gets current IP_TTL value.
        /// @attention See fd-operation threading rules in the section comment above.
        turbo::Result<int> get_ip_ttl() const;

        /// Binds the socket to a specific device (interface) using SO_BINDTODEVICE (Linux only).
        /// @attention See fd-operation threading rules in the section comment above.
        turbo::Status bind_to_device(const std::string &iface);

        /// Retrieves TCP_INFO structure for this socket (Linux/macOS).
        /// @attention See fd-operation threading rules in the section comment above.
        turbo::Status get_tcp_info(void *out, socklen_t *len) const;

        /// Convenience overload returning a human-readable string with key TCP info.
        /// @attention See fd-operation threading rules in the section comment above.
        turbo::Result<std::string> get_tcp_info_string() const;

    protected:
        // -------------------------------------------------------------------------
        // Static trampolines (used by EventLoop callbacks)
        // -------------------------------------------------------------------------

        /// Static trampoline for read events.
        static turbo::Status on_read_call(FileHandle handle, EventData *data, turbo::Time cur);

        /// Static trampoline for write events.
        static turbo::Status on_write_call(FileHandle handle, EventData *data, turbo::Time cur);

        /// Static trampoline for error events.
        static turbo::Status on_error_call(FileHandle handle, EventData *data, turbo::Time cur);

        /// Static trampoline for timeout events.
        static turbo::Status on_read_timeout_call(FileHandle handle, EventData *data, turbo::Time expire,
                                                  turbo::Time cur);

        static turbo::Status on_write_timeout_call(FileHandle handle, EventData *data, turbo::Time expire,
                                                   turbo::Time cur);

        // -------------------------------------------------------------------------
        // Helpers for cross‑thread emit
        // -------------------------------------------------------------------------

        /// Internal implementation of event state change (called on loop thread).
        void attach_impl();

        void detach_impl();

        struct TaskProxy {
            SocketWare *s{nullptr};
            void *arg{nullptr};
            SocketFactor func{nullptr};
        };

        struct SyncTaskProxy : public TaskProxy {
            std::promise<int> promise;
        };

        /// Trampoline function for posting emit operation via post_task.
        static void attach_trampoline(void *arg);

        static void detach_trampoline(void *arg);

        static void run_async_trampoline(void *arg);

        static void run_sync_trampoline(void *arg);
        static void run_proxy_free(void *arg);

        // -------------------------------------------------------------------------
        // Resume (rate limiting) helpers
        // -------------------------------------------------------------------------

        static void read_timer_callback(void *arg, turbo::Time dl, turbo::Time curr);

        static void write_timer_callback(void *arg, turbo::Time dl, turbo::Time curr);

        /// Callback for resuming read events after a timer.
        static void resume_read_callback(void *arg, turbo::Time, turbo::Time);

        /// Callback for resuming write events after a timer.
        static void resume_write_callback(void *arg, turbo::Time, turbo::Time);

        // -------------------------------------------------------------------------
        // Member variables
        // -------------------------------------------------------------------------

        /// EventData used to register with EventLoop.
        EventData _event_data;

        /// Underlying socket file descriptor.
        FileHandle _file_handle{kInvalidFileHandle};

        /// Associated EventLoop instance.
        EventLoop *_event_loop{nullptr};

        /// Flag indicating whether read should be enabled (used for pending emit).
        bool _enable_read_flag{false};

        /// Flag indicating whether write should be enabled (used for pending emit).
        bool _enable_write_flag{false};

        turbo::Status _status;

        ////////////////////////////////////////////////////////////////////////////
        /// timer region

        /// Timeout for read events.
        turbo::Time _read_timeout{turbo::Time::future_infinite()};

        /// Timeout for write events.
        turbo::Time _write_timeout{turbo::Time::future_infinite()};

        /// 0 is invalid timer_id
        uint64_t _read_timer_id{TimerBase::kInvalidTimerId};

        uint64_t _write_timer_id{TimerBase::kInvalidTimerId};

        /// Timeout for read events.
        turbo::Time _resume_read_time{turbo::Time::future_infinite()};

        /// Timeout for write events.
        turbo::Time _resume_write_time{turbo::Time::future_infinite()};

        /// Timer ID for delayed read resumption.
        uint64_t _resume_read_timer_id{TimerBase::kInvalidTimerId};

        /// Timer ID for delayed write resumption.
        uint64_t _resume_write_timer_id{TimerBase::kInvalidTimerId};
    };
} // namespace xio
