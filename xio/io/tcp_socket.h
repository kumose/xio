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

#include <xio/io/socket_ware.h>
#include <fermat/container/cord_buffer.h>
#include <functional>
#include <string>

namespace xio {
    /// TCP socket for already‑established connections.
    /// Does NOT handle connection establishment; use Connector for client side
    /// or Listener for server side.
    /// Inherits SocketWare to provide event‑driven I/O with read/write buffers.
    class TcpSocket : public SocketWare {
    public:
        /// default 64 * 16k = 1M
        static constexpr size_t MAX_APPEND_IOVEC = 64;

    public:
        /// return 0 means stop read/write,
        using LimitedCallback = std::function<size_t(TcpSocket *, turbo::Time cur)>;
        using LimitedFeedback = std::function<void(TcpSocket *, turbo::Time cur, size_t)>;
        /// Callback invoked when data is available in the input buffer.
        /// The user should consume data from input_buffer() within this callback.
        using IOCallback = std::function<void(TcpSocket *, turbo::Time cur)>;

        /// Callback invoked on socket error (errno value provided).
        using ErrorCallback = std::function<void(TcpSocket *, int err, turbo::Time cur)>;

        using TimeoutCallback = std::function<void(TcpSocket *, turbo::Time dl, turbo::Time cur)>;

        TcpSocket() = default;

        ~TcpSocket() override = default;

        /// Sets the read callback.
        void set_read_callback(IOCallback cb) { _read_cb = std::move(cb); }
        /// Sets the write callback.
        void set_write_callback(IOCallback cb) { _write_cb = std::move(cb); }
        void set_write_feedback(LimitedFeedback cb) { _write_feedback = std::move(cb); }
        void set_read_feedback(LimitedFeedback cb) { _read_feedback = std::move(cb); }
        /// Sets the error callback.
        void set_error_callback(ErrorCallback cb) { _error_cb = std::move(cb); }
        /// Sets the close callback.
        void set_close_callback(IOCallback cb) { _close_cb = std::move(cb); }
        void set_read_timeout_callback(TimeoutCallback cb) { _read_timeout_cb = std::move(cb); }
        void set_write_timeout_callback(TimeoutCallback cb) { _write_timeout_cb = std::move(cb); }
        void set_read_limiter(LimitedCallback cb) { _read_limiter = std::move(cb); }
        void set_write_limiter(LimitedCallback cb) { _write_limiter = std::move(cb); }
        void set_write_complete_callback(IOCallback cb) { _write_complete_cb = std::move(cb); }

        /// Attaches the socket to an EventLoop, a file descriptor, and a user context.
        /// This method is thread‑safe; if called from outside the loop thread,
        /// the operation is forwarded asynchronously and blocks until complete.
        /// @param loop The EventLoop that will drive I/O events.
        /// @param fd   The already‑connected, non‑blocking socket file descriptor.
        /// @param ctx  Arbitrary user pointer (can be retrieved later via ctx()).
        void attach(EventLoop *loop, FileHandle fd, void *ctx);

        /// Starts reading from the socket. If called from outside the loop thread,
        /// the operation is forwarded synchronously (blocks until completion).
        /// The function reads as much data as possible into the input buffer,
        /// invoking the read callback when new data arrives.
        /// On EAGAIN, it enables read events; on error or closure, it invokes the
        /// appropriate callback and closes the socket.
        turbo::Status start_read();

        /// Starts writing to the socket. If called from outside the loop thread,
        /// the operation is forwarded synchronously.
        /// It writes as much data as possible from the output buffer to the socket,
        /// invoking the write callback when the output buffer becomes empty.
        /// On EAGAIN, it enables write events; on error, invokes the error callback.
        turbo::Status start_write();

        /// Calls start_read() and start_write() together.
        turbo::Status start_all();

        /// Internal version of start_read() that must be called on the loop thread.
        turbo::Status start_read_in_loop();

        /// Internal version of start_write() that must be called on the loop thread.
        turbo::Status start_write_in_loop();

        /// Internal version of start_all() that must be called on the loop thread.
        turbo::Status start_all_in_loop();

        // SocketWare overrides
        /// Called by the event loop when the socket becomes readable.
        /// It performs a non‑blocking read and invokes the read callback if data was read.
        turbo::Status on_read(turbo::Time cur) override;

        /// Called by the event loop when the socket becomes writable.
        /// It writes pending data from the output buffer and enables/disables events accordingly.
        turbo::Status on_write(turbo::Time cur) override;

        /// Called by the event loop on socket error.
        turbo::Status on_error(turbo::Time cur) override;

        /// Called when a read timeout expires.
        turbo::Status on_read_timeout(turbo::Time expire, turbo::Time cur) override;

        /// Called when a write timeout expires.
        turbo::Status on_write_timeout(turbo::Time expire, turbo::Time cur) override;

        /// Returns a mutable reference to the input buffer.
        CordBufferType &input_buffer() { return _input_buffer; }
        /// Returns a const reference to the input buffer.
        const CordBufferType &input_buffer() const { return _input_buffer; }

        /// Returns a mutable reference to the output buffer.
        CordBufferType &output_buffer() { return _output_buffer; }
        /// Returns a const reference to the output buffer.
        const CordBufferType &output_buffer() const { return _output_buffer; }

        /// Returns true if the input buffer is empty.
        bool input_empty() const { return _input_buffer.size() == 0; }
        /// Returns true if the output buffer is empty.
        bool output_empty() const { return _output_buffer.size() == 0; }
        /// Returns the number of bytes in the input buffer.
        size_t input_buffer_size() const { return _input_buffer.size(); }
        /// Returns the number of bytes in the output buffer.
        size_t output_buffer_size() const { return _output_buffer.size(); }

        /// Returns the user context pointer (mutable).
        void *ctx() { return _ctx; }
        /// Returns the user context pointer (const).
        const void *ctx() const { return _ctx; }
        /// Sets the user context pointer.
        void set_ctx(void *ctx) { _ctx = ctx; }

        /// Reads data from a file descriptor into a CordBuffer, appending as needed.
        static IoResult read_to_cord(FileHandle fd, CordBufferType &buffer,
                                     size_t max_size = std::numeric_limits<size_t>::max());

        /// Writes data from a CordBuffer to a file descriptor, removing written bytes.
        static IoResult write_from_cord(FileHandle fd, const CordBufferType &buffer,
                                        size_t max_size = std::numeric_limits<size_t>::max());

        /// Similar to read_to_cord, but returns number of bytes read.
        /// need resize the buffer size.
        static IoResult read_to_buffer(FileHandle fd, BufferType &buffer);

        /// Similar to write_from_cord, but returns number of bytes written.
        static IoResult write_from_buffer(FileHandle fd, const BufferType &buffer);

        /// Reads data from a file descriptor into a plain memory buffer.
        static IoResult read_to_buffer(FileHandle fd, void *buffer, size_t size);

        /// Writes data from a plain memory buffer to a file descriptor.
        static IoResult write_from_buffer(FileHandle fd, const void *buffer, size_t size);

    private:
        /// Reads data from the socket into _input_buffer, handling EAGAIN and errors.
        void do_read(turbo::Time cur);

        /// Writes data from _output_buffer to the socket, handling EAGAIN and errors.
        void do_write(turbo::Time cur);

        /// Context structure for cross‑thread start operations.
        struct StartCtx {
            /// Promise to carry the result.
            std::promise<turbo::Status> promise;
            /// Pointer to the TcpSocket instance.
            TcpSocket *self;
        };

        static size_t unlimited_io_callback(TcpSocket *s, turbo::Time cur);

        static void unlimited_io_feedback(TcpSocket *s, turbo::Time cur, size_t n);

        static void noop_timeout_callback(TcpSocket *s, turbo::Time dl, turbo::Time cur);

        /// Trampoline for posting start_read to the loop thread.
        static void start_read_trampoline(void *arg);

        /// Trampoline for posting start_write to the loop thread.
        static void start_write_trampoline(void *arg);

        /// Trampoline for posting start_all to the loop thread.
        static void start_all_trampoline(void *arg);

        /// User context pointer (typically points to the owning business object).
        void *_ctx{nullptr};

        LimitedCallback _read_limiter{unlimited_io_callback};
        LimitedCallback _write_limiter{unlimited_io_callback};
        LimitedFeedback _read_feedback{unlimited_io_feedback};
        LimitedFeedback _write_feedback{unlimited_io_feedback};
        IOCallback _read_cb;
        IOCallback _write_cb;
        IOCallback _write_complete_cb;
        ErrorCallback _error_cb;
        IOCallback _close_cb;
        TimeoutCallback _read_timeout_cb{noop_timeout_callback};
        TimeoutCallback _write_timeout_cb{noop_timeout_callback};

        /// Input buffer for received data.
        CordBufferType _input_buffer;
        /// Output buffer for pending data to send.
        CordBufferType _output_buffer;
    };
} // namespace xio
