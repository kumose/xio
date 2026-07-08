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

#include <xio/io/socket_ware.h>
#include <functional>
#include <turbo/base/endpoint.h>

namespace xio {
    /// Connector for establishing a single TCP connection (non‑blocking).
    /// Does NOT handle reconnection; upper layer implements retry logic.
    /// Success callback receives the connected file descriptor and the EventLoop.
    class Connector : public SocketWare {
    public:
        using base_type  = SocketWare;
        /// Callback invoked when the connection is successfully established.
        /// It receives the connected file descriptor and the associated EventLoop.
        using SuccessCallback = std::function<void(FileHandle fd, EventLoop *loop)>;
        /// Callback invoked when the connection fails (e.g., network error, refused).
        /// It receives the errno value.
        using FailCallback = std::function<void(int err)>;
        /// Callback invoked when the connection attempt times out.
        using TimeoutCallback = std::function<void()>;

        Connector() = default;

        ~Connector() override = default;

        /// Binds the connector to an EventLoop, target endpoint, and an existing socket file descriptor.
        /// The fd must already be created and set to non‑blocking mode.
        void bind(EventLoop *loop, turbo::EndPoint ep, FileHandle fd);

        /// Resets the file descriptor (e.g., for reuse after failure). The new fd must be non‑blocking.
        void reset_fd(FileHandle fd);

        /// Sets the callback to be called on connection timeout.
        void set_timeout_callback(TimeoutCallback timeout_cb);

        /// Sets the callback to be called on successful connection.
        void set_succ_callback(SuccessCallback succ_cb);

        /// Sets the callback to be called on connection failure.
        void set_fail_callback(FailCallback fail_cb);

        /// Initiates the connection. This method may be called from any thread;
        /// the actual operation is forwarded to the EventLoop thread.
        void connect(turbo::Duration timeout = turbo::Duration::seconds(1));

        /// Returns total connection attempts.
        uint32_t try_times() const { return _try_times; }
        /// Returns number of timeouts occurred.
        uint32_t try_timeouts() const { return _try_timeouts; }
        /// Returns number of failures (non‑timeout errors).
        uint32_t fail_times() const { return _fail_times; }
        /// Returns number of successful connections.
        uint32_t succ_times() const { return _succ_times; }
        /// Returns number of timeouts (same as try_timeouts).
        uint32_t timeout_times() const { return _timeout_times; }
        /// Returns number of times reset_fd was called.
        uint32_t reset_times() const { return _reset_times; }

        // SocketWare overrides (only on_write, on_error, on_timeout are used)
        turbo::Status on_read(turbo::Time cur) override { return turbo::OkStatus(); }

        /// Called when the socket becomes writable; used to detect connection completion.
        turbo::Status on_write(turbo::Time cur) override;

        /// Called when an error occurs on the socket; reports failure.
        turbo::Status on_error(turbo::Time cur) override;

        /// Called when the connection timeout expires (set by enable_write with absolute timeout).
        turbo::Status on_read_timeout(turbo::Time expire, turbo::Time cur) override { return turbo::OkStatus(); }
        turbo::Status on_write_timeout(turbo::Time expire, turbo::Time cur) override;

    private:
        /// Internal implementation that runs on the loop thread.
        void connect_in_loop(turbo::Duration timeout);

        /// Cleans up socket and disables events.
        void cleanup();

        /// Notifies success, transfers fd ownership, updates statistics.
        void notify_success(FileHandle fd);

        /// Notifies failure, updates statistics.
        void notify_failure(int err);

        /// Notifies timeout, updates statistics.
        void notify_timeout();

        /// Context for cross‑thread connect call.
        struct PostCtx {
            std::promise<void> promise;
            Connector *self;
            turbo::Duration timeout;
        };

        /// Trampoline for posting connect to loop thread.
        static void post_connect_task(void *args);

        /// Remote endpoint (IP and port).
        turbo::EndPoint _end_point;
        SuccessCallback _succ_cb;
        FailCallback _fail_cb;
        TimeoutCallback _timeout_cb;
        bool _connected{false};
        bool _timedout{false};

        /// Total connection attempts.
        uint32_t _try_times{0};
        /// Number of timeouts occurred.
        uint32_t _try_timeouts{0};
        /// Number of failures (non‑timeout errors).
        uint32_t _fail_times{0};
        /// Number of successful connections.
        uint32_t _succ_times{0};
        /// Number of timeouts (same as _try_timeouts, kept for clarity).
        uint32_t _timeout_times{0};
        /// Number of times reset_fd was called.
        uint32_t _reset_times{0};
    };
} // namespace xio
