/************************************************************************
Modifications Copyright 2017-2019 eBay Inc.
Author/Developer(s): Jung-Sang Ahn

Original Copyright:
See URL: https://github.com/datatechnology/cornerstone

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    https://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
**************************************************************************/

#define ASIO_HAS_STD_CHRONO 1
#if defined(__EDG_VERSION__)
#undef __EDG_VERSION__
#endif

#define  _CRT_SECURE_NO_WARNINGS

#include <xio/raft/xio_service.h>

#include <xio/raft/buffer_serializer.h>
#include <xio/raft/callback.h>
#include <xio/raft/crc32.h>
#include <xio/raft/global_mgr.h>
#include <xio/raft/internal_timer.h>
#include <xio/raft/rpc_listener.h>
#include <xio/raft/raft_server.h>
#include <xio/raft/raft_server_handler.h>
#include <xio/raft/strfmt.h>
#include <xio/logging.h>

#include <xio/xio.h>

namespace xio {
    using io_service = io_context;
}

#define ERROR_CODE xio::error_code

#include <atomic>
#include <ctime>
#include <exception>
#include <list>
#include <thread>


//#define SSL_LIBRARY_NOT_FOUND (1)
#ifdef SSL_LIBRARY_NOT_FOUND
#include <xio/raft/mock_ssl.h>
using ssl_socket = mock_ssl_socket;
using ssl_context = mock_ssl_context;
#else
#include <xio/ssl.h>

using ssl_socket = xio::ssl::stream<xio::ip::tcp::socket &>;
using ssl_context = xio::ssl::context;

namespace {
    std::string ssl_context_method_to_string(ssl_context::method method) {
        switch (method) {
            case ssl_context::sslv2: return "sslv2";
            case ssl_context::sslv2_client: return "sslv2_client";
            case ssl_context::sslv2_server: return "sslv2_server";
            case ssl_context::sslv3: return "sslv3";
            case ssl_context::sslv3_client: return "sslv3_client";
            case ssl_context::sslv3_server: return "sslv3_server";
            case ssl_context::tlsv1: return "tlsv1";
            case ssl_context::tlsv1_client: return "tlsv1_client";
            case ssl_context::tlsv1_server: return "tlsv1_server";
            case ssl_context::sslv23: return "sslv23";
            case ssl_context::sslv23_client: return "sslv23_client";
            case ssl_context::sslv23_server: return "sslv23_server";
            case ssl_context::tlsv11: return "tlsv11";
            case ssl_context::tlsv11_client: return "tlsv11_client";
            case ssl_context::tlsv11_server: return "tlsv11_server";
            case ssl_context::tlsv12: return "tlsv12";
            case ssl_context::tlsv12_client: return "tlsv12_client";
            case ssl_context::tlsv12_server: return "tlsv12_server";
            case ssl_context::tlsv13: return "tlsv13";
            case ssl_context::tlsv13_client: return "tlsv13_client";
            case ssl_context::tlsv13_server: return "tlsv13_server";
            case ssl_context::tls: return "tls";
            case ssl_context::tls_client: return "tls_client";
            case ssl_context::tls_server: return "tls_server";
            default:
                return "unknown (" + std::to_string(static_cast<int>(method)) + ")";
        }
    }
}
#endif

// Note: both req & resp header structures have been modified by Jung-Sang Ahn.
//       They MUST NOT be combined with the original code.

// request header:
//     byte         marker (req = 0x0)  (1),
//     msg_type     type                (1),
//     int32        src                 (4),
//     int32        dst                 (4),
//     ulong        term                (8),
//     ulong        last_log_term       (8),
//     ulong        last_log_idx        (8),
//     ulong        commit_idx          (8),
//     int32        log data size       (4),
//     ulong        flags + CRC32       (8),
//     -------------------------------------
//                  total               (54)
#define RPC_REQ_HEADER_SIZE (4*3 + 8*5 + 1*2)

// response header:
//     byte         marker (resp = 0x1) (1),
//     msg_type     type                (1),
//     int32        src                 (4),
//     int32        dst                 (4),
//     ulong        term                (8),
//     ulong        next_idx            (8),
//     bool         accepted            (1),
//     int32        ctx data dize       (4),
//     ulong        flags + CRC32       (8),
//     -------------------------------------
//                  total               (39)
#define RPC_RESP_HEADER_SIZE (4*3 + 8*3 + 1*3)

#define DATA_SIZE_LEN (4)
#define CRC_FLAGS_LEN (8)

// === RPC Flags =========

// If set, RPC message includes custom meta given by user.
#define INCLUDE_META (0x1)

// If set, RPC message (response) includes additional hints.
#define INCLUDE_HINT (0x2)

// If set, each log entry will contain timestamp.
#define INCLUDE_LOG_TIMESTAMP (0x4)

// If set, CRC number represents the entire message.
#define CRC_ON_ENTIRE_MESSAGE (0x8)

// If set, each log entry will contain a CRC on the payload.
#define CRC_ON_PAYLOAD (0x10)

// If set, RPC message (response) includes result code
#define INCLUDE_RESULT_CODE (0x20)

// If set, and
//   - If it is used in a request (leader -> follower),
//     the follower has been excluded from the quorum,
//     determined by the leader (i.e., sender).
//   - If it is used in a response (follower -> leader),
//     the follower is marked down by itself.
#define MARK_DOWN (0x40)

// =======================

namespace xio::raft {
    static const size_t SSL_GRACE_PERIOD_MS = 500;
    static const size_t SEND_RETRY_MS = 500;
    static const size_t SEND_RETRY_MAX = 6;

    xio_service::meta_cb_params req_to_params(req_msg *req, resp_msg *resp) {
        return xio_service::meta_cb_params
        ((int) req->get_type(),
         req->get_src(),
         req->get_dst(),
         req->get_term(),
         req->get_last_log_term(),
         req->get_last_log_idx(),
         req->get_commit_idx(),
         req,
         resp);
    }

    // === ASIO Abstraction ===
    //     (to switch SSL <-> unsecure on-the-fly)
    struct pending_req_pkg {
        pending_req_pkg(ptr<req_msg> &req,
                        rpc_handler &when_done,
                        uint64_t timeout_ms = 0)
            : req_(req)
              , when_done_(when_done)
              , timeout_ms_(timeout_ms) {
        }

        ptr<req_msg> req_;
        rpc_handler when_done_;
        uint64_t timeout_ms_;
    };

    // NOTE:
    //   When ssl is enabled, xio::async_write() and xio::async_read() on same
    //   xio::ssl::stream<xio::ip::tcp::socket&> object are not thread safe.
    //
    //   Without restriction, streaming mode can lead to concurrent async_write()/
    //   async_read() calls on same ssl stream object.
    //
    //   To avoid the concurrency, when ssl and steaming mode are both enabled,
    //   xio::strand<xio::io_context::executor_type> is used to serialize
    //   async_write() and async_read() calls on same ssl stream object.
    //
    //   Application has the right to decide whether use streaming mode when ssl is enabled.
    class aa {
    public:
        template<typename BB, typename FF, typename Strand = void>
        static void write(bool is_ssl,
                          ssl_socket &_ssl_socket,
                          xio::ip::tcp::socket &tcp_socket,
                          const BB &buffer,
                          FF func,
                          Strand *strand = nullptr) {
            if (is_ssl && strand) {
                xio::post(*strand, [&_ssl_socket, buffer, func, strand]() mutable {
                    xio::async_write(_ssl_socket, buffer,
                                     xio::bind_executor(*strand, func));
                });
            } else if (is_ssl) {
                xio::async_write(_ssl_socket, buffer, func);
            } else {
                xio::async_write(tcp_socket, buffer, func);
            }
        }

        template<typename BB, typename FF, typename Strand = void>
        static void read(bool is_ssl,
                         ssl_socket &_ssl_socket,
                         xio::ip::tcp::socket &tcp_socket,
                         const BB &buffer,
                         FF func,
                         Strand *strand = nullptr) {
            if (is_ssl && strand) {
                xio::post(*strand, [&_ssl_socket, buffer, func, strand]() mutable {
                    xio::async_read(_ssl_socket, buffer,
                                    xio::bind_executor(*strand, func));
                });
            } else if (is_ssl) {
                xio::async_read(_ssl_socket, buffer, func);
            } else {
                xio::async_read(tcp_socket, buffer, func);
            }
        }
    };

    // xio service implementation
    class xio_service_impl {
    public:
        xio_service_impl(const xio_service::options &_opt = xio_service::options());

        ~xio_service_impl();

        const xio_service::options &get_options() const { return my_opt_; }

        xio::io_service &get_io_svc() {
            if (my_opt_.custom_io_context_) {
                return *my_opt_.custom_io_context_;
            }
            return *io_svc_;
        }

        uint64_t assign_client_id() { return client_id_counter_.fetch_add(1); }

    private:
#ifndef SSL_LIBRARY_NOT_FOUND
        std::string get_password(std::size_t size,
                                 xio::ssl::context_base::password_purpose purpose);
#endif
        void stop();

        void worker_entry();

        void timer_handler(ERROR_CODE err);

    private:
        std::unique_ptr<xio::io_service> io_svc_;
        ssl_context ssl_server_ctx_;
        ssl_context ssl_client_ctx_;
        std::atomic_int continue_;
        std::atomic<uint8_t> stopping_status_;
        std::mutex stopping_lock_;
        std::condition_variable stopping_cv_;
        std::atomic<uint32_t> num_active_workers_;
        std::atomic<uint32_t> worker_id_;
        std::list<ptr<std::thread> > worker_handles_;
        xio_service::options my_opt_;
        xio::steady_timer xio_timer_;
        std::atomic<uint64_t> client_id_counter_;
        friend xio_service;
    };

    // rpc session
    class rpc_session;
    typedef std::function<void(const ptr<rpc_session> &)> session_closed_callback;

    class rpc_session
            : public std::enable_shared_from_this<rpc_session>
              , public raft_server_handler {
    public:
        rpc_session(uint64_t id,
                    xio_service_impl *_impl,
                    xio::io_service &io,
                    ssl_context &ssl_ctx,
                    bool _enable_ssl,
                    ptr<msg_handler> &handler,
                    session_closed_callback &callback)
            : session_id_(id)
              , impl_(_impl)
              , handler_(handler)
              , socket_(io)
              , ssl_socket_(socket_, ssl_ctx)
              , ssl_enabled_(_enable_ssl)
              , ssl_strand_(io.get_executor())
              , use_strand_(ssl_enabled_ && impl_->get_options().streaming_mode_)
              , flags_(0x0)
              , log_data_()
              , header_(buffer::alloc(RPC_REQ_HEADER_SIZE))
              , callback_(callback)
              , src_id_(-1)
              , is_leader_(false)
              , cached_port_(0)
              , crc_header_(0)
              , crc_from_msg_(0) {
            TLOG(TRACE, "xio rpc session created: {}. {}",
                 (const void*)this,
                 (ssl_enabled_ ? "SSL ENABLED" : "UNSECURED"));
        }

        __nocopy__(rpc_session);

    public:
        ~rpc_session() {
            close_socket();
            TLOG(TRACE, "xio rpc session destroyed: {}", (void*)this);
        }

    public:
        void prepare_handshake() {
            // this is safe since we only expose ctor to cs_new
            ptr<rpc_session> self = this->shared_from_this();

            cached_address_ = socket_.remote_endpoint().address().to_string();
            cached_port_ = socket_.remote_endpoint().port();
            TLOG(INFO, "session {}" " got connection from {}:{} (as a server)",
                 session_id_,
                 cached_address_.c_str(),
                 cached_port_);

            if (ssl_enabled_) {
#ifdef SSL_LIBRARY_NOT_FOUND
                assert(0); // Should not reach here.
#else
                ssl_socket_.async_handshake
                (xio::ssl::stream_base::server,
                 std::bind(&rpc_session::handle_handshake,
                           this,
                           self,
                           std::placeholders::_1));
#endif
            } else {
                this->start(self);
            }
        }

        void handle_handshake(ptr<rpc_session> self,
                              const ERROR_CODE &err) {
            if (!err) {
                TLOG(INFO, "session {}" " handshake with {}:{} succeeded (as a server)",
                     session_id_,
                     cached_address_.c_str(),
                     cached_port_);
                this->start(self);
            } else {
                TLOG(ERROR, "session {}" " handshake with {}:{} failed: error {}, {}",
                     session_id_,
                     cached_address_.c_str(),
                     cached_port_,
                     err.value(),
                     err.message().c_str());

                // Lazy stop.
                ptr<xio::steady_timer> timer =
                        cs_new<xio::steady_timer>(impl_->get_io_svc());
                timer->expires_after
                (std::chrono::duration_cast<std::chrono::nanoseconds>
                    (std::chrono::milliseconds(SSL_GRACE_PERIOD_MS)));
                timer->async_wait([self, this, timer]
            (const ERROR_CODE &err) -> void {
                        if (err) {
                            TLOG(ERROR, "session {}" " error happend during "
                                 "async wait: {}, {}",
                                 session_id_,
                                 err.value(),
                                 err.message().c_str());
                        }
                        this->stop();
                    });
            }
        }

        void start(ptr<rpc_session> self) {
            header_->pos(0);
            aa::read(ssl_enabled_, ssl_socket_, socket_,
                     xio::buffer(header_->data(), RPC_REQ_HEADER_SIZE),
                     [this, self]
             (const ERROR_CODE &err, size_t) -> void {
                         if (err) {
                             TLOG(ERROR, "session {}" " failed to read rpc header from socket {}:{} "
                                  "due to error {}, {}, ref count {}" "",
                                  session_id_,
                                  cached_address_.c_str(),
                                  cached_port_,
                                  err.value(),
                                  err.message().c_str(),
                                  self.use_count());
                             this->stop();
                             return;
                         }

                         // NOTE:
                         //  due to async_read() above, header_ size will be always
                         //  equal to or greater than RPC_REQ_HEADER_SIZE.

                         // Deprecate `buffer::get` and use `buffer_serializer`.

                         // header_->pos(0);
                         buffer_serializer h_bs(header_);
                         byte *header_data = header_->data_begin();
                         crc_header_ = crc32_8(header_data,
                                               RPC_REQ_HEADER_SIZE - CRC_FLAGS_LEN,
                                               0);

                         // header_->pos(RPC_REQ_HEADER_SIZE - CRC_FLAGS_LEN);
                         h_bs.pos(RPC_REQ_HEADER_SIZE - CRC_FLAGS_LEN);
                         // uint64_t flags_and_crc = header_->get_ulong();
                         uint64_t flags_and_crc = h_bs.get_u64();
                         crc_from_msg_ = flags_and_crc & (uint32_t) 0xffffffff;
                         flags_ = (flags_and_crc >> 32);

                         // Verify CRC (if entire message validation is disbaled).
                         if (!(flags_ & CRC_ON_ENTIRE_MESSAGE) &&
                             crc_header_ != crc_from_msg_) {
                             TLOG(ERROR, "header CRC mismatch: local calculation {}, from message {}",
                                  crc_header_, crc_from_msg_);

                             if (impl_->get_options().corrupted_msg_handler_) {
                                 impl_->get_options().corrupted_msg_handler_(header_, nullptr);
                             }

                             this->stop();
                             return;
                         }

                         // header_->pos(0);
                         // byte marker = header_->get_byte();
                         h_bs.pos(0);
                         byte marker = h_bs.get_u8();
                         if (marker != 0x0) {
                             // Means that this is not RPC_REQ, shouldn't happen.
                             TLOG(ERROR, "Wrong packet: expected REQ, got {}", marker);

                             if (impl_->get_options().corrupted_msg_handler_) {
                                 impl_->get_options().corrupted_msg_handler_(header_, nullptr);
                             }

                             this->stop();
                             return;
                         }

                         msg_type m_type = (msg_type) h_bs.get_u8();
                         if (!is_valid_msg(m_type)) {
                             TLOG(ERROR, "Wrong message type: got {}", (uint8_t)m_type);

                             if (impl_->get_options().corrupted_msg_handler_) {
                                 impl_->get_options().corrupted_msg_handler_(header_, nullptr);
                             }

                             this->stop();
                             return;
                         }

                         // header_->pos(RPC_REQ_HEADER_SIZE - CRC_FLAGS_LEN - DATA_SIZE_LEN);
                         // int32 data_size = header_->get_int();
                         h_bs.pos(RPC_REQ_HEADER_SIZE - CRC_FLAGS_LEN - DATA_SIZE_LEN);
                         int32 data_size = h_bs.get_i32();
                         // Up to 1GB.
                         if (data_size < 0 || data_size > 0x40000000) {
                             TLOG(ERROR, "bad log data size in the header {}, stop "
                                  "this session to protect further corruption",
                                  data_size);

                             if (impl_->get_options().corrupted_msg_handler_) {
                                 impl_->get_options().corrupted_msg_handler_(header_, nullptr);
                             }

                             this->stop();
                             return;
                         }

                         if (data_size == 0) {
                             // Don't carry data, immediately process request.
                             this->read_complete(header_, nullptr);
                         } else {
                             // Carry some data, need to read further.
                             ptr<buffer> log_ctx = buffer::alloc((size_t) data_size);
                             aa::read(ssl_enabled_, ssl_socket_, socket_,
                                      xio::buffer(log_ctx->data(),
                                                  (size_t) data_size),
                                      std::bind(&rpc_session::read_log_data,
                                                self,
                                                log_ctx,
                                                std::placeholders::_1,
                                                std::placeholders::_2),
                                      use_strand_ ? &ssl_strand_ : nullptr);
                         }
                     },
                     use_strand_ ? &ssl_strand_ : nullptr);
        }

        void stop() {
            invoke_connection_callback(false);
            close_socket();
            if (callback_) {
                callback_(this->shared_from_this());
            }
            handler_.reset();
        }

        ssl_socket::lowest_layer_type &socket() {
            return ssl_socket_.lowest_layer();
        }

    private:
        void invoke_connection_callback(bool is_open) {
            if (is_leader_ && src_id_ != handler_->get_leader()) {
                // Leader has been changed without closing session.
                is_leader_ = false;
            }

            cb_func::ConnectionArgs
                    args(session_id_,
                         cached_address_,
                         cached_port_,
                         src_id_,
                         is_leader_);
            cb_func::Param cb_param(handler_->get_id(),
                                    handler_->get_leader(),
                                    -1,
                                    &args);
            handler_->invoke_callback
            (is_open ? cb_func::ConnectionOpened : cb_func::ConnectionClosed,
             &cb_param);
        }

        void close_socket() {
            // MONSTOR-9378: Do nothing (the same as in `xio_rpc_client`),
            // early closing socket before destroying this instance
            // may cause problem, especially when SSL is enabled.
#if 0
            if (socket_.is_open()) {
                std::unique_lock<std::mutex> l(socket_lock_, std::try_to_lock);
                if (l.owns_lock() && socket_.is_open()) {
                    socket_.close();
                }
            }
#endif
        }

        void read_log_data(ptr<buffer> log_ctx,
                           const ERROR_CODE &err,
                           size_t bytes_read) {
            if (!err) {
                this->read_complete(header_, log_ctx);
            } else {
                TLOG(ERROR, "session {}" " failed to read rpc log data from socket due "
                     "to error {}, {}",
                     session_id_,
                     err.value(),
                     err.message().c_str());
                this->stop();
            }
        }

        void read_complete(ptr<buffer> hdr, ptr<buffer> log_ctx) {
            ptr<rpc_session> self = this->shared_from_this();

            try {
                // Deprecate `buffer::get` and use `buffer_serializer`.
                // hdr->pos(1);
                // msg_type t = (msg_type)hdr->get_byte();
                // int32 src = hdr->get_int();
                // int32 dst = hdr->get_int();
                // ulong term = hdr->get_ulong();
                // ulong last_term = hdr->get_ulong();
                // ulong last_idx = hdr->get_ulong();
                // ulong commit_idx = hdr->get_ulong();

                buffer_serializer h_bs(header_);
                h_bs.pos(1);
                msg_type t = (msg_type) h_bs.get_u8();
                int32 src = h_bs.get_i32();
                int32 dst = h_bs.get_i32();
                ulong term = h_bs.get_u64();
                ulong last_term = h_bs.get_u64();
                ulong last_idx = h_bs.get_u64();
                ulong commit_idx = h_bs.get_u64();
                int32 log_data_size = h_bs.get_i32();

                if (flags_ & CRC_ON_ENTIRE_MESSAGE) {
                    // Calculate the CRC of `log_ctx`.
                    uint32_t crc_payload =
                            log_ctx
                                ? crc32_8(log_ctx->data_begin(),
                                          log_ctx->size(),
                                          crc_header_)
                                : crc_header_;
                    if (crc_payload != crc_from_msg_) {
                        TLOG(ERROR, "request CRC mismatch: local calculation {}, from message {}",
                             crc_payload, crc_from_msg_);

                        if (impl_->get_options().corrupted_msg_handler_) {
                            impl_->get_options().corrupted_msg_handler_(header_, log_ctx);
                        }

                        this->stop();
                        return;
                    }
                }

                if (src_id_ == -1) {
                    // It means this is the first message on this session.
                    // Invoke callback function of new connection.
                    src_id_ = src;
                    invoke_connection_callback(true);
                } else if (is_leader_ && src_id_ != handler_->get_leader()) {
                    // Leader has been changed without closing session.
                    is_leader_ = false;
                }

                if (!is_leader_) {
                    // If leader flag is not set, we identify whether the endpoint
                    // server is leader based on the message type (only leader
                    // can send below message types).
                    if (t == msg_type::append_entries_request ||
                        t == msg_type::sync_log_request ||
                        t == msg_type::join_cluster_request ||
                        t == msg_type::leave_cluster_request ||
                        t == msg_type::install_snapshot_request ||
                        t == msg_type::priority_change_request ||
                        t == msg_type::custom_notification_request) {
                        is_leader_ = true;
                        cb_func::ConnectionArgs
                                args(session_id_,
                                     cached_address_,
                                     cached_port_,
                                     src_id_,
                                     is_leader_);
                        cb_func::Param cb_param(handler_->get_id(),
                                                handler_->get_leader(),
                                                -1,
                                                &args);
                        handler_->invoke_callback(cb_func::NewSessionFromLeader,
                                                  &cb_param);
                    }
                }

                std::string meta_str;
                ptr<req_msg> req = cs_new<req_msg>
                        (term, t, src, dst, last_term, last_idx, commit_idx);
                if (flags_ & MARK_DOWN) {
                    req->set_extra_flags(
                        req->get_extra_flags() | req_msg::EXCLUDED_FROM_THE_QUORUM);
                }

                if (log_data_size > 0 && log_ctx) {
                    buffer_serializer ss(log_ctx);
                    size_t log_ctx_size = log_ctx->size();

                    // If flag is set, read meta first.
                    if (flags_ & INCLUDE_META) {
                        size_t meta_len = 0;
                        const byte *meta_raw = (const byte *) ss.get_bytes(meta_len);
                        if (meta_len) {
                            meta_str = std::string((const char *) meta_raw, meta_len);
                        }
                    }

                    size_t LOG_ENTRY_SIZE = 8 + 1 + 4;
                    if (flags_ & INCLUDE_LOG_TIMESTAMP) {
                        LOG_ENTRY_SIZE += 8;
                    }
                    if (flags_ & CRC_ON_PAYLOAD) {
                        LOG_ENTRY_SIZE += 5;
                    }

                    while (log_ctx_size > ss.pos()) {
                        if (log_ctx_size - ss.pos() < LOG_ENTRY_SIZE) {
                            // Possibly corrupted packet. Stop here.
                            TLOG(WARNING, "wrong log ctx size {} pos {}, stop this session",
                                 log_ctx_size, ss.pos());

                            if (impl_->get_options().corrupted_msg_handler_) {
                                impl_->get_options().corrupted_msg_handler_(header_, log_ctx);
                            }

                            this->stop();
                            return;
                        }
                        ulong term = ss.get_u64();
                        log_val_type val_type = (log_val_type) ss.get_u8();
                        uint64_t timestamp = (flags_ & INCLUDE_LOG_TIMESTAMP) ? ss.get_u64() : 0;
                        bool has_crc32 = (flags_ & CRC_ON_PAYLOAD) ? (ss.get_u8() != 0) : false;
                        uint32_t crc32 = (flags_ & CRC_ON_PAYLOAD) ? ss.get_u32() : 0;

                        size_t val_size = ss.get_i32();
                        if (log_ctx_size - ss.pos() < val_size) {
                            // Out-of-bound size.
                            TLOG(WARNING, "wrong value size {} log ctx {} {}, "
                                 "stop this session",
                                 val_size, log_ctx_size, ss.pos());

                            if (impl_->get_options().corrupted_msg_handler_) {
                                impl_->get_options().corrupted_msg_handler_(header_, log_ctx);
                            }

                            this->stop();
                            return;
                        }

                        ptr<buffer> buf(buffer::alloc(val_size));
                        ss.get_buffer(buf);
                        ptr<log_entry> entry(
                            cs_new<log_entry>(term, buf, val_type, timestamp, has_crc32, crc32, false));

                        if ((flags_ & CRC_ON_PAYLOAD) && has_crc32) {
                            // Verify CRC.
                            uint32_t crc_payload = crc32_8(buf->data_begin(),
                                                           buf->size(),
                                                           0);
                            if (crc_payload != crc32) {
                                TLOG(ERROR, "log entry CRC mismatch: local calculation {}, "
                                     "from message {}", crc_payload, crc32);

                                if (impl_->get_options().corrupted_msg_handler_) {
                                    impl_->get_options().corrupted_msg_handler_(header_, log_ctx);
                                }

                                this->stop();
                                return;
                            }
                        }

                        req->log_entries().push_back(entry);
                    }
                }

                // If callback is given, verify meta
                // (if meta is empty, invoke callback according to the flag).
                if (impl_->get_options().read_req_meta_ &&
                    (!meta_str.empty() ||
                     impl_->get_options().invoke_req_cb_on_empty_meta_)) {
                    if (!impl_->get_options().read_req_meta_
                        (req_to_params(req.get(), nullptr), meta_str)) {
                        this->stop();
                        return;
                    }
                }

                // === RAFT server processes the request here. ===
                ptr<resp_msg> resp = raft_server_handler::process_req(handler_.get(), *req);
                if (!resp) {
                    TLOG(WARNING, "no response is returned from raft message handler");
                    this->stop();
                    return;
                }

                if (resp->has_async_cb()) {
                    // Response will be ready later, setup a callback function
                    // (only for auto-forwarding with `client_request` type
                    //  in async handling mode).
                    ptr<cmd_result<ptr<buffer> > > ret = resp->call_async_cb();

                    // WARNING: `self` should be captured to avoid releasing this `rpc_session`.
                    ret->when_ready(
                        [this, self, req, resp]
                (cmd_result<ptr<buffer>, ptr<std::exception> > &res,
                 ptr<std::exception> &exp) {
                            resp->set_ctx(res.get());
                            on_resp_ready(req, resp);
                            // This is needed to avoid circular reference.
                            res.reset();
                        }
                    );
                } else {
                    // Response should already be ready when we reach here.
                    if (resp->has_cb()) {
                        // If callback function exists, get new response message.
                        resp = resp->call_cb(resp);
                    }
                    on_resp_ready(req, resp);
                }
            } catch (std::exception &ex) {
                TLOG(ERROR, "session {}" " failed to process request message "
                     "due to error: {}",
                     this->session_id_,
                     ex.what());
                this->stop();
            }
        }

        void on_resp_ready(ptr<req_msg> req, ptr<resp_msg> resp) {
            ptr<rpc_session> self = this->shared_from_this();

            try {
                ptr<buffer> resp_ctx = resp->get_ctx();
                int32 resp_ctx_size = (resp_ctx) ? resp_ctx->size() : 0;
                int32 result_code_size = sizeof(int32_t);

                uint32_t flags = 0x0;
                size_t resp_meta_size = 0;
                std::string resp_meta_str;
                if (impl_->get_options().write_resp_meta_) {
                    resp_meta_str = impl_->get_options().write_resp_meta_
                            (req_to_params(req.get(), resp.get()));
                    if (!resp_meta_str.empty()) {
                        // Meta callback for response is given, set the flag.
                        flags |= INCLUDE_META;
                        resp_meta_size = sizeof(int32) + resp_meta_str.size();
                    }
                }

                size_t resp_hint_size = 0;
                if (resp->get_next_batch_size_hint_in_bytes()) {
                    // Hint is given, set the flag.
                    flags |= INCLUDE_HINT;
                    // For future extension, we will put 2-byte version and 2-byte length.
                    resp_hint_size += sizeof(uint16_t) * 2 + sizeof(int64);
                }

                if (resp->get_extra_flags() & resp_msg::SELF_MARK_DOWN) {
                    // The follower marks itself down.
                    flags |= MARK_DOWN;
                }

                size_t carried_data_size = resp_meta_size + resp_hint_size + resp_ctx_size;

                if (req->get_type() == msg_type::client_request ||
                    req->get_type() == msg_type::add_server_request ||
                    req->get_type() == msg_type::remove_server_request) {
                    flags |= INCLUDE_RESULT_CODE;
                    carried_data_size += result_code_size;
                }

                int buf_size = RPC_RESP_HEADER_SIZE + carried_data_size;
                ptr<buffer> resp_buf = buffer::alloc(buf_size);
                buffer_serializer bs(resp_buf);

                const byte RESP_MARKER = 0x1;
                bs.put_u8(RESP_MARKER);
                bs.put_u8(resp->get_type());
                bs.put_i32(resp->get_src());
                bs.put_i32(resp->get_dst());
                bs.put_u64(resp->get_term());
                bs.put_u64(resp->get_next_idx());
                bs.put_u8(resp->get_accepted());
                bs.put_i32(carried_data_size);

                // Calculate CRC32 on header only.
                uint32_t crc_val = crc32_8(resp_buf->data_begin(),
                                           RPC_RESP_HEADER_SIZE - CRC_FLAGS_LEN,
                                           0);

                uint64_t flags_crc = ((uint64_t) flags << 32) | crc_val;
                bs.put_u64(flags_crc);

                // Handling meta if the flag is set.
                if (flags & INCLUDE_META) {
                    bs.put_str(resp_meta_str);
                }
                // Put hint if the flag is set.
                if (flags & INCLUDE_HINT) {
                    const uint16_t CUR_HINT_VERSION = 0;
                    bs.put_u16(CUR_HINT_VERSION);
                    bs.put_u16(sizeof(ulong));
                    bs.put_i64(resp->get_next_batch_size_hint_in_bytes());
                }

                if (resp_ctx_size) {
                    resp_ctx->pos(0);
                    bs.put_buffer(*resp_ctx);
                }

                /* Put result code at the end to avoid breaking backward compatibility */
                if (flags & INCLUDE_RESULT_CODE) {
                    bs.put_i32(resp->get_result_code());
                }

                aa::write(ssl_enabled_, ssl_socket_, socket_,
                          xio::buffer(resp_buf->data_begin(), resp_buf->size()),
                          [this, self, resp_buf]
                  (ERROR_CODE err_code, size_t) -> void {
                              // To avoid releasing `resp_buf` before the write is done.
                              (void) resp_buf;
                              if (!err_code) {
                                  this->start(self);
                              } else {
                                  TLOG(ERROR, "session {}" " failed to send response to peer due "
                                       "to error {}, reason: {}",
                                       session_id_,
                                       err_code.value(),
                                       err_code.message().c_str());
                                  this->stop();
                              }
                          },
                          use_strand_ ? &ssl_strand_ : nullptr);
            } catch (std::exception &ex) {
                TLOG(ERROR, "session {}" " failed to process request message "
                     "due to error: {}",
                     this->session_id_,
                     ex.what());
                this->stop();
            }
        }

    private:
        uint64_t session_id_;
        xio_service_impl *impl_;
        ptr<msg_handler> handler_;
        xio::ip::tcp::socket socket_;
        ssl_socket ssl_socket_;
        bool ssl_enabled_;
        xio::strand<xio::io_context::executor_type> ssl_strand_;
        bool use_strand_;
        uint32_t flags_;
        ptr<buffer> log_data_;
        ptr<buffer> header_;
        session_closed_callback callback_;

        /**
         * Source server (endpoint) ID, used to check whether it is leader.
         * This value is `-1` at the beginning, which denotes this session
         * hasn't received any message from the endpoint.
         * Note that this ID should not be changed throughout the life time
         * of the session.
         */
        int32 src_id_;

        /**
         * `true` if the endpoint server was leader when it was last seen.
         */
        bool is_leader_;

        std::string cached_address_;
        uint32_t cached_port_;

        /**
         * Locally calculated CRC number of the request header.
         */
        uint32_t crc_header_;

        /**
         * CRC number from the request header.
         */
        uint32_t crc_from_msg_;
    };

    // rpc listener implementation
    class xio_rpc_listener
            : public rpc_listener
              , public std::enable_shared_from_this<xio_rpc_listener> {
    public:
        xio_rpc_listener(xio_service_impl *_impl,
                         xio::io_service &io,
                         ssl_context &ssl_ctx,
                         ushort port,
                         bool _enable_ssl)
            : impl_(_impl)
              , io_svc_(io)
              , ssl_ctx_(ssl_ctx)
              , handler_()
              , stopped_(false)
              , acceptor_(io, xio::ip::tcp::endpoint(xio::ip::tcp::v4(), port))
              , session_id_cnt_(1)
              , ssl_enabled_(_enable_ssl) {
            TLOG(INFO, "Raft ASIO listener initiated, {}",
                 ssl_enabled_ ? "SSL ENABLED" : "UNSECURED");
        }

        __nocopy__(xio_rpc_listener);

    public:
        virtual void stop() override {
            auto_lock(listener_lock_);
            stopped_ = true;
            acceptor_.close();
        }

        virtual void listen(ptr<msg_handler> &handler) override {
            std::lock_guard<std::mutex> guard(listener_lock_);
            handler_ = handler;
            stopped_ = false;
            start(guard);
        }

        virtual void shutdown() override {
            {
                auto_lock(session_lock_);
                for (auto &entry: active_sessions_) {
                    ptr<rpc_session> s = entry;
                    s->stop();
                    s.reset();
                }
                active_sessions_.clear();
            }

            auto_lock(listener_lock_);
            handler_.reset();
        }

    private:
        void start(std::lock_guard<std::mutex> &listener_lock) {
            if (!acceptor_.is_open()) {
                return;
            }

            ptr<xio_rpc_listener> self(this->shared_from_this());
            session_closed_callback cb =
                    std::bind(&xio_rpc_listener::remove_session,
                              self,
                              std::placeholders::_1);

            ptr<rpc_session> session =
                    cs_new<rpc_session>
                    (session_id_cnt_.fetch_add(1),
                     impl_, io_svc_, ssl_ctx_, ssl_enabled_,
                     handler_, cb);

            acceptor_.async_accept(session->socket(),
                                   std::bind(&xio_rpc_listener::handle_accept,
                                             this,
                                             self,
                                             session,
                                             std::placeholders::_1));
        }

        void handle_accept(ptr<xio_rpc_listener> self,
                           ptr<rpc_session> session,
                           const ERROR_CODE &err) {
            if (!err) {
                xio::ip::tcp::no_delay option(true);
                session->socket().set_option(option);
                TLOG(INFO, "receive a incoming rpc connection");
                session->prepare_handshake();
            } else {
                TLOG(ERROR, "failed to accept a rpc connection due to error {}, {}",
                     err.value(), err.message().c_str());
            }

            std::lock_guard<std::mutex> guard(listener_lock_);
            if (!stopped_) {
                // Re-listen only when not stopped,
                // otherwise crash happens as this class or `acceptor_`
                // may be destroyed in the meantime.
                this->start(guard);
            }
        }

        void remove_session(const ptr<rpc_session> &session) {
            auto_lock(session_lock_);

            for (auto it = active_sessions_.begin();
                 it != active_sessions_.end(); ++it) {
                if (*it == session) {
                    active_sessions_.erase(it);
                    break;
                }
            }
        }

    private:
        xio_service_impl *impl_;
        xio::io_service &io_svc_;
        ssl_context &ssl_ctx_;

        std::mutex listener_lock_;
        ptr<msg_handler> handler_;
        bool stopped_;
        xio::ip::tcp::acceptor acceptor_;

        std::vector<ptr<rpc_session> > active_sessions_;
        std::atomic<uint64_t> session_id_cnt_;
        std::mutex session_lock_;
        bool ssl_enabled_;
    };

    class xio_rpc_client
            : public rpc_client
              , public std::enable_shared_from_this<xio_rpc_client> {
    public:
        xio_rpc_client(xio_service_impl *_impl,
                       xio::io_service &io_svc,
                       ssl_context &ssl_ctx,
                       std::string &host,
                       std::string &port,
                       bool ssl_enabled)
            : impl_(_impl)
              , resolver_(io_svc)
              , socket_(io_svc)
              , ssl_socket_(socket_, ssl_ctx)
              , attempting_conn_(false)
              , host_(host)
              , port_(port)
              , ssl_enabled_(ssl_enabled)
              , ssl_ready_(false)
              , ssl_strand_(io_svc.get_executor())
              , use_strand_(ssl_enabled_ && impl_->get_options().streaming_mode_)
              , num_send_fails_(0)
              , abandoned_(false)
              , socket_busy_(false)
              , client_id_(impl_->assign_client_id())
              , operation_timer_(io_svc) {
            if (ssl_enabled_) {
#ifdef SSL_LIBRARY_NOT_FOUND
                assert(0); // Should not reach here.
#else
                if (_impl->get_options().skip_verification_) {
                    ssl_socket_.set_verify_mode(xio::ssl::verify_none);
                } else {
                    ssl_socket_.set_verify_mode(xio::ssl::verify_peer);
                }

                ssl_socket_.set_verify_callback
                (std::bind(&xio_rpc_client::verify_certificate,
                           this,
                           std::placeholders::_1,
                           std::placeholders::_2));
#endif
            }
            TLOG(TRACE, "xio client created: {}. {}",
                 (void*)this,
                 ssl_enabled_ ? "SSL ENABLED" : "UNSECURED");
        }

        virtual ~xio_rpc_client() {
            TLOG(TRACE, "xio client destroyed: {}", (void*)this);
            close_socket();
        }

    public:
        uint64_t get_id() const override {
            return client_id_;
        }

        bool is_abandoned() const override {
            return abandoned_;
        }

#ifndef SSL_LIBRARY_NOT_FOUND
        bool verify_certificate(bool preverified,
                                xio::ssl::verify_context &ctx) {
            if (impl_->get_options().verify_sn_) {
                char subject_name[256];
                X509 *cert = X509_STORE_CTX_get_current_cert(ctx.native_handle());
                X509_NAME_oneline(X509_get_subject_name(cert), subject_name, 256);
                TLOG(DEBUG, "given subject: {}", subject_name);
                if (!impl_->get_options().verify_sn_(subject_name)) {
                    return false;
                }
            }
            return preverified;
        }
#endif

        ssl_socket::lowest_layer_type &socket() {
            return ssl_socket_.lowest_layer();
        }

        void send_retry(ptr<xio_rpc_client> self,
                        ptr<xio::steady_timer> timer,
                        ptr<req_msg> &req,
                        rpc_handler &when_done,
                        uint64_t send_timeout_ms,
                        const ERROR_CODE &err) {
            if (err || num_send_fails_ >= SEND_RETRY_MAX) {
                if (err) {
                    TLOG(ERROR, "error happened during async wait: {}", err.value());
                } else {
                    TLOG(ERROR, "connection to {}:{} timeout (SSL {})",
                         host_.c_str(), port_.c_str(),
                         ( ssl_enabled_ ? "enabled" : "disabled" ));
                }
                abandoned_ = true;
                std::string err_msg =
                        lstrfmt("timeout while connecting to %s").fmt(host_.c_str());
                handle_error(req, err_msg, when_done);
                return;
            }
            register_req_send(req, when_done, send_timeout_ms);
        }

        virtual void send(ptr<req_msg> &req,
                          rpc_handler &when_done,
                          uint64_t send_timeout_ms = 0) __override__ {
            if (impl_->get_options().streaming_mode_) {
                pre_send(req, when_done, send_timeout_ms);
            } else {
                register_req_send(req, when_done, send_timeout_ms);
            }
        }

        void handle_error(ptr<req_msg> req,
                          std::string &err_msg,
                          rpc_handler when_done) {
            close_socket(err_msg);

            // In streaming mode, all `when_done` will be invoked in `close_socket()`.
            // Otherwise, `close_socket()` will do nothing, hence `when_done`
            // should directly be invoked here.
            if (!impl_->get_options().streaming_mode_) {
                ptr<resp_msg> resp;
                ptr<rpc_exception> except(cs_new<rpc_exception>(err_msg, req));
                when_done(resp, except);
            }
        }

        void pre_send(ptr<req_msg> &req,
                      rpc_handler &when_done,
                      uint64_t send_timeout_ms) {
            bool immediate_action_needed = false;
            {
                auto_lock(pending_write_reqs_lock_);
                pending_write_reqs_.push_back(
                    cs_new<pending_req_pkg>(req, when_done, send_timeout_ms));
                immediate_action_needed = (pending_write_reqs_.size() == 1);
                TLOG(DEBUG, "start to send msg to peer {}, start_log_idx: {}" ", "
                     "size: {}" ", pending write reqs: {}" "",
                     req->get_dst(), req->get_last_log_idx(),
                     req->log_entries().size(), pending_write_reqs_.size());
            }

            if (immediate_action_needed) {
                register_req_send(req, when_done, send_timeout_ms);
            }
        }

        void register_req_send(ptr<req_msg> &req,
                               rpc_handler &when_done,
                               uint64_t send_timeout_ms) {
            if (abandoned_) {
                TLOG(ERROR, "client {} to {}:{} is already stale (SSL {})",
                     (void*)this, host_.c_str(), port_.c_str(),
                     ( ssl_enabled_ ? "enabled" : "disabled" ));
                std::string err_msg = lstrfmt("abandoned client to %s").fmt(host_.c_str());
                handle_error(req, err_msg, when_done);
                return;
            }

            ptr<xio_rpc_client> self = this->shared_from_this();
            while (!socket().is_open()) {
                // Dummy one-time loop
                TLOG(DEBUG, "socket {} to {}:{} is not opened yet",
                     (void*)this, host_.c_str(), port_.c_str());

                // WARNING:
                //   Only one thread can establish connection at a time.
                //   Since we don't re-use RPC client upon connection failure,
                //   this flag will never be cleared.
                bool exp = false;
                bool desired = true;
                if (!attempting_conn_.compare_exchange_strong(exp, desired)) {
                    // Other thread is attempting connection, just wait.
                    TLOG(WARNING, "cannot send req as other thread is racing on opening "
                         "connection to ({}:{}), count {}",
                         host_.c_str(), port_.c_str(), num_send_fails_.load());
                    num_send_fails_.fetch_add(1);

                    ptr<xio::steady_timer> timer =
                            cs_new<xio::steady_timer>(impl_->get_io_svc());
                    timer->expires_after
                    (std::chrono::duration_cast<std::chrono::nanoseconds>
                        (std::chrono::milliseconds(SEND_RETRY_MS)));
                    timer->async_wait(std::bind(&xio_rpc_client::send_retry,
                                                this,
                                                self,
                                                timer,
                                                req,
                                                when_done,
                                                send_timeout_ms,
                                                std::placeholders::_1));
                    return;
                }

                if (socket().is_open()) {
                    // Already opened, skip async_connect.
                    TLOG(WARNING, "race: socket to {}:{} is already opened, escape",
                         host_.c_str(), port_.c_str());
                    break;
                }

                if (impl_->get_options().custom_resolver_) {
                    impl_->get_options().custom_resolver_(
                        host_,
                        port_,
                        [this, self, req, when_done, send_timeout_ms]
                (const std::string &resolved_host,
                 const std::string &resolved_port,
                 std::error_code err) {
                            if (!err) {
                                TLOG(INFO, "custom resolver: {}:{} to {}:{}",
                                     host_.c_str(), port_.c_str(),
                                     resolved_host.c_str(), resolved_port.c_str());
                                execute_resolver(self, req, resolved_host, resolved_port,
                                                 when_done, send_timeout_ms);
                            } else {
                                std::string err_msg = lstrfmt("failed to resolve "
                                            "host %s by given "
                                            "custom resolver "
                                            "due to error %d, %s")
                                        .fmt(host_.c_str(),
                                             err.value(),
                                             err.message().c_str());
                                handle_error(req, err_msg, when_done);
                            }
                        });
                } else {
                    execute_resolver(self, req, host_, port_, when_done, send_timeout_ms);
                }
                return;
            }

            if (ssl_enabled_ && !ssl_ready_) {
                // TCP socket is opened, but SSL handshake is not done yet.
                // Since other thread is doing it, this thread should just wait.
                TLOG(WARNING, "cannot send req as SSL is not ready yet ({}:{}), count {}",
                     host_.c_str(), port_.c_str(), num_send_fails_.load());
                num_send_fails_.fetch_add(1);

                ptr<xio::steady_timer> timer =
                        cs_new<xio::steady_timer>(impl_->get_io_svc());
                timer->expires_after
                (std::chrono::duration_cast<std::chrono::nanoseconds>
                    (std::chrono::milliseconds(SEND_RETRY_MS)));
                timer->async_wait(std::bind(&xio_rpc_client::send_retry,
                                            this,
                                            self,
                                            timer,
                                            req,
                                            when_done,
                                            send_timeout_ms,
                                            std::placeholders::_1));
                return;
            }

            // Socket should be idle now. If not, it should be a bug.
            set_busy_flag(true);

            // If we reach here, that means connection is valid.
            // Reset the counter.
            num_send_fails_ = 0;

            // serialize req, send and read response
            std::vector<ptr<buffer> > log_entry_bufs;
            int32 log_data_size(0);

            uint32_t flags = 0x0;
            size_t LOG_ENTRY_SIZE = 8 + 1 + 4;
            if (impl_->get_options().replicate_log_timestamp_) {
                LOG_ENTRY_SIZE += 8;
                flags |= INCLUDE_LOG_TIMESTAMP;
            }
            if (impl_->get_options().crc_on_payload_) {
                LOG_ENTRY_SIZE += 5;
                flags |= CRC_ON_PAYLOAD;
            }

            if (req->get_extra_flags() & req_msg::EXCLUDED_FROM_THE_QUORUM) {
                // If the request is excluded from the quorum, set the flag.
                flags |= MARK_DOWN;
            }

            for (auto &entry: req->log_entries()) {
                ptr<log_entry> &le = entry;
                ptr<buffer> entry_buf = buffer::alloc
                        (LOG_ENTRY_SIZE + le->get_buf().size());
#if 0
                entry_buf->put(le->get_term());
                entry_buf->put((byte) le->get_val_type());
                entry_buf->put((int32) le->get_buf().size());
                le->get_buf().pos(0);
                entry_buf->put(le->get_buf());
                entry_buf->pos(0);
#else
                buffer_serializer ss(entry_buf);
                ss.put_u64(le->get_term());
                ss.put_u8(le->get_val_type());
                if (impl_->get_options().replicate_log_timestamp_) {
                    ss.put_u64(le->get_timestamp());
                }
                if (impl_->get_options().crc_on_payload_) {
                    ss.put_u8(le->has_crc32() ? 1 : 0);
                    ss.put_u32(le->get_crc32());
                }
                ss.put_i32(le->get_buf().size());
                ss.put_raw(le->get_buf().data_begin(), le->get_buf().size());
#endif
                log_entry_bufs.push_back(entry_buf);
                log_data_size += (int32) entry_buf->size();
            }

            size_t meta_size = 0;
            std::string meta_str;
            if (impl_->get_options().write_req_meta_) {
                meta_str = impl_->get_options().write_req_meta_
                        (req_to_params(req.get(), nullptr));
                if (!meta_str.empty()) {
                    // If callback for meta is given, set flag.
                    flags |= INCLUDE_META;
                    meta_size = sizeof(int32) + meta_str.size();
                }
            }

            ptr<buffer> req_buf =
                    buffer::alloc(RPC_REQ_HEADER_SIZE + meta_size + log_data_size);

            // Deprecate `buffer::put` and use `buffer_serializer`.

            // req_buf->pos(0);
            // byte* req_buf_data = req_buf->data();

            // byte marker = 0x0;
            // req_buf->put(marker);
            // req_buf->put((byte)req->get_type());
            // req_buf->put(req->get_src());
            // req_buf->put(req->get_dst());
            // req_buf->put(req->get_term());
            // req_buf->put(req->get_last_log_term());
            // req_buf->put(req->get_last_log_idx());
            // req_buf->put(req->get_commit_idx());
            // req_buf->put((int32)meta_size + log_data_size);

            buffer_serializer req_buf_bs(req_buf);
            req_buf_bs.put_u8(0x0);
            req_buf_bs.put_u8((byte) req->get_type());
            req_buf_bs.put_i32(req->get_src());
            req_buf_bs.put_i32(req->get_dst());
            req_buf_bs.put_u64(req->get_term());
            req_buf_bs.put_u64(req->get_last_log_term());
            req_buf_bs.put_u64(req->get_last_log_idx());
            req_buf_bs.put_u64(req->get_commit_idx());
            req_buf_bs.put_i32((int32) meta_size + log_data_size);

            // Calculate CRC32 on header-only.
            uint32_t crc_header = crc32_8(req_buf->data_begin(),
                                          RPC_REQ_HEADER_SIZE - CRC_FLAGS_LEN,
                                          0);

            uint64_t flags_and_crc = ((uint64_t) flags << 32) | crc_header;
            // req_buf->put((ulong)flags_and_crc);
            size_t crc_pos = req_buf_bs.pos();
            req_buf_bs.put_u64(flags_and_crc);

            // From now on, it will contain the payload (== meta + log entries).
            // byte* req_buf_payload = req_buf->data();
            size_t payload_pos = req_buf_bs.pos();

            // Handling meta if the flag is set.
            if (flags & INCLUDE_META) {
                // req_buf->put( (byte*)meta_str.data(), meta_str.size() );
                req_buf_bs.put_bytes((byte *) meta_str.data(), meta_str.size());
            }

            for (auto &it: log_entry_bufs) {
                // req_buf->put(*(it));
                req_buf_bs.put_buffer(*(it));
            }
            // req_buf->pos(0);

            if (impl_->get_options().crc_on_entire_message_) {
                uint32_t crc_payload = crc32_8(req_buf->data_begin() + payload_pos,
                                               meta_size + log_data_size,
                                               crc_header);
                // Overwrite CRC field.
                flags |= CRC_ON_ENTIRE_MESSAGE;
                flags_and_crc = ((uint64_t) flags << 32) | crc_payload;
                req_buf_bs.pos(crc_pos);
                req_buf_bs.put_u64(flags_and_crc);
            }

            if (send_timeout_ms != 0) {
                operation_timer_.expires_after
                (std::chrono::duration_cast<std::chrono::nanoseconds>
                    (std::chrono::milliseconds(send_timeout_ms)));
                operation_timer_.async_wait(std::bind(&xio_rpc_client::cancel_socket,
                                                      this,
                                                      std::placeholders::_1));
            }


            // Note: without passing `req_buf` to callback function, it will be
            //       unreachable before the write is done so that it is freed
            //       and the memory corruption will occur.
            aa::write(ssl_enabled_, ssl_socket_, socket_,
                      xio::buffer(req_buf->data(), req_buf->size()),
                      std::bind(&xio_rpc_client::sent,
                                self,
                                req,
                                req_buf,
                                when_done,
                                std::placeholders::_1,
                                std::placeholders::_2),
                      use_strand_ ? &ssl_strand_ : nullptr);
        }

    private:
        void execute_resolver(ptr<xio_rpc_client> self,
                              ptr<req_msg> req,
                              const std::string &host,
                              const std::string &port,
                              rpc_handler when_done,
                              uint64_t send_timeout_ms) {
            resolver_.async_resolve
            (
                host,
                port,
                xio::ip::tcp::resolver::flags::all_matching,
                [self, this, req, when_done, host, port, send_timeout_ms](
            const std::error_code &err,
            xio::ip::tcp::resolver::results_type endpoints) -> void {
                    if (!err) {
                        if (send_timeout_ms != 0) {
                            operation_timer_.expires_after
                            (std::chrono::duration_cast<std::chrono::nanoseconds>
                                (std::chrono::milliseconds(send_timeout_ms)));
                            operation_timer_.async_wait(
                                std::bind(&xio_rpc_client::cancel_socket,
                                          this,
                                          std::placeholders::_1));
                        }
                        xio::async_connect
                        (socket(),
                         endpoints,
                         std::bind(&xio_rpc_client::connected,
                                   self,
                                   req,
                                   when_done,
                                   send_timeout_ms,
                                   std::placeholders::_1,
                                   std::placeholders::_2));
                    } else {
                        std::string err_msg = lstrfmt("failed to resolve host %s "
                                    "due to error %d, %s")
                                .fmt(host.c_str(),
                                     err.value(),
                                     err.message().c_str());
                        handle_error(req, err_msg, when_done);
                    }
                });
        }

        void set_busy_flag(bool to) {
            if (impl_->get_options().streaming_mode_) {
                return;
            }

            if (to == true) {
                bool exp = false;
                if (!socket_busy_.compare_exchange_strong(exp, true)) {
                    TLOG(FATAL, "socket {} is already in use, race happened on connection to {}:{}",
                         (void*)this, host_.c_str(), port_.c_str());
                    assert(0);
                }
            } else {
                bool exp = true;
                if (!socket_busy_.compare_exchange_strong(exp, false)) {
                    TLOG(FATAL, "socket {} is already idle, race happened on connection to {}:{}",
                         (void*)this, host_.c_str(), port_.c_str());
                    assert(0);
                }
            }
        }

        void close_socket(std::string err_msg = std::string()) {
            // Do nothing,
            // early closing socket before destroying this instance
            // may cause problem, especially when SSL is enabled.
#if 0
            if (socket().is_open()) {
                std::unique_lock<std::mutex> l(socket_lock_, std::try_to_lock);
                if (l.owns_lock() && socket().is_open()) {
                    socket().close();
                    TLOG(DEBUG, "socket to {}:{} closed", host_.c_str(), port_.c_str());
                } else {
                    TLOG(WARNING, "race: socket to {}:{} is already closed",
                         host_.c_str(), port_.c_str());
                }
            }
#endif
            if (!impl_->get_options().streaming_mode_) {
                return;
            }

            // In streaming mode, it should invoke all `when_done` callbacks,
            // in chronological order.

            // clear write queue and read queue here
            // from oldest to latest, read queue first
            cancel_pending_requests(pending_read_reqs_, pending_read_reqs_lock_, err_msg);
            cancel_pending_requests(pending_write_reqs_, pending_write_reqs_lock_, err_msg);
        }

        void cancel_pending_requests(std::list<ptr<pending_req_pkg> > &reqs_list,
                                     std::mutex &lock,
                                     std::string &err_msg) {
            std::list<ptr<pending_req_pkg> > reqs_to_cancel;
            {
                auto_lock(lock);
                reqs_to_cancel = std::move(reqs_list);
            }

            for (auto &pkg: reqs_to_cancel) {
                ptr<resp_msg> rsp;
                if (err_msg.empty()) {
                    err_msg = lstrfmt("socket to host %s is closed").fmt(host_.c_str());
                }
                ptr<rpc_exception> except(cs_new<rpc_exception>(err_msg, pkg->req_));
                pkg->when_done_(rsp, except);
            }
            reqs_to_cancel.clear();
        }

        void cancel_socket(const ERROR_CODE &err) {
            if (err) {
                // Timer was cancelled itself, it's OK.
                return;
            }

            if (socket().is_open()) {
                TLOG(WARNING, "cancelling operations due to socket ({}:{}) timeout",
                     host_.c_str(), port_.c_str());
                abandoned_ = true;
                socket_.cancel();
            }
        }

        void connected(ptr<req_msg> &req,
                       rpc_handler &when_done,
                       uint64_t send_timeout_ms,
                       const std::error_code &err,
                       const xio::ip::tcp::endpoint &) {
            operation_timer_.cancel();
            if (!err) {
                xio::ip::tcp::no_delay option(true);
                socket().set_option(option);
                TLOG(INFO, "{} connected to {}:{} (as a client)",
                     (void*)this, host_.c_str(), port_.c_str());
                if (ssl_enabled_) {
#ifdef SSL_LIBRARY_NOT_FOUND
                    assert(0); // Should not reach here.
#else
                    ssl_socket_.async_handshake
                    (xio::ssl::stream_base::client,
                     std::bind(&xio_rpc_client::handle_handshake,
                               this,
                               req,
                               when_done,
                               send_timeout_ms,
                               std::placeholders::_1));
#endif
                } else {
                    this->register_req_send(req, when_done, send_timeout_ms);
                }
            } else {
                abandoned_ = true;
                std::string err_msg = sstrfmt("failed to connect to "
                            "peer %d, %s:%s, error %d, %s")
                        .fmt(req->get_dst(), host_.c_str(),
                             port_.c_str(), err.value(),
                             err.message().c_str());
                handle_error(req, err_msg, when_done);
            }
        }

        void handle_handshake(ptr<req_msg> &req,
                              rpc_handler &when_done,
                              uint64_t send_timeout_ms,
                              const ERROR_CODE &err) {
            ptr<xio_rpc_client> self = this->shared_from_this();

            if (!err) {
                TLOG(INFO, "handshake with {}:{} succeeded (as a client)",
                     host_.c_str(), port_.c_str());
                ssl_ready_ = true;
                this->register_req_send(req, when_done, send_timeout_ms);
            } else {
                abandoned_ = true;
                TLOG(ERROR, "failed SSL handshake with peer {}, {}:{}, error {}, {}",
                     req->get_dst(), host_.c_str(), port_.c_str(), err.value(),
                     err.message().c_str());

                // Immediately stop.
                std::string err_msg = sstrfmt("failed SSL handshake with peer %d, %s:%s, "
                            "error %d, %s")
                        .fmt(req->get_dst(), host_.c_str(),
                             port_.c_str(), err.value(),
                             err.message().c_str());
                handle_error(req, err_msg, when_done);
            }
        }

        void sent(ptr<req_msg> &req,
                  ptr<buffer> &buf,
                  rpc_handler &when_done,
                  std::error_code err,
                  size_t bytes_transferred) {
            // Now we can safely free the `req_buf`.
            (void) buf;
            if (!err) {
                if (impl_->get_options().streaming_mode_) {
                    post_send(req, when_done);
                } else {
                    register_response_read(req, when_done);
                }
            } else {
                operation_timer_.cancel();
                abandoned_ = true;
                std::string err_msg = sstrfmt("failed to send request to peer %d, %s:%s, "
                            "error %d, %s")
                        .fmt(req->get_dst(), host_.c_str(),
                             port_.c_str(), err.value(),
                             err.message().c_str());
                handle_error(req, err_msg, when_done);
            }
        }

        void response_read(ptr<req_msg> &req,
                           rpc_handler &when_done,
                           ptr<buffer> &resp_buf,
                           std::error_code err,
                           size_t bytes_transferred) {
            ptr<xio_rpc_client> self(this->shared_from_this());
            if (err) {
                abandoned_ = true;
                std::string err_msg = sstrfmt("failed to read response to peer %d, %s:%s, "
                            "error %d, %s")
                        .fmt(req->get_dst(), host_.c_str(),
                             port_.c_str(), err.value(),
                             err.message().c_str());
                handle_error(req, err_msg, when_done);
                return;
            }

            buffer_serializer bs(resp_buf);
            uint32_t crc_local = crc32_8(resp_buf->data_begin(),
                                         RPC_RESP_HEADER_SIZE - CRC_FLAGS_LEN,
                                         0);
            bs.pos(RPC_RESP_HEADER_SIZE - CRC_FLAGS_LEN);
            uint64_t flags_and_crc = bs.get_u64();
            uint32_t crc_buf = flags_and_crc & (uint32_t) 0xffffffff;
            uint32_t flags = (flags_and_crc >> 32);

            if (crc_local != crc_buf) {
                std::string err_msg = sstrfmt("CRC mismatch in response from "
                            "peer %d, %s:%s, "
                            "local calculation %u, from buffer %u")
                        .fmt(req->get_dst(), host_.c_str(),
                             port_.c_str(), crc_local, crc_buf);
                handle_error(req, err_msg, when_done);
                return;
            }

            bs.pos(1);
            byte msg_type_val = bs.get_u8();
            int32 src = bs.get_i32();
            int32 dst = bs.get_i32();
            ulong term = bs.get_u64();
            ulong nxt_idx = bs.get_u64();
            byte accepted_val = bs.get_u8();
            int32 carried_data_size = bs.get_i32();
            ptr<resp_msg> rsp
            (cs_new<resp_msg>
            (term, (msg_type) msg_type_val, src, dst,
             nxt_idx, accepted_val == 1));

            if (!(flags & INCLUDE_META) &&
                impl_->get_options().read_resp_meta_ &&
                impl_->get_options().invoke_resp_cb_on_empty_meta_) {
                // If callback is given, but meta is empty, and
                // the "always invoke" flag is set, invoke it.
                bool meta_ok = handle_custom_resp_meta
                        (req, rsp, when_done, std::string());
                if (!meta_ok) return;
            }

            if (flags & MARK_DOWN) {
                // Mark down flag is set in the response.
                rsp->set_extra_flags(rsp->get_extra_flags() | resp_msg::SELF_MARK_DOWN);
            }

            if (carried_data_size) {
                ptr<buffer> ctx_buf = buffer::alloc(carried_data_size);
                aa::read(ssl_enabled_, ssl_socket_, socket_,
                         xio::buffer(ctx_buf->data(), carried_data_size),
                         std::bind(&xio_rpc_client::ctx_read,
                                   self,
                                   req,
                                   rsp,
                                   when_done,
                                   ctx_buf,
                                   flags,
                                   std::placeholders::_1,
                                   std::placeholders::_2),
                         use_strand_ ? &ssl_strand_ : nullptr);
            } else {
                operation_timer_.cancel();
                set_busy_flag(false);
                ptr<rpc_exception> except;
                when_done(rsp, except);
                post_read();
            }
        }

        void ctx_read(ptr<req_msg> &req,
                      ptr<resp_msg> &rsp,
                      rpc_handler &when_done,
                      ptr<buffer> &ctx_buf,
                      uint32_t flags,
                      std::error_code err,
                      size_t bytes_transferred) {
            if (!(flags & INCLUDE_META) &&
                !(flags & INCLUDE_HINT) &&
                !(flags & INCLUDE_RESULT_CODE)) {
                // Neither meta nor hint nor result code exists,
                // just use the buffer as it is for ctx.
                ctx_buf->pos(0);
                rsp->set_ctx(ctx_buf);

                operation_timer_.cancel();
                set_busy_flag(false);
                ptr<rpc_exception> except;
                when_done(rsp, except);
                post_read();
                return;
            }

            // Otherwise: buffer contains composite data.
            buffer_serializer bs(ctx_buf);
            int remaining_len = ctx_buf->size();

            // 1) Custom meta.
            if (flags & INCLUDE_META) {
                size_t resp_meta_len = 0;
                void *resp_meta_raw = bs.get_bytes(resp_meta_len);

                // If callback is given, verify meta
                // (if meta is empty, invoke callback according to the flag).
                if (impl_->get_options().read_resp_meta_ &&
                    (resp_meta_len ||
                     impl_->get_options().invoke_resp_cb_on_empty_meta_)) {
                    bool meta_ok = handle_custom_resp_meta
                    (req, rsp, when_done,
                     std::string((const char *) resp_meta_raw,
                                 resp_meta_len));
                    if (!meta_ok) return;
                }
                remaining_len -= sizeof(int32) + resp_meta_len;
            }

            // 2) Hint.
            if (flags & INCLUDE_HINT) {
                size_t hint_len = 0;
                uint16_t hint_version = bs.get_u16();
                (void) hint_version;
                hint_len = bs.get_u16();
                rsp->set_next_batch_size_hint_in_bytes(bs.get_i64());
                remaining_len -= sizeof(uint16_t) * 2 + hint_len;
            }

            // 3) Context.
            assert(remaining_len >= 0);
            size_t ctx_len = remaining_len;
            if (flags & INCLUDE_RESULT_CODE) {
                ctx_len -= sizeof(int32_t);
            }
            if (ctx_len) {
                // It has context, read it.
                ptr<buffer> actual_ctx = buffer::alloc(ctx_len);
                bs.get_buffer(actual_ctx);
                rsp->set_ctx(actual_ctx);
                remaining_len -= ctx_len;
            }

            // 4) Result code
            if (flags & INCLUDE_RESULT_CODE) {
                assert((size_t)remaining_len >= sizeof(int32_t));
                cmd_result_code res = static_cast<cmd_result_code>(bs.get_i32());
                rsp->set_result_code(res);
            }

            operation_timer_.cancel();
            set_busy_flag(false);
            ptr<rpc_exception> except;
            when_done(rsp, except);
            post_read();
        }

        bool handle_custom_resp_meta(ptr<req_msg> &req,
                                     ptr<resp_msg> &rsp,
                                     rpc_handler &when_done,
                                     const std::string &meta_str) {
            bool meta_ok = impl_->get_options().read_resp_meta_
                    (req_to_params(req.get(), rsp.get()), meta_str);

            if (!meta_ok) {
                // Callback function returns false, should return failure.
                std::string err_msg = sstrfmt("response meta verification failed: "
                            "from peer %d, %s:%s")
                        .fmt(req->get_dst(), host_.c_str(),
                             port_.c_str());
                handle_error(req, err_msg, when_done);
                return false;
            }
            return true;
        }

        void register_response_read(ptr<req_msg> &req,
                                    rpc_handler &when_done) {
            ptr<xio_rpc_client> self(this->shared_from_this());
            ptr<buffer> resp_buf(buffer::alloc(RPC_RESP_HEADER_SIZE));
            aa::read(ssl_enabled_, ssl_socket_, socket_,
                     xio::buffer(resp_buf->data(), resp_buf->size()),
                     std::bind(&xio_rpc_client::response_read,
                               self,
                               req,
                               when_done,
                               resp_buf,
                               std::placeholders::_1,
                               std::placeholders::_2),
                     use_strand_ ? &ssl_strand_ : nullptr);
        }

        void post_send(ptr<req_msg> &req, rpc_handler &when_done) {
            // first process read
            bool immediate_action_needed = false;
            {
                auto_lock(pending_read_reqs_lock_);
                pending_read_reqs_.push_back(cs_new<pending_req_pkg>(req, when_done));
                immediate_action_needed = (pending_read_reqs_.size() == 1);
                TLOG(DEBUG, "msg to peer {} has been write down, start_log_idx: {}" ", "
                     "size: {}" ", pending read reqs: {}" "", req->get_dst(),
                     req->get_last_log_idx(),
                     req->log_entries().size(), pending_read_reqs_.size());
            }

            if (immediate_action_needed) {
                register_response_read(req, when_done);
            }

            // next process write
            ptr<pending_req_pkg> next_req_pkg{nullptr};
            {
                auto_lock(pending_write_reqs_lock_);
                // NOTE:
                //   The queue can be empty even though there was no `pop_front`,
                //   due to `close_socket()` when connection is suddenly closed.
                if (pending_write_reqs_.size()) {
                    pending_write_reqs_.pop_front();
                }
                if (pending_write_reqs_.size() > 0) {
                    next_req_pkg = *pending_write_reqs_.begin();
                    TLOG(DEBUG, "trigger next write, start_log_idx: {}" ", "
                         "pending write reqs: {}" "",
                         next_req_pkg->req_->get_last_log_idx(),
                         pending_write_reqs_.size());
                }
            }

            if (next_req_pkg) {
                register_req_send(next_req_pkg->req_,
                                  next_req_pkg->when_done_,
                                  next_req_pkg->timeout_ms_);
            }
        }

        void post_read() {
            if (!impl_->get_options().streaming_mode_) {
                return;
            }

            // trigger next read
            ptr<pending_req_pkg> next_req_pkg{nullptr};
            {
                auto_lock(pending_read_reqs_lock_);
                // NOTE:
                //   The queue can be empty even though there was no `pop_front`,
                //   due to `close_socket()` when connection is suddenly closed.
                if (pending_read_reqs_.size()) {
                    pending_read_reqs_.pop_front();
                }
                if (pending_read_reqs_.size() > 0) {
                    next_req_pkg = *pending_read_reqs_.begin();
                    TLOG(DEBUG, "trigger next read, start_log_idx: {}" ", "
                         "pending read reqs: {}" "",
                         next_req_pkg->req_->get_last_log_idx(),
                         pending_read_reqs_.size());
                }
            }

            if (next_req_pkg) {
                register_response_read(next_req_pkg->req_, next_req_pkg->when_done_);
            }
        }

    private:
        xio_service_impl *impl_;
        xio::ip::tcp::resolver resolver_;
        xio::ip::tcp::socket socket_;
        ssl_socket ssl_socket_;
        // `true` if attempting connection is in progress.
        // Other threads should not do anything.
        std::atomic<bool> attempting_conn_;
        std::string host_;
        std::string port_;
        bool ssl_enabled_;
        std::atomic<bool> ssl_ready_;
        xio::strand<xio::io_context::executor_type> ssl_strand_;
        bool use_strand_;
        std::atomic<size_t> num_send_fails_;
        std::atomic<bool> abandoned_;
        std::atomic<bool> socket_busy_;
        uint64_t client_id_;
        xio::steady_timer operation_timer_;

        /**
         * Queue of request which is pending for reading.
         */
        std::list<ptr<pending_req_pkg> > pending_read_reqs_;

        /**
         * Lock for `pending_read_reqs_`.
         */
        std::mutex pending_read_reqs_lock_;

        /**
         * Queue of request which is pending for writing.
         */
        std::list<ptr<pending_req_pkg> > pending_write_reqs_;

        /**
         * Lock for `pending_write_reqs_`.
         */
        std::mutex pending_write_reqs_lock_;
    };
} // namespace xio::raft

using namespace xio::raft;

void _free_timer_(void *ptr) {
    xio::steady_timer *timer = static_cast<xio::steady_timer *>(ptr);
    delete timer;
}

void _timer_handler_(ptr<delayed_task> &task, ERROR_CODE err) {
    if (!err) {
        task->execute();
    }
}

// `ssl_context` constructor with `SSL_CTX*` is supported by ASIO later than 1.16.1.
#if (ASIO_VERSION >= 101601) && \
    (OPENSSL_VERSION_NUMBER >= 0x10100000L) && \
    !defined(LIBRESSL_VERSION_NUMBER)

#define DEFAULT_SERVER_CTX ssl_context::tlsv12_server
#define DEFAULT_CLIENT_CTX ssl_context::tlsv12_client

namespace {
    ssl_context get_or_create_ssl_context(std::function<SSL_CTX*(void)> ctx_provider_func,
                                          ssl_context::method method) {
        if (ctx_provider_func) {
            return ssl_context(ctx_provider_func());
        } else {
            return ssl_context(method);
        }
    }
}

#else

#define DEFAULT_SERVER_CTX ssl_context::sslv23
#define DEFAULT_CLIENT_CTX ssl_context::sslv23

#endif

xio_service_impl::xio_service_impl(const xio_service::options &opt)
    : io_svc_(opt.custom_io_context_ ? nullptr : (new xio::io_context()))
#if (ASIO_VERSION >= 101601) && \
    (OPENSSL_VERSION_NUMBER >= 0x10100000L) && \
    !defined(LIBRESSL_VERSION_NUMBER)
    , ssl_server_ctx_(get_or_create_ssl_context(opt.ssl_context_provider_server_,
                                                DEFAULT_SERVER_CTX))
      , ssl_client_ctx_(get_or_create_ssl_context(opt.ssl_context_provider_client_,
                                                  DEFAULT_CLIENT_CTX))
#else
      , ssl_server_ctx_(DEFAULT_SERVER_CTX) // Any version
      , ssl_client_ctx_(DEFAULT_CLIENT_CTX)
#endif
      , continue_(1)
      , stopping_status_(0)
      , stopping_lock_()
      , stopping_cv_()
      , num_active_workers_(0)
      , worker_id_(0)
      , my_opt_(opt)
      , xio_timer_(get_io_svc())
      , client_id_counter_(1) {
    if (my_opt_.enable_ssl_) {
#ifdef SSL_LIBRARY_NOT_FOUND
        assert(0); // Should not reach here.
#else

        // Provider gives properly configured server contex
        if (!my_opt_.ssl_context_provider_server_) {
            TLOG(INFO, "server SSL context method {}",
                 ssl_context_method_to_string(DEFAULT_SERVER_CTX));

            // For server (listener)
            ssl_server_ctx_.set_options(ssl_context::default_workarounds |
                                        ssl_context::no_sslv2 |
                                        ssl_context::single_dh_use);
            ssl_server_ctx_.set_password_callback
            (std::bind(&xio_service_impl::get_password,
                       this,
                       std::placeholders::_1,
                       std::placeholders::_2));
            ssl_server_ctx_.use_certificate_chain_file
                    (my_opt_.server_cert_file_);
            ssl_server_ctx_.use_private_key_file(my_opt_.server_key_file_,
                                                 ssl_context::pem);
        } else {
            TLOG(INFO, "custom server SSL context is given");
        }

        // Provider gives properly configured client contex
        if (!my_opt_.ssl_context_provider_client_) {
            TLOG(INFO, "client SSL context method {}",
                 ssl_context_method_to_string(DEFAULT_CLIENT_CTX));

            // For client
            ssl_client_ctx_.load_verify_file(my_opt_.root_cert_file_);
        } else {
            TLOG(INFO, "custom client SSL context is given");
        }
#endif
    }

    if (my_opt_.custom_io_context_) {
        // If the external io_context is provided, we should not create
        // internal threads.
        TLOG(INFO, "custom io_context is provided, thread pool will not be created");
        return;
    }

    // set expires_after to a very large value so that
    // this will not affect the overall performance
    xio_timer_.expires_after
    (std::chrono::duration_cast<std::chrono::nanoseconds>
        (std::chrono::milliseconds(100)));
    xio_timer_.async_wait
    (std::bind(&xio_service_impl::timer_handler,
               this,
               std::placeholders::_1));

    unsigned int cpu_cnt = my_opt_.thread_pool_size_;
    if (!cpu_cnt) {
        cpu_cnt = std::thread::hardware_concurrency();
    }
    if (!cpu_cnt) {
        cpu_cnt = 1;
    }

    try {
        for (unsigned int i = 0; i < cpu_cnt; ++i) {
            ptr<std::thread> t =
                    cs_new<std::thread>(std::bind(&xio_service_impl::worker_entry, this));
            worker_handles_.push_back(t);
        }
    } catch (...) {
        stop();
        throw;
    }
}

xio_service_impl::~xio_service_impl() {
    stop();
}

#ifndef SSL_LIBRARY_NOT_FOUND
std::string xio_service_impl::get_password
(std::size_t size,
 xio::ssl::context_base::password_purpose purpose) {
    // TODO: Implement here if need to use cert with passphrase.
    return "test";
}
#endif

void xio_service_impl::worker_entry() {
    uint32_t worker_id = worker_id_.fetch_add(1);
    std::string thread_name = "xraft_w_" + std::to_string(worker_id);
#ifdef __linux__
    pthread_setname_np(pthread_self(), thread_name.c_str());
#elif __APPLE__
    pthread_setname_np(thread_name.c_str());
#endif

    TLOG(INFO, "spawned xio worker thread {}, current threads: {}",
         thread_name.c_str(),
         num_active_workers_.load());

    if (my_opt_.worker_start_) {
        my_opt_.worker_start_(worker_id);
    }

    static std::atomic<size_t> exception_count(0);
    static timer_helper timer(60 * 1000000); // 1 min.
    const size_t MAX_COUNT = 10;

    do {
        try {
            num_active_workers_.fetch_add(1);
            get_io_svc().run();
            num_active_workers_.fetch_sub(1);
        } catch (std::exception &ee) {
            // LCOV_EXCL_START
            num_active_workers_.fetch_sub(1);
            exception_count++;
            TLOG(ERROR, "xio worker thread got exception: {}, "
                 "current number of workers: {}, "
                 "exception count (in 1-min window): {}, "
                 "stopping status {}",
                 ee.what(),
                 num_active_workers_.load(),
                 exception_count.load(),
                 stopping_status_.load());
            // LCOV_EXCL_STOP
        }

        // LCOV_EXCL_START
        if (timer.timeout_and_reset()) {
            exception_count = 0;
        } else if (exception_count > MAX_COUNT) {
            TLOG(FATAL, "too many exceptions ({}) in 1-min time window.",
                 exception_count.load());
            exception_count = 0;
            abort();
        }
        // LCOV_EXCL_STOP
    } while (stopping_status_ != 1);

    if (my_opt_.worker_stop_) {
        my_opt_.worker_stop_(worker_id);
    }

    TLOG(INFO, "end of xio worker thread {}, remaining threads: {}",
         thread_name.c_str(),
         num_active_workers_.load());
}

void xio_service_impl::timer_handler(ERROR_CODE err) {
    if (continue_.load() == 1) {
        xio_timer_.expires_after
        (std::chrono::duration_cast<std::chrono::nanoseconds>
            (std::chrono::hours(1000)));
        xio_timer_.async_wait
        (std::bind(&xio_service_impl::timer_handler,
                   this,
                   std::placeholders::_1));
    }

    uint8_t exp = 0;
    std::unique_lock<std::mutex> lock(stopping_lock_);
    if (stopping_status_.compare_exchange_strong(exp, 2)) {
        // 0 means that stop() is not waiting for CV now.
        // make it 2, to avoid stop() waits for CV.
    } else {
        stopping_cv_.notify_all();
    }
}

void xio_service_impl::stop() {
    if (my_opt_.custom_io_context_) {
        // If custom io_context is provided, nothing to do here.
        TLOG(INFO, "custom io_context is provided, no need to stop the xio service "
             "and worker threads");
        return;
    }

    int running = 1;
    if (continue_.compare_exchange_strong(running, 0)) {
        std::unique_lock<std::mutex> lock(stopping_lock_);
        xio_timer_.cancel();

        uint8_t exp = 0;
        if (stopping_status_.compare_exchange_strong(exp, 1)) {
            // 0 means that timer_handler() is not yet called.
            // make it 1, timer_handler() will call notify().
            stopping_cv_.wait_for(lock, std::chrono::seconds(1));
        }
    }

    // Stop all workers.
    stopping_status_ = 1;

    get_io_svc().stop();
    while (!get_io_svc().stopped()) {
        std::this_thread::yield();
    }

    for (ptr<std::thread> &t: worker_handles_) {
        if (t && t->joinable()) {
            t->join();
        }
    }
}

xio_service::xio_service(const options &_opt)
    : impl_(new xio_service_impl(_opt)) {
}

xio_service::~xio_service() {
    delete impl_;
}

void xio_service::schedule(ptr<delayed_task> &task, int32 milliseconds) {
    if (task->get_impl_context() == nilptr) {
        task->set_impl_context(new xio::steady_timer(impl_->get_io_svc()),
                               &_free_timer_);
    }
    // ensure it's not in cancelled state
    task->reset();

    xio::steady_timer *timer = static_cast<xio::steady_timer *>
            (task->get_impl_context());
    timer->expires_after
    (std::chrono::duration_cast<std::chrono::nanoseconds>
        (std::chrono::milliseconds(milliseconds)));
    timer->async_wait(std::bind(&_timer_handler_,
                                task,
                                std::placeholders::_1));
}

void xio_service::cancel_impl(ptr<delayed_task> &task) {
    if (task->get_impl_context() != nilptr) {
        static_cast<xio::steady_timer *>(task->get_impl_context())->cancel();
    }
}

void xio_service::stop() {
    impl_->stop();
}

uint32_t xio_service::get_active_workers() {
    return impl_->num_active_workers_.load();
}

ptr<rpc_client> xio_service::create_client(const std::string &endpoint) {
    // NOTE:
    //   Abandoned regular expression due to bug in GCC < 4.9.
    //   And also support `endpoint` which doesn't start with `tcp://`.
#if 0
    // the endpoint is expecting to be protocol://host:port,
    // and we only support tcp for this factory
    // which is endpoint must be tcp://hostname:port
    static std::regex reg("^tcp://(([a-zA-Z0-9\\-]+\\.)*([a-zA-Z0-9]+)):([0-9]+)$");
    std::smatch mresults;
    if (!std::regex_match(endpoint, mresults, reg) || mresults.size() != 5) {
        return ptr<rpc_client>();
    }
#endif
    bool valid_address = false;
    std::string hostname;
    std::string port;
    size_t pos = endpoint.rfind(":");
    do {
        if (pos == std::string::npos) break;
        int port_num = std::stoi(endpoint.substr(pos + 1));
        if (!port_num) break;
        port = std::to_string(port_num);

        size_t pos2 = endpoint.rfind("://", pos - 1);
        hostname = (pos2 == std::string::npos)
                       ? endpoint.substr(0, pos)
                       : endpoint.substr(pos2 + 3, pos - pos2 - 3);

        if (hostname.empty()) break;
        valid_address = true;
    } while (false);

    if (!valid_address) {
        TLOG(ERROR, "invalid endpoint: {}", endpoint.c_str());
        return ptr<rpc_client>();
    }

    return cs_new<xio_rpc_client>
    (impl_,
     impl_->get_io_svc(),
     impl_->ssl_client_ctx_,
     hostname,
     port,
     impl_->my_opt_.enable_ssl_);
}

ptr<rpc_listener> xio_service::create_rpc_listener(ushort listening_port) {
    try {
        return cs_new<xio_rpc_listener>
        (impl_,
         impl_->get_io_svc(),
         impl_->ssl_server_ctx_,
         listening_port,
         impl_->my_opt_.enable_ssl_);
    } catch (std::exception &ee) {
        // Most likely exception happens due to wrong endpoint.
        TLOG(ERROR, "got exception: {}", ee.what());
        return nullptr;
    }
}

// ==========================
// NOTE:
//   We put Asio-related global manager functions to here,
//   to avoid unnecessary dependency requirements (e.g., SSL)
//   for those who don't want to use Asio.
ptr<xio_service> xraft_global_mgr::init_xio_service
(const xio_service_options &xio_opt) {
    xraft_global_mgr *mgr = get_instance();
    if (!mgr) return nullptr;

    std::lock_guard<std::mutex> l(mgr->xio_service_lock_);
    if (mgr->xio_service_) return mgr->xio_service_;

    mgr->xio_service_ = cs_new<xio_service>(xio_opt);
    return mgr->xio_service_;
}

ptr<xio_service> xraft_global_mgr::get_xio_service() {
    // NOTE:
    //   Basic assumption is that this function is not called frequently,
    //   only once at the initialization time. Hence it is ok to acquire
    //   lock for a such read-only operation.
    xraft_global_mgr *mgr = get_instance();
    if (!mgr) return nullptr;

    std::lock_guard<std::mutex> l(mgr->xio_service_lock_);
    return mgr->xio_service_;
}
