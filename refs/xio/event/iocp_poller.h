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

#include <turbo/base/macros.h>
#if defined(OS_WIN)

#include <winsock2.h>

#include <xio/event/poller.h>

namespace xio {

    class IocpPoller : public Poller {
    public:
        IocpPoller();

        ~IocpPoller() override;

        turbo::Status initialize(const PollerConfig &pc) override;

        turbo::Result<turbo::Time> poll(turbo::Duration dur,
                                        fermat::Vector<EventData *> &datas) override;

        turbo::Status handle_event(EventData * TURBO_RESTRICT data, EventType enable_event,
                                   EventType disable_event, turbo::Time cur) override;

        turbo::Status remove_event(EventData * TURBO_RESTRICT data, turbo::Time cur) override;

        bool supports_disk_async() const override { return false; }

        std::string_view name() const override { return "iocp"; }

    private:
        struct SocketEntry;

        SocketEntry *find_entry(EventData *data);
        SocketEntry *get_or_create_entry(EventData *data);
        void erase_entry(EventData *data);
        turbo::Status apply_mask_change(EventData *data, int32_t old_mask, int32_t new_mask);
        turbo::Status associate_socket(SocketEntry *entry);
        turbo::Status post_read(SocketEntry *entry);
        turbo::Status post_write(SocketEntry *entry);
        void cancel_read(SocketEntry *entry);
        void cancel_write(SocketEntry *entry);

        static int32_t completion_to_mask(const SocketEntry *entry, const OVERLAPPED *ov);
        static void merge_ready(EventData *data, int32_t ready,
                                fermat::Vector<EventData *> &datas);

        HANDLE _iocp{INVALID_HANDLE_VALUE};
        bool _wsa_started{false};
        fermat::Vector<SocketEntry> _entries;
    };

}  // namespace xio

#endif  // defined(OS_WIN)