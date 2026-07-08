// Copyright (C) 2026 Kumo inc. and its affiliates. All Rights Reserved.
//
// Timer benchmark notes:
// - next_timeout() is O(1) (cached _next_trigger_at); btree begin() is not on that path.
// - PauseTiming around timer destruction: ~Timer purges pending tasks and dominates
//   at large N if left in the timed section (common pitfall).
// - Wheel TriggerSpreadFar drives 1ms ticks up to N*1000ms; very slow at large N.
// - Disabled cases below are kept as comments for manual --benchmark_filter runs.

#include <benchmark/benchmark.h>

#include <algorithm>
#include <atomic>
#include <cstdint>
#include <vector>

#include <xio/timer/btree_timer.h>
#include <xio/timer/wheel_timer.h>
#include <turbo/times/time.h>

namespace {
    inline turbo::Duration bench_ms(int64_t value) {
        return turbo::Duration::milliseconds(value);
    }

    static void noop_cb(void *, turbo::Time, turbo::Time) {
        // no operation
    }

    static std::atomic<int64_t> g_fire_count{0};

    static void count_cb(void *, turbo::Time, turbo::Time) {
        g_fire_count.fetch_add(1, std::memory_order_relaxed);
    }

    template<typename Timer>
    void drive_until(Timer &timer, turbo::Time until) {
        for (int guard = 0; guard < 1000000; ++guard) {
            const turbo::Time next = timer.next_timeout();
            if (next > until) {
                return;
            }
            timer.on_trigger(next);
        }
    }

    // -------------------------------------------------------------------------
    // Insert (destructor paused: measures insert only, not purge)
    // -------------------------------------------------------------------------

    template<typename Timer>
    static void BM_InsertSameMs(benchmark::State &state) {
        const int64_t n = state.range(0);

        for (auto _: state) {
            {
                Timer timer;
                const turbo::Time expire = turbo::Time::current_time() + bench_ms(10'000);
                for (int64_t i = 0; i < n; ++i) {
                    benchmark::DoNotOptimize(timer.run_at(expire, noop_cb, nullptr));
                }
                state.PauseTiming();
            }
            state.ResumeTiming();
        }
        state.SetItemsProcessed(state.iterations() * n);
    }

    template<typename Timer>
    static void BM_InsertSpreadMs(benchmark::State &state) {
        const int64_t n = state.range(0);

        for (auto _: state) {
            {
                Timer timer;
                const turbo::Time t0 = turbo::Time::current_time();
                for (int64_t i = 1; i <= n; ++i) {
                    benchmark::DoNotOptimize(timer.run_at(t0 + bench_ms(i), noop_cb, nullptr));
                }
                state.PauseTiming();
            }
            state.ResumeTiming();
        }
        state.SetItemsProcessed(state.iterations() * n);
    }

    template<typename Timer>
    static void BM_InsertSpreadFarMs(benchmark::State &state) {
        const int64_t n = state.range(0);

        for (auto _: state) {
            {
                Timer timer;
                const turbo::Time t0 = turbo::Time::current_time();
                for (int64_t i = 1; i <= n; ++i) {
                    benchmark::DoNotOptimize(
                        timer.run_at(t0 + bench_ms(i * 1000), noop_cb, nullptr));
                }
                state.PauseTiming();
            }
            state.ResumeTiming();
        }
        state.SetItemsProcessed(state.iterations() * n);
    }

    template<typename Timer>
    static void BM_InsertClusteredMs(benchmark::State &state) {
        const int64_t n = state.range(0);
        const int64_t buckets = std::max<int64_t>(1, state.range(1));

        for (auto _: state) {
            {
                Timer timer;
                const turbo::Time t0 = turbo::Time::current_time();
                for (int64_t i = 0; i < n; ++i) {
                    const int64_t due = (i % buckets) + 1;
                    benchmark::DoNotOptimize(timer.run_at(t0 + bench_ms(due), noop_cb, nullptr));
                }
                state.PauseTiming();
            }
            state.ResumeTiming();
        }
        state.SetItemsProcessed(state.iterations() * n);
    }

    // -------------------------------------------------------------------------
    // Cancel (schedule + ctor/dtor paused; timed section is cancel only)
    // -------------------------------------------------------------------------

    template<typename Timer>
    static void BM_CancelSameMs(benchmark::State &state) {
        const int64_t n = state.range(0);

        for (auto _: state) {
            {
                Timer timer;
                std::vector<uint64_t> ids(static_cast<size_t>(n));

                state.PauseTiming();
                const turbo::Time expire = turbo::Time::current_time() + bench_ms(10'000);
                for (int64_t i = 0; i < n; ++i) {
                    ids[static_cast<size_t>(i)] = timer.run_at(expire, noop_cb, nullptr);
                }
                state.ResumeTiming();

                for (int64_t i = 0; i < n; ++i) {
                    timer.cancel(ids[static_cast<size_t>(i)]);
                }

                state.PauseTiming();
            }
            state.ResumeTiming();
        }
        state.SetItemsProcessed(state.iterations() * n);
    }

    template<typename Timer>
    static void BM_CancelSpreadMs(benchmark::State &state) {
        const int64_t n = state.range(0);

        for (auto _: state) {
            {
                Timer timer;
                std::vector<uint64_t> ids(static_cast<size_t>(n));

                state.PauseTiming();
                const turbo::Time t0 = turbo::Time::current_time();
                for (int64_t i = 0; i < n; ++i) {
                    ids[static_cast<size_t>(i)] =
                            timer.run_at(t0 + bench_ms(i + 1), noop_cb, nullptr);
                }
                state.ResumeTiming();

                for (int64_t i = 0; i < n; ++i) {
                    timer.cancel(ids[static_cast<size_t>(i)]);
                }

                state.PauseTiming();
            }
            state.ResumeTiming();
        }
        state.SetItemsProcessed(state.iterations() * n);
    }

    template<typename Timer>
    static void BM_CancelSpreadLifo(benchmark::State &state) {
        const int64_t n = state.range(0);

        for (auto _: state) {
            {
                Timer timer;
                std::vector<uint64_t> ids(static_cast<size_t>(n));

                state.PauseTiming();
                const turbo::Time t0 = turbo::Time::current_time();
                for (int64_t i = 0; i < n; ++i) {
                    ids[static_cast<size_t>(i)] =
                            timer.run_at(t0 + bench_ms(i + 1), noop_cb, nullptr);
                }
                state.ResumeTiming();

                for (int64_t i = n - 1; i >= 0; --i) {
                    timer.cancel(ids[static_cast<size_t>(i)]);
                }

                state.PauseTiming();
            }
            state.ResumeTiming();
        }
        state.SetItemsProcessed(state.iterations() * n);
    }

    template<typename Timer>
    static void BM_CancelSpreadHalf(benchmark::State &state) {
        const int64_t n = state.range(0);

        for (auto _: state) {
            {
                Timer timer;
                std::vector<uint64_t> ids(static_cast<size_t>(n));

                state.PauseTiming();
                const turbo::Time t0 = turbo::Time::current_time();
                for (int64_t i = 0; i < n; ++i) {
                    ids[static_cast<size_t>(i)] =
                            timer.run_at(t0 + bench_ms(i + 1), noop_cb, nullptr);
                }
                state.ResumeTiming();

                for (int64_t i = 0; i < n; i += 2) {
                    timer.cancel(ids[static_cast<size_t>(i)]);
                }

                state.PauseTiming();
            }
            state.ResumeTiming();
        }
        state.SetItemsProcessed(state.iterations() * (n / 2 + n % 2));
    }

    // -------------------------------------------------------------------------
    // Trigger (schedule paused; destructor paused after callbacks)
    // -------------------------------------------------------------------------

    template<typename Timer>
    static void BM_TriggerBatchSameMs(benchmark::State &state) {
        const int64_t n = state.range(0);

        for (auto _: state) {
            {
                Timer timer;
                const turbo::Time t0 = turbo::Time::current_time();
                const turbo::Time expire = t0 + bench_ms(500);

                state.PauseTiming();
                for (int64_t i = 0; i < n; ++i) {
                    timer.run_at(expire, count_cb, nullptr);
                }
                g_fire_count.store(0, std::memory_order_relaxed);
                state.ResumeTiming();

                timer.on_trigger(expire);
                benchmark::DoNotOptimize(g_fire_count.load(std::memory_order_relaxed));
                state.PauseTiming();
            }
            state.ResumeTiming();
        }
        state.SetItemsProcessed(state.iterations() * n);
    }

    template<typename Timer>
    static void BM_TriggerSpreadMs(benchmark::State &state) {
        const int64_t n = state.range(0);

        for (auto _: state) {
            {
                Timer timer;
                const turbo::Time t0 = turbo::Time::current_time();

                state.PauseTiming();
                for (int64_t i = 1; i <= n; ++i) {
                    timer.run_at(t0 + bench_ms(i), count_cb, nullptr);
                }
                g_fire_count.store(0, std::memory_order_relaxed);
                state.ResumeTiming();

                drive_until(timer, t0 + bench_ms(n));
                benchmark::DoNotOptimize(g_fire_count.load(std::memory_order_relaxed));
                state.PauseTiming();
            }
            state.ResumeTiming();
        }
        state.SetItemsProcessed(state.iterations() * n);
    }

    // Wheel advances 1ms per on_trigger step toward N*1000ms — very slow for Wheel at large N.
    template<typename Timer>
    static void BM_TriggerSpreadFarMs(benchmark::State &state) {
        const int64_t n = state.range(0);

        for (auto _: state) {
            {
                Timer timer;
                const turbo::Time t0 = turbo::Time::current_time();

                state.PauseTiming();
                for (int64_t i = 1; i <= n; ++i) {
                    timer.run_at(t0 + bench_ms(i * 1000), count_cb, nullptr);
                }
                g_fire_count.store(0, std::memory_order_relaxed);
                state.ResumeTiming();

                drive_until(timer, t0 + bench_ms(n * 1000));
                benchmark::DoNotOptimize(g_fire_count.load(std::memory_order_relaxed));
                state.PauseTiming();
            }
            state.ResumeTiming();
        }
        state.SetItemsProcessed(state.iterations() * n);
    }

    template<typename Timer>
    static void BM_TriggerEmpty(benchmark::State &state) {
        for (auto _: state) {
            {
                Timer timer;
                timer.on_trigger(turbo::Time::current_time() + bench_ms(100));
                benchmark::ClobberMemory();
                state.PauseTiming();
            }
            state.ResumeTiming();
        }
    }

    // -------------------------------------------------------------------------
    // Poll / lifecycle
    // -------------------------------------------------------------------------

    // Setup once; timed loop is only next_timeout() (O(1), independent of N).
    template<typename Timer>
    static void BM_NextTimeoutPoll(benchmark::State &state) {
        Timer timer;
        const turbo::Time t0 = turbo::Time::current_time();
        const int64_t n = state.range(0);
        for (int64_t i = 1; i <= n; ++i) {
            timer.run_at(t0 + bench_ms(i), noop_cb, nullptr);
        }

        for (auto _: state) {
            benchmark::DoNotOptimize(timer.next_timeout());
        }
    }

    template<typename Timer>
    static void BM_NextTimeoutAfterInsert(benchmark::State &state) {
        const int64_t n = state.range(0);

        for (auto _: state) {
            turbo::Time next{};
            {
                Timer timer;
                const turbo::Time t0 = turbo::Time::current_time();
                state.PauseTiming();
                for (int64_t i = 1; i <= n; ++i) {
                    timer.run_at(t0 + bench_ms(i), noop_cb, nullptr);
                }
                state.ResumeTiming();
                next = timer.next_timeout();
                state.PauseTiming();
            }
            state.ResumeTiming();
            benchmark::DoNotOptimize(next);
        }
    }

    template<typename Timer>
    static void BM_DestroySpread(benchmark::State &state) {
        const int64_t n = state.range(0);

        for (auto _: state) {
            state.PauseTiming();
            auto *timer = new Timer();
            const turbo::Time t0 = turbo::Time::current_time();
            for (int64_t i = 1; i <= n; ++i) {
                timer->run_at(t0 + bench_ms(i), noop_cb, nullptr);
            }
            state.ResumeTiming();
            delete timer;
        }
    }

    template<typename Timer>
    static void BM_ScheduleCancelCycle(benchmark::State &state) {
        for (auto _: state) {
            {
                Timer timer;
                const turbo::Time expire = turbo::Time::current_time() + bench_ms(10'000);
                const uint64_t id = timer.run_at(expire, noop_cb, nullptr);
                timer.cancel(id);
                benchmark::ClobberMemory();
                state.PauseTiming();
            }
            state.ResumeTiming();
        }
        state.SetItemsProcessed(state.iterations() * 2);
    }

    template<typename Timer>
    static void BM_InsertTriggerSameMs(benchmark::State &state) {
        const int64_t n = state.range(0);

        for (auto _: state) {
            {
                Timer timer;
                const turbo::Time expire = turbo::Time::current_time() + bench_ms(1000);
                for (int64_t i = 0; i < n; ++i) {
                    timer.run_at(expire, count_cb, nullptr);
                }
                timer.on_trigger(expire);
                benchmark::ClobberMemory();
                state.PauseTiming();
            }
            state.ResumeTiming();
        }
        state.SetItemsProcessed(state.iterations() * (n + 1));
    }

    // -------------------------------------------------------------------------
    // Registration
    // -------------------------------------------------------------------------

    constexpr int64_t kN100 = 100;
    constexpr int64_t kN1000 = 1000;
    // N=10000: many cases exceed ~10ms/iter; enable manually when needed.
    // constexpr int64_t kN10000 = 10000;

#define REGISTER_TIMER_BENCH(BM, suffix)                                              \
    BENCHMARK_TEMPLATE(BM, xio::WheelTimer)->Name(#BM "/WheelTimer" suffix)       \
            ->Unit(benchmark::kNanosecond);                                           \
    BENCHMARK_TEMPLATE(BM, xio::BtreeTimer)->Name(#BM "/BtreeTimer" suffix)

#define REGISTER_TIMER_BENCH_ARGS2(BM, suffix, a, b)                                    \
    BENCHMARK_TEMPLATE(BM, xio::WheelTimer)                                       \
            ->Name(#BM "/WheelTimer" suffix)->Arg(a)->Arg(b)->Unit(benchmark::kNanosecond); \
    BENCHMARK_TEMPLATE(BM, xio::BtreeTimer)                                       \
            ->Name(#BM "/BtreeTimer" suffix)->Arg(a)->Arg(b)->Unit(benchmark::kNanosecond)

#define REGISTER_TIMER_BENCH_CLUSTERED(BM, suffix, n, k)                              \
    BENCHMARK_TEMPLATE(BM, xio::WheelTimer)                                       \
            ->Name(#BM "/WheelTimer" suffix)->Args({n, k})->Unit(benchmark::kNanosecond); \
    BENCHMARK_TEMPLATE(BM, xio::BtreeTimer)                                       \
            ->Name(#BM "/BtreeTimer" suffix)->Args({n, k})->Unit(benchmark::kNanosecond)

    // Insert
    REGISTER_TIMER_BENCH_ARGS2(BM_InsertSameMs, "", kN100, kN1000);
    REGISTER_TIMER_BENCH_ARGS2(BM_InsertSpreadMs, "", kN100, kN1000);
    REGISTER_TIMER_BENCH_ARGS2(BM_InsertSpreadFarMs, "", kN100, kN1000);
    REGISTER_TIMER_BENCH_CLUSTERED(BM_InsertClusteredMs, "/100x10", kN100, 10);
    REGISTER_TIMER_BENCH_CLUSTERED(BM_InsertClusteredMs, "/1000x100", kN1000, 100);
    // Wheel ~125ms/iter at 10k*1000 buckets; Btree ~84ms/iter.
    // REGISTER_TIMER_BENCH_CLUSTERED(BM_InsertClusteredMs, "/10000x1000", kN10000, 1000);

    // Cancel
    REGISTER_TIMER_BENCH_ARGS2(BM_CancelSameMs, "", kN100, kN1000);
    REGISTER_TIMER_BENCH_ARGS2(BM_CancelSpreadMs, "", kN100, kN1000);
    REGISTER_TIMER_BENCH_ARGS2(BM_CancelSpreadLifo, "", kN100, kN1000);
    REGISTER_TIMER_BENCH_ARGS2(BM_CancelSpreadHalf, "", kN100, kN1000);

    // Trigger
    REGISTER_TIMER_BENCH_ARGS2(BM_TriggerBatchSameMs, "", kN100, kN1000);
    REGISTER_TIMER_BENCH_ARGS2(BM_TriggerSpreadMs, "", kN100, kN1000);
    // Wheel ~340ms/iter at N=100 (1ms tick advance); Btree ~17ms/iter.
    BENCHMARK_TEMPLATE(BM_TriggerSpreadFarMs, xio::BtreeTimer)
            ->Name("BM_TriggerSpreadFarMs/BtreeTimer")->Arg(kN100)->Arg(kN1000)
            ->Unit(benchmark::kNanosecond);
    // BENCHMARK_TEMPLATE(BM_TriggerSpreadFarMs, xio::WheelTimer)
    //         ->Name("BM_TriggerSpreadFarMs/WheelTimer")->Arg(kN100)
    //         ->Unit(benchmark::kNanosecond);
    REGISTER_TIMER_BENCH(BM_TriggerEmpty, "");

    // Poll: N only affects one-time setup; Arg(100) is representative.
    BENCHMARK_TEMPLATE(BM_NextTimeoutPoll, xio::WheelTimer)
            ->Name("BM_NextTimeoutPoll/WheelTimer")->Arg(kN100)->Unit(benchmark::kNanosecond);
    BENCHMARK_TEMPLATE(BM_NextTimeoutPoll, xio::BtreeTimer)
            ->Name("BM_NextTimeoutPoll/BtreeTimer")->Arg(kN100)->Unit(benchmark::kNanosecond);
    BENCHMARK_TEMPLATE(BM_NextTimeoutAfterInsert, xio::WheelTimer)
            ->Name("BM_NextTimeoutAfterInsert/WheelTimer")->Arg(kN100)->Unit(benchmark::kNanosecond);
    BENCHMARK_TEMPLATE(BM_NextTimeoutAfterInsert, xio::BtreeTimer)
            ->Name("BM_NextTimeoutAfterInsert/BtreeTimer")->Arg(kN100)->Unit(benchmark::kNanosecond);

    REGISTER_TIMER_BENCH_ARGS2(BM_DestroySpread, "", kN100, kN1000);
    // Btree ~260ms/iter at N=10000.
    // REGISTER_TIMER_BENCH_ARGS2(BM_DestroySpread, "", kN10000, kN10000);

    REGISTER_TIMER_BENCH(BM_ScheduleCancelCycle, "");
    REGISTER_TIMER_BENCH_ARGS2(BM_InsertTriggerSameMs, "", kN100, kN1000);

#undef REGISTER_TIMER_BENCH
#undef REGISTER_TIMER_BENCH_ARGS2
#undef REGISTER_TIMER_BENCH_CLUSTERED
} // namespace

BENCHMARK_MAIN();
