//
// detail/impl/thread_context.ipp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef XIO_DETAIL_IMPL_THREAD_CONTEXT_IPP
#define XIO_DETAIL_IMPL_THREAD_CONTEXT_IPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>
#include <xio/detail/thread_context.h>
#include <xio/detail/push_options.h>

namespace xio {


    namespace detail {
        thread_info_base *thread_context::top_of_thread_call_stack() {
            return thread_call_stack::top();
        }
    } // namespace detail

} // namespace xio

#include <xio/detail/pop_options.h>

#endif // XIO_DETAIL_IMPL_THREAD_CONTEXT_IPP
