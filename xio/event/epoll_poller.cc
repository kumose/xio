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
#if defined(OS_LINUX)

#include <xio/event/epoll_poller.h>

#include <cerrno>
#include <cstdint>
#include <unistd.h>

#include <sys/epoll.h>

#include <turbo/log/logging.h>
#include <turbo/times/time.h>
#include <turbo/utility/status.h>

namespace xio {

namespace {

constexpr int kDefaultEpollSize = 65536;
constexpr size_t kDefaultEventBatch = 256;

int32_t event_type_to_mask(EventType event) {
    return static_cast<int32_t>(event);
}

int epoll_wait_timeout_ms(turbo::Duration dur) {
    const int64_t ms = turbo::Duration::to_milliseconds(dur);
    if (ms < 0) {
        return -1;  // 无限等待
    }
    return static_cast<int>(ms);
}

}  // namespace

EpollPoller::EpollPoller() = default;

EpollPoller::~EpollPoller() {
    if (_epfd >= 0) {
        close(_epfd);
        _epfd = -1;
    }
}

turbo::Status EpollPoller::initialize(const PollerConfig &pc) {
    TURBO_UNUSED(pc);
    if (_epfd >= 0) {
        return turbo::OkStatus();
    }
    _epfd = epoll_create(kDefaultEpollSize);
    if (_epfd < 0) {
        return turbo::internal_error("epoll_create failed, errno=", errno);
    }
    _events.resize(kDefaultEventBatch);
    return turbo::OkStatus();
}

int EpollPoller::event_mask_to_epoll(int32_t mask) {
    int ep = 0;
    if (mask & static_cast<int32_t>(EventType::kEventRead))
        ep |= EPOLLIN;
    if (mask & static_cast<int32_t>(EventType::kEventWrite))
        ep |= EPOLLOUT;
    if (mask & static_cast<int32_t>(EventType::kEventError))
        ep |= EPOLLERR;
    if (mask & static_cast<int32_t>(EventType::kEventHangUp))
        ep |= EPOLLHUP;
    if (mask & static_cast<int32_t>(EventType::kEventReadHup))
        ep |= EPOLLRDHUP;
    if (mask & static_cast<int32_t>(EventType::kEventPriority))
        ep |= EPOLLPRI;
    return ep;
}

int32_t EpollPoller::epoll_events_to_mask(uint32_t epoll_events) {
    int32_t mask = 0;
    if (epoll_events & EPOLLIN)
        mask |= static_cast<int32_t>(EventType::kEventRead);
    if (epoll_events & EPOLLOUT)
        mask |= static_cast<int32_t>(EventType::kEventWrite);
    if (epoll_events & EPOLLERR)
        mask |= static_cast<int32_t>(EventType::kEventError);
    if (epoll_events & EPOLLHUP)
        mask |= static_cast<int32_t>(EventType::kEventHangUp);
    if (epoll_events & EPOLLRDHUP)
        mask |= static_cast<int32_t>(EventType::kEventReadHup);
    if (epoll_events & EPOLLPRI)
        mask |= static_cast<int32_t>(EventType::kEventPriority);
    return mask;
}

turbo::Status EpollPoller::apply_mask_change(EventData *data, int32_t old_mask,
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
        if (epoll_ctl(_epfd, EPOLL_CTL_DEL, data->handle, nullptr) < 0) {
            return turbo::internal_error("epoll_ctl(DEL) failed, errno=", errno);
        }
        return turbo::OkStatus();
    }
    epoll_event ev{};
    ev.events = event_mask_to_epoll(new_mask);
    ev.data.ptr = data;
    const int op = (old_mask == 0) ? EPOLL_CTL_ADD : EPOLL_CTL_MOD;
    if (epoll_ctl(_epfd, op, data->handle, &ev) < 0) {
        return turbo::internal_error("epoll_ctl(ADD/MOD) failed, errno=", errno);
    }
    return turbo::OkStatus();
}

turbo::Status EpollPoller::handle_event(EventData *data, EventType enable_event,
                                        EventType disable_event, turbo::Time cur) {
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

turbo::Status EpollPoller::remove_event(EventData *data, turbo::Time cur) {
    TURBO_UNUSED(cur);
    const int32_t old_mask = data->active_event;
    if (old_mask == 0) {
        return turbo::OkStatus();
    }
    data->active_event = 0;
    return apply_mask_change(data, old_mask, 0);
}

turbo::Result<turbo::Time> EpollPoller::poll(turbo::Duration dur,
                                             fermat::Vector<EventData *> &datas) {
    datas.clear();

    const int timeout_ms = epoll_wait_timeout_ms(dur);
    _events.resize(kDefaultEventBatch);

    int n = 0;
    for (;;) {
        n = epoll_wait(_epfd, _events.data(),
                       static_cast<int>(_events.size()), timeout_ms);
        if (n < 0) {
            if (errno == EINTR) {
                continue;
            }
            return turbo::internal_error("epoll_wait failed, errno=", errno);
        }
        if (n == static_cast<int>(_events.size())) {
            // 事件数组刚好填满，可能还有更多事件，翻倍后重试
            _events.resize(_events.size() * 2);
            continue;
        }
        break;
    }

    datas.reserve(static_cast<size_t>(n));
    for (int i = 0; i < n; ++i) {
        auto *data = static_cast<EventData *>(_events[static_cast<size_t>(i)].data.ptr);
        DKCHECK(data != nullptr);
        const int32_t ready = epoll_events_to_mask(_events[static_cast<size_t>(i)].events);
        data->triggered = static_cast<EventType>(ready);
        datas.push_back(data);
    }

    return turbo::Time::current_time();
}

}  // namespace xio

#endif  // defined(OS_LINUX)