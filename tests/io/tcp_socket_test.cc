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

#include <xio/io/tcp_socket.h>
#include <xio/event/event_loop.h>
#include <turbo/base/fd_utility.h>
#include <gtest/gtest.h>
#include <atomic>
#include <chrono>
#include <thread>
#include <cstring>
#include <random>

#if defined(OS_WIN)
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

namespace xio {
    /// Helper: find an available TCP port by binding to port 0.
    static uint16_t get_available_port() {
        int sock = ::socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0) return 0;
        struct sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = 0;
        addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        if (::bind(sock, (struct sockaddr *) &addr, sizeof(addr)) != 0) {
            ::close(sock);
            return 0;
        }
        socklen_t len = sizeof(addr);
        if (::getsockname(sock, (struct sockaddr *) &addr, &len) != 0) {
            ::close(sock);
            return 0;
        }
        ::close(sock);
        return ntohs(addr.sin_port);
    }

    /// Helper: create a listening TCP socket on localhost.
    static FileHandle create_listener(uint16_t port) {
        FileHandle fd = ::socket(AF_INET, SOCK_STREAM, 0);
        if (fd == kInvalidFileHandle) return kInvalidFileHandle;
        int opt = 1;
#if defined(OS_WIN)
        if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (const char *) &opt, sizeof(opt)) != 0) {
            closesocket(fd);
            return kInvalidFileHandle;
        }
#else
        if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) != 0) {
            close(fd);
            return kInvalidFileHandle;
        }
#endif
        struct sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        if (::bind(fd, (struct sockaddr *) &addr, sizeof(addr)) != 0) {
#if defined(OS_WIN)
            closesocket(fd);
#else
            close(fd);
#endif
            return kInvalidFileHandle;
        }
        if (::listen(fd, 5) != 0) {
#if defined(OS_WIN)
            closesocket(fd);
#else
            close(fd);
#endif
            return kInvalidFileHandle;
        }
        return fd;
    }

    /// Helper: connect to a listening socket.
    static FileHandle connect_to(uint16_t port) {
        FileHandle fd = ::socket(AF_INET, SOCK_STREAM, 0);
        if (fd == kInvalidFileHandle) return kInvalidFileHandle;
        struct sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        if (::connect(fd, (struct sockaddr *) &addr, sizeof(addr)) != 0) {
#if defined(OS_WIN)
            closesocket(fd);
#else
            close(fd);
#endif
            return kInvalidFileHandle;
        }
        // Set non-blocking after connection.
        turbo::make_non_blocking(fd);
        return fd;
    }

    /// Helper: accept a connection from a listening socket.
    static FileHandle accept_connection(FileHandle listen_fd) {
        struct sockaddr_in addr;
        socklen_t len = sizeof(addr);
        FileHandle fd = ::accept(listen_fd, (struct sockaddr *) &addr, &len);
        if (fd == kInvalidFileHandle) return kInvalidFileHandle;
        turbo::make_non_blocking(fd);
        return fd;
    }

    /// Helper: write data to a socket (blocking, but used only for test control).
    static bool write_string(FileHandle fd, const std::string &data) {
        ssize_t n = send(fd, data.data(), data.size(), 0);
        return n == static_cast<ssize_t>(data.size());
    }

    /// Helper: read all available data from a socket (non-blocking, best effort).
    static std::string read_all(FileHandle fd) {
        char buf[4096];
        std::string result;
        while (true) {
            ssize_t n = recv(fd, buf, sizeof(buf), 0);
            if (n <= 0) break;
            result.append(buf, n);
        }
        return result;
    }

    ////////////////////////////////////////////////////////////////////////////////
    /// Test fixture: creates EventLoop and a real TCP connection between two sockets.
    /// The "server" side is used to inject data, the "client" side is attached to TcpSocket.
    ////////////////////////////////////////////////////////////////////////////////

    class TcpSocketTest : public ::testing::Test {
    protected:
        void SetUp() override {
            // Start EventLoop.
            EventLoopOption opt;
            auto st = _loop.start(opt);
            ASSERT_TRUE(st.ok());

            // Find an available port.
            _port = get_available_port();
            ASSERT_NE(_port, 0);

            // Create listener.
            _listen_fd = create_listener(_port);
            ASSERT_NE(_listen_fd, kInvalidFileHandle);

            // Connect client.
            _client_fd = connect_to(_port);
            ASSERT_NE(_client_fd, kInvalidFileHandle);

            // Accept server side.
            _server_fd = accept_connection(_listen_fd);
            ASSERT_NE(_server_fd, kInvalidFileHandle);
        }

        void TearDown() override {
            // Close all fds.
            if (_server_fd != kInvalidFileHandle) {
#if defined(OS_WIN)
                closesocket(_server_fd);
#else
                close(_server_fd);
#endif
            }
            if (_client_fd != kInvalidFileHandle) {
#if defined(OS_WIN)
                closesocket(_client_fd);
#else
                close(_client_fd);
#endif
            }
            if (_listen_fd != kInvalidFileHandle) {
#if defined(OS_WIN)
                closesocket(_listen_fd);
#else
                close(_listen_fd);
#endif
            }
            _loop.stop();
        }

        /// Attach TcpSocket to the client side.
        void attach_socket(TcpSocket &sock, void *ctx = nullptr) {
            sock.attach(&_loop, _client_fd, ctx);
            _client_fd = kInvalidFileHandle; // ownership transferred
        }

        EventLoop _loop;
        uint16_t _port{0};
        FileHandle _listen_fd{kInvalidFileHandle};
        FileHandle _server_fd{kInvalidFileHandle};
        FileHandle _client_fd{kInvalidFileHandle};
    };

    ////////////////////////////////////////////////////////////////////////////////
    /// Test: Basic attach and read.
    ////////////////////////////////////////////////////////////////////////////////

    TEST_F(TcpSocketTest, AttachAndRead) {
        TcpSocket sock;
        attach_socket(sock);

        std::atomic<bool> read_called{false};
        std::string received;
        sock.set_read_callback([&](TcpSocket *s, turbo::Time cur) {
            read_called = true;
            auto &buf = s->input_buffer();
            KLOG(INFO) << buf.size();
            received.append(buf.flatten());
            buf.clear();
        });

        std::string msg = "Hello, real TCP!";
        ASSERT_TRUE(write_string(_server_fd, msg));

        auto st = sock.start_read();
        ASSERT_TRUE(st.ok());


        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        EXPECT_TRUE(read_called.load());
        EXPECT_EQ(received, msg);
        sock.detach();
        _loop.sync();
    }

    ////////////////////////////////////////////////////////////////////////////////
    /// Test: Write data from TcpSocket.
    ////////////////////////////////////////////////////////////////////////////////

    TEST_F(TcpSocketTest, WriteAndComplete) {
        TcpSocket sock;
        attach_socket(sock);
        std::string msg = "Data from client";
        std::string msg_send = "Data from client";
        std::atomic<bool> write_complete_called{false};
        sock.set_write_callback([&](TcpSocket *s, turbo::Time) {
            s->output_buffer().append(msg_send).ignore_error();
            msg_send.clear();
        });
        sock.set_write_complete_callback([&](TcpSocket *, turbo::Time) {
            write_complete_called = true;
        });


        auto st = sock.start_write();
        ASSERT_TRUE(st.ok());

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        std::string received = read_all(_server_fd);
        EXPECT_EQ(received, msg);
        EXPECT_TRUE(write_complete_called.load());
        sock.detach();
        _loop.sync();
    }

    ////////////////////////////////////////////////////////////////////////////////
    /// Test: Read limiter.
    ////////////////////////////////////////////////////////////////////////////////

    TEST_F(TcpSocketTest, ReadLimiter) {
        TcpSocket sock;
        attach_socket(sock);

        std::atomic<size_t> read_quota{10};
        sock.set_read_limiter([&](TcpSocket *, turbo::Time) {
            return read_quota.load();
        });

        size_t feedback_total = 0;
        sock.set_read_feedback([&](TcpSocket *, turbo::Time, size_t n) {
            feedback_total = n;
        });

        sock.set_read_callback([&](TcpSocket *s, turbo::Time) {
            // Consume data to keep buffer empty.
            s->input_buffer().clear();
        });


        std::string msg(20, 'x');
        ASSERT_TRUE(write_string(_server_fd, msg));

        auto st = sock.start_read();
        ASSERT_TRUE(st.ok());


        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        EXPECT_EQ(feedback_total, 10u);
        sock.detach();
        _loop.sync();
    }

    ////////////////////////////////////////////////////////////////////////////////
    /// Test: Write limiter.
    ////////////////////////////////////////////////////////////////////////////////

    TEST_F(TcpSocketTest, WriteLimiter) {
        TcpSocket sock;
        attach_socket(sock);

        std::atomic<size_t> write_quota{5};
        sock.set_write_limiter([&](TcpSocket *, turbo::Time) {
            if (write_quota.load() == 0) {
                return 0ul;
            }
            return write_quota.fetch_sub(5);
        });
        std::string msg(20, 'y');
        size_t feedback_total = 0;
        sock.set_write_feedback([&](TcpSocket *, turbo::Time, size_t n) {
            KLOG(INFO) << "call feed back:" << n;
            feedback_total += n;
        });
        sock.set_write_callback([&](TcpSocket *s, turbo::Time) {
            s->output_buffer().append(msg).ignore_error();
        });

        auto st = sock.start_write();
        ASSERT_TRUE(st.ok());

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        std::string received = read_all(_server_fd);
        EXPECT_EQ(received.size(), 5u);
        EXPECT_EQ(feedback_total, 5u);
        sock.detach();
        _loop.sync();
    }

    ////////////////////////////////////////////////////////////////////////////////
    /// Test: Write callback.
    ////////////////////////////////////////////////////////////////////////////////

    TEST_F(TcpSocketTest, WriteCallback) {
        TcpSocket sock;
        attach_socket(sock);

        std::atomic<int> write_cb_count{0};
        sock.set_write_callback([&](TcpSocket *s, turbo::Time) {
            write_cb_count++;
            s->disable_write().ignore_error();
        });
        sock.set_write_complete_callback([&](TcpSocket *s, turbo::Time) {
        });

        sock.output_buffer().append("data");
        auto st = sock.start_write();
        ASSERT_TRUE(st.ok());

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        EXPECT_EQ(write_cb_count.load(), 1);
        sock.detach();
        _loop.sync();
    }

    ////////////////////////////////////////////////////////////////////////////////
    /// Test: Peer close (server closes connection).
    ////////////////////////////////////////////////////////////////////////////////

    TEST_F(TcpSocketTest, PeerClose) {
        TcpSocket sock;
        attach_socket(sock);

        std::atomic<bool> close_called{false};
        sock.set_close_callback([&](TcpSocket *, turbo::Time) {
            close_called = true;
        });

        // Close server side.
#if defined(OS_WIN)
        closesocket(_server_fd);
#else
        close(_server_fd);
#endif
        _server_fd = kInvalidFileHandle;

        auto st = sock.start_read();
        ASSERT_TRUE(st.ok());

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        EXPECT_TRUE(close_called.load());
        sock.detach();
        _loop.sync();
    }

    ////////////////////////////////////////////////////////////////////////////////
    /// Test: Read timeout.
    ////////////////////////////////////////////////////////////////////////////////

    TEST_F(TcpSocketTest, ReadTimeout) {
        TcpSocket sock;
        attach_socket(sock);

        std::atomic<bool> timeout_called{false};
        sock.set_read_timeout_callback([&](TcpSocket *, turbo::Time, turbo::Time) {
            timeout_called = true;
        });
        auto func = [](SocketWare *s, void *arg) {
            TcpSocket *sk = reinterpret_cast<TcpSocket *>(s);
            sk->read_timeout_at(*static_cast<turbo::Time *>(arg));
        };
        // Set read timeout to 50ms.
        auto t = turbo::Time::current_time() + turbo::Duration::milliseconds(50);
        sock.run_async(func, &t);
        auto st = sock.start_read();
        ASSERT_TRUE(st.ok());

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        EXPECT_TRUE(timeout_called.load());
        sock.detach();
        _loop.sync();
    }

    ////////////////////////////////////////////////////////////////////////////////
    /// Test: Write timeout (fill send buffer to force blocking).
    ////////////////////////////////////////////////////////////////////////////////

    TEST_F(TcpSocketTest, WriteTimeout) {
        TcpSocket sock;
        attach_socket(sock);

        // First, fill the server side's receive buffer to block client send.
        // We'll send a large amount of data without reading on server side.
        std::string large_data(1024 * 1024, 'z'); // 1MB
        // Send some data to keep server's receive buffer occupied.
        write_string(_server_fd, large_data);
        // Now client's send buffer should be full after a few writes.

        std::atomic<bool> timeout_called{false};
        sock.set_write_timeout_callback([&](TcpSocket *, turbo::Time, turbo::Time) {
            timeout_called = true;
        });
        sock.set_write_callback([&](TcpSocket *s, turbo::Time) {
            KLOG(INFO)<<"call write back";
        });
        sock.set_write_limiter([&](TcpSocket *, turbo::Time) {
            return 0;
        });

        sock.set_read_limiter([&](TcpSocket *, turbo::Time) {
            return 0;
        });

        size_t feedback_total = 0;
        sock.set_read_feedback([&](TcpSocket *, turbo::Time, size_t n) {
            feedback_total = n;
        });

        sock.set_read_callback([&](TcpSocket *s, turbo::Time) {
            // Consume data to keep buffer empty.
            s->input_buffer().clear();
        });


        // Put enough data to fill the socket send buffer.
        //std::string msg(256 *  1024 * 1024, 'w'); // 256KB
       // sock.output_buffer().append(msg);

        auto func = [](SocketWare *s, void *arg) {
            TcpSocket *sk = reinterpret_cast<TcpSocket *>(s);
            sk->write_timeout_at(*static_cast<turbo::Time *>(arg));
            sk->start_write();
            KLOG(INFO)<<"enable";

        };
        // Set read timeout to 50ms.
        auto t = turbo::Time::current_time() + turbo::Duration::milliseconds(50);
        sock.run_async(func, &t);

        //auto st = sock.start_write();
       // ASSERT_TRUE(st.ok());

        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        _loop.sync();
        EXPECT_TRUE(timeout_called.load());
        sock.detach();
        _loop.sync();
    }

    ////////////////////////////////////////////////////////////////////////////////
    /// Test: Error callback (socket error).
    ////////////////////////////////////////////////////////////////////////////////

    TEST_F(TcpSocketTest, ErrorCallback) {
        TcpSocket sock;
        attach_socket(sock);

        std::atomic<bool> error_called{false};
        sock.set_error_callback([&](TcpSocket *, int err, turbo::Time) {
            error_called = true;
        });
        sock.set_write_timeout_callback([&](TcpSocket *, turbo::Time, turbo::Time) {
        });
        sock.set_write_callback([&](TcpSocket *s, turbo::Time) {
            sock.output_buffer().append(std::string(16 * 1024, 'x'));
        });
        sock.set_write_complete_callback([&](TcpSocket *s, turbo::Time) {
        });
        sock.set_close_callback([&](TcpSocket *s, turbo::Time) {
            KLOG(INFO) << "close called";
        });
        // Force an error by closing the server socket and then writing.
#if defined(OS_WIN)
        closesocket(_server_fd);
#else
        ::shutdown(_server_fd, 1);
        ::shutdown(_server_fd, 2);
#endif
        _server_fd = kInvalidFileHandle;

        sock.output_buffer().append("data");
        auto st = sock.start_write();
        ASSERT_TRUE(st.ok());

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        EXPECT_TRUE(error_called.load());
        sock.detach();
        _loop.sync();
    }
} // namespace xio
