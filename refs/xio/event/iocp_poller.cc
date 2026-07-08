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

#include <turbo/base/macros.h>
#if defined(OS_WIN)

#include <xio/event/iocp_poller.h>

#include <cstdint>

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#include <turbo/log/logging.h>
#include <turbo/times/time.h>
#include <turbo/utility/status.h>

namespace xio {

namespace {

int32_t event_type_to_mask(EventType event) {
    return static_cast<int32_t>(event);
}

bool wants_read(int32_t mask) {
    return (mask & static_cast<int32_t>(EventType::kEventRead)) != 0;
}

bool wants_write(int32_t mask) {
    return (mask & static_cast<int32_t>(EventType::kEventWrite)) != 0;
}

SOCKET to_socket(FileHandle handle) {
    return reinterpret_cast<SOCKET>(handle);
}

bool poll_duration_is_infinite(turbo::Duration dur) {
    return turbo::Duration::to_milliseconds(dur) < 0;
}

DWORD iocp_wait_timeout_ms(turbo::Duration dur) {
    if (poll_duration_is_infinite(dur)) {
        return INFINITE;
    }
    return static_cast<DWORD>(turbo::Duration::to_milliseconds(dur));
}

}  // namespace

struct IocpPoller::SocketEntry {
    EventData *data{nullptr};
    SOCKET sock{INVALID_SOCKET};
    OVERLAPPED read_ol{};
    OVERLAPPED write_ol{};
    bool associated{false};
    bool read_pending{false};
    bool write_pending{false};
};

IocpPoller::IocpPoller() = default;

IocpPoller::~IocpPoller() {
    for (auto &entry : _entries) {
        cancel_read(&entry);
        cancel_write(&entry);
    }
    _entries.clear();
    if (_iocp != INVALID_HANDLE_VALUE) {
        CloseHandle(_iocp);
        _iocp = INVALID_HANDLE_VALUE;
    }
    if (_wsa_started) {
        WSACleanup();
        _wsa_started = false;
    }
}

turbo::Status IocpPoller::initialize(const PollerConfig &pc) {
    TURBO_UNUSED(pc);
    if (_iocp != INVALID_HANDLE_VALUE) {
        return turbo::OkStatus();
    }
    if (!_wsa_started) {
        WSADATA wsa_data{};
        const int rc = WSAStartup(MAKEWORD(2, 2), &wsa_data);
        if (rc != 0) {
            return turbo::internal_error("WSAStartup failed, rc=", rc);
        }
        _wsa_started = true;
    }
    _iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 0);
    if (_iocp == nullptr) {
        return turbo::internal_error("CreateIoCompletionPort failed, error=", GetLastError());
    }
    return turbo::OkStatus();
}

IocpPoller::SocketEntry *IocpPoller::find_entry(EventData *data) {
    for (auto &entry : _entries) {
        if (entry.data == data) {
            return &entry;
        }
    }
    return nullptr;
}

IocpPoller::SocketEntry *IocpPoller::get_or_create_entry(EventData *data) {
    SocketEntry *entry = find_entry(data);
    if (entry != nullptr) {
        return entry;
    }
    SocketEntry created{};
    created.data = data;
    created.sock = to_socket(data->handle);
    _entries.push_back(created);
    return &_entries.back();
}

void IocpPoller::erase_entry(EventData *data) {
    for (size_t i = 0; i < _entries.size(); ++i) {
        if (_entries[i].data == data) {
            _entries[i] = _entries.back();
            _entries.pop_back();
            return;
        }
    }
}

turbo::Status IocpPoller::associate_socket(SocketEntry *entry) {
    if (entry->associated) {
        return turbo::OkStatus();
    }
    if (entry->sock == INVALID_SOCKET) {
        return turbo::invalid_argument_error("invalid socket");
    }
    if (CreateIoCompletionPort(reinterpret_cast<HANDLE>(entry->sock), _iocp,
                               reinterpret_cast<ULONG_PTR>(entry->data), 0) == nullptr) {
        return turbo::internal_error("associate socket to IOCP failed, error=", GetLastError());
    }
    entry->associated = true;
    return turbo::OkStatus();
}

void IocpPoller::cancel_read(SocketEntry *entry) {
    if (!entry->read_pending) {
        return;
    }
    CancelIoEx(reinterpret_cast<HANDLE>(entry->sock), &entry->read_ol);
    entry->read_pending = false;
}

void IocpPoller::cancel_write(SocketEntry *entry) {
    if (!entry->write_pending) {
        return;
    }
    CancelIoEx(reinterpret_cast<HANDLE>(entry->sock), &entry->write_ol);
    entry->write_pending = false;
}

turbo::Status IocpPoller::post_read(SocketEntry *entry) {
    if (!wants_read(entry->data->active_event) || entry->read_pending) {
        return turbo::OkStatus();
    }
    memset(&entry->read_ol, 0, sizeof(entry->read_ol));

    WSABUF buf{};
    buf.len = 0;
    buf.buf = nullptr;
    DWORD bytes = 0;
    DWORD flags = 0;
    const int rc = WSARecv(entry->sock, &buf, 1, &bytes, &flags, &entry->read_ol, nullptr);
    if (rc == 0) {
        entry->read_pending = false;
        return turbo::OkStatus();
    }
    const int err = WSAGetLastError();
    if (err == WSA_IO_PENDING) {
        entry->read_pending = true;
        return turbo::OkStatus();
    }
    return turbo::internal_error("WSARecv failed, error=", err);
}

turbo::Status IocpPoller::post_write(SocketEntry *entry) {
    if (!wants_write(entry->data->active_event) || entry->write_pending) {
        return turbo::OkStatus();
    }
    memset(&entry->write_ol, 0, sizeof(entry->write_ol));

    WSABUF buf{};
    buf.len = 0;
    buf.buf = nullptr;
    DWORD bytes = 0;
    const int rc = WSASend(entry->sock, &buf, 1, &bytes, 0, &entry->write_ol, nullptr);
    if (rc == 0) {
        entry->write_pending = false;
        return turbo::OkStatus();
    }
    const int err = WSAGetLastError();
    if (err == WSA_IO_PENDING) {
        entry->write_pending = true;
        return turbo::OkStatus();
    }
    return turbo::internal_error("WSASend failed, error=", err);
}

turbo::Status IocpPoller::apply_mask_change(EventData *data, int32_t old_mask,
                                            int32_t new_mask) {
    if (data->handle == kInvalidFileHandle) {
        return turbo::invalid_argument_error("invalid FileHandle");
    }
    if (old_mask == new_mask) {
        return turbo::OkStatus();
    }
    if (new_mask == 0) {
        if (old_mask == 0) {
            return turbo::OkStatus();
        }
        SocketEntry *entry = find_entry(data);
        if (entry != nullptr) {
            cancel_read(entry);
            cancel_write(entry);
            erase_entry(data);
        }
        return turbo::OkStatus();
    }

    SocketEntry *entry = get_or_create_entry(data);
    entry->sock = to_socket(data->handle);
    turbo::Status st = associate_socket(entry);
    if (!st.ok()) {
        return st;
    }

    const bool old_read = wants_read(old_mask);
    const bool new_read = wants_read(new_mask);
    if (!new_read && old_read) {
        cancel_read(entry);
    } else if (new_read && !old_read) {
        st = post_read(entry);
        if (!st.ok()) {
            return st;
        }
    }

    const bool old_write = wants_write(old_mask);
    const bool new_write = wants_write(new_mask);
    if (!new_write && old_write) {
        cancel_write(entry);
    } else if (new_write && !old_write) {
        st = post_write(entry);
        if (!st.ok()) {
            return st;
        }
    }

    return turbo::OkStatus();
}

int32_t IocpPoller::completion_to_mask(const SocketEntry *entry, const OVERLAPPED *ov) {
    DKCHECK(entry != nullptr);
    DKCHECK(ov != nullptr);
    if (ov == &entry->read_ol) {
        return static_cast<int32_t>(EventType::kEventRead);
    }
    if (ov == &entry->write_ol) {
        return static_cast<int32_t>(EventType::kEventWrite);
    }
    return 0;
}

void IocpPoller::merge_ready(EventData *data, int32_t ready,
                             fermat::Vector<EventData *> &datas) {
    if (ready == 0) {
        return;
    }
    DKCHECK(data != nullptr);
    for (size_t i = 0; i < datas.size(); ++i) {
        if (datas[i] == data) {
            datas[i]->triggered = static_cast<EventType>(
                static_cast<int32_t>(datas[i]->triggered) | ready);
            return;
        }
    }
    data->triggered = static_cast<EventType>(ready);
    datas.push_back(data);
}

turbo::Status IocpPoller::handle_event(EventData *data,
                                       EventType enable_event,
                                       EventType disable_event,
                                       turbo::Time cur) {
    TURBO_UNUSED(cur);
    const int32_t enable_bits = event_type_to_mask(enable_event);
    const int32_t disable_bits = event_type_to_mask(disable_event);
    if (enable_bits == 0 && disable_bits == 0) {
        return turbo::OkStatus();
    }

    const int32_t old_mask = data->active_event;
    if (old_mask == 0 && enable_bits == 0) {
        return turbo::OkStatus();
    }

    const int32_t new_mask = (old_mask | enable_bits) & ~disable_bits;
    if (new_mask == old_mask) {
        return turbo::OkStatus();
    }

    data->active_event = new_mask;
    return apply_mask_change(data, old_mask, new_mask);
}

turbo::Status IocpPoller::remove_event(EventData *data, turbo::Time cur) {
    TURBO_UNUSED(cur);
    const int32_t old_mask = data->active_event;
    if (old_mask == 0) {
        return turbo::OkStatus();
    }
    data->active_event = 0;
    return apply_mask_change(data, old_mask, 0);
}

turbo::Result<turbo::Time> IocpPoller::poll(turbo::Duration dur,
                                            fermat::Vector<EventData *> &datas) {
    datas.clear();

    const DWORD timeout_ms = iocp_wait_timeout_ms(dur);

    bool first_wait = true;
    for (;;) {
        DWORD bytes = 0;
        ULONG_PTR key = 0;
        OVERLAPPED *ov = nullptr;
        const DWORD wait_ms = first_wait ? timeout_ms : 0;
        first_wait = false;

        const BOOL ok = GetQueuedCompletionStatus(_iocp, &bytes, &key, &ov, wait_ms);
        if (!ok) {
            const DWORD err = GetLastError();
            if (err == WAIT_TIMEOUT) {
                break;
            }
            if (ov == nullptr) {
                return turbo::internal_error("GetQueuedCompletionStatus failed, error=", err);
            }
        }

        auto *data = reinterpret_cast<EventData *>(key);
        DKCHECK(data != nullptr);
        SocketEntry *entry = find_entry(data);
        DKCHECK(entry != nullptr);
        DKCHECK(ov != nullptr);
        if (ov == &entry->read_ol) {
            entry->read_pending = false;
        } else if (ov == &entry->write_ol) {
            entry->write_pending = false;
        }

        int32_t ready = completion_to_mask(entry, ov);
        if (!ok) {
            ready |= static_cast<int32_t>(EventType::kEventError);
            if (bytes == 0) {
                ready |= static_cast<int32_t>(EventType::kEventHangUp);
            }
        }
        merge_ready(data, ready, datas);

        if (ov == &entry->read_ol) {
            (void) post_read(entry);
        } else if (ov == &entry->write_ol) {
            (void) post_write(entry);
        }
    }

    return turbo::Time::current_time();
}

}  // namespace xio

#endif  // defined(OS_WIN)