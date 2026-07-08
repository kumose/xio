//
// detail/impl/posix_mutex.ipp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef XIO_DETAIL_IMPL_POSIX_MUTEX_IPP
#define XIO_DETAIL_IMPL_POSIX_MUTEX_IPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>

#if defined(XIO_HAS_PTHREADS)

#include <xio/detail/posix_mutex.h>
#include <xio/detail/throw_error.h>
#include <xio/error.h>

#include <xio/detail/push_options.h>

namespace xio {


    namespace detail {
        posix_mutex::posix_mutex() {
            int error = ::pthread_mutex_init(&mutex_, 0);
            xio::error_code ec(error,
                               xio::error::get_system_category());
            xio::detail::throw_error(ec, "mutex");
        }
    } // namespace detail

} // namespace xio

#include <xio/detail/pop_options.h>

#endif // defined(XIO_HAS_PTHREADS)

#endif // XIO_DETAIL_IMPL_POSIX_MUTEX_IPP
