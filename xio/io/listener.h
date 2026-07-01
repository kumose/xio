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
#include <memory>
#include <turbo/base/endpoint.h>

namespace xio {
    /// Listener for accepting incoming TCP connections.
    /// Inherits SocketWare to reuse event-driven I/O.
    class Listener : public SocketWare {
    public:
        /// Factory callback that creates a SocketWare object for each new connection.
        using ConnectionFactory = std::function<std::unique_ptr<SocketWare>(FileHandle fd, EventLoop *loop)>;

        Listener() = default;

        ~Listener() override = default;

        /// Binds the listener to an EventLoop, local endpoint, and an existing socket file descriptor.
        /// The fd must already be created, bound to the endpoint, and set to non‑blocking mode.
        void bind(EventLoop *loop, turbo::EndPoint ep, FileHandle fd);

        /// Resets the file descriptor (e.g., for reuse after failure). The new fd must already be bound.
        void reset_fd(FileHandle fd);

        /// Starts listening on the bound socket and enables accept events.
        /// This method is thread‑safe; if called from outside the loop thread,
        /// the operation is forwarded asynchronously and blocks until complete.
        /// @param backlog The maximum length of the pending connections queue.
        /// @return OkStatus on success.
        turbo::Status listen(int backlog = SOMAXCONN);

        /// Stops listening and closes the socket.
        void stop();

        /// Sets the factory invoked for each accepted connection.
        void set_connection_factory(ConnectionFactory factory) {
            _factory = std::move(factory);
        }

        /// Returns total number of accepted connections.
        uint32_t accepted_times() const { return _accepted_times; }
        /// Returns number of accept failures (e.g., temporary resource shortage).
        uint32_t accept_fail_times() const { return _accept_fail_times; }

        // SocketWare overrides
        turbo::Status on_read(turbo::Time cur) override;

        turbo::Status on_write(turbo::Time cur) override { return turbo::OkStatus(); }

        turbo::Status on_error(turbo::Time cur) override;

        turbo::Status on_read_timeout(turbo::Time expire, turbo::Time cur) override;

        turbo::Status on_write_timeout(turbo::Time expire, turbo::Time cur) override;

    private:
        /// Internal implementation that runs on the loop thread.
        turbo::Status listen_in_loop(int backlog);

        /// Context for cross‑thread listen call.
        struct ListenCtx {
            std::promise<turbo::Status> promise;
            Listener *self;
            int backlog;
        };

        /// Trampoline for posting listen to loop thread.
        static void post_listen_task(void *args);

        ConnectionFactory _factory;
        uint32_t _accepted_times{0};
        uint32_t _accept_fail_times{0};
    };
} // namespace xio
