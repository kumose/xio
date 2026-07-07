//
// detail/resolver_service_base.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_DETAIL_RESOLVER_SERVICE_BASE_HPP
#define ASIO_DETAIL_RESOLVER_SERVICE_BASE_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>
#include <xio/error.h>
#include <xio/execution_context.h>
#include <xio/detail/noncopyable.h>
#include <xio/detail/resolve_op.h>
#include <xio/detail/resolver_thread_pool.h>
#include <xio/detail/socket_ops.h>
#include <xio/detail/socket_types.h>

#if defined(ASIO_HAS_IOCP)
#include <xio/detail/win_iocp_io_context.h>
#else // defined(ASIO_HAS_IOCP)
#include <xio/detail/scheduler.h>
#endif // defined(ASIO_HAS_IOCP)

#include <xio/detail/push_options.h>

namespace xio {


    namespace detail {
        class resolver_service_base {
        public:
            // The implementation type of the resolver. A cancellation token is used to
            // indicate to the background thread that the operation has been cancelled.
            typedef socket_ops::shared_cancel_token_type implementation_type;

            // Constructor.
            ASIO_DECL resolver_service_base(execution_context &context);

            // Destructor.
            ASIO_DECL ~resolver_service_base();

            // Construct a new resolver implementation.
  ASIO_DECL void construct(implementation_type &impl);

            // Destroy a resolver implementation.
  ASIO_DECL void destroy(implementation_type &);

            // Move-construct a new resolver implementation.
  ASIO_DECL void move_construct(implementation_type &impl,
                                implementation_type &other_impl);

            // Move-assign from another resolver implementation.
  ASIO_DECL void move_assign(implementation_type &impl,
                             resolver_service_base &other_service,
                             implementation_type &other_impl);

            // Move-construct a new timer implementation.
            void converting_move_construct(implementation_type &impl,
                                           resolver_service_base &, implementation_type &other_impl) {
                move_construct(impl, other_impl);
            }

            // Move-assign from another timer implementation.
            void converting_move_assign(implementation_type &impl,
                                        resolver_service_base &other_service,
                                        implementation_type &other_impl) {
                move_assign(impl, other_service, other_impl);
            }

            // Cancel pending asynchronous operations.
  ASIO_DECL void cancel(implementation_type &impl);

        protected:
#if !defined(ASIO_WINDOWS_RUNTIME)
            // Helper class to perform exception-safe cleanup of addrinfo objects.
            class auto_addrinfo
                    : private xio::detail::noncopyable {
            public:
                explicit auto_addrinfo(xio::detail::addrinfo_type *ai)
                    : ai_(ai) {
                }

                ~auto_addrinfo() {
                    if (ai_)
                        socket_ops::freeaddrinfo(ai_);
                }

                operator xio::detail::addrinfo_type *() {
                    return ai_;
                }

            private:
                xio::detail::addrinfo_type *ai_;
            };
#endif // !defined(ASIO_WINDOWS_RUNTIME)

            // Private thread pool used for performing asynchronous host resolution.
            resolver_thread_pool &thread_pool_;
        };
    } // namespace detail

} // namespace xio

#include <xio/detail/pop_options.h>


#endif // ASIO_DETAIL_RESOLVER_SERVICE_BASE_HPP
