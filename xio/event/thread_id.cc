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

#include <limits>
#include <thread>
#include <xio/event/thread_id.h>
#include <turbo/base/macros/likely.h>
#include <turbo/threading/thread_id_name_manager.h>
namespace xio {

    static constexpr uint64_t kNThreadId = std::numeric_limits<uint64_t>::max();
    thread_local uint64_t tls_thread_id{kNThreadId};

    std::atomic<uint64_t> g_thread_id{0};

    void initialize_thread_id() {
        tls_thread_id = g_thread_id.fetch_add(1);
        static const std::thread::id main_id = std::this_thread::get_id();
    }

    uint64_t ThreadId::get_id() {
        if (TURBO_UNLIKELY(tls_thread_id == kNThreadId)) {
            initialize_thread_id();
        }
        return tls_thread_id;
    }

}  // namespace xio
