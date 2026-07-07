//
// detail/impl/posix_tss_ptr.ipp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_DETAIL_IMPL_POSIX_TSS_PTR_IPP
#define ASIO_DETAIL_IMPL_POSIX_TSS_PTR_IPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>

#if defined(ASIO_HAS_PTHREADS)

#include <xio/detail/posix_tss_ptr.h>
#include <xio/detail/throw_error.h>
#include <xio/error.h>

#include <xio/detail/push_options.h>

namespace xio {


    namespace detail {
        void posix_tss_ptr_create(pthread_key_t &key) {
            int error = ::pthread_key_create(&key, 0);
            xio::error_code ec(error,
                               xio::error::get_system_category());
            xio::detail::throw_error(ec, "tss");
        }
    } // namespace detail

} // namespace xio

#include <xio/detail/pop_options.h>

#endif // defined(ASIO_HAS_PTHREADS)

#endif // ASIO_DETAIL_IMPL_POSIX_TSS_PTR_IPP
