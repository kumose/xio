//
// detail/win_iocp_wait_op.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef XIO_DETAIL_WIN_IOCP_WAIT_OP_HPP
#define XIO_DETAIL_WIN_IOCP_WAIT_OP_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>

#if defined(XIO_HAS_IOCP)

#include <xio/detail/bind_handler.h>
#include <xio/detail/buffer_sequence_adapter.h>
#include <xio/detail/fenced_block.h>
#include <xio/detail/handler_alloc_helpers.h>
#include <xio/detail/handler_work.h>
#include <xio/detail/memory.h>
#include <xio/detail/reactor_op.h>
#include <xio/detail/socket_ops.h>
#include <xio/error.h>

#include <xio/detail/push_options.h>

namespace xio {


    namespace detail {
        template<typename Handler, typename IoExecutor>
        class win_iocp_wait_op : public reactor_op {
        public:
            XIO_DEFINE_HANDLER_PTR(win_iocp_wait_op);

            win_iocp_wait_op(socket_ops::weak_cancel_token_type cancel_token,
                             Handler &handler, const IoExecutor &io_ex)
                : reactor_op(xio::error_code(),
                             &win_iocp_wait_op::do_perform,
                             &win_iocp_wait_op::do_complete),
                  cancel_token_(cancel_token),
                  handler_(static_cast<Handler &&>(handler)),
                  work_(handler_, io_ex) {
            }

            static status do_perform(reactor_op *) {
                return done;
            }

            static void do_complete(void *owner, operation *base,
                                    const xio::error_code &result_ec,
                                    std::size_t /*bytes_transferred*/) {
                xio::error_code ec(result_ec);

                // Take ownership of the operation object.
                XIO_ASSUME(base != 0);
                win_iocp_wait_op *o(static_cast<win_iocp_wait_op *>(base));
                ptr p = {xio::detail::addressof(o->handler_), o, o};

                XIO_HANDLER_COMPLETION((*o));

                // Take ownership of the operation's outstanding work.
                handler_work<Handler, IoExecutor> w(
                    static_cast<handler_work<Handler, IoExecutor> &&>(
                        o->work_));

                // The reactor may have stored a result in the operation object.
                if (o->ec_)
                    ec = o->ec_;

                // Map non-portable errors to their portable counterparts.
                if (ec.value() == ERROR_NETNAME_DELETED) {
                    if (o->cancel_token_.expired())
                        ec = xio::error::operation_aborted;
                    else
                        ec = xio::error::connection_reset;
                } else if (ec.value() == ERROR_PORT_UNREACHABLE) {
                    ec = xio::error::connection_refused;
                }

                XIO_ERROR_LOCATION(ec);

                // Make a copy of the handler so that the memory can be deallocated before
                // the upcall is made. Even if we're not about to make an upcall, a
                // sub-object of the handler may be the true owner of the memory associated
                // with the handler. Consequently, a local copy of the handler is required
                // to ensure that any owning sub-object remains valid until after we have
                // deallocated the memory here.
                detail::binder1<Handler, xio::error_code>
                        handler(o->handler_, ec);
                p.h = xio::detail::addressof(handler.handler_);
                p.reset();

                // Make the upcall if required.
                if (owner) {
                    fenced_block b(fenced_block::half);
                    XIO_HANDLER_INVOCATION_BEGIN((handler.arg1_));
                    w.complete(handler, handler.handler_);
                    XIO_HANDLER_INVOCATION_END;
                }
            }

        private:
            socket_ops::weak_cancel_token_type cancel_token_;
            Handler handler_;
            handler_work<Handler, IoExecutor> work_;
        };
    } // namespace detail

} // namespace xio

#include <xio/detail/pop_options.h>

#endif // defined(XIO_HAS_IOCP)

#endif // XIO_DETAIL_WIN_IOCP_WAIT_OP_HPP
