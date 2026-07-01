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

#include <atomic>
#include <chrono>
#include <cstdlib>
#include <thread>
#include <vector>

#include <gtest/gtest.h>
#include <xio/timer/btree_timer.h>
#include <turbo/times/time.h>

namespace {

    turbo::Duration ms(int64_t value) { return turbo::Duration::milliseconds(value); }

    int64_t time_diff_ms(turbo::Time a, turbo::Time b) {
        return turbo::Duration::to_milliseconds(a - b);
    }

    // Drive the btree timer with explicit timestamps (no EventLoop).
    void drive_wheel(xio::BtreeTimer &timer, turbo::Time until) {
        for (int guard = 0; guard < 100000; ++guard) {
            const turbo::Time next = timer.next_timeout();
            if (next > until) {
                return;
            }
            timer.on_trigger(next);
        }
        FAIL() << "drive_wheel: too many iterations";
    }

    struct FireRecord {
        std::vector<turbo::Time> fired_at;

        static void on_fire(void *param, turbo::Time /*expire*/, turbo::Time /*cur*/) {
            auto *self = static_cast<FireRecord *>(param);
            self->fired_at.push_back(turbo::Time::current_time());
        }
    };

    struct ScheduleInCallbackCtx {
        xio::BtreeTimer *timer{nullptr};
        turbo::Time t0;
        FireRecord parent;
        FireRecord child;
    };

    void schedule_child_from_parent(void *param, turbo::Time expire, turbo::Time cur) {
        auto *ctx = static_cast<ScheduleInCallbackCtx *>(param);
        FireRecord::on_fire(&ctx->parent, expire, cur);
        // schedule_task uses wall-clock now; expire at current time => same batch as parent.
        ctx->timer->run_at(turbo::Time::current_time(), FireRecord::on_fire, &ctx->child);
    }

    struct SelfCancelCtx {
        xio::BtreeTimer *timer{nullptr};
        uint64_t timer_id{xio::BtreeTimer::kInvalidTimerId};
        std::atomic<int> fires{0};
    };

    void self_cancel_cb(void *param, turbo::Time /*expire*/, turbo::Time /*cur*/) {
        auto *ctx = static_cast<SelfCancelCtx *>(param);
        ++ctx->fires;
        ctx->timer->cancel(ctx->timer_id);
    }

}  // namespace

TEST(BtreeTimerTest, FireOnce) {
    xio::BtreeTimer timer;
    const turbo::Time t0 = turbo::Time::current_time();

    FireRecord r1;
    FireRecord r2;
    FireRecord r3;

    ASSERT_NE(xio::BtreeTimer::kInvalidTimerId,
              timer.run_at(t0 + ms(20), FireRecord::on_fire, &r1));
    ASSERT_NE(xio::BtreeTimer::kInvalidTimerId,
              timer.run_at(t0 + ms(20), FireRecord::on_fire, &r2));
    ASSERT_NE(xio::BtreeTimer::kInvalidTimerId,
              timer.run_at(t0 + ms(40), FireRecord::on_fire, &r3));

    drive_wheel(timer, t0 + ms(19));
    EXPECT_EQ(r1.fired_at.size(), 0u);
    EXPECT_EQ(r2.fired_at.size(), 0u);
    EXPECT_EQ(r3.fired_at.size(), 0u);

    drive_wheel(timer, t0 + ms(25));
    EXPECT_EQ(r1.fired_at.size(), 1u);
    EXPECT_EQ(r2.fired_at.size(), 1u);
    EXPECT_EQ(r3.fired_at.size(), 0u);

    drive_wheel(timer, t0 + ms(45));
    EXPECT_EQ(r3.fired_at.size(), 1u);
}

TEST(BtreeTimerTest, SlowFast) {
    xio::BtreeTimer timer;
    const turbo::Time t0 = turbo::Time::current_time();

    FireRecord slow;
    FireRecord fast;

    timer.run_at(t0 + ms(10), FireRecord::on_fire, &slow);
    timer.run_at(t0 + ms(5), FireRecord::on_fire, &fast);

    drive_wheel(timer, t0 + ms(5));
    ASSERT_EQ(fast.fired_at.size(), 1u);
    ASSERT_EQ(slow.fired_at.size(), 0u);

    drive_wheel(timer, t0 + ms(10));
    ASSERT_EQ(slow.fired_at.size(), 1u);
}

TEST(BtreeTimerTest, CancelBeforeFire) {
    xio::BtreeTimer timer;
    const turbo::Time t0 = turbo::Time::current_time();

    FireRecord record;
    const uint64_t id = timer.run_at(t0 + ms(10), FireRecord::on_fire, &record);
    ASSERT_NE(xio::BtreeTimer::kInvalidTimerId, id);

    timer.cancel(id);
    drive_wheel(timer, t0 + ms(20));
    EXPECT_TRUE(record.fired_at.empty());
}

TEST(BtreeTimerTest, CancelInvalidId) {
    xio::BtreeTimer timer;
    timer.cancel(xio::BtreeTimer::kInvalidTimerId);
    timer.cancel(0xdeadbeefULL);
}

TEST(BtreeTimerTest, PastExpireFiresImmediately) {
    xio::BtreeTimer timer;
    const turbo::Time t0 = turbo::Time::current_time();

    FireRecord record;
    timer.run_at(t0 - ms(1), FireRecord::on_fire, &record);

    drive_wheel(timer, t0);
    ASSERT_EQ(record.fired_at.size(), 1u);
}

TEST(BtreeTimerTest, ScheduleInCallback) {
    xio::BtreeTimer timer;
    ScheduleInCallbackCtx ctx;
    ctx.timer = &timer;
    ctx.t0 = turbo::Time::current_time();

    timer.run_at(ctx.t0 + ms(20), schedule_child_from_parent, &ctx);
    drive_wheel(timer, ctx.t0 + ms(20));

    EXPECT_EQ(ctx.parent.fired_at.size(), 1u);
    EXPECT_EQ(ctx.child.fired_at.size(), 1u);
}

TEST(BtreeTimerTest, SelfCancelInCallback) {
    xio::BtreeTimer timer;
    SelfCancelCtx ctx;
    ctx.timer = &timer;

    ctx.timer_id = timer.run_every(ms(5), self_cancel_cb, &ctx);
    ASSERT_NE(xio::BtreeTimer::kInvalidTimerId, ctx.timer_id);

    drive_wheel(timer, turbo::Time::current_time() + ms(50));
    EXPECT_EQ(ctx.fires.load(), 1);
}

TEST(BtreeTimerTest, RunEvery) {
    xio::BtreeTimer timer;
    const turbo::Time t0 = turbo::Time::current_time();

    FireRecord record;
    timer.run_every(t0 + ms(10), ms(10), FireRecord::on_fire, &record);

    drive_wheel(timer, t0 + ms(35));
    EXPECT_GE(record.fired_at.size(), 3u);
    EXPECT_LE(record.fired_at.size(), 4u);
}

TEST(BtreeTimerTest, NextTimeoutMovesEarlier) {
    xio::BtreeTimer timer;
    const turbo::Time t0 = turbo::Time::current_time();
    const turbo::Time before = timer.next_timeout();

    timer.run_at(t0 + ms(5), FireRecord::on_fire, nullptr);
    const turbo::Time after = timer.next_timeout();

    EXPECT_LE(time_diff_ms(after, t0 + ms(5)), 2);
    EXPECT_LT(after, before + ms(1000));
}

TEST(BtreeTimerTest, DestructorWithPending) {
    const turbo::Time t0 = turbo::Time::current_time();
    FireRecord record;
    {
        xio::BtreeTimer timer;
        timer.run_at(t0 + ms(100), FireRecord::on_fire, &record);
        timer.run_at(t0 + ms(200), FireRecord::on_fire, &record);
    }
    EXPECT_TRUE(record.fired_at.empty());
}

TEST(BtreeTimerTest, RealTimeSleep) {
    xio::BtreeTimer timer;
    const turbo::Time t0 = turbo::Time::current_time();

    FireRecord record;
    timer.run_after(ms(50), FireRecord::on_fire, &record);

    std::this_thread::sleep_for(std::chrono::milliseconds(60));
    drive_wheel(timer, turbo::Time::current_time());

    ASSERT_EQ(record.fired_at.size(), 1u);
    const int64_t delay = time_diff_ms(record.fired_at[0], t0);
    EXPECT_GE(delay, 45);
    EXPECT_LE(delay, 120);
}
