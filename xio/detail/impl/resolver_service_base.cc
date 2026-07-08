//
// detail/impl/resolver_service_base.ipp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef XIO_DETAIL_IMPL_RESOLVER_SERVICE_BASE_IPP
#define XIO_DETAIL_IMPL_RESOLVER_SERVICE_BASE_IPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>
#include <xio/config.h>
#include <xio/detail/memory.h>
#include <xio/detail/resolver_service_base.h>

#include <xio/detail/push_options.h>

namespace xio {


    namespace detail {
        resolver_service_base::resolver_service_base(execution_context &context)
            : thread_pool_(xio::use_service<resolver_thread_pool>(context)) {
        }

        resolver_service_base::~resolver_service_base() {
        }

        void resolver_service_base::construct(
            resolver_service_base::implementation_type &impl) {
            impl.reset(static_cast<void *>(0), socket_ops::noop_deleter());
        }

        void resolver_service_base::destroy(
            resolver_service_base::implementation_type &impl) {
            XIO_HANDLER_OPERATION((thread_pool_.context(),
                                    "resolver", &impl, 0, "cancel"));

            impl.reset();
        }

        void resolver_service_base::move_construct(implementation_type &impl,
                                                   implementation_type &other_impl) {
            impl = static_cast<implementation_type &&>(other_impl);
        }

        void resolver_service_base::move_assign(implementation_type &impl,
                                                resolver_service_base &, implementation_type &other_impl) {
            destroy(impl);
            impl = static_cast<implementation_type &&>(other_impl);
        }

        void resolver_service_base::cancel(
            resolver_service_base::implementation_type &impl) {
            XIO_HANDLER_OPERATION((thread_pool_.context(),
                                    "resolver", &impl, 0, "cancel"));

            impl.reset(static_cast<void *>(0), socket_ops::noop_deleter());
        }
    } // namespace detail

} // namespace xio

#include <xio/detail/pop_options.h>

#endif // XIO_DETAIL_IMPL_RESOLVER_SERVICE_BASE_IPP
