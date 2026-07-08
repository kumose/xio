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

#include <xio/io/listener.h>
#include <turbo/base/fd_utility.h>
#include <cstring>
#include <cerrno>
#include <future>

#if defined(OS_WIN)
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

namespace xio {
    void Listener::bind(EventLoop *loop, turbo::EndPoint ep, FileHandle fd) {
        // The endpoint is stored (may be used for logging or future reuse), but the fd is already bound.
        (void) ep;
        SocketWare::bind(fd, loop);
    }

    void Listener::reset_fd(FileHandle fd) {
        if (_file_handle != kInvalidFileHandle) {
            close();
        }
        SocketWare::bind(fd, _event_loop);
    }

    void Listener::post_listen_task(void *args) {
        auto *ctx = static_cast<ListenCtx *>(args);
        auto st = ctx->self->listen_in_loop(ctx->backlog);
        ctx->promise.set_value(std::move(st));
    }

    turbo::Status Listener::listen(int backlog) {
        if (_event_loop->is_in_loop_thread()) {
            return listen_in_loop(backlog);
        }
        ListenCtx ctx;
        ctx.self = this;
        ctx.backlog = backlog;
        auto fut = ctx.promise.get_future();
        _event_loop->post_task(post_listen_task, &ctx);
        return fut.get();
    }

    turbo::Status Listener::listen_in_loop(int backlog) {
        DKCHECK(_event_loop->is_in_loop_thread());
        if (_file_handle == kInvalidFileHandle) {
            return turbo::failed_precondition_error("Not bound");
        }
        if (::listen(_file_handle, backlog) != 0) {
            return turbo::internal_error("listen() failed", errno);
        }
        // Enable read event (accept)
        return enable_read();
    }

    void Listener::stop() {
        if (_file_handle != kInvalidFileHandle) {
            close();
        }
    }

    turbo::Status Listener::on_read(turbo::Time /*cur*/) {
        while (true) {
            struct sockaddr_in client_addr;
            socklen_t addrlen = sizeof(client_addr);
            FileHandle client_fd = ::accept(_file_handle, (struct sockaddr *) &client_addr, &addrlen);
            if (client_fd == kInvalidFileHandle) {
                int err = errno;
                if (err == EAGAIN || err == EWOULDBLOCK) {
                    break;
                }
                ++_accept_fail_times;
                break;
            }
            if (turbo::make_non_blocking(client_fd) != 0) {
#if defined(OS_WIN)
                closesocket(client_fd);
#else
                ::close(client_fd);
#endif
                ++_accept_fail_times;
                continue;
            }
            if (_factory) {
                auto conn = _factory(client_fd, _event_loop);
                if (conn) {
                    conn->bind(client_fd, _event_loop);
                    ++_accepted_times;
                    // Factory is responsible for storing the connection.
                    conn.release(); // transfer ownership
                } else {
#if defined(OS_WIN)
                    closesocket(client_fd);
#else
                    ::close(client_fd);
#endif
                    ++_accept_fail_times;
                }
            } else {
#if defined(OS_WIN)
                closesocket(client_fd);
#else
                ::close(client_fd);
#endif
                ++_accept_fail_times;
            }
        }
        return turbo::OkStatus();
    }

    turbo::Status Listener::on_error(turbo::Time /*cur*/) {
        disable_read().ignore_error();
        close();
        return turbo::OkStatus();
    }

    turbo::Status Listener::on_read_timeout(turbo::Time /*expire*/, turbo::Time /*cur*/) {
        // Accept timeout is not used for listener; simply return OK.
        return turbo::OkStatus();
    }

    turbo::Status Listener::on_write_timeout(turbo::Time /*expire*/, turbo::Time /*cur*/) {
        return turbo::OkStatus();
    }
} // namespace xio
