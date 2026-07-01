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
#include <gtest/gtest.h>
#include <sys/eventfd.h>
#include <unistd.h>
#include <fcntl.h>
#include <thread>
#include <turbo/times/time.h>
#include <xio/event/event_data.h>

namespace xio {
    class EpollPollerTest : public ::testing::Test {
    protected:
        void SetUp() override {
            poller_ = std::make_unique<EpollPoller>();
            PollerConfig pc{};
            ASSERT_TRUE(poller_->initialize(pc).ok());
        }

        void TearDown() override {
            poller_.reset();
        }

        // Helper: create a non-blocking eventfd for testing.
        int create_test_fd() {
            int fd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
            EXPECT_GE(fd, 0);
            return fd;
        }

        // Helper: initialize EventData with given fd.
        void setup_event_data(EventData *data, int fd) {
            data->handle = fd;
            data->active_event = 0;
            data->triggered = EventType::kEventNone;
        }

        std::unique_ptr<EpollPoller> poller_;
    };

    // Test: initialization and re-initialization.
    TEST_F(EpollPollerTest, InitializeAndDestroy) {
        PollerConfig pc{};
        EXPECT_TRUE(poller_->initialize(pc).ok()); // Already initialized, should be ok.
    }

    // Test: enable read event and poll for readability.
    TEST_F(EpollPollerTest, EnableReadAndPoll) {
        int fd = create_test_fd();
        EventData data;
        setup_event_data(&data, fd);

        turbo::Time cur = turbo::Time::current_time();
        ASSERT_TRUE(poller_->enable_event(&data, EventType::kEventRead, cur).ok());

        // Write to eventfd to make it readable.
        uint64_t val = 1;
        ssize_t n = write(fd, &val, sizeof(val));
        ASSERT_EQ(n, sizeof(val));

        fermat::Vector<EventData *> ready;
        auto res = poller_->poll(turbo::Duration::milliseconds(100), ready);
        ASSERT_TRUE(res.ok());
        ASSERT_EQ(ready.size(), 1u);
        EXPECT_EQ(ready[0], &data);
        EXPECT_TRUE(static_cast<int32_t>(ready[0]->triggered) & static_cast<int32_t>(EventType::kEventRead));

        close(fd);
    }

    // Test: enable write event and poll for writability.
    TEST_F(EpollPollerTest, EnableWriteAndPoll) {
        int pipefd[2];
        ASSERT_EQ(pipe2(pipefd, O_NONBLOCK), 0);
        EventData data;
        setup_event_data(&data, pipefd[1]); // write end

        turbo::Time cur = turbo::Time::current_time();
        ASSERT_TRUE(poller_->enable_event(&data, EventType::kEventWrite, cur).ok());

        fermat::Vector<EventData *> ready;
        auto res = poller_->poll(turbo::Duration::milliseconds(100), ready);
        ASSERT_TRUE(res.ok());
        ASSERT_EQ(ready.size(), 1u);
        EXPECT_EQ(ready[0], &data);
        EXPECT_TRUE(static_cast<int32_t>(ready[0]->triggered) & static_cast<int32_t>(EventType::kEventWrite));

        close(pipefd[0]);
        close(pipefd[1]);
    }

    // Test: disable event.
    TEST_F(EpollPollerTest, DisableEvent) {
        int fd = create_test_fd();
        EventData data;
        setup_event_data(&data, fd);

        turbo::Time cur = turbo::Time::current_time();
        // Enable read.
        ASSERT_TRUE(poller_->enable_event(&data, EventType::kEventRead, cur).ok());
        // Disable read.
        ASSERT_TRUE(poller_->disable_event(&data, EventType::kEventRead, cur).ok());

        // Write to eventfd but poll should not return because read is disabled.
        uint64_t val = 1;
        ssize_t unused_write = write(fd, &val, sizeof(val));
        (void)unused_write;

        fermat::Vector<EventData *> ready;
        auto res = poller_->poll(turbo::Duration::milliseconds(50), ready);
        ASSERT_TRUE(res.ok());
        EXPECT_TRUE(ready.empty());

        close(fd);
    }

    TEST_F(EpollPollerTest, HandleEvent) {
        int pipefd[2];
        ASSERT_EQ(pipe2(pipefd, O_NONBLOCK), 0);
        EventData data;
        setup_event_data(&data, pipefd[0]);  // 读端

        turbo::Time cur = turbo::Time::current_time();
        // Enable read on pipe read end.
        ASSERT_TRUE(poller_->handle_event(&data, EventType::kEventRead, EventType::kEventNone, cur).ok());

        // Write to pipe to make it readable.
        char buf[] = "x";
        ASSERT_EQ(write(pipefd[1], buf, 1), 1);

        fermat::Vector<EventData*> ready;
        auto res = poller_->poll(turbo::Duration::milliseconds(100), ready);
        ASSERT_TRUE(res.ok());
        ASSERT_EQ(ready.size(), 1u);
        EXPECT_EQ(ready[0], &data);

        // Consume the data to clear readable state.
        char read_buf[1];
        ASSERT_EQ(read(pipefd[0], read_buf, 1), 1);

        // Now disable read and enable write on the *read end*? Actually, we want to test disabling read.
        // Instead of enabling write (which doesn't make sense on read end), just disable read.
        ASSERT_TRUE(poller_->handle_event(&data, EventType::kEventNone, EventType::kEventRead, cur).ok());

        // Write again; should NOT trigger read because read is disabled.
        ASSERT_EQ(write(pipefd[1], buf, 1), 1);
        ready.clear();
        res = poller_->poll(turbo::Duration::milliseconds(50), ready);
        ASSERT_TRUE(res.ok());
        EXPECT_TRUE(ready.empty());

        close(pipefd[0]);
        close(pipefd[1]);
    }

    // Test: remove event.
    TEST_F(EpollPollerTest, RemoveEvent) {
        int fd = create_test_fd();
        EventData data;
        setup_event_data(&data, fd);

        turbo::Time cur = turbo::Time::current_time();
        ASSERT_TRUE(poller_->enable_event(&data, EventType::kEventRead, cur).ok());
        ASSERT_TRUE(poller_->remove_event(&data, cur).ok());

        // After removal, write should not be detected.
        uint64_t val = 1;
        ssize_t unused_write = write(fd, &val, sizeof(val));
        (void)unused_write;

        fermat::Vector<EventData *> ready;
        auto res = poller_->poll(turbo::Duration::milliseconds(50), ready);
        ASSERT_TRUE(res.ok());
        EXPECT_TRUE(ready.empty());

        close(fd);
    }

    // Test: poll timeout.
    TEST_F(EpollPollerTest, PollTimeout) {
        int fd = create_test_fd();
        EventData data;
        setup_event_data(&data, fd);
        turbo::Time cur = turbo::Time::current_time();
        ASSERT_TRUE(poller_->enable_event(&data, EventType::kEventRead, cur).ok());

        // No write, poll should timeout.
        fermat::Vector<EventData *> ready;
        auto res = poller_->poll(turbo::Duration::milliseconds(50), ready);
        ASSERT_TRUE(res.ok());
        EXPECT_TRUE(ready.empty());

        close(fd);
    }

    // Test: poll with infinite wait (should not return unless event occurs).
    // We use a short finite wait for practical testing, but we can check that negative duration works.
    TEST_F(EpollPollerTest, PollInfiniteTimeout) {
        int fd = create_test_fd();
        EventData data;
        setup_event_data(&data, fd);
        turbo::Time cur = turbo::Time::current_time();
        ASSERT_TRUE(poller_->enable_event(&data, EventType::kEventRead, cur).ok());

        // Write after a short delay to simulate event.
        std::thread writer([fd]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            uint64_t val = 1;
            ssize_t unused_write = write(fd, &val, sizeof(val));
            (void)unused_write;
        });

        fermat::Vector<EventData *> ready;
        // Negative duration means infinite wait (should block until event).
        auto res = poller_->poll(turbo::Duration::milliseconds(-1), ready);
        writer.join();
        ASSERT_TRUE(res.ok());
        ASSERT_EQ(ready.size(), 1u);
        EXPECT_EQ(ready[0], &data);

        close(fd);
    }

    // Test: supports_disk_async returns false for epoll.
    TEST_F(EpollPollerTest, SupportsDiskAsync) {
        EXPECT_FALSE(poller_->supports_disk_async());
    }

    // Test: name returns "epoll".
    TEST_F(EpollPollerTest, Name) {
        EXPECT_EQ(poller_->name(), "epoll");
    }
} // namespace xio

#endif  // defined(OS_LINUX)
