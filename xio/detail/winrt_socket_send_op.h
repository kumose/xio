//
// detail/winrt_socket_send_op.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef XIO_DETAIL_WINRT_SOCKET_SEND_OP_HPP
#define XIO_DETAIL_WINRT_SOCKET_SEND_OP_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>

#if defined(XIO_WINDOWS_RUNTIME)

#include <xio/detail/bind_handler.h>
#include <xio/detail/buffer_sequence_adapter.h>
#include <xio/detail/fenced_block.h>
#include <xio/detail/handler_alloc_helpers.h>
#include <xio/detail/handler_work.h>
#include <xio/detail/memory.h>
#include <xio/detail/winrt_async_op.h>
#include <xio/error.h>

#include <xio/detail/push_options.h>

namespace xio {


    namespace detail {
        template<typename ConstBufferSequence, typename Handler, typename IoExecutor>
        class winrt_socket_send_op :
                public winrt_async_op<unsigned int> {
        public:
            XIO_DEFINE_HANDLER_PTR(winrt_socket_send_op);

            winrt_socket_send_op(const ConstBufferSequence &buffers,
                                 Handler &handler, const IoExecutor &io_ex)
                : winrt_async_op<unsigned int>(&winrt_socket_send_op::do_complete),
                  buffers_(buffers),
                  handler_(static_cast<Handler &&>(handler)),
                  work_(handler_, io_ex) {
            }

            static void do_complete(void *owner, operation *base,
                                    const xio::error_code &, std::size_t) {
                // Take ownership of the operation object.
                XIO_ASSUME(base != 0);
                winrt_socket_send_op *o(static_cast<winrt_socket_send_op *>(base));
                ptr p = {xio::detail::addressof(o->handler_), o, o};

                XIO_HANDLER_COMPLETION((*o));

                // Take ownership of the operation's outstanding work.
                handler_work<Handler, IoExecutor> w(
                    static_cast<handler_work<Handler, IoExecutor> &&>(
                        o->work_));

#if defined(XIO_ENABLE_BUFFER_DEBUGGING)
// Check whether buffers are still valid.
if (owner) {
    buffer_sequence_adapter<xio::const_buffer,
        ConstBufferSequence>::validate(o->buffers_);
}
#endif // defined(XIO_ENABLE_BUFFER_DEBUGGING)

// Make a copy of the handler so that the memory can be deallocated before
// the upcall is made. Even if we're not about to make an upcall, a
// sub-object of the handler may be the true owner of the memory associated
// with the handler. Consequently, a local copy of the handler is required
// to ensure that any owning sub-object remains valid until after we have
// deallocated the memory here.
detail::binder2<Handler, xio::error_code, std::size_t>
handler(o->handler_, o->ec_, o->result_);
p.h= xio::detail::addressof (handler.handler_);
p.reset();

// Make the upcall if required.
    if (owner) {
    fenced_block b(fenced_block::half);
    XIO_HANDLER_INVOCATION_BEGIN((handler.arg1_, handler.arg2_));
    w.complete(handler, handler.handler_);
    XIO_HANDLER_INVOCATION_END;
}
  }

private:
ConstBufferSequence buffers_;
Handler handler_;
handler_work<Handler, IoExecutor> executor_;
};

} // namespace detail
} // namespace xio

#include <xio/detail/pop_options.h>

#endif // defined(XIO_WINDOWS_RUNTIME)

#endif // XIO_DETAIL_WINRT_SOCKET_SEND_OP_HPP
