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
// Timer benchmark: EventLoop (WheelTimer / BtreeTimer)
// Measures insertion (schedule) throughput for same and spread expiration times.

#include <benchmark/benchmark.h>
#include <atomic>
#include <thread>
#include <vector>

#include <xio/event/event_loop.h>
#include <turbo/times/time.h>

namespace {

using namespace xio;

// Common no-op callback matching the required signature.
static void event_loop_cb(void*, turbo::Time, turbo::Time) {}  // for EventLoop

// -----------------------------------------------------------------------------
// Helper to start an EventLoop with a specific timer type.
// Returns nullptr on failure.
// -----------------------------------------------------------------------------
static EventLoop* start_event_loop(bool wheel_timer) {
    EventLoopOption opt;
    opt.wheel_timer = wheel_timer;
    opt.disk_io = false;   // use epoll/kqueue, not io_uring
    EventLoop* loop = new EventLoop();
    auto st = loop->start(opt);
    if (!st.ok()) {
        delete loop;
        return nullptr;
    }
    return loop;
}

////////////////////////////////////////////////////////////////////////////////
// Group: Insert same expiration time (all timers expire at the same future time)
////////////////////////////////////////////////////////////////////////////////

// EventLoop with WheelTimer: schedule N timers from loop thread.
static void BM_InsertSameMs_EventLoopWheel(benchmark::State& state) {
    const int64_t n = state.range(0);
    EventLoop* loop = start_event_loop(true);
    if (!loop) {
        state.SkipWithError("Failed to start EventLoop(Wheel)");
        return;
    }
    for (auto _ : state) {
        std::promise<void> ready;
        auto future = ready.get_future();
        // Post a task that runs on the loop thread to add all timers.
        loop->post_task(AsyncTask{
            .arg = new std::tuple<EventLoop*, int64_t, std::promise<void>*>(
                loop, n, &ready),
            .func = [](void* arg) {
                auto* tup = static_cast<std::tuple<EventLoop*, int64_t, std::promise<void>*>*>(arg);
                auto* loop = std::get<0>(*tup);
                int64_t n = std::get<1>(*tup);
                auto* promise = std::get<2>(*tup);
                const auto expire = turbo::Time::current_time() + turbo::Duration::milliseconds(10000);
                for (int64_t i = 0; i < n; ++i) {
                    loop->run_at_in_loop(expire, event_loop_cb, nullptr);
                }
                promise->set_value();
                delete tup;
            },
            .free_func = empty_data_releaser  // no extra cleanup needed
        });
        future.wait();   // ensure insertion is complete before measuring
        // No extra work to measure; pause timing for cleanup and reset.
        state.PauseTiming();
        loop->stop();    // must stop before deletion
        delete loop;
        loop = start_event_loop(true);
        if (!loop) state.SkipWithError("Failed to restart EventLoop(Wheel)");
        state.ResumeTiming();
    }
    loop->stop();
    delete loop;
    state.SetItemsProcessed(state.iterations() * n);
}
BENCHMARK(BM_InsertSameMs_EventLoopWheel)->Arg(100)->Arg(1000)->Unit(benchmark::kNanosecond);

// EventLoop with BtreeTimer: schedule N timers from loop thread.
static void BM_InsertSameMs_EventLoopBtree(benchmark::State& state) {
    const int64_t n = state.range(0);
    EventLoop* loop = start_event_loop(false);
    if (!loop) {
        state.SkipWithError("Failed to start EventLoop(Btree)");
        return;
    }
    for (auto _ : state) {
        std::promise<void> ready;
        auto future = ready.get_future();
        loop->post_task(AsyncTask{
            .arg = new std::tuple<EventLoop*, int64_t, std::promise<void>*>(
                loop, n, &ready),
            .func = [](void* arg) {
                auto* tup = static_cast<std::tuple<EventLoop*, int64_t, std::promise<void>*>*>(arg);
                auto* loop = std::get<0>(*tup);
                int64_t n = std::get<1>(*tup);
                auto* promise = std::get<2>(*tup);
                const auto expire = turbo::Time::current_time() + turbo::Duration::milliseconds(10000);
                for (int64_t i = 0; i < n; ++i) {
                    loop->run_at_in_loop(expire, event_loop_cb, nullptr);
                }
                promise->set_value();
                delete tup;
            },
            .free_func = empty_data_releaser
        });
        future.wait();
        state.PauseTiming();
        loop->stop();
        delete loop;
        loop = start_event_loop(false);
        if (!loop) state.SkipWithError("Failed to restart EventLoop(Btree)");
        state.ResumeTiming();
    }
    loop->stop();
    delete loop;
    state.SetItemsProcessed(state.iterations() * n);
}
BENCHMARK(BM_InsertSameMs_EventLoopBtree)->Arg(100)->Arg(1000)->Unit(benchmark::kNanosecond);

////////////////////////////////////////////////////////////////////////////////
// Group: Insert spread expiration times (each timer expires at i milliseconds)
////////////////////////////////////////////////////////////////////////////////

// EventLoop with WheelTimer: schedule N timers with increasing expiry.
static void BM_InsertSpreadMs_EventLoopWheel(benchmark::State& state) {
    const int64_t n = state.range(0);
    EventLoop* loop = start_event_loop(true);
    if (!loop) {
        state.SkipWithError("Failed to start EventLoop(Wheel)");
        return;
    }
    for (auto _ : state) {
        std::promise<void> ready;
        auto future = ready.get_future();
        loop->post_task(AsyncTask{
            .arg = new std::tuple<EventLoop*, int64_t, std::promise<void>*>(
                loop, n, &ready),
            .func = [](void* arg) {
                auto* tup = static_cast<std::tuple<EventLoop*, int64_t, std::promise<void>*>*>(arg);
                auto* loop = std::get<0>(*tup);
                int64_t n = std::get<1>(*tup);
                auto* promise = std::get<2>(*tup);
                const auto t0 = turbo::Time::current_time();
                for (int64_t i = 1; i <= n; ++i) {
                    auto expire = t0 + turbo::Duration::milliseconds(i);
                    loop->run_at_in_loop(expire, event_loop_cb, nullptr);
                }
                promise->set_value();
                delete tup;
            },
            .free_func = empty_data_releaser
        });
        future.wait();
        state.PauseTiming();
        loop->stop();
        delete loop;
        loop = start_event_loop(true);
        if (!loop) state.SkipWithError("Failed to restart EventLoop(Wheel)");
        state.ResumeTiming();
    }
    loop->stop();
    delete loop;
    state.SetItemsProcessed(state.iterations() * n);
}
BENCHMARK(BM_InsertSpreadMs_EventLoopWheel)->Arg(100)->Arg(1000)->Unit(benchmark::kNanosecond);

// EventLoop with BtreeTimer: schedule N timers with increasing expiry.
static void BM_InsertSpreadMs_EventLoopBtree(benchmark::State& state) {
    const int64_t n = state.range(0);
    EventLoop* loop = start_event_loop(false);
    if (!loop) {
        state.SkipWithError("Failed to start EventLoop(Btree)");
        return;
    }
    for (auto _ : state) {
        std::promise<void> ready;
        auto future = ready.get_future();
        loop->post_task(AsyncTask{
            .arg = new std::tuple<EventLoop*, int64_t, std::promise<void>*>(
                loop, n, &ready),
            .func = [](void* arg) {
                auto* tup = static_cast<std::tuple<EventLoop*, int64_t, std::promise<void>*>*>(arg);
                auto* loop = std::get<0>(*tup);
                int64_t n = std::get<1>(*tup);
                auto* promise = std::get<2>(*tup);
                const auto t0 = turbo::Time::current_time();
                for (int64_t i = 1; i <= n; ++i) {
                    auto expire = t0 + turbo::Duration::milliseconds(i);
                    loop->run_at_in_loop(expire, event_loop_cb, nullptr);
                }
                promise->set_value();
                delete tup;
            },
            .free_func = empty_data_releaser
        });
        future.wait();
        state.PauseTiming();
        loop->stop();
        delete loop;
        loop = start_event_loop(false);
        if (!loop) state.SkipWithError("Failed to restart EventLoop(Btree)");
        state.ResumeTiming();
    }
    loop->stop();
    delete loop;
    state.SetItemsProcessed(state.iterations() * n);
}
BENCHMARK(BM_InsertSpreadMs_EventLoopBtree)->Arg(100)->Arg(1000)->Unit(benchmark::kNanosecond);

} // namespace

BENCHMARK_MAIN();
