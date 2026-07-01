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

#include <xio/event/event_loop.h>
#include <gtest/gtest.h>
#include <atomic>
#include <chrono>
#include <thread>
#include <future>
#include <cstring>

#if defined(OS_WIN)
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#endif

namespace xio {

/// Cross‑platform helper: create a connected, non‑blocking socket pair.
/// Returns two connected sockets (fd1, fd2). On failure, both are kInvalidFileHandle.
static std::pair<FileHandle, FileHandle> create_socketpair() {
#if defined(OS_WIN)
    // Windows: simulate socketpair with local TCP loopback.
    SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_sock == INVALID_SOCKET) return {kInvalidFileHandle, kInvalidFileHandle};

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = 0;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    if (bind(listen_sock, (struct sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        closesocket(listen_sock);
        return {kInvalidFileHandle, kInvalidFileHandle};
    }
    if (listen(listen_sock, 1) == SOCKET_ERROR) {
        closesocket(listen_sock);
        return {kInvalidFileHandle, kInvalidFileHandle};
    }
    int addr_len = sizeof(addr);
    if (getsockname(listen_sock, (struct sockaddr*)&addr, &addr_len) == SOCKET_ERROR) {
        closesocket(listen_sock);
        return {kInvalidFileHandle, kInvalidFileHandle};
    }
    SOCKET client = socket(AF_INET, SOCK_STREAM, 0);
    if (client == INVALID_SOCKET) {
        closesocket(listen_sock);
        return {kInvalidFileHandle, kInvalidFileHandle};
    }
    // Set non‑blocking before connect
    u_long mode = 1;
    ioctlsocket(client, FIONBIO, &mode);
    connect(client, (struct sockaddr*)&addr, sizeof(addr));
    // Wait for accept (simple select)
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(listen_sock, &fds);
    struct timeval tv = {1, 0};
    if (select(0, &fds, NULL, NULL, &tv) <= 0) {
        closesocket(client);
        closesocket(listen_sock);
        return {kInvalidFileHandle, kInvalidFileHandle};
    }
    SOCKET server = accept(listen_sock, NULL, NULL);
    if (server == INVALID_SOCKET) {
        closesocket(client);
        closesocket(listen_sock);
        return {kInvalidFileHandle, kInvalidFileHandle};
    }
    ioctlsocket(server, FIONBIO, &mode);
    // Client may still be in connecting state; set it blocking temporarily to complete.
    mode = 0;
    ioctlsocket(client, FIONBIO, &mode);
    closesocket(listen_sock);
    return {server, client};
#else
    int fds[2];
    if (socketpair(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0, fds) != 0) {
        return {kInvalidFileHandle, kInvalidFileHandle};
    }
    return {fds[0], fds[1]};
#endif
}

/// Helper: write one byte to a socket to trigger read event.
static void trigger_read(FileHandle fd) {
    char c = 'x';
#if defined(OS_WIN)
    send(static_cast<SOCKET>(fd), &c, 1, 0);
#else
    ::write(fd, &c, 1);
#endif
}

/// Global read callback: increments a counter.
static turbo::Status read_counter_cb(FileHandle fd, EventData *data, turbo::Time cur) {
    char buf[64];
#if defined(OS_WIN)
    if (recv(static_cast<SOCKET>(fd), buf, sizeof(buf), 0) > 0) {
        ++(*static_cast<std::atomic<int>*>(data->data));
    }
#else
    if (::read(fd, buf, sizeof(buf)) > 0) {
        ++(*static_cast<std::atomic<int>*>(data->data));
    }
#endif
    return turbo::OkStatus();
}

/// Global write callback: increments a counter.
static turbo::Status write_counter_cb(FileHandle fd, EventData *data, turbo::Time cur) {
    ++(*static_cast<std::atomic<int>*>(data->data));
    return turbo::OkStatus();
}

/// Global read timeout callback: increments a counter.
static turbo::Status read_timeout_cb(FileHandle fd, EventData *data, turbo::Time expire, turbo::Time cur) {
    ++(*static_cast<std::atomic<int>*>(data->data));
    return turbo::OkStatus();
}

/// Global write timeout callback: increments a counter.
static turbo::Status write_timeout_cb(FileHandle fd, EventData *data, turbo::Time expire, turbo::Time cur) {
    ++(*static_cast<std::atomic<int>*>(data->data));
    return turbo::OkStatus();
}

/// Global error callback: increments a counter.
static turbo::Status error_counter_cb(FileHandle fd, EventData *data, turbo::Time cur) {
    ++(*static_cast<std::atomic<int>*>(data->data));
    return turbo::OkStatus();
}

/// Task that sets an atomic flag to true.
static void set_true_task(void *arg) {
    *static_cast<std::atomic<bool>*>(arg) = true;
}

static void inc_counter_task(void *arg) {
    ++(*static_cast<std::atomic<int>*>(arg));
}

static void loop_task_counter(void *arg) {
    ++(*static_cast<std::atomic<int>*>(arg));
}

static void exit_task_set_true(void *arg) {
    *static_cast<std::atomic<bool>*>(arg) = true;
}

static void push_one(void *arg) { static_cast<std::vector<int>*>(arg)->push_back(1); }
static void push_two(void *arg) { static_cast<std::vector<int>*>(arg)->push_back(2); }
static void push_three(void *arg) { static_cast<std::vector<int>*>(arg)->push_back(3); }

/// Context for adding exit tasks.
struct AddExitTasksCtx {
    EventLoop *loop;
    std::vector<int> *order;
    std::promise<void> promise;
};

static void add_exit_tasks(void *arg) {
    auto *ctx = static_cast<AddExitTasksCtx*>(arg);
    ctx->loop->post_exit_task(push_one, ctx->order);
    ctx->loop->post_exit_task(push_two, ctx->order);
    ctx->loop->post_exit_task(push_three, ctx->order);
    ctx->promise.set_value();
    delete ctx;
}

/// Context for PostTaskInLoop test.
struct PostInLoopCtx {
    EventLoop *loop;
    std::atomic<bool> *flag;
};

static void outer_task(void *arg) {
    auto *ctx = static_cast<PostInLoopCtx*>(arg);
    ctx->loop->post_task_in_loop(set_true_task, ctx->flag);
}

////////////////////////////////////////////////////////////////////////////////
/// Group: EventLoop lifecycle
////////////////////////////////////////////////////////////////////////////////

/// Test that start() and stop() work correctly; status() and ok() reflect state.
TEST(EventLoopTest, StartStop) {
    EventLoop loop;
    EventLoopOption opt;
    EXPECT_FALSE(loop.ok());
    auto st = loop.start(opt);
    ASSERT_TRUE(st.ok());
    EXPECT_TRUE(loop.ok());
    // Main thread is not the loop thread.
    EXPECT_FALSE(loop.is_in_loop_thread());
    loop.stop();
    EXPECT_FALSE(loop.ok());
}

/// Test that loop_thread_id() returns a non-zero value after start.
TEST(EventLoopTest, LoopThreadId) {
    EventLoop loop;
    EventLoopOption opt;
    loop.start(opt);
    EXPECT_NE(loop.loop_thread_id(), EventLoop::kInvalidLoopThreadId);
    loop.stop();
}

////////////////////////////////////////////////////////////////////////////////
/// Group: Post task (cross-thread and in-loop)
////////////////////////////////////////////////////////////////////////////////

/// Test posting a task from a different thread; the task should be executed on the loop thread.
TEST(EventLoopTest, PostTaskCrossThread) {
    EventLoop loop;
    EventLoopOption opt;
    loop.start(opt);

    std::atomic<bool> executed{false};
    loop.post_task(set_true_task, &executed);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    EXPECT_TRUE(executed.load());
    loop.stop();
}

/// Test batch posting of multiple tasks; all should be executed.
TEST(EventLoopTest, PostTaskBatch) {
    EventLoop loop;
    EventLoopOption opt;
    loop.start(opt);

    const int N = 100;
    std::atomic<int> counter{0};
    std::vector<AsyncTask> tasks;
    for (int i = 0; i < N; ++i) {
        tasks.push_back(AsyncTask{&counter, inc_counter_task, empty_data_releaser});
    }
    loop.post_task(turbo::span<AsyncTask>(tasks));
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    EXPECT_EQ(counter.load(), N);
    loop.stop();
}

/// Test post_task_in_loop from within a task posted to the loop thread.
TEST(EventLoopTest, PostTaskInLoop) {
    EventLoop loop;
    EventLoopOption opt;
    loop.start(opt);

    std::atomic<bool> flag{false};
    PostInLoopCtx ctx{&loop, &flag};
    loop.post_task(outer_task, &ctx);
    for (int i = 0; i < 100 && !flag.load(); ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    EXPECT_TRUE(flag.load());
    loop.stop();
}

////////////////////////////////////////////////////////////////////////////////
/// Group: Timers
////////////////////////////////////////////////////////////////////////////////

/// Test run_after: a timer fires after a specified duration.
TEST(EventLoopTest, RunAfter) {
    EventLoop loop;
    EventLoopOption opt;
    loop.start(opt);

    std::atomic<bool> fired{false};
    auto cb = [](void *arg, turbo::Time, turbo::Time) {
        *static_cast<std::atomic<bool>*>(arg) = true;
    };
    loop.run_after(turbo::Duration::milliseconds(50), cb, &fired);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_TRUE(fired.load());
    loop.stop();
}

/// Test run_at: a timer fires at an absolute time.
TEST(EventLoopTest, RunAt) {
    EventLoop loop;
    EventLoopOption opt;
    loop.start(opt);

    std::atomic<bool> fired{false};
    auto cb = [](void *arg, turbo::Time, turbo::Time) {
        *static_cast<std::atomic<bool>*>(arg) = true;
    };
    auto expire = turbo::Time::current_time() + turbo::Duration::milliseconds(50);
    loop.run_at(expire, cb, &fired);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_TRUE(fired.load());
    loop.stop();
}

/// Test multiple timers with different delays; all should fire.
TEST(EventLoopTest, MultipleTimers) {
    EventLoop loop;
    EventLoopOption opt;
    loop.start(opt);

    std::atomic<int> count{0};
    auto cb = [](void *arg, turbo::Time, turbo::Time) {
        ++(*static_cast<std::atomic<int>*>(arg));
    };
    loop.run_after(turbo::Duration::milliseconds(20), cb, &count);
    loop.run_after(turbo::Duration::milliseconds(40), cb, &count);
    loop.run_after(turbo::Duration::milliseconds(60), cb, &count);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_EQ(count.load(), 3);
    loop.stop();
}

////////////////////////////////////////////////////////////////////////////////
/// Group: I/O events using socketpair (read/write with separate timeouts)
////////////////////////////////////////////////////////////////////////////////

/// Test enabling read event; data written to the socket should trigger read callback.
TEST(EventLoopTest, EnableReadEvent) {
    EventLoop loop;
    EventLoopOption opt;
    loop.start(opt);

    auto sock = create_socketpair();
    ASSERT_NE(sock.first, kInvalidFileHandle);
    ASSERT_NE(sock.second, kInvalidFileHandle);
    std::atomic<int> read_counter{0};
    EventData data;
    data.handle = sock.first;
    data.data = &read_counter;
    data.read_callback = read_counter_cb;

    auto st = loop.enable_events(&data, EventType::kEventRead);
    ASSERT_TRUE(st.ok());

    trigger_read(sock.second);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    EXPECT_EQ(read_counter.load(), 1);

    loop.remove_events(&data);
#if defined(OS_WIN)
    closesocket(sock.first);
    closesocket(sock.second);
#else
    close(sock.first);
    close(sock.second);
#endif
    loop.stop();
}

/// Test disabling read event; after disable, further writes should not trigger read callback.
TEST(EventLoopTest, DisableReadEvent) {
    EventLoop loop;
    EventLoopOption opt;
    loop.start(opt);

    auto sock = create_socketpair();
    ASSERT_NE(sock.first, kInvalidFileHandle);
    ASSERT_NE(sock.second, kInvalidFileHandle);
    std::atomic<int> read_counter{0};
    EventData data;
    data.handle = sock.first;
    data.data = &read_counter;
    data.read_callback = read_counter_cb;

    loop.enable_events(&data, EventType::kEventRead);
    trigger_read(sock.second);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    EXPECT_EQ(read_counter.load(), 1);

    loop.disable_events(&data, EventType::kEventRead);
    trigger_read(sock.second);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    // read count should remain 1 because read is disabled.
    EXPECT_EQ(read_counter.load(), 1);

    loop.remove_events(&data);
#if defined(OS_WIN)
    closesocket(sock.first);
    closesocket(sock.second);
#else
    close(sock.first);
    close(sock.second);
#endif
    loop.stop();
}

/// Test enabling write event; the write callback should fire (socket is writable).
TEST(EventLoopTest, WriteEvent) {
    EventLoop loop;
    EventLoopOption opt;
    loop.start(opt);

    auto sock = create_socketpair();
    ASSERT_NE(sock.second, kInvalidFileHandle);
    std::atomic<int> write_counter{0};
    EventData data;
    data.handle = sock.second;   // write end
    data.data = &write_counter;
    data.write_callback = write_counter_cb;

    auto st = loop.enable_events(&data, EventType::kEventWrite);
    ASSERT_TRUE(st.ok());

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    EXPECT_GE(write_counter.load(), 1);

    loop.remove_events(&data);
#if defined(OS_WIN)
    closesocket(sock.first);
    closesocket(sock.second);
#else
    close(sock.first);
    close(sock.second);
#endif
    loop.stop();
}


/// Test error callback: when one end of a socket pair is closed, the other end receives an error event.
TEST(EventLoopTest, ErrorCallback) {
    EventLoop loop;
    EventLoopOption opt;
    loop.start(opt);

    auto sock = create_socketpair();
    ASSERT_NE(sock.first, kInvalidFileHandle);
    ASSERT_NE(sock.second, kInvalidFileHandle);
    // Close one end to trigger error on the other.
#if defined(OS_WIN)
    closesocket(sock.first);
#else
    close(sock.first);
#endif
    std::atomic<int> error_counter{0};
    EventData data;
    data.handle = sock.second;
    data.data = &error_counter;
    data.error_callback = error_counter_cb;
    data.read_callback = read_counter_cb;

    auto st = loop.enable_events(&data, EventType::kEventRead);
    ASSERT_TRUE(st.ok());

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    EXPECT_GE(error_counter.load(), 1);

    loop.remove_events(&data);
#if defined(OS_WIN)
    closesocket(sock.second);
#else
    close(sock.second);
#endif
    loop.stop();
}

////////////////////////////////////////////////////////////////////////////////
/// Group: Cross-thread handle_events
////////////////////////////////////////////////////////////////////////////////

/// Test calling handle_events from a different thread; it should block until the operation completes.
TEST(EventLoopTest, HandleEventsCrossThread) {
    EventLoop loop;
    EventLoopOption opt;
    loop.start(opt);

    auto sock = create_socketpair();
    ASSERT_NE(sock.first, kInvalidFileHandle);
    std::atomic<int> read_counter{0};
    EventData data;
    data.handle = sock.first;
    data.data = &read_counter;
    data.read_callback = read_counter_cb;

    // Call from test thread (cross‑thread) – should block until done.
    auto st = loop.handle_events(&data, EventType::kEventRead, EventType::kEventNone);
    ASSERT_TRUE(st.ok());

    trigger_read(sock.second);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    EXPECT_EQ(read_counter.load(), 1);

    loop.remove_events(&data);
#if defined(OS_WIN)
    closesocket(sock.first);
    closesocket(sock.second);
#else
    close(sock.first);
    close(sock.second);
#endif
    loop.stop();
}

////////////////////////////////////////////////////////////////////////////////
/// Group: Loop tasks and exit tasks
////////////////////////////////////////////////////////////////////////////////

/// Test post_loop_task: a task posted this way runs repeatedly before every loop iteration.
TEST(EventLoopTest, PostLoopTask) {
    EventLoop loop;
    EventLoopOption opt;
    loop.start(opt);

    std::atomic<int> count{0};
    loop.post_loop_task(loop_task_counter, &count);
    // Let the loop run for a while; count should increase.
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    EXPECT_GT(count.load(), 0);
    loop.stop();
}

/// Test cancel_loop_task: a previously posted loop task can be cancelled and will no longer run.
TEST(EventLoopTest, CancelLoopTask) {
    EventLoop loop;
    EventLoopOption opt;
    loop.start(opt);

    std::atomic<int> count{0};
    loop.post_loop_task(loop_task_counter, &count);
    loop.cancel_loop_task(loop_task_counter, &count);
    int before = count.load();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_EQ(count.load(), before);
    loop.stop();
}

/// Test post_exit_task: a task posted this way runs once when the loop exits.
TEST(EventLoopTest, PostExitTask) {
    EventLoop loop;
    EventLoopOption opt;
    loop.start(opt);

    std::atomic<bool> exit_fired{false};
    loop.post_exit_task(exit_task_set_true, &exit_fired);
    loop.stop();
    EXPECT_TRUE(exit_fired.load());
}

/// Test cancel_exit_task: a posted exit task can be cancelled and will not run.
TEST(EventLoopTest, CancelExitTask) {
    EventLoop loop;
    EventLoopOption opt;
    loop.start(opt);

    std::atomic<bool> exit_fired{false};
    loop.post_exit_task(exit_task_set_true, &exit_fired);
    loop.cancel_exit_task(exit_task_set_true, &exit_fired);
    loop.stop();
    EXPECT_FALSE(exit_fired.load());
}

/// Test exit task order: exit tasks are executed in LIFO order (reverse of posting order).
TEST(EventLoopTest, ExitTaskOrder) {
    EventLoop loop;
    EventLoopOption opt;
    loop.start(opt);

    std::vector<int> order;
    auto *ctx = new AddExitTasksCtx{&loop, &order, {}};
    auto future = ctx->promise.get_future();
    loop.post_task(add_exit_tasks, ctx);
    future.wait();

    loop.stop();
    // Expected order: 3, 2, 1 because tasks are added as 1,2,3 but executed in reverse.
    std::vector<int> expected = {3, 2, 1};
    EXPECT_EQ(order, expected);
}

////////////////////////////////////////////////////////////////////////////////
/// Group: Wakeup and backend info
////////////////////////////////////////////////////////////////////////////////

/// Test wakeup(): calling it from outside the loop should not crash.
TEST(EventLoopTest, Wakeup) {
    EventLoop loop;
    EventLoopOption opt;
    loop.start(opt);
    loop.wakeup();
    loop.stop();
}

/// Test backend name: should return a string indicating the underlying poller.
TEST(EventLoopTest, BackendName) {
    EventLoop loop;
    EventLoopOption opt;
    loop.start(opt);
    auto name = loop.backend();
    EXPECT_TRUE(name == "epoll" || name == "kqueue" || name == "iocp" || name == "io_uring");
    loop.stop();
}

/// Test supports_disk_async: returns true if the poller supports asynchronous disk I/O.
TEST(EventLoopTest, SupportsDiskAsync) {
    EventLoop loop;
    EventLoopOption opt;
    opt.disk_io = true;
    auto st = loop.start(opt);
    if (st.ok()) {
        (void)loop.supports_disk_async();
    }
    loop.stop();
}

} // namespace xio