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

#include <cstdint>
#include <turbo/utility/status.h>

namespace xio {

#if defined(_WIN32)
    using FileHandle = uintptr_t;
#else
    using FileHandle = int;        // fd
#endif

    static constexpr FileHandle kInvalidFileHandle{-1};

    class EventData;
    class EventLoop;
    class Poller;
    class Waker;

    /// Type of function used to release user data attached to an EventData.
    typedef void (*data_releaser)(void *data);

    void empty_data_releaser(void *data);

    /// Type of a task function that can be posted to EventLoop.
    using task_func = void (*)(void *arg);

    /// A generic task that can be posted to EventLoop.
    struct AsyncTask {
        void *arg{nullptr}; ///< User argument passed to `func`.
        task_func func{nullptr}; ///< Function to execute.
        data_releaser free_func{empty_data_releaser}; ///< Function to release `arg`.
    };

    using initialize_func = turbo::Status (*)(void *arg);

    /// A generic task that can be posted to EventLoop.
    struct InitializeTask {
        void *arg{nullptr}; ///< User argument passed to `func`.
        initialize_func func{nullptr}; ///< Function to execute.
    };


    struct PollerConfig {
    };

    struct EventLoopOption {
        bool disk_io{false};
        bool wheel_timer{true};
        PollerConfig poller_config;
        std::vector<InitializeTask> initialize_tasks;
    };

}  // namespace xio
