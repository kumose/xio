//
// impl/awaitable.ipp
// ~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_IMPL_AWAITABLE_IPP
#define ASIO_IMPL_AWAITABLE_IPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>

#if defined(ASIO_HAS_CO_AWAIT)

#include <xio/awaitable.h>
#include <xio/detail/call_stack.h>

#include <xio/detail/push_options.h>

namespace xio {


    namespace detail {
        void awaitable_launch_context::launch(void (*pump_fn)(void *), void *arg) {
            call_stack<awaitable_launch_context>::context ctx(this);
            pump_fn(arg);
        }

        bool awaitable_launch_context::is_launching() {
            return !!call_stack<awaitable_launch_context>::contains(this);
        }
    } // namespace detail

} // namespace xio

#include <xio/detail/pop_options.h>

#endif // defined(ASIO_HAS_CO_AWAIT)

#endif // ASIO_IMPL_AWAITABLE_IPP
