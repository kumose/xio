//
// detail/reactive_socket_recvmsg_op.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef XIO_DETAIL_REACTIVE_SOCKET_RECVMSG_OP_HPP
#define XIO_DETAIL_REACTIVE_SOCKET_RECVMSG_OP_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>
#include <xio/detail/bind_handler.h>
#include <xio/detail/buffer_sequence_adapter.h>
#include <xio/detail/fenced_block.h>
#include <xio/detail/handler_alloc_helpers.h>
#include <xio/detail/handler_work.h>
#include <xio/detail/memory.h>
#include <xio/detail/reactor_op.h>
#include <xio/detail/socket_ops.h>
#include <xio/socket_base.h>

#include <xio/detail/push_options.h>

namespace xio {


    namespace detail {
        template<typename MutableBufferSequence>
        class reactive_socket_recvmsg_op_base : public reactor_op {
        public:
            reactive_socket_recvmsg_op_base(const xio::error_code &success_ec,
                                            socket_type socket, const MutableBufferSequence &buffers,
                                            socket_base::message_flags in_flags,
                                            socket_base::message_flags &out_flags, func_type complete_func)
                : reactor_op(success_ec,
                             &reactive_socket_recvmsg_op_base::do_perform, complete_func),
                  socket_(socket),
                  buffers_(buffers),
                  in_flags_(in_flags),
                  out_flags_(out_flags) {
            }

            static status do_perform(reactor_op *base) {
                XIO_ASSUME(base != 0);
                reactive_socket_recvmsg_op_base *o(
                    static_cast<reactive_socket_recvmsg_op_base *>(base));

                buffer_sequence_adapter<xio::mutable_buffer,
                    MutableBufferSequence> bufs(o->buffers_);

                status result = socket_ops::non_blocking_recvmsg(o->socket_,
                                                                 bufs.buffers(), bufs.count(),
                                                                 o->in_flags_, o->out_flags_,
                                                                 o->ec_, o->bytes_transferred_)
                                    ? done
                                    : not_done;

                XIO_HANDLER_REACTOR_OPERATION((*o, "non_blocking_recvmsg",
                                                o->ec_, o->bytes_transferred_));

                return result;
            }

        private:
            socket_type socket_;
            MutableBufferSequence buffers_;
            socket_base::message_flags in_flags_;
            socket_base::message_flags &out_flags_;
        };

        template<typename MutableBufferSequence, typename Handler, typename IoExecutor>
        class reactive_socket_recvmsg_op :
                public reactive_socket_recvmsg_op_base<MutableBufferSequence> {
        public:
            typedef Handler handler_type;
            typedef IoExecutor io_executor_type;

            XIO_DEFINE_HANDLER_PTR(reactive_socket_recvmsg_op);

            reactive_socket_recvmsg_op(const xio::error_code &success_ec,
                                       socket_type socket, const MutableBufferSequence &buffers,
                                       socket_base::message_flags in_flags,
                                       socket_base::message_flags &out_flags, Handler &handler,
                                       const IoExecutor &io_ex)
                : reactive_socket_recvmsg_op_base<MutableBufferSequence>(
                      success_ec, socket, buffers, in_flags, out_flags,
                      &reactive_socket_recvmsg_op::do_complete),
                  handler_(static_cast<Handler &&>(handler)),
                  work_(handler_, io_ex) {
            }

            static void do_complete(void *owner, operation *base,
                                    const xio::error_code & /*ec*/,
                                    std::size_t /*bytes_transferred*/) {
                // Take ownership of the handler object.
                XIO_ASSUME(base != 0);
                reactive_socket_recvmsg_op *o(
                    static_cast<reactive_socket_recvmsg_op *>(base));
                ptr p = {xio::detail::addressof(o->handler_), o, o};

                XIO_HANDLER_COMPLETION((*o));

                // Take ownership of the operation's outstanding work.
                handler_work<Handler, IoExecutor> w(
                    static_cast<handler_work<Handler, IoExecutor> &&>(
                        o->work_));

                XIO_ERROR_LOCATION(o->ec_);

                // Make a copy of the handler so that the memory can be deallocated before
                // the upcall is made. Even if we're not about to make an upcall, a
                // sub-object of the handler may be the true owner of the memory associated
                // with the handler. Consequently, a local copy of the handler is required
                // to ensure that any owning sub-object remains valid until after we have
                // deallocated the memory here.
                detail::binder2<Handler, xio::error_code, std::size_t>
                        handler(o->handler_, o->ec_, o->bytes_transferred_);
                p.h = xio::detail::addressof(handler.handler_);
                p.reset();

                // Make the upcall if required.
                if (owner) {
                    fenced_block b(fenced_block::half);
                    XIO_HANDLER_INVOCATION_BEGIN((handler.arg1_, handler.arg2_));
                    w.complete(handler, handler.handler_);
                    XIO_HANDLER_INVOCATION_END;
                }
            }

            static void do_immediate(operation *base, bool, const void *io_ex) {
                // Take ownership of the handler object.
                XIO_ASSUME(base != 0);
                reactive_socket_recvmsg_op *o(
                    static_cast<reactive_socket_recvmsg_op *>(base));
                ptr p = {xio::detail::addressof(o->handler_), o, o};

                XIO_HANDLER_COMPLETION((*o));

                // Take ownership of the operation's outstanding work.
                immediate_handler_work<Handler, IoExecutor> w(
                    static_cast<handler_work<Handler, IoExecutor> &&>(
                        o->work_));

                XIO_ERROR_LOCATION(o->ec_);

                // Make a copy of the handler so that the memory can be deallocated before
                // the upcall is made. Even if we're not about to make an upcall, a
                // sub-object of the handler may be the true owner of the memory associated
                // with the handler. Consequently, a local copy of the handler is required
                // to ensure that any owning sub-object remains valid until after we have
                // deallocated the memory here.
                detail::binder2<Handler, xio::error_code, std::size_t>
                        handler(o->handler_, o->ec_, o->bytes_transferred_);
                p.h = xio::detail::addressof(handler.handler_);
                p.reset();

                XIO_HANDLER_INVOCATION_BEGIN((handler.arg1_, handler.arg2_));
                w.complete(handler, handler.handler_, io_ex);
                XIO_HANDLER_INVOCATION_END;
            }

        private:
            Handler handler_;
            handler_work<Handler, IoExecutor> work_;
        };
    } // namespace detail

} // namespace xio

#include <xio/detail/pop_options.h>

#endif // XIO_DETAIL_REACTIVE_SOCKET_RECVMSG_OP_HPP
