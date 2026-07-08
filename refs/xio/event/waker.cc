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

#include <xio/event/waker.h>

#include <cerrno>
#include <cstdint>

#include <xio/event/poller.h>
#include <turbo/utility/status.h>
#include <turbo/times/time.h>

#if defined(OS_LINUX)
#include <sys/eventfd.h>
#endif

#if !defined(OS_WIN)
#include <fcntl.h>
#include <unistd.h>
#else
#include <winsock2.h>
#include <ws2tcpip.h>
#endif

namespace xio {
    namespace {
#if defined(OS_WIN)
        bool set_nonblocking_socket(SOCKET sock) {
            u_long mode = 1;
            return ioctlsocket(sock, FIONBIO, &mode) == 0;
        }
#endif
    } // namespace

    Waker::Waker() = default;

    Waker::~Waker() { DKCHECK(!is_open()); }

    turbo::Status Waker::open() {
        if (is_open()) {
            return turbo::OkStatus();
        }
        platform_open();
        if (!is_open()) {
            return turbo::internal_error("Waker::open failed");
        }
        _wake_pending.store(false, std::memory_order_release);
        return turbo::OkStatus();
    }

    void Waker::close() {
        if (_poller != nullptr && _registered_data != nullptr) {
            unregister(_registered_data, _poller);
        }
        _poller = nullptr;
        _registered_data = nullptr;
        platform_close();
        _wake_pending.store(false, std::memory_order_release);
        _event.handle = kInvalidFileHandle;
        _event.data = nullptr;
        _event.set_out_loop();
    }

    turbo::Status Waker::register_with(EventData *data, EventLoop *loop, Poller *poller, turbo::Time cur) {
        DKCHECK(data != nullptr);
        DKCHECK(loop != nullptr);
        DKCHECK(poller != nullptr);
        if (!is_open()) {
            turbo::Status st = open();
            if (!st.ok()) {
                return st;
            }
        }
        turbo::Status st = event_data_setup_waker(data, this, loop);
        if (!st.ok()) {
            return st;
        }
        // Use Poller::enable_event (new interface)
        st = poller->enable_event(data, EventType::kEventRead, cur);
        if (!st.ok()) {
            return st;
        }
        _poller = poller;
        _registered_data = data;
        return turbo::OkStatus();
    }

    void Waker::unregister(EventData *data, Poller *poller) {
        if (data == nullptr || poller == nullptr) {
            return;
        }
        // remove_event requires a Time parameter; in practice the implementation doesn't use it.
        // We pass current time to avoid "瞎编".
        turbo::Time now = turbo::Time::current_time();
        poller->remove_event(data, now).ignore_error();
        if (_registered_data == data && _poller == poller) {
            _poller = nullptr;
            _registered_data = nullptr;
        }
    }

    void Waker::signal() {
        if (!is_open()) {
            return;
        }
        if (_wake_pending.exchange(true, std::memory_order_acq_rel)) {
            return;
        }
        platform_signal();
    }

    turbo::Status Waker::consume() {
        if (!is_open()) {
            return turbo::OkStatus();
        }
        platform_drain();
        _wake_pending.store(false, std::memory_order_release);
        return turbo::OkStatus();
    }

    void Waker::platform_open() {
#if defined(OS_LINUX) && !defined(OS_MACOSX)
        const int fd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
        if (fd < 0) {
            return;
        }
        _read_handle = fd;
        _write_handle = fd;
#elif defined(OS_WIN)
        WSADATA wsa_data{};
        if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) {
            return;
        }
        const SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (listen_sock == INVALID_SOCKET) {
            return;
        }
        _listen_sock = static_cast<FileHandle>(listen_sock);
        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        addr.sin_port = 0;
        if (bind(listen_sock, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) != 0) {
            return;
        }
        if (listen(listen_sock, 1) != 0) {
            return;
        }
        int addr_len = sizeof(addr);
        getsockname(listen_sock, reinterpret_cast<sockaddr *>(&addr), &addr_len);

        SOCKET client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (client == INVALID_SOCKET) {
            return;
        }
        if (connect(client, reinterpret_cast<sockaddr *>(&addr), addr_len) != 0) {
            closesocket(client);
            return;
        }
        SOCKET server = accept(listen_sock, nullptr, nullptr);
        if (server == INVALID_SOCKET) {
            closesocket(client);
            return;
        }
        closesocket(listen_sock);
        _listen_sock = kInvalidFileHandle;
        set_nonblocking_socket(client);
        set_nonblocking_socket(server);
        _read_handle = static_cast<FileHandle>(server);
        _write_handle = static_cast<FileHandle>(client);
#else
        int fds[2] = {-1, -1};
#if defined(__APPLE__)
        if (pipe(fds) != 0) {
            return;
        }
        fcntl(fds[0], F_SETFL, O_NONBLOCK);
        fcntl(fds[1], F_SETFL, O_NONBLOCK);
        fcntl(fds[0], F_SETFD, FD_CLOEXEC);
        fcntl(fds[1], F_SETFD, FD_CLOEXEC);
#else
        if (pipe2(fds, O_NONBLOCK | O_CLOEXEC) != 0) {
            return;
        }
#endif
        _read_handle = fds[0];
        _write_handle = fds[1];
#endif
    }

    void Waker::platform_close() {
#if defined(OS_LINUX) && !defined(OS_MACOSX)
        if (_read_handle != kInvalidFileHandle) {
            ::close(_read_handle);
        }
#elif defined(OS_WIN)
        if (_read_handle != kInvalidFileHandle) {
            closesocket(static_cast<SOCKET>(_read_handle));
        }
        if (_write_handle != kInvalidFileHandle && _write_handle != _read_handle) {
            closesocket(static_cast<SOCKET>(_write_handle));
        }
        if (_listen_sock != kInvalidFileHandle) {
            closesocket(static_cast<SOCKET>(_listen_sock));
            _listen_sock = kInvalidFileHandle;
        }
#else
        if (_read_handle != kInvalidFileHandle) {
            ::close(_read_handle);
        }
        if (_write_handle != kInvalidFileHandle && _write_handle != _read_handle) {
            ::close(_write_handle);
        }
#endif
        _read_handle = kInvalidFileHandle;
        _write_handle = kInvalidFileHandle;
    }

    void Waker::platform_signal() {
#if defined(OS_LINUX) && !defined(OS_MACOSX)
        const uint64_t one = 1;
        for (;;) {
            const ssize_t n = ::write(_write_handle, &one, sizeof(one));
            if (n >= 0) {
                return;
            }
            if (errno == EINTR) {
                continue;
            }
            if (errno == EAGAIN) {
                return;
            }
            return;
        }
#else
        const char byte = 1;
        for (;;) {
            const ssize_t n =
#if defined(OS_WIN)
        ::send(static_cast<SOCKET>(_write_handle), &byte, 1, 0);
#else
        ::write(_write_handle, &byte, 1);
#endif
        if (n >= 0) {
            return;
        }
#if defined(OS_WIN)
        if (WSAGetLastError() == WSAEWOULDBLOCK) {
            return;
        }
        return;
#else
        if (errno == EINTR) {
            continue;
        }
        if (errno == EAGAIN) {
            return;
        }
        return;
#endif
    }
#endif
    }

    void Waker::platform_drain() {
        for (;;) {
#if defined(OS_LINUX) && !defined(OS_MACOSX)
            uint64_t scratch = 0;
            const ssize_t n = ::read(_read_handle, &scratch, sizeof(scratch));
#else
            char buf[64];
            const ssize_t n =
#if defined(OS_WIN)
            ::recv(static_cast<SOCKET>(_read_handle), buf, sizeof(buf), 0);
#else
            ::read(_read_handle, buf, sizeof(buf));
#endif
#endif
            if (n > 0) {
                continue;
            }
            if (n == 0) {
                return;
            }
#if defined(OS_WIN)
            if (WSAGetLastError() == WSAEWOULDBLOCK) {
                return;
            }
            return;
#else
            if (errno == EINTR) {
                continue;
            }
            if (errno == EAGAIN) {
                return;
            }
            return;
#endif
        }
    }

    turbo::Status event_data_on_waker_ready(FileHandle handle, EventData *data, turbo::Time cur) {
        TURBO_UNUSED(cur);
        TURBO_UNUSED(handle);
        auto *waker = static_cast<Waker *>(data->data);
        return waker->consume();
    }

    turbo::Status event_data_setup_waker(EventData *data, Waker *waker, EventLoop *loop) {
        if (data == nullptr || waker == nullptr || loop == nullptr) {
            return turbo::invalid_argument_error("event_data_setup_waker: null argument");
        }
        if (!waker->is_open()) {
            turbo::Status st = waker->open();
            if (!st.ok()) {
                return st;
            }
        }
        data->handle = waker->read_handle();
        data->data = waker;
        data->data_deleter = empty_data_releaser;
        data->read_callback = event_data_on_waker_ready;
        data->write_callback = empty_handler_callback;
        data->error_callback = empty_handler_callback;
        data->event_status |= EventDataStatus::kEventWaker;
        return turbo::OkStatus();
    }
} // namespace xio
