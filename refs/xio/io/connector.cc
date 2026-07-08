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

#include <xio/io/connector.h>
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
    void Connector::bind(EventLoop *loop, turbo::EndPoint ep, FileHandle fd) {
        _end_point = ep;
        SocketWare::bind(fd, loop);
    }

    void Connector::reset_fd(FileHandle fd) {
        if (_file_handle != kInvalidFileHandle) {
            close();
        }
        ++_reset_times;
        SocketWare::bind(fd, _event_loop);
    }

    void Connector::set_timeout_callback(TimeoutCallback timeout_cb) {
        _timeout_cb = std::move(timeout_cb);
    }

    void Connector::set_succ_callback(SuccessCallback succ_cb) {
        _succ_cb = std::move(succ_cb);
    }

    void Connector::set_fail_callback(FailCallback fail_cb) {
        _fail_cb = std::move(fail_cb);
    }

    void Connector::post_connect_task(void *args) {
        auto *ctx = static_cast<PostCtx *>(args);
        ctx->self->connect_in_loop(ctx->timeout);
        ctx->promise.set_value();
    }

    void Connector::connect(turbo::Duration timeout) {
        if (_event_loop->is_in_loop_thread()) {
            connect_in_loop(timeout);
            return;
        }
        PostCtx ctx{{}, this, timeout};
        auto fut = ctx.promise.get_future();
        _event_loop->post_task(post_connect_task, &ctx);
        fut.wait();
    }

    void Connector::connect_in_loop(turbo::Duration timeout) {
        DKCHECK(_event_loop->is_in_loop_thread());
        if (_connected || _timedout) return;

        ++_try_times;

        struct sockaddr_in addr;
        std::memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(_end_point.port);
        addr.sin_addr = _end_point.ip;

        int ret = ::connect(_file_handle, (struct sockaddr *) &addr, sizeof(addr));
        if (ret == 0) {
            notify_success(_file_handle);
            return;
        }
        int err = errno;
        if (err == EINPROGRESS || err == EWOULDBLOCK) {
            /// Enable write event and set absolute timeout. Base class will call on_timeout on expiry.
            turbo::Time abs_timeout = turbo::Time::current_time() + timeout;
            _status = enable_write();
            read_timeout_at(abs_timeout);
        } else {
            notify_failure(err);
        }
    }

    void Connector::cleanup() {
        if (_file_handle != kInvalidFileHandle) {
            _status = disable_all();
            close();
        }
    }

    void Connector::notify_success(FileHandle fd) {
        _connected = true;
        ++_succ_times;
        /// Transfer ownership of fd to caller.
        _file_handle = kInvalidFileHandle;
        if (_succ_cb) _succ_cb(fd, _event_loop);
    }

    void Connector::notify_failure(int err) {
        cleanup();
        ++_fail_times;
        if (_fail_cb) _fail_cb(err);
    }

    void Connector::notify_timeout() {
        _timedout = true;
        cleanup();
        ++_try_timeouts;
        ++_timeout_times;
        if (_timeout_cb) _timeout_cb();
        else if (_fail_cb) _fail_cb(ETIMEDOUT);
    }

    turbo::Status Connector::on_write(turbo::Time cur) {
        if (_connected || _timedout) return turbo::OkStatus();
        int err = 0;
        socklen_t len = sizeof(err);
        if (getsockopt(_file_handle, SOL_SOCKET, SO_ERROR, (char *) &err, &len) < 0) {
            err = errno;
        }
        if (err == 0) {
            notify_success(_file_handle);
        } else {
            notify_failure(err);
        }
        return turbo::OkStatus();
    }

    turbo::Status Connector::on_error(turbo::Time cur) {
        if (!_connected && !_timedout) {
            notify_failure(ECONNABORTED);
        }
        return turbo::OkStatus();
    }

    turbo::Status Connector::on_write_timeout(turbo::Time expire, turbo::Time cur) {
        if (!_connected && !_timedout) {
            notify_timeout();
        }
        return turbo::OkStatus();
    }
} // namespace xio
