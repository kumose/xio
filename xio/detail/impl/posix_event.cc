//
// detail/impl/posix_event.ipp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_DETAIL_IMPL_POSIX_EVENT_IPP
#define ASIO_DETAIL_IMPL_POSIX_EVENT_IPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>

#if defined(ASIO_HAS_PTHREADS)

#include <xio/detail/posix_event.h>
#include <xio/detail/throw_error.h>
#include <xio/error.h>

#include <xio/detail/push_options.h>

namespace xio {


    namespace detail {
        posix_event::posix_event()
            : state_(0) {


#if (defined(__MACH__) && defined(__APPLE__)) \
      || (defined(__ANDROID__) && (__ANDROID_API__ < 21))
int error = ::pthread_cond_init(&cond_, 0);
#else // (defined(__MACH__) && defined(__APPLE__))
// || (defined(__ANDROID__) && (__ANDROID_API__ < 21))
::pthread_condattr_t attr;
int error = ::pthread_condattr_init(&attr);
  if (error== 0)
  {
    error = ::pthread_condattr_setclock(&attr, CLOCK_MONOTONIC);
    if (error == 0)
      error = ::pthread_cond_init(&cond_, &attr);
    ::pthread_condattr_destroy(&attr);
  }
#endif // (defined(__MACH__) && defined(__APPLE__))
// || (defined(__ANDROID__) && (__ANDROID_API__ < 21))

xio::error_code ec(error,
                   xio::error::get_system_category());
xio::detail::throw_error(ec, "event");
}

} // namespace detail
} // namespace xio

#include <xio/detail/pop_options.h>

#endif // defined(ASIO_HAS_PTHREADS)

#endif // ASIO_DETAIL_IMPL_POSIX_EVENT_IPP
