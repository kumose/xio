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

#include <atomic>
#include <condition_variable>
#include <future>
#include <memory>
#include <mutex>
#include <thread>

#include <fermat/container/vector.h>
#include <xio/event/event_data.h>
#include <xio/event/poller.h>
#include <xio/event/waker.h>
#include <xio/timer/wheel_timer.h>
#include <turbo/container/span.h>
#include <turbo/times/time.h>
#include <turbo/utility/status.h>

namespace xio {
    /// Single-threaded event loop that drives I/O, timers, and posted tasks.
    ///
    /// EventLoop follows the "one loop per thread" model. All operations except
    /// `post_task` and similar cross‑thread calls must be performed on the loop
    /// thread (checked via `is_in_loop_thread()`). The loop dispatches I/O events
    /// registered via `EventData`, triggers timers, and executes `AsyncTask`s
    /// posted from any thread.
    enum class EventLoopStatus {
        /// Not started yet.
        kEventLoopNone,
        /// Running normally.
        kEventLoopRunning,
        /// In the process of stopping.
        kEventLoopStoping,
        /// Stopped, cannot be restarted.
        kEventLoopStopped
    };

    class EventLoop {
    public:
        /// Type used to identify the loop thread.
        using LoopThreadId = uint64_t;
        /// Invalid loop thread identifier.
        static constexpr LoopThreadId kInvalidLoopThreadId = 0;

        /// Default constructor.
        EventLoop() = default;

        /// Destructor, cleans up resources.
        ~EventLoop();

        /// Copy constructor is deleted.
        EventLoop(const EventLoop &) = delete;

        /// Copy assignment is deleted.
        EventLoop &operator=(const EventLoop &) = delete;

        /// Starts the event loop thread and initializes poller and waker.
        ///
        /// This method is thread-safe and can be called once per instance.
        /// @param option Configuration options.
        /// @return OkStatus on success, or an error status.
        turbo::Status start(const EventLoopOption &option);

        /// Stops the event loop thread and waits for it to finish.
        /// This method internally calls sync() to ensure all pending tasks are
        /// processed before the thread exits.
        void stop();

        /// Synchronously waits for all currently posted tasks to be executed.
        /// If called from the event loop thread, this function returns immediately
        /// to avoid deadlock. Otherwise, it blocks until all tasks that have been
        /// posted via post_task, post_loop_task, post_exit_task, etc., are processed.
        /// This is useful when the caller needs to guarantee that prior asynchronous
        /// operations have completed before proceeding.
        void sync();

        /// Posts a task that runs before every loop iteration (function pointer version).
        /// @param func The function to execute.
        /// @param arg  User argument.
        void post_loop_task(task_func func, void *arg);

        /// Posts a task that runs before every loop iteration (AsyncTask version).
        /// @param task The AsyncTask to add.
        void post_loop_task(AsyncTask task);

        /// Cancels a previously posted loop task (function pointer version).
        /// @param func The function pointer of the task to cancel.
        /// @param arg  The argument pointer of the task to cancel.
        void cancel_loop_task(task_func func, void *arg);

        /// Cancels a previously posted loop task (AsyncTask version).
        /// @param task The AsyncTask to cancel.
        void cancel_loop_task(AsyncTask task);

        /// Posts a task that runs once when the loop exits (function pointer version).
        /// @param func The function to execute.
        /// @param arg  User argument.
        void post_exit_task(task_func func, void *arg);

        /// Posts a task that runs once when the loop exits (AsyncTask version).
        /// @param task The AsyncTask to add.
        void post_exit_task(AsyncTask task);

        /// Cancels a previously posted exit task (function pointer version).
        /// @param func The function pointer of the task to cancel.
        /// @param arg  The argument pointer of the task to cancel.
        void cancel_exit_task(task_func func, void *arg);

        /// Cancels a previously posted exit task (AsyncTask version).
        /// @param task The AsyncTask to cancel.
        void cancel_exit_task(AsyncTask task);

        /// Returns the identifier of the loop thread.
        LoopThreadId loop_thread_id() const { return _loop_thread_id; }

        /// Returns true if the caller is on the event loop thread.
        bool is_in_loop_thread() const;

        /// Atomically enables and disables events for an EventData (cross‑thread, blocking).
        /// @param data The EventData object.
        /// @param enable Events to enable.
        /// @param disable Events to disable.
        /// @return OkStatus on success, or an error status.
        turbo::Status handle_events(EventData *data, EventType enable, EventType disable);

        /// Enables events for an EventData.
        /// @param data The EventData object.
        /// @param event Events to enable.
        turbo::Status enable_events(EventData *data, EventType event) {
            return handle_events(data, event, EventType::kEventNone);
        }

        /// Disables events for an EventData.
        /// @param data The EventData object.
        /// @param event_type Events to disable.
        turbo::Status disable_events(EventData *data, EventType event_type) {
            return handle_events(data, EventType::kEventNone, event_type);
        }

        /// Removes an EventData from the poller and cancels its timer (cross‑thread, blocking).
        /// @param data The EventData to remove.
        /// @return OkStatus on success.
        turbo::Status remove_events(EventData *data);

        /// Posts a single task to be executed on the loop thread (function pointer version).
        /// @param func The function to execute.
        /// @param arg User argument.
        void post_task(task_func func, void *arg);

        /// Posts a single task to be executed on the loop thread (AsyncTask version).
        /// @param task The AsyncTask to add.
        void post_task(AsyncTask task);

        /// Batch version of post_task.
        /// @param tasks A span containing the tasks to be posted.
        void post_task(turbo::span<AsyncTask> tasks);

        /// Schedules a timer at absolute time.
        /// @param expire_time Absolute time when the timer should fire.
        /// @param callback Function called with (param, expire_time, current_time).
        /// @param param User data passed to callback.
        uint64_t run_at(turbo::Time expire_time, timer_callback callback, void *param = nullptr);

        /// Schedules a timer after a relative duration.
        /// @param duration Relative time after which the timer should fire.
        /// @param callback Function called with (param, expire_time, current_time).
        /// @param param User data passed to callback.
        uint64_t run_after(turbo::Duration duration, timer_callback callback, void *param = nullptr);

        void cancel_timer(uint64_t timer_id);

        /// Wakes up the loop thread if it is blocked in poll().
        void wakeup();

        /// Returns true if the underlying poller supports asynchronous disk I/O.
        bool supports_disk_async() const;

        /// Returns the name of the active poller backend.
        std::string_view backend() const;

        /// Atomically updates event mask on the loop thread (must be on loop thread).
        /// @param data The EventData object.
        /// @param enable_event Events to enable.
        /// @param disable_event Events to disable.
        /// @param cur Current time.
        /// @return OkStatus on success.
        turbo::Status handle_events_in_loop(EventData *data, EventType enable_event,
                                            EventType disable_event, turbo::Time cur);

        /// Enable events on the loop thread.
        turbo::Status enable_events_in_loop(EventData *data, EventType event, turbo::Time cur) {
            return handle_events_in_loop(data, event, EventType::kEventNone, cur);
        }

        /// Disable events on the loop thread.
        turbo::Status disable_events_in_loop(EventData *data, EventType event_type, turbo::Time cur) {
            return handle_events_in_loop(data, EventType::kEventNone, event_type, cur);
        }

        /// Remove events and timer from the poller on the loop thread.
        /// @param data The EventData to remove.
        /// @param cur Current time.
        turbo::Status remove_events_in_loop(EventData *data, turbo::Time cur);

        /// Post a task to be executed on the loop thread (function pointer version, loop thread only).
        void post_task_in_loop(task_func func, void *arg);

        /// Batch post tasks on the loop thread (loop thread only).
        void post_task_in_loop(turbo::span<AsyncTask> tasks);

        /// Post a single AsyncTask on the loop thread (loop thread only).
        void post_task_in_loop(AsyncTask task);

        /// Schedule a timer on the loop thread (loop thread only).
        uint64_t run_at_in_loop(turbo::Time expire_time, timer_callback callback, void *param = nullptr);

        /// Schedule a relative timer on the loop thread (loop thread only).
        uint64_t run_after_in_loop(turbo::Duration duration, timer_callback callback, void *param = nullptr);

        void cancel_timer_in_loop(uint64_t timer_id);

        /// Returns the current status of the EventLoop.
        turbo::Status status() const { return _status; }

        /// Returns true if the EventLoop is in a valid state.
        [[nodiscard]] bool ok() const { return _status.ok(); }

    private:
        /// Entry point of the loop thread.
        void loop_main();

        /// Single iteration of the loop (poll + dispatch + timers).
        void run_once();

        /// Execute all pending posted tasks.
        void drain_post_tasks();

        /// Compute timeout for poll().
        /// @param now Current time.
        turbo::Duration poll_timeout(turbo::Time now) const;

        /// Dispatch ready I/O events.
        static void dispatch_ready(const fermat::Vector<EventData *> &ready);

        /// Common implementation for all post variants.
        void post_task_impl(AsyncTask task);

        /// Deprecated, kept for compatibility.
        turbo::Status handle_events_impl(EventData *data, EventType event_type, turbo::Time cur);

        /// Release all resources (timers, tasks, waker, poller).
        void clear_resource();

        /// Context for run_at cross‑thread forwarding.
        struct TimerPostCtx {
            EventLoop *loop{nullptr};
            turbo::Time expire_time;
            timer_callback callback{nullptr};
            void *param{nullptr};
            uint64_t timer_id{0};
            std::promise<uint64_t> promise;
        };

        /// Context for handle_events cross‑thread forwarding.
        struct HandleEventsPostCtx {
            EventLoop *loop{nullptr};
            EventData *data{nullptr};
            EventType enable_event{EventType::kEventNone};
            EventType disable_event{EventType::kEventNone};
            std::promise<turbo::Status> promise;
        };


        /// Context for posting loop/exit tasks from other threads.
        struct PostTaskCtx {
            AsyncTask task;
            EventLoop *loop{nullptr};
        };

        struct StopToken {
            std::promise<int> promise;
        };

        /// Free function for PostTaskCtx.
        static void post_task_ctx_free(void *arg);

        /// Implementation of post_loop_task on loop thread.
        static void post_loop_task_impl(void *arg);

        /// Implementation of cancel_loop_task on loop thread.
        static void cancel_loop_task_impl(void *arg);

        /// Implementation of post_exit_task on loop thread.
        static void post_exit_task_impl(void *arg);

        /// Implementation of cancel_exit_task on loop thread.
        static void cancel_exit_task_impl(void *arg);

        static void run_stop_token(void *arg);

        /// Trampoline for run_at cross‑thread.
        static void run_at_post_trampoline(void *arg);
        static void cancel_timer_post_trampoline(void *arg);

        /// Trampoline for handle_events cross‑thread.
        static void handle_events_post_trampoline(void *arg);

        /// Trampoline for remove_events cross‑thread.
        static void remove_events_post_trampoline(void *arg);

        /// Release a posted task.
        static void release_post_task(AsyncTask &task);

        /// Execute a posted task.
        static void run_post_task(AsyncTask &task);

        /// Clear all pending posted tasks.
        void clear_post_tasks();

        /// Clear all timers.
        void clear_timers();

        /// Thread-local pointer to the current EventLoop.
        static thread_local EventLoop *tls_event_loop;

        /// Stored configuration options.
        EventLoopOption _option;
        /// I/O multiplexing backend.
        std::unique_ptr<Poller> _poller;
        /// Cross‑thread wakeup mechanism.
        Waker _waker;
        /// Timer backend (WheelTimer or BtreeTimer).
        std::unique_ptr<TimerBase> _timer;

        /// Protects _remote_tasks.
        std::mutex _task_mutex;
        /// Tasks posted from loop thread.
        fermat::Vector<AsyncTask> _local_tasks;
        /// Tasks posted from other threads.
        fermat::Vector<AsyncTask> _remote_tasks;

        /// Tasks executed before every loop iteration.
        fermat::Vector<AsyncTask> _loop_tasks;
        /// Tasks executed once during loop exit (LIFO).
        fermat::Vector<AsyncTask> _loop_exit_tasks;

        /// Identifier of the loop thread.
        LoopThreadId _loop_thread_id{kInvalidLoopThreadId};
        /// The actual OS thread.
        std::thread _thread;
        /// Mutex for lifecycle synchronization.
        std::mutex _lifecycle_mutex;
        /// Condition variable for lifecycle synchronization.
        std::condition_variable _lifecycle_cv;
        /// Current status of the loop.
        EventLoopStatus _loop_status{EventLoopStatus::kEventLoopNone};
        /// Flag to stop the loop.
        std::atomic<bool> _running{true};
        /// Current status (Ok if running or stopped cleanly).
        turbo::Status _status{turbo::failed_precondition_error("uninitialize")};
    };
} // namespace xio
