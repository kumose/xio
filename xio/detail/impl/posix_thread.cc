//
// detail/impl/posix_thread.ipp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef XIO_DETAIL_IMPL_POSIX_THREAD_IPP
#define XIO_DETAIL_IMPL_POSIX_THREAD_IPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>

#if defined(XIO_HAS_PTHREADS)

#include <xio/detail/posix_thread.h>
#include <xio/detail/throw_error.h>
#include <xio/error.h>

#include <xio/detail/push_options.h>

namespace xio {


    namespace detail {
        posix_thread::~posix_thread() {
            if (arg_)
                std::terminate();
        }

        void posix_thread::join() {
            if (arg_) {
                ::pthread_join(arg_->thread_, 0);
                arg_->destroy();
                arg_ = 0;
            }
        }

        std::size_t posix_thread::hardware_concurrency() {


#if defined(_SC_NPROCESSORS_ONLN)
long result = sysconf(_SC_NPROCESSORS_ONLN);
  if (result> 0)
    return result;
#endif // defined(_SC_NPROCESSORS_ONLN)
return 0;
}

posix_thread::func_base *posix_thread::start_thread(func_base *arg) {
    int error = ::pthread_create(&arg->thread_, 0,
                                 XIO_VERSIONED_NAME(detail_posix_thread_function), arg);
    if (error != 0) {
        arg->destroy();
        xio::error_code ec(error,
                           xio::error::get_system_category());
        xio::detail::throw_error(ec, "thread");
    }
    return arg;
}

void *XIO_VERSIONED_NAME(detail_posix_thread_function)(void *arg)
{
  static_cast<posix_thread::func_base*>(arg)->run();
  return 0;
}

} // namespace detail
} // namespace xio

#include <xio/detail/pop_options.h>

#endif // defined(XIO_HAS_PTHREADS)

#endif // XIO_DETAIL_IMPL_POSIX_THREAD_IPP
