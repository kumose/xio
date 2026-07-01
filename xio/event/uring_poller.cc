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

#include <xio/event/uring_poller.h>

#include <cerrno>
#include <cstdint>

#include <poll.h>

#include <turbo/log/logging.h>
#include <turbo/times/time.h>
#include <turbo/utility/status.h>

namespace xio {

namespace {

    constexpr unsigned kUringQueueDepth = 4096;

    int32_t event_type_to_mask(EventType event) {
        return static_cast<int32_t>(event);
    }

    bool poll_duration_is_infinite(turbo::Duration dur) {
        return turbo::Duration::to_milliseconds(dur) < 0;
    }

    void duration_to_kernel_timespec(turbo::Duration dur, struct __kernel_timespec &ts) {
        const int64_t timeout_ms = turbo::Duration::to_milliseconds(dur);
        ts.tv_sec = static_cast<int64_t>(timeout_ms / 1000);
        ts.tv_nsec = static_cast<long>((timeout_ms % 1000) * 1000000L);
    }

}  // namespace

    UringPoller::UringPoller() = default;

    UringPoller::~UringPoller() {
        if (_initialized) {
            io_uring_queue_exit(&_ring);
            _initialized = false;
        }
    }

    turbo::Status UringPoller::initialize(const PollerConfig &pc) {
        TURBO_UNUSED(pc);
        if (_initialized) {
            return turbo::OkStatus();
        }
        const int rc = io_uring_queue_init(kUringQueueDepth, &_ring, 0);
        if (rc < 0) {
            return turbo::internal_error("io_uring_queue_init failed, errno=", -rc);
        }
        _initialized = true;
        return turbo::OkStatus();
    }

    turbo::Status UringPoller::submit_queue() {
        const int rc = io_uring_submit(&_ring);
        if (rc < 0) {
            return turbo::internal_error("io_uring_submit failed, errno=", -rc);
        }
        return turbo::OkStatus();
    }

    unsigned UringPoller::poll_mask_from_active(int32_t mask) {
        unsigned poll_mask = 0;
        if (mask & static_cast<int32_t>(EventType::kEventRead)) {
            poll_mask |= POLLIN;
        }
        if (mask & static_cast<int32_t>(EventType::kEventWrite)) {
            poll_mask |= POLLOUT;
        }
        if (mask & static_cast<int32_t>(EventType::kEventError)) {
            poll_mask |= POLLERR;
        }
        if (mask & static_cast<int32_t>(EventType::kEventHangUp)) {
            poll_mask |= POLLHUP;
        }
        if (mask & static_cast<int32_t>(EventType::kEventReadHup)) {
#ifdef POLLRDHUP
            poll_mask |= POLLRDHUP;
#endif
        }
        if (mask & static_cast<int32_t>(EventType::kEventPriority)) {
            poll_mask |= POLLPRI;
        }
        return poll_mask;
    }

    turbo::Status UringPoller::apply_mask_change(EventData *data, int32_t old_mask,
                                                 int32_t new_mask) {
        DKCHECK(data != nullptr);
        if (data->handle == kInvalidFileHandle) {
            return turbo::invalid_argument_error("invalid FileHandle");
        }

        if (old_mask != 0) {
            struct io_uring_sqe *sqe = io_uring_get_sqe(&_ring);
            DKCHECK(sqe != nullptr);
            io_uring_prep_poll_remove(sqe, reinterpret_cast<__u64>(data));
        }

        if (new_mask != 0) {
            struct io_uring_sqe *sqe = io_uring_get_sqe(&_ring);
            DKCHECK(sqe != nullptr);
            const unsigned poll_mask = poll_mask_from_active(new_mask);
            io_uring_prep_poll_multishot(sqe, data->handle, poll_mask);
            io_uring_sqe_set_data(sqe, data);
        }

        return submit_queue();
    }

    int32_t UringPoller::poll_events_to_mask(int poll_events) {
        int32_t mask = 0;
        if (poll_events & POLLIN) {
            mask |= static_cast<int32_t>(EventType::kEventRead);
        }
        if (poll_events & POLLOUT) {
            mask |= static_cast<int32_t>(EventType::kEventWrite);
        }
        if (poll_events & POLLERR) {
            mask |= static_cast<int32_t>(EventType::kEventError);
        }
        if (poll_events & POLLHUP) {
            mask |= static_cast<int32_t>(EventType::kEventHangUp);
        }
#ifdef POLLRDHUP
        if (poll_events & POLLRDHUP) {
            mask |= static_cast<int32_t>(EventType::kEventReadHup);
        }
#endif
        if (poll_events & POLLPRI) {
            mask |= static_cast<int32_t>(EventType::kEventPriority);
        }
        return mask;
    }

    void UringPoller::merge_ready(EventData *data, int32_t ready,
                                  fermat::Vector<EventData *> &datas) {
        if (ready == 0) {
            return;
        }
        DKCHECK(data != nullptr);
        for (size_t i = 0; i < datas.size(); ++i) {
            if (datas[i] == data) {
                datas[i]->triggered = static_cast<EventType>(
                        static_cast<int32_t>(datas[i]->triggered) |
                        static_cast<int32_t>(ready));
                return;
            }
        }
        data->triggered = static_cast<EventType>(ready);
        datas.push_back(data);
    }

    turbo::Status UringPoller::handle_event(EventData *data, EventType enable_event,
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

    turbo::Status UringPoller::remove_event(EventData *data, turbo::Time cur) {
        TURBO_UNUSED(cur);
        const int32_t old_mask = data->active_event;
        if (old_mask == 0) {
            return turbo::OkStatus();
        }
        data->active_event = 0;
        return apply_mask_change(data, old_mask, 0);
    }

    turbo::Result<turbo::Time> UringPoller::poll(turbo::Duration dur,
                                                 fermat::Vector<EventData *> &datas) {
        datas.clear();

        struct __kernel_timespec ts {};
        struct __kernel_timespec *ts_ptr = nullptr;
        if (!poll_duration_is_infinite(dur)) {
            duration_to_kernel_timespec(dur, ts);
            ts_ptr = &ts;
        }

        bool first_wait = true;
        for (;;) {
            struct io_uring_cqe *cqe = nullptr;
            const int wait_rc =
                    io_uring_wait_cqe_timeout(&_ring, &cqe, first_wait ? ts_ptr : nullptr);
            first_wait = false;

            if (wait_rc == -ETIME) {
                break;
            }
            if (wait_rc < 0) {
                if (wait_rc == -EINTR) {
                    continue;
                }
                return turbo::internal_error("io_uring_wait_cqe_timeout failed, errno=", -wait_rc);
            }

            do {
                auto *data = static_cast<EventData *>(io_uring_cqe_get_data(cqe));
                int32_t ready = 0;
                if (cqe->res < 0) {
                    ready = static_cast<int32_t>(EventType::kEventError);
                } else {
                    ready = poll_events_to_mask(cqe->res);
                }
                merge_ready(data, ready, datas);
                io_uring_cqe_seen(&_ring, cqe);
            } while (io_uring_peek_cqe(&_ring, &cqe) == 0);

            break;
        }

        return turbo::Time::current_time();
    }

}  // namespace xio

#endif  // defined(OS_LINUX)