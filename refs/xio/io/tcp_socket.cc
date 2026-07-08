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
#include <turbo/base/fd_utility.h>
#include <cstring>
#include <cerrno>

#if defined(OS_WIN)
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
#if defined(_WIN64)
typedef long long ssize_t;
#else
typedef int ssize_t;
#endif
#define ERRNO WSAGetLastError()
#define EWOULDBLOCK WSAEWOULDBLOCK
#else
#include <sys/uio.h>
#include <unistd.h>
#define ERRNO errno
#endif


namespace xio {
    /// Attaches the socket to an EventLoop, file descriptor, and user context.
    void TcpSocket::attach(EventLoop *loop, FileHandle fd, void *ctx) {
        bind(fd, loop);
        _ctx = ctx;
    }

    /// Trampoline for start_read().
    void TcpSocket::start_read_trampoline(void *arg) {
        auto *ctx = static_cast<StartCtx *>(arg);
        auto st = ctx->self->start_read_in_loop();
        ctx->promise.set_value(std::move(st));
    }

    /// Trampoline for start_write().
    void TcpSocket::start_write_trampoline(void *arg) {
        auto *ctx = static_cast<StartCtx *>(arg);
        auto st = ctx->self->start_write_in_loop();
        ctx->promise.set_value(std::move(st));
    }

    /// Trampoline for start_all().
    void TcpSocket::start_all_trampoline(void *arg) {
        auto *ctx = static_cast<StartCtx *>(arg);
        auto st = ctx->self->start_all_in_loop();
        ctx->promise.set_value(std::move(st));
    }

    /// Starts reading. Thread‑safe.
    turbo::Status TcpSocket::start_read() {
        if (_event_loop->is_in_loop_thread()) {
            return start_read_in_loop();
        }
        StartCtx ctx{{}, this};
        auto fut = ctx.promise.get_future();
        _event_loop->post_task(start_read_trampoline, &ctx);
        return fut.get();
    }

    /// Starts writing. Thread‑safe.
    turbo::Status TcpSocket::start_write() {
        if (_event_loop->is_in_loop_thread()) {
            return start_write_in_loop();
        }
        StartCtx ctx{{}, this};
        auto fut = ctx.promise.get_future();
        _event_loop->post_task(start_write_trampoline, &ctx);
        return fut.get();
    }

    /// Starts both reading and writing.
    turbo::Status TcpSocket::start_all() {
        if (_event_loop->is_in_loop_thread()) {
            return start_all_in_loop();
        }
        StartCtx ctx{{}, this};
        auto fut = ctx.promise.get_future();
        _event_loop->post_task(start_all_trampoline, &ctx);
        return fut.get();
    }

    /// Internal version of start_read() – must be called on loop thread.
    turbo::Status TcpSocket::start_read_in_loop() {
        DKCHECK(_event_loop->is_in_loop_thread());
        do_read(turbo::Time::current_time());
        return turbo::OkStatus();
    }

    /// Internal version of start_write() – must be called on loop thread.
    turbo::Status TcpSocket::start_write_in_loop() {
        DKCHECK(_event_loop->is_in_loop_thread());
        do_write(turbo::Time::current_time());
        return turbo::OkStatus();
    }

    /// Internal version of start_all() – must be called on loop thread.
    turbo::Status TcpSocket::start_all_in_loop() {
        DKCHECK(_event_loop->is_in_loop_thread());
        do_read(turbo::Time::current_time());
        do_write(turbo::Time::current_time());
        return turbo::OkStatus();
    }

    /// Default unlimited limiter.
    size_t TcpSocket::unlimited_io_callback(TcpSocket *, turbo::Time) {
        return std::numeric_limits<size_t>::max();
    }

    void TcpSocket::unlimited_io_feedback(TcpSocket *s, turbo::Time cur, size_t n) {
        TURBO_UNUSED(s);
        TURBO_UNUSED(cur);
        TURBO_UNUSED(n);
    }

    void TcpSocket::noop_timeout_callback(TcpSocket *s, turbo::Time dl, turbo::Time cur) {
        TURBO_UNUSED(s);
        TURBO_UNUSED(cur);
        TURBO_UNUSED(dl);
    }

    /// Reads as much as possible from socket into input buffer, respecting read limiter.
    /// Never closes fd; only reports via callbacks.
    void TcpSocket::do_read(turbo::Time cur) {
        size_t quota = _read_limiter(this, cur);
        IoResult r;
        size_t total_read = 0;
        while (total_read < quota) {
            r = read_to_cord(_file_handle, _input_buffer, quota - total_read);
            total_read += r.size;
            if (r.state == IOState::kNoMoreData || r.state == IOState::kPeerClose || r.state == IOState::kError) {
                break;
            }
        }
        _read_feedback(this, cur, total_read);
        /////////////////////////////////////////////
        /// may have data to read, but this disable read should be
        ///  leave to _read_cb to operate
        /// if (r.state == IOState::kNone) {
        ///
        ///}
        ///
        /////////////////////////////////////////////
        /// alread enabled read, no need
        /// if (r.state == IOState::kNoMoreData) {
        ///
        ///}
        if (_input_buffer.size() > 0) {
            _read_cb(this, cur);
        }
        switch (r.state) {
            case IOState::kPeerClose: {
                _close_cb(this, cur);
                break;
            }
            case IOState::kError: {
                _error_cb(this, r.error, cur);
                break;
            }
            default:
                enable_read().ignore_error();
                break;
        }
    }

    /// Writes as much as possible from output buffer to socket, respecting write limiter.
    void TcpSocket::do_write(turbo::Time cur) {
        size_t quota = _write_limiter(this, cur);
        if (quota == 0) {
            return;
        }
        _write_cb(this, cur);
        IoResult r;
        size_t total_write = 0;
        size_t need_write = std::min(_output_buffer.size(), quota);
        while (total_write < need_write) {
            r = write_from_cord(_file_handle, _output_buffer, need_write - total_write);
            total_write += r.size;
            _output_buffer.pop_front(r.size);
            if (r.state == IOState::kNoMoreData || r.state == IOState::kPeerClose || r.state == IOState::kError) {
                break;
            }
        }
        _write_feedback(this, cur, total_write);
        if (_output_buffer.size() == 0) {
            _write_complete_cb(this, cur);
        }

        switch (r.state) {
            case IOState::kPeerClose: {
                _close_cb(this, cur);
                break;
            }
            case IOState::kError: {
                _error_cb(this, r.error, cur);
                break;
            }
            default: {
                if (_output_buffer.size() > 0) {
                    enable_write().ignore_error();
                }
                break;
            }
        }
    }

    /// Socket readable event.
    turbo::Status TcpSocket::on_read(turbo::Time cur) {
        do_read(cur);
        return turbo::OkStatus();
    }

    /// Socket writable event.
    turbo::Status TcpSocket::on_write(turbo::Time cur) {
        do_write(cur);
        return turbo::OkStatus();
    }

    /// Socket error event.
    turbo::Status TcpSocket::on_error(turbo::Time cur) {
        _error_cb(this, ECONNABORTED, cur);
        return turbo::OkStatus();
    }

    /// Read timeout event.
    turbo::Status TcpSocket::on_read_timeout(turbo::Time expire, turbo::Time cur) {
        _read_timeout_cb(this, expire, cur);
        return turbo::OkStatus();
    }

    /// Write timeout event.
    turbo::Status TcpSocket::on_write_timeout(turbo::Time expire, turbo::Time cur) {
        _write_timeout_cb(this, expire, cur);
        return turbo::OkStatus();
    }

    /// Reads data from fd into CordBuffer using scatter/gather I/O.
    /// Uses WSARecv on Windows and readv on POSIX platforms.
    /// @param fd Socket file descriptor.
    /// @param buffer Target CordBuffer.
    /// @param max_size Maximum bytes to read in this call.
    /// @return IoResult with total bytes read and state/error.
    IoResult TcpSocket::read_to_cord(FileHandle fd, CordBufferType &buffer,
                                     size_t max_size) {
        if (max_size == 0) return {0, IOState::kNone, 0};

        fermat::Vector<fermat::BufferRef<kAlignment> > new_refs;
        size_t total_wanted = 0;

#if defined(OS_WIN)
        fermat::Vector<WSABUF> iovs;
        iovs.reserve(MAX_APPEND_IOVEC);
#else
        fermat::Vector<struct iovec> iovs;
        iovs.reserve(MAX_APPEND_IOVEC);
#endif

        /// Try to use the existing tail block's writable span.
        auto span = buffer.output_next();

        size_t take = std::min<size_t>(span.size(), max_size);
#if defined(OS_WIN)
        WSABUF wbuf;
        wbuf.buf = static_cast<CHAR *>(span.data());
        wbuf.len = static_cast<ULONG>(take);
        iovs.push_back(wbuf);
#else
        iovs.push_back({const_cast<char *>(span.data()), take});
#endif
        total_wanted += take;

        /// Allocate additional blocks if more data is requested.
        while (total_wanted < max_size && iovs.size() < MAX_APPEND_IOVEC) {
            size_t block_size = std::min<size_t>(kBufferBlockSize, max_size - total_wanted);
            auto ref = fermat::BufferRef<kAlignment>::create_write_able(block_size);
#if defined(OS_WIN)
            WSABUF wbuf;
            wbuf.buf = static_cast<CHAR *>(ref.buffer->data());
            wbuf.len = static_cast<ULONG>(block_size);
            iovs.push_back(wbuf);
#else
            iovs.push_back({const_cast<char *>(ref.buffer->data()), block_size});
#endif
            total_wanted += block_size;
            new_refs.push_back(std::move(ref));
        }

        /// Perform the actual read using the appropriate system call.
        ssize_t nread = 0;
#if defined(OS_WIN)
        DWORD bytes_recvd = 0;
        DWORD flags = 0;
        int ret = WSARecv(fd, iovs.data(), static_cast<DWORD>(iovs.size()),
                          &bytes_recvd, &flags, NULL, NULL);
        if (ret == SOCKET_ERROR) {
            int err = WSAGetLastError();
            if (err == WSAEWOULDBLOCK) return {0, IOState::kNoMoreData, 0};
            return {0, IOState::kNoMoreData, err};
        }
        nread = static_cast<ssize_t>(bytes_recvd);
#else
        nread = readv(fd, iovs.data(), static_cast<int>(iovs.size()));
        if (nread < 0) {
            buffer.output_backup(span.size());
            int err = errno;
            if (err == EAGAIN) return {0, IOState::kNoMoreData, 0};
            return {0, IOState::kNoMoreData, err};
        }
#endif

        if (nread == 0) {
            buffer.output_backup(span.size());
            return {0, IOState::kPeerClose, 0};
        }

        size_t remaining = static_cast<size_t>(nread);
        size_t iov_idx = 0;

        /// Handle the existing tail block: possibly shrink it if only partially used.
        size_t first_len;
#if defined(OS_WIN)
        first_len = iovs[0].len;
#else
        first_len = iovs[0].iov_len;
#endif
        if (remaining < first_len) {
            buffer.output_backup(first_len - remaining);
            return {static_cast<int64_t>(nread), IOState::kNone, 0};
        }
        remaining -= first_len;
        iov_idx = 1;


        /// Commit the newly allocated blocks that received data.
        size_t new_idx = 0;
        while (iov_idx < iovs.size() && remaining > 0) {
            size_t block_len;
#if defined(OS_WIN)
            block_len = iovs[iov_idx].len;
#else
            block_len = iovs[iov_idx].iov_len;
#endif
            size_t to_commit = std::min(remaining, block_len);
            auto &ref = new_refs[new_idx++];
            ref.range.length = static_cast<uint32_t>(to_commit);
            ref.range.offset = 0;
            buffer.append_writeable(std::move(ref)).ignore_error();
            remaining -= to_commit;
            ++iov_idx;
        }
        return {static_cast<int64_t>(nread), IOState::kNone, 0};
    }

    /// Writes data from CordBuffer to fd, without modifying the buffer.
    /// Uses writev (or WSASend on Windows) to send multiple blocks at once.
    /// @param fd Socket file descriptor.
    /// @param buffer Source CordBuffer (const, not modified).
    /// @param max_size Maximum bytes to send in this call.
    /// @return IoResult with total bytes sent and state/error.
    IoResult TcpSocket::write_from_cord(FileHandle fd, const CordBufferType &buffer, size_t max_size) {
        if (buffer.size() == 0) return {0, IOState::kNone, 0};

        // Collect up to MAX_APPEND_IOVEC non‑empty blocks.
        fermat::Vector<struct iovec> iovs;
        iovs.reserve(MAX_APPEND_IOVEC);
        size_t total = 0;
        const size_t nblocks = buffer.buffer_count();

        for (size_t i = 0; i < nblocks && iovs.size() < MAX_APPEND_IOVEC; ++i) {
            const auto *ref = buffer.buffer_at(i);
            if (!ref || ref->size() == 0) continue;
            size_t to_send = std::min<size_t>(ref->size(), max_size - total);
            if (to_send == 0) break;
            iovs.push_back({const_cast<char *>(ref->data()), to_send});
            total += to_send;
            if (total >= max_size) break;
        }

        if (iovs.empty()) return {0, IOState::kNone, 0};

#if defined(OS_WIN)
        // Windows: use WSASend instead of writev.
        std::vector<WSABUF> wsabufs(iovs.size());
        for (size_t i = 0; i < iovs.size(); ++i) {
            wsabufs[i].buf = static_cast<CHAR *>(iovs[i].iov_base);
            wsabufs[i].len = static_cast<ULONG>(iovs[i].iov_len);
        }
        DWORD bytes_sent = 0;
        int ret = WSASend(fd, wsabufs.data(), static_cast<DWORD>(wsabufs.size()), &bytes_sent, 0, NULL, NULL);
        if (ret == SOCKET_ERROR) {
            int err = WSAGetLastError();
            if (err == WSAEWOULDBLOCK) {
                return {0, IOState::kNoMoreData, 0};
            }
            return {0, IOState::kNoMoreData, err};
        }
        return {static_cast<int64_t>(bytes_sent), IOState::kNone, 0};
#else
        // POSIX: use writev.
        ssize_t n = writev(fd, iovs.data(), iovs.size());
        if (n < 0) {
            int err = errno;
            if (err == EAGAIN || err == EWOULDBLOCK) {
                return {0, IOState::kNoMoreData, 0};
            }
            return {0, IOState::kNoMoreData, err};
        }
        if (n == 0) {
            // No bytes written (unlikely for writev, but handle gracefully)
            return {0, IOState::kPeerClose, 0};
        }
        return {static_cast<int64_t>(n), IOState::kNone, 0};
#endif
    }

    /// Reads data into a plain memory buffer (e.g., std::vector<char>).
    IoResult TcpSocket::read_to_buffer(FileHandle fd, BufferType &buffer) {
        if (buffer.empty()) return {0, IOState::kNone, 0};
        ssize_t n = recv(fd, buffer.data(), buffer.size(), 0);
        if (n > 0) {
            buffer.resize(n);
            return {n, IOState::kNone, 0};
        }
        if (n == 0) {
            return {0, IOState::kPeerClose, 0};
        }
        int err = ERRNO;
#if defined(EWOULDBLOCK) && (EWOULDBLOCK != EAGAIN)
        if (err == EAGAIN || err == EWOULDBLOCK) {
#else
        if (err == EAGAIN) {
#endif
            return {0, IOState::kNoMoreData, 0};
        }
        // Other error: size=0, state=kNoMoreData, error=err
        return {0, IOState::kError, err};
    }

    /// Writes data from a plain memory buffer to fd.
    IoResult TcpSocket::write_from_buffer(FileHandle fd, const BufferType &buffer) {
        if (buffer.empty()) return {0, IOState::kNone, 0};
        ssize_t n = send(fd, buffer.data(), buffer.size(), 0);
        if (n > 0) {
            return {n, IOState::kNone, 0};
        }
        if (n == 0) {
            return {0, IOState::kPeerClose, 0};
        }
        int err = ERRNO;
#if defined(EWOULDBLOCK) && (EWOULDBLOCK != EAGAIN)
        if (err == EAGAIN || err == EWOULDBLOCK) {
#else
        if (err == EAGAIN) {
#endif
            return {0, IOState::kNoMoreData, 0};
        }
        // Other error: size=0, state=kNoMoreData, error=err
        return {0, IOState::kError, err};
    }

    IoResult TcpSocket::read_to_buffer(FileHandle fd, void *buf, size_t size) {
        if (size == 0) return {0, IOState::kNone, 0};
        ssize_t n = recv(fd, static_cast<char *>(buf), size, 0);
        if (n > 0) {
            return {n, IOState::kNone, 0};
        }
        if (n == 0) {
            return {0, IOState::kPeerClose, 0};
        }
        int err = ERRNO;
#if defined(EWOULDBLOCK) && (EWOULDBLOCK != EAGAIN)
        if (err == EAGAIN || err == EWOULDBLOCK) {
#else
        if (err == EAGAIN) {
#endif
            return {0, IOState::kNoMoreData, 0};
        }
        // Other error: size=0, state=kNoMoreData, error=err
        return {0, IOState::kError, err};
    }

    IoResult TcpSocket::write_from_buffer(FileHandle fd, const void *buf, size_t size) {
        if (size == 0) return {0, IOState::kNone, 0};
        ssize_t n = send(fd, static_cast<const char *>(buf), size, 0);
        if (n > 0) {
            return {n, IOState::kNone, 0};
        }
        if (n == 0) {
            return {0, IOState::kPeerClose, 0};
        }
        int err = ERRNO;
#if defined(EWOULDBLOCK) && (EWOULDBLOCK != EAGAIN)
        if (err == EAGAIN || err == EWOULDBLOCK) {
#else
        if (err == EAGAIN) {
#endif
            return {0, IOState::kNoMoreData, 0};
        }
        // Other error: size=0, state=kNoMoreData, error=err
        return {0, IOState::kError, err};
    }
} // namespace xio
