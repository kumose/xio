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
#if defined(OS_MACOSX)

#include <xio/event/kqueue_poller.h>

#include <cerrno>
#include <cstdint>

#include <sys/event.h>
#include <sys/time.h>
#include <unistd.h>

#include <turbo/log/logging.h>
#include <turbo/times/time.h>
#include <turbo/utility/status.h>

namespace xio {

namespace {

constexpr size_t kDefaultEventBatch = 256;

int32_t event_type_to_mask(EventType event) {
    return static_cast<int32_t>(event);
}

bool wants_read(int32_t mask) {
    return (mask & static_cast<int32_t>(EventType::kEventRead)) != 0;
}

bool wants_write(int32_t mask) {
    return (mask & static_cast<int32_t>(EventType::kEventWrite)) != 0;
}

void push_kevent_change(fermat::Vector<struct kevent> &changes,
                        FileHandle fd, int16_t filter, uint16_t flags,
                        uint16_t fflags, EventData *data) {
    struct kevent kev {};
    EV_SET(&kev, fd, filter, flags, fflags, 0, data);
    changes.push_back(kev);
}

bool poll_duration_is_infinite(turbo::Duration dur) {
    return turbo::Duration::to_milliseconds(dur) < 0;
}

void duration_to_timespec(turbo::Duration dur, struct timespec &ts) {
    const int64_t timeout_ms = turbo::Duration::to_milliseconds(dur);
    ts.tv_sec = static_cast<time_t>(timeout_ms / 1000);
    ts.tv_nsec = static_cast<long>((timeout_ms % 1000) * 1000000L);
}

}  // namespace

KqueuePoller::KqueuePoller() = default;

KqueuePoller::~KqueuePoller() {
    if (_kqfd >= 0) {
        close(_kqfd);
        _kqfd = -1;
    }
}

turbo::Status KqueuePoller::initialize(const PollerConfig &pc) {
    TURBO_UNUSED(pc);
    if (_kqfd >= 0) {
        return turbo::OkStatus();
    }
    _kqfd = kqueue();
    if (_kqfd < 0) {
        return turbo::internal_error("kqueue failed, errno=", errno);
    }
    _events.resize(kDefaultEventBatch);
    _changes.reserve(8);
    return turbo::OkStatus();
}

turbo::Status KqueuePoller::apply_mask_change(EventData *data, int32_t old_mask,
                                             int32_t new_mask) {
    if (data->handle == kInvalidFileHandle) {
        return turbo::invalid_argument_error("invalid FileHandle");
    }
    if (old_mask == new_mask) {
        return turbo::OkStatus();
    }

    _changes.clear();

    if (new_mask == 0) {
        if (old_mask == 0) {
            return turbo::OkStatus();
        }
        if (wants_read(old_mask)) {
            push_kevent_change(_changes, data->handle, EVFILT_READ,
                               EV_DELETE, 0, nullptr);
        }
        if (wants_write(old_mask)) {
            push_kevent_change(_changes, data->handle, EVFILT_WRITE,
                               EV_DELETE, 0, nullptr);
        }
    } else {
        const bool old_read = wants_read(old_mask);
        const bool new_read = wants_read(new_mask);
        if (new_read && !old_read) {
#if defined(NOTE_RDHUP)
            push_kevent_change(_changes, data->handle, EVFILT_READ,
                               EV_ADD | EV_ENABLE, NOTE_RDHUP, data);
#else
            push_kevent_change(_changes, data->handle, EVFILT_READ,
                               EV_ADD | EV_ENABLE, 0, data);
#endif
        } else if (!new_read && old_read) {
            push_kevent_change(_changes, data->handle, EVFILT_READ,
                               EV_DELETE, 0, nullptr);
        }

        const bool old_write = wants_write(old_mask);
        const bool new_write = wants_write(new_mask);
        if (new_write && !old_write) {
            push_kevent_change(_changes, data->handle, EVFILT_WRITE,
                               EV_ADD | EV_ENABLE, 0, data);
        } else if (!new_write && old_write) {
            push_kevent_change(_changes, data->handle, EVFILT_WRITE,
                               EV_DELETE, 0, nullptr);
        }
    }

    if (_changes.empty()) {
        return turbo::OkStatus();
    }

    if (kevent(_kqfd, _changes.data(), static_cast<int>(_changes.size()),
               nullptr, 0, nullptr) < 0) {
        return turbo::internal_error("kevent change failed, errno=", errno);
    }
    return turbo::OkStatus();
}

int32_t KqueuePoller::kevent_to_mask(const struct kevent &kev) {
    int32_t mask = 0;
    if (kev.filter == EVFILT_READ) {
        mask |= static_cast<int32_t>(EventType::kEventRead);
    } else if (kev.filter == EVFILT_WRITE) {
        mask |= static_cast<int32_t>(EventType::kEventWrite);
    }

    if (kev.flags & EV_ERROR) {
        mask |= static_cast<int32_t>(EventType::kEventError);
    }
    if (kev.flags & EV_EOF) {
        mask |= static_cast<int32_t>(EventType::kEventHangUp);
    }
#if defined(NOTE_RDHUP)
    if (kev.fflags & NOTE_RDHUP) {
        mask |= static_cast<int32_t>(EventType::kEventReadHup);
    }
#endif
    return mask;
}

void KqueuePoller::merge_ready(EventData *data, int32_t ready,
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

turbo::Status KqueuePoller::handle_event(EventData *data,
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

turbo::Status KqueuePoller::remove_event(EventData *data, turbo::Time cur) {
    TURBO_UNUSED(cur);
    const int32_t old_mask = data->active_event;
    if (old_mask == 0) {
        return turbo::OkStatus();
    }
    data->active_event = 0;
    return apply_mask_change(data, old_mask, 0);
}

turbo::Result<turbo::Time> KqueuePoller::poll(turbo::Duration dur,
                                              fermat::Vector<EventData *> &datas) {
    datas.clear();

    struct timespec ts {};
    struct timespec *ts_ptr = nullptr;
    if (!poll_duration_is_infinite(dur)) {
        duration_to_timespec(dur, ts);
        ts_ptr = &ts;
    }
    _events.resize(kDefaultEventBatch);

    for (;;) {
        const int n = kevent(_kqfd, nullptr, 0, _events.data(),
                             static_cast<int>(_events.size()), ts_ptr);
        if (n < 0) {
            if (errno == EINTR) {
                continue;
            }
            return turbo::internal_error("kevent wait failed, errno=", errno);
        }

        if (n == static_cast<int>(_events.size())) {
            _events.resize(_events.size() * 2);
            continue;
        }

        datas.reserve(static_cast<size_t>(n));
        for (int i = 0; i < n; ++i) {
            auto *data = static_cast<EventData *>(_events[static_cast<size_t>(i)].udata);
            DKCHECK(data != nullptr);
            const int32_t ready = kevent_to_mask(_events[static_cast<size_t>(i)]);
            merge_ready(data, ready, datas);
        }
        break;
    }

    return turbo::Time::current_time();
}

}  // namespace xio

#endif  // defined(OS_MACOSX)