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

#include <xio/io/socket_ware.h>
#include <turbo/base/fd_utility.h>
#include <turbo/utility/status.h>
#include <cstring>
#include <cerrno>
#include <turbo/memory/object_pool.h>

#if !defined(OS_WIN)
#include <fcntl.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <sys/socket.h>
#endif

#if defined(OS_WIN)
#include <mstcpip.h>
#define ERRNO WSAGetLastError()
#define EWOULDBLOCK WSAEWOULDBLOCK
#else
#define ERRNO errno
#endif

namespace xio {
    // -----------------------------------------------------------------------------
    // Core binding and lifecycle
    // -----------------------------------------------------------------------------

    void SocketWare::bind(FileHandle fd, EventLoop *loop) {
        _file_handle = fd;
        _event_loop = loop;
        _event_data.handle = fd;
        _event_data.data = this;
        _event_data.read_callback = SocketWare::on_read_call;
        _event_data.write_callback = SocketWare::on_write_call;
        _event_data.error_callback = SocketWare::on_error_call;
    }

    void SocketWare::read_timeout_at(turbo::Time time) {
        DKCHECK(_event_loop->is_in_loop_thread());
        cancel_read_timer();
        _read_timeout = time;
        _read_timer_id = _event_loop->run_at(_read_timeout, read_timer_callback, this);
    }

    void SocketWare::cancel_read_timer() {
        DKCHECK(_event_loop->is_in_loop_thread());
        if (_read_timer_id != TimerBase::kInvalidTimerId) {
            _event_loop->cancel_timer(_read_timer_id);
            _read_timer_id = TimerBase::kInvalidTimerId;
        }
    }

    void SocketWare::write_timeout_at(turbo::Time time) {
        DKCHECK(_event_loop->is_in_loop_thread());
        cancel_write_timer();
        _write_timeout = time;
        _write_timer_id = _event_loop->run_at(_write_timeout, write_timer_callback, this);
    }

    void SocketWare::cancel_write_timer() {
        DKCHECK(_event_loop->is_in_loop_thread());
        if (_write_timer_id != TimerBase::kInvalidTimerId) {
            _event_loop->cancel_timer(_write_timer_id);
            _write_timer_id = TimerBase::kInvalidTimerId;
        }
    }

    // -----------------------------------------------------------------------------
    // emit – cross‑thread safe event enable/disable
    // -----------------------------------------------------------------------------

    void SocketWare::attach_impl() {
        if (_enable_read_flag) {
            (void) enable_read();
        } else {
            (void) disable_read();
        }
        if (_enable_write_flag) {
            (void) enable_write();
        } else {
            (void) disable_write();
        }
    }

    void SocketWare::attach_trampoline(void *arg) {
        auto *self = static_cast<SocketWare *>(arg);
        self->attach_impl();
    }


    void SocketWare::emit(bool enable_read, bool enable_write) {
        _enable_read_flag = enable_read;
        _enable_write_flag = enable_write;
        if (_event_loop->is_in_loop_thread()) {
            attach_impl();
        } else {
            _event_loop->post_task(attach_trampoline, this);
        }
    }

    void SocketWare::detach_trampoline(void *arg) {
        auto *self = static_cast<SocketWare *>(arg);
        self->detach_impl();
    }

    void SocketWare::run_async_trampoline(void *arg) {
        auto *self = static_cast<TaskProxy *>(arg);
        self->func(self->s, self->arg);
    }

    void SocketWare::run_sync_trampoline(void *arg) {
        auto *self = static_cast<SyncTaskProxy *>(arg);
        self->func(self->s, self->arg);
        self->promise.set_value(0);
    }

    void SocketWare::run_proxy_free(void *arg) {
        turbo::return_object<TaskProxy>(reinterpret_cast<TaskProxy *>(arg));
    }

    void SocketWare::detach_impl() {
        disable_all().ignore_error();
    }


    void SocketWare::detach() {
        if (_event_loop->is_in_loop_thread()) {
            detach_impl();
        } else {
            _event_loop->post_task(detach_trampoline, this);
        }
    }

    void SocketWare::run_async(SocketFactor func, void *arg) {
        if (_event_loop->is_in_loop_thread()) {
            func(this, arg);
            return;
        }
        auto ptr = turbo::get_object<TaskProxy>();
        ptr->arg = arg;
        ptr->s = this;
        ptr->func = func;
        AsyncTask task;
        task.arg = ptr;
        task.func = run_async_trampoline;
        task.free_func = run_proxy_free;
        _event_loop->post_task(task);
    }

    void SocketWare::run_sync(SocketFactor func, void *arg) {
        if (_event_loop->is_in_loop_thread()) {
            func(this, arg);
            return;
        }
        SyncTaskProxy proxy;
        proxy.arg = arg;
        proxy.s = this;
        proxy.func = func;
        auto f = proxy.promise.get_future();
        _event_loop->post_task(run_sync_trampoline, &proxy);
        f.wait();
    }

    // -----------------------------------------------------------------------------
    // Event control (must be called on loop thread)
    // -----------------------------------------------------------------------------

    turbo::Status SocketWare::enable_read() {
        DKCHECK(_event_loop->is_in_loop_thread());
        return _event_loop->enable_events_in_loop(&_event_data, EventType::kEventRead, turbo::Time::current_time());
    }

    turbo::Status SocketWare::enable_write() {
        DKCHECK(_event_loop->is_in_loop_thread());
        return _event_loop->enable_events_in_loop(&_event_data, EventType::kEventWrite, turbo::Time::current_time());
    }

    turbo::Status SocketWare::disable_read() {
        DKCHECK(_event_loop->is_in_loop_thread());
        return _event_loop->disable_events_in_loop(&_event_data, EventType::kEventRead, turbo::Time::current_time());
    }

    turbo::Status SocketWare::disable_write() {
        DKCHECK(_event_loop->is_in_loop_thread());
        return _event_loop->disable_events_in_loop(&_event_data, EventType::kEventWrite, turbo::Time::current_time());
    }

    turbo::Status SocketWare::disable_all() {
        DKCHECK(_event_loop->is_in_loop_thread());
        return _event_loop->remove_events_in_loop(&_event_data, turbo::Time::current_time());
    }

    // -----------------------------------------------------------------------------
    // Resume (rate limiting)
    // -----------------------------------------------------------------------------


    void SocketWare::read_timer_callback(void *arg, turbo::Time dl, turbo::Time curr) {
        auto *self = static_cast<SocketWare *>(arg);
        self->_read_timer_id = TimerBase::kInvalidTimerId;
        (void) self->on_read_timeout(dl, curr);
    }

    void SocketWare::write_timer_callback(void *arg, turbo::Time dl, turbo::Time curr) {
        auto *self = static_cast<SocketWare *>(arg);
        self->_write_timer_id = TimerBase::kInvalidTimerId;
        (void) self->on_write_timeout(dl, curr);
    }

    void SocketWare::resume_read_callback(void *arg, turbo::Time, turbo::Time) {
        auto *self = static_cast<SocketWare *>(arg);
        self->_resume_read_timer_id = TimerBase::kInvalidTimerId;
        (void) self->enable_read();
    }

    void SocketWare::resume_write_callback(void *arg, turbo::Time, turbo::Time) {
        auto *self = static_cast<SocketWare *>(arg);
        self->_resume_write_timer_id = TimerBase::kInvalidTimerId;
        (void) self->enable_write();
    }

    turbo::Status SocketWare::resume_read_at(turbo::Time time) {
        DKCHECK(_event_loop->is_in_loop_thread());
        cancel_resume_read_timer();
        if (time <= turbo::Time::current_time()) {
            return enable_read();
        }
        _resume_read_time = time;
        _resume_read_timer_id = _event_loop->run_at(time, resume_read_callback, this);
        return turbo::OkStatus();
    }

    void SocketWare::cancel_resume_read_timer() {
        DKCHECK(_event_loop->is_in_loop_thread());
        if (_resume_read_timer_id != TimerBase::kInvalidTimerId) {
            _event_loop->cancel_timer(_resume_read_timer_id);
            _resume_read_timer_id = TimerBase::kInvalidTimerId;
        }
    }

    turbo::Status SocketWare::resume_write_at(turbo::Time time) {
        DKCHECK(_event_loop->is_in_loop_thread());
        cancel_resume_write_timer();
        if (time <= turbo::Time::current_time()) {
            return enable_write();
        }
        _resume_write_time = time;
        _resume_write_timer_id = _event_loop->run_at(time, resume_write_callback, this);
        return turbo::OkStatus();
    }

    void SocketWare::cancel_resume_write_timer() {
        DKCHECK(_event_loop->is_in_loop_thread());
        if (_resume_write_timer_id != TimerBase::kInvalidTimerId) {
            _event_loop->cancel_timer(_resume_write_timer_id);
            _resume_write_timer_id = TimerBase::kInvalidTimerId;
        }
    }

    void SocketWare::cancel_resume_all_timer() {
        cancel_resume_read_timer();
        cancel_resume_write_timer();
    }


    turbo::Status SocketWare::resume_all_at(turbo::Time time) {
        auto st = resume_read_at(time);
        if (!st.ok()) return st;
        return resume_write_at(time);
    }


    void SocketWare::cancel_all_timer() {
        cancel_read_timer();
        cancel_write_timer();
        cancel_resume_all_timer();
    }


    // -----------------------------------------------------------------------------
    // Socket shutdown and close
    // -----------------------------------------------------------------------------

    void SocketWare::shutdown_read() {
        if (_file_handle != kInvalidFileHandle) {
#if defined(OS_WIN)
            ::shutdown(_file_handle, SD_RECEIVE);
#else
            ::shutdown(_file_handle, SHUT_RD);
#endif
        }
    }

    void SocketWare::shutdown_write() {
        if (_file_handle != kInvalidFileHandle) {
#if defined(OS_WIN)
            ::shutdown(_file_handle, SD_SEND);
#else
            ::shutdown(_file_handle, SHUT_WR);
#endif
        }
    }

    void SocketWare::shutdown_all() {
        if (_file_handle != kInvalidFileHandle) {
#if defined(OS_WIN)
            ::shutdown(_file_handle, SD_BOTH);
#else
            ::shutdown(_file_handle, SHUT_RDWR);
#endif
        }
    }

    void SocketWare::close() {
        if (_file_handle != kInvalidFileHandle) {
#if defined(OS_WIN)
            ::closesocket(_file_handle);
#else
            ::close(_file_handle);
#endif
            _file_handle = kInvalidFileHandle;
        }
    }

    // -----------------------------------------------------------------------------
    // Socket options wrappers
    // -----------------------------------------------------------------------------

    turbo::Status SocketWare::make_nonblocking() {
        if (_file_handle == kInvalidFileHandle) {
            return turbo::failed_precondition_error("Socket not bound");
        }
        if (turbo::make_non_blocking(_file_handle) != 0) {
            return turbo::internal_error("Failed to set non‑blocking mode", errno);
        }
        return turbo::OkStatus();
    }

    turbo::Status SocketWare::set_send_buffer_size(int size) {
        if (setsockopt(_file_handle, SOL_SOCKET, SO_SNDBUF, (const char *) &size, sizeof(size)) != 0) {
            return turbo::internal_error("setsockopt(SO_SNDBUF) failed", ERRNO);
        }
        return turbo::OkStatus();
    }

    turbo::Result<int> SocketWare::get_send_buffer_size() const {
        int size = 0;
        socklen_t len = sizeof(size);
        if (getsockopt(_file_handle, SOL_SOCKET, SO_SNDBUF, (char *) &size, &len) != 0) {
            return turbo::internal_error("getsockopt(SO_SNDBUF) failed", ERRNO);
        }
        return size;
    }

    turbo::Status SocketWare::set_recv_buffer_size(int size) {
        if (setsockopt(_file_handle, SOL_SOCKET, SO_RCVBUF, (const char *) &size, sizeof(size)) != 0) {
            return turbo::internal_error("setsockopt(SO_RCVBUF) failed", ERRNO);
        }
        return turbo::OkStatus();
    }

    turbo::Result<int> SocketWare::get_recv_buffer_size() const {
        int size = 0;
        socklen_t len = sizeof(size);
        if (getsockopt(_file_handle, SOL_SOCKET, SO_RCVBUF, (char *) &size, &len) != 0) {
            return turbo::internal_error("getsockopt(SO_RCVBUF) failed", ERRNO);
        }
        return size;
    }

    turbo::Status SocketWare::set_tcp_nodelay(bool on) {
#if !defined(OS_WIN)
        int flag = on ? 1 : 0;
        if (setsockopt(_file_handle, IPPROTO_TCP, TCP_NODELAY, (const char *) &flag, sizeof(flag)) != 0) {
            return turbo::internal_error("setsockopt(TCP_NODELAY) failed", ERRNO);
        }
        return turbo::OkStatus();
#else
        (void) on;
        return turbo::unimplemented_error("TCP_NODELAY not supported on Windows");
#endif
    }

    turbo::Result<bool> SocketWare::get_tcp_nodelay() const {
#if !defined(OS_WIN)
        int flag = 0;
        socklen_t len = sizeof(flag);
        if (getsockopt(_file_handle, IPPROTO_TCP, TCP_NODELAY, (char *) &flag, &len) != 0) {
            return turbo::internal_error("getsockopt(TCP_NODELAY) failed", ERRNO);
        }
        return flag != 0;
#else
        return turbo::unimplemented_error("TCP_NODELAY not supported on Windows");
#endif
    }

    turbo::Status SocketWare::set_reuse_addr(bool on) {
        int flag = on ? 1 : 0;
        if (setsockopt(_file_handle, SOL_SOCKET, SO_REUSEADDR, (const char *) &flag, sizeof(flag)) != 0) {
            return turbo::internal_error("setsockopt(SO_REUSEADDR) failed", ERRNO);
        }
        return turbo::OkStatus();
    }

    turbo::Result<bool> SocketWare::get_reuse_addr() const {
        int flag = 0;
        socklen_t len = sizeof(flag);
        if (getsockopt(_file_handle, SOL_SOCKET, SO_REUSEADDR, (char *) &flag, &len) != 0) {
            return turbo::internal_error("getsockopt(SO_REUSEADDR) failed", ERRNO);
        }
        return flag != 0;
    }

    turbo::Status SocketWare::set_reuse_port(bool on) {
#if defined(SO_REUSEPORT)
        int flag = on ? 1 : 0;
        if (setsockopt(_file_handle, SOL_SOCKET, SO_REUSEPORT, (const char *) &flag, sizeof(flag)) != 0) {
            return turbo::internal_error("setsockopt(SO_REUSEPORT) failed", ERRNO);
        }
        return turbo::OkStatus();
#else
        (void) on;
        return turbo::unimplemented_error("SO_REUSEPORT not supported");
#endif
    }

    turbo::Result<bool> SocketWare::get_reuse_port() const {
#if defined(SO_REUSEPORT)
        int flag = 0;
        socklen_t len = sizeof(flag);
        if (getsockopt(_file_handle, SOL_SOCKET, SO_REUSEPORT, (char *) &flag, &len) != 0) {
            return turbo::internal_error("getsockopt(SO_REUSEPORT) failed", ERRNO);
        }
        return flag != 0;
#else
        return false;
#endif
    }

    turbo::Status SocketWare::set_keep_alive(bool on) {
        int flag = on ? 1 : 0;
        if (setsockopt(_file_handle, SOL_SOCKET, SO_KEEPALIVE, (const char *) &flag, sizeof(flag)) != 0) {
            return turbo::internal_error("setsockopt(SO_KEEPALIVE) failed", ERRNO);
        }
        return turbo::OkStatus();
    }

    turbo::Result<bool> SocketWare::get_keep_alive() const {
        int flag = 0;
        socklen_t len = sizeof(flag);
        if (getsockopt(_file_handle, SOL_SOCKET, SO_KEEPALIVE, (char *) &flag, &len) != 0) {
            return turbo::internal_error("getsockopt(SO_KEEPALIVE) failed", ERRNO);
        }
        return flag != 0;
    }

    // Keep‑alive helpers (POSIX)
#if !defined(OS_WIN)
    static int keepalive_idle_opt() {
#if defined(TCP_KEEPIDLE)
        return TCP_KEEPIDLE;
#elif defined(TCP_KEEPALIVE)
        return TCP_KEEPALIVE;
#else
        return -1;
#endif
    }

    static int keepalive_interval_opt() {
#if defined(TCP_KEEPINTVL)
        return TCP_KEEPINTVL;
#else
        return -1;
#endif
    }

    static int keepalive_count_opt() {
#if defined(TCP_KEEPCNT)
        return TCP_KEEPCNT;
#else
        return -1;
#endif
    }
#endif

    turbo::Status SocketWare::set_tcp_keep_idle(int seconds) {
#if !defined(OS_WIN)
        int opt = keepalive_idle_opt();
        if (opt == -1) return turbo::unimplemented_error("TCP keep‑idle not supported");
        if (setsockopt(_file_handle, IPPROTO_TCP, opt, &seconds, sizeof(seconds)) != 0) {
            return turbo::internal_error("setsockopt(TCP_KEEPIDLE) failed", ERRNO);
        }
        return turbo::OkStatus();
#else
        (void) seconds;
        return turbo::unimplemented_error("TCP keep‑idle not supported on Windows");
#endif
    }

    turbo::Status SocketWare::set_tcp_keep_interval(int seconds) {
#if !defined(OS_WIN)
        int opt = keepalive_interval_opt();
        if (opt == -1) return turbo::unimplemented_error("TCP keep‑interval not supported");
        if (setsockopt(_file_handle, IPPROTO_TCP, opt, &seconds, sizeof(seconds)) != 0) {
            return turbo::internal_error("setsockopt(TCP_KEEPINTVL) failed", ERRNO);
        }
        return turbo::OkStatus();
#else
        (void) seconds;
        return turbo::unimplemented_error("TCP keep‑interval not supported on Windows");
#endif
    }

    turbo::Status SocketWare::set_tcp_keep_count(int count) {
#if !defined(OS_WIN)
        int opt = keepalive_count_opt();
        if (opt == -1) return turbo::unimplemented_error("TCP keep‑count not supported");
        if (setsockopt(_file_handle, IPPROTO_TCP, opt, &count, sizeof(count)) != 0) {
            return turbo::internal_error("setsockopt(TCP_KEEPCNT) failed", ERRNO);
        }
        return turbo::OkStatus();
#else
        (void) count;
        return turbo::unimplemented_error("TCP keep‑count not supported on Windows");
#endif
    }

    turbo::Status SocketWare::set_linger(bool on, int seconds) {
        struct linger l;
        l.l_onoff = on ? 1 : 0;
        l.l_linger = seconds;
        if (setsockopt(_file_handle, SOL_SOCKET, SO_LINGER, (const char *) &l, sizeof(l)) != 0) {
            return turbo::internal_error("setsockopt(SO_LINGER) failed", ERRNO);
        }
        return turbo::OkStatus();
    }

    turbo::Result<std::pair<bool, int> > SocketWare::get_linger() const {
        struct linger l;
        socklen_t len = sizeof(l);
        if (getsockopt(_file_handle, SOL_SOCKET, SO_LINGER, (char *) &l, &len) != 0) {
            return turbo::internal_error("getsockopt(SO_LINGER) failed", ERRNO);
        }
        return std::make_pair(l.l_onoff != 0, l.l_linger);
    }

    turbo::Status SocketWare::set_oob_inline(bool on) {
        int flag = on ? 1 : 0;
        if (setsockopt(_file_handle, SOL_SOCKET, SO_OOBINLINE, (const char *) &flag, sizeof(flag)) != 0) {
            return turbo::internal_error("setsockopt(SO_OOBINLINE) failed", ERRNO);
        }
        return turbo::OkStatus();
    }

    turbo::Status SocketWare::set_ip_tos(int tos) {
        if (setsockopt(_file_handle, IPPROTO_IP, IP_TOS, (const char *) &tos, sizeof(tos)) != 0) {
            return turbo::internal_error("setsockopt(IP_TOS) failed", ERRNO);
        }
        return turbo::OkStatus();
    }

    turbo::Result<int> SocketWare::get_ip_tos() const {
        int tos = 0;
        socklen_t len = sizeof(tos);
        if (getsockopt(_file_handle, IPPROTO_IP, IP_TOS, (char *) &tos, &len) != 0) {
            return turbo::internal_error("getsockopt(IP_TOS) failed", ERRNO);
        }
        return tos;
    }

    turbo::Status SocketWare::set_ip_ttl(int ttl) {
        if (setsockopt(_file_handle, IPPROTO_IP, IP_TTL, (const char *) &ttl, sizeof(ttl)) != 0) {
            return turbo::internal_error("setsockopt(IP_TTL) failed", ERRNO);
        }
        return turbo::OkStatus();
    }

    turbo::Result<int> SocketWare::get_ip_ttl() const {
        int ttl = 0;
        socklen_t len = sizeof(ttl);
        if (getsockopt(_file_handle, IPPROTO_IP, IP_TTL, (char *) &ttl, &len) != 0) {
            return turbo::internal_error("getsockopt(IP_TTL) failed", ERRNO);
        }
        return ttl;
    }

    turbo::Status SocketWare::bind_to_device(const std::string &iface) {
#if defined(SO_BINDTODEVICE) && !defined(OS_WIN)
        if (setsockopt(_file_handle, SOL_SOCKET, SO_BINDTODEVICE, iface.c_str(), iface.size()) != 0) {
            return turbo::internal_error("setsockopt(SO_BINDTODEVICE) failed", ERRNO);
        }
        return turbo::OkStatus();
#else
        (void) iface;
        return turbo::unimplemented_error("SO_BINDTODEVICE not supported");
#endif
    }

    turbo::Status SocketWare::get_tcp_info(void *out, socklen_t *len) const {
#if defined(TCP_INFO) && !defined(OS_WIN)
        if (getsockopt(_file_handle, IPPROTO_TCP, TCP_INFO, (char *) out, len) != 0) {
            return turbo::internal_error("getsockopt(TCP_INFO) failed", ERRNO);
        }
        return turbo::OkStatus();
#elif defined(TCP_CONNECTION_INFO) && defined(__APPLE__)
        if (getsockopt(_file_handle, IPPROTO_TCP, TCP_CONNECTION_INFO, (char *) out, len) != 0) {
            return turbo::internal_error("getsockopt(TCP_CONNECTION_INFO) failed", ERRNO);
        }
        return turbo::OkStatus();
#else
        (void) out; (void) len;
        return turbo::unimplemented_error("TCP_INFO not supported");
#endif
    }

    turbo::Result<std::string> SocketWare::get_tcp_info_string() const {
#if defined(TCP_INFO) && !defined(OS_WIN)
        struct tcp_info info;
        socklen_t len = sizeof(info);
        auto st = get_tcp_info(&info, &len);
        if (!st.ok()) return st;
        char buf[512];
        snprintf(buf, sizeof(buf),
                 "state=%u rtt=%u rttvar=%u snd_cwnd=%u snd_ssthresh=%u "
                 "rcv_space=%u rcv_ssthresh=%u",
                 info.tcpi_state, info.tcpi_rtt, info.tcpi_rttvar,
                 info.tcpi_snd_cwnd, info.tcpi_snd_ssthresh,
                 info.tcpi_rcv_space, info.tcpi_rcv_ssthresh);
        return std::string(buf);
#elif defined(TCP_CONNECTION_INFO) && defined(__APPLE__)
        struct tcp_connection_info info;
        socklen_t len = sizeof(info);
        auto st = get_tcp_info(&info, &len);
        if (!st.ok()) return st;
        char buf[512];
        snprintf(buf, sizeof(buf),
                 "state=%u sendbuf=%u recvbuf=%u cc_algo=%s",
                 info.tcpi_state, info.tcpi_sndbuf, info.tcpi_rcvbuf, info.tcpi_cc_algo);
        return std::string(buf);
#else
        return turbo::unimplemented_error("TCP_INFO not supported");
#endif
    }

    /// -----------------------------------------------------------------------------
    /// Static trampolines – proxy only, real implementation in derived classes
    /// -----------------------------------------------------------------------------

    turbo::Status SocketWare::on_read_call(FileHandle /*fd*/, EventData *data, turbo::Time cur) {
        auto *self = static_cast<SocketWare *>(data->data);
        return self->on_read(cur);
    }

    turbo::Status SocketWare::on_write_call(FileHandle /*fd*/, EventData *data, turbo::Time cur) {
        auto *self = static_cast<SocketWare *>(data->data);
        return self->on_write(cur);
    }

    turbo::Status SocketWare::on_error_call(FileHandle /*fd*/, EventData *data, turbo::Time cur) {
        auto *self = static_cast<SocketWare *>(data->data);
        return self->on_error(cur);
    }

    turbo::Status SocketWare::on_write_timeout_call(FileHandle /*fd*/, EventData *data, turbo::Time expire,
                                                    turbo::Time cur) {
        auto *self = static_cast<SocketWare *>(data->data);
        return self->on_write_timeout(expire, cur);
    }

    turbo::Status SocketWare::on_read_timeout_call(FileHandle /*fd*/, EventData *data, turbo::Time expire,
                                                   turbo::Time cur) {
        auto *self = static_cast<SocketWare *>(data->data);
        return self->on_read_timeout(expire, cur);
    }
} // namespace xio
