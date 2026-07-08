//
// detail/winrt_resolve_op.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef XIO_DETAIL_WINRT_RESOLVE_OP_HPP
#define XIO_DETAIL_WINRT_RESOLVE_OP_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>

#if defined(XIO_WINDOWS_RUNTIME)

#include <xio/detail/bind_handler.h>
#include <xio/detail/fenced_block.h>
#include <xio/detail/handler_alloc_helpers.h>
#include <xio/detail/handler_work.h>
#include <xio/detail/memory.h>
#include <xio/detail/winrt_async_op.h>
#include <xio/ip/basic_resolver_results.h>
#include <xio/error.h>

#include <xio/detail/push_options.h>

namespace xio {


    namespace detail {
        template<typename Protocol, typename Handler, typename IoExecutor>
        class winrt_resolve_op :
                public winrt_async_op<
                    Windows::Foundation::Collections::IVectorView <
                    Windows::Networking::EndpointPair ^>^>
                {
        public:
            XIO_DEFINE_HANDLER_PTR(winrt_resolve_op);

            typedef typename Protocol::endpoint endpoint_type;
            typedef xio::ip::basic_resolver_query<Protocol> query_type;
            typedef xio::ip::basic_resolver_results<Protocol> results_type;

            winrt_resolve_op(const query_type &query,
                             Handler &handler, const IoExecutor &io_ex)
                : winrt_async_op<
                    Windows::Foundation::Collections::IVectorView <
                    Windows::Networking::EndpointPair ^>

            ^
            >
            (
                        &winrt_resolve_op::do_complete),
                    query_(query),
                    handler_(static_cast<Handler &&>(handler)
            )
            ,
            work_(handler_, io_ex) {
            }

            static void do_complete(void *owner, operation *base,
                                    const xio::error_code &, std::size_t) {
                // Take ownership of the operation object.
                XIO_ASSUME(base != 0);
                winrt_resolve_op *o(static_cast<winrt_resolve_op *>(base));
                ptr p = {xio::detail::addressof(o->handler_), o, o};

                XIO_HANDLER_COMPLETION((*o));

                // Take ownership of the operation's outstanding work.
                handler_work<Handler, IoExecutor> w(
                    static_cast<handler_work<Handler, IoExecutor> &&>(
                        o->work_));

                results_type results = results_type();
                if (!o->ec_) {
                    try {
                        results = results_type::create(o->result_, o->query_.hints(),
                                                       o->query_.host_name(), o->query_.service_name());
                    } catch (Platform::Exception

                    ^
                    e
                    )
                    {
                        o->ec_ = xio::error_code(e->HResult,
                                                 xio::system_category());
                    }
                }

                // Make a copy of the handler so that the memory can be deallocated before
                // the upcall is made. Even if we're not about to make an upcall, a
                // sub-object of the handler may be the true owner of the memory associated
                // with the handler. Consequently, a local copy of the handler is required
                // to ensure that any owning sub-object remains valid until after we have
                // deallocated the memory here.
                detail::binder2<Handler, xio::error_code, results_type>
                        handler(o->handler_, o->ec_, results);
                p.h = xio::detail::addressof(handler.handler_);
                p.reset();

                // Make the upcall if required.
                if (owner) {
                    fenced_block b(fenced_block::half);
                    XIO_HANDLER_INVOCATION_BEGIN((handler.arg1_, "..."));
                    w.complete(handler, handler.handler_);
                    XIO_HANDLER_INVOCATION_END;
                }
            }

        private:
            query_type query_;
            Handler handler_;
            handler_work<Handler, IoExecutor> executor_;
        };
    } // namespace detail

} // namespace xio

#include <xio/detail/pop_options.h>

#endif // defined(XIO_WINDOWS_RUNTIME)

#endif // XIO_DETAIL_WINRT_RESOLVE_OP_HPP
