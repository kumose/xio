//
// detail/global.hpp
// ~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_DETAIL_GLOBAL_HPP
#define ASIO_DETAIL_GLOBAL_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>

#if !defined(ASIO_HAS_THREADS)
#include <xio/detail/null_global.h>
#elif defined(ASIO_WINDOWS)
#include <xio/detail/win_global.h>
#elif defined(ASIO_HAS_PTHREADS)
#include <xio/detail/posix_global.h>
#else
#include <xio/detail/std_global.h>
#endif

namespace xio {
    ASIO_INLINE_NAMESPACE_BEGIN

    namespace detail {
        template<typename T>
        inline T &global() {
#if !defined(ASIO_HAS_THREADS)
            return null_global<T>();
#elif defined(ASIO_WINDOWS)
            return win_global<T>();
#elif defined(ASIO_HAS_PTHREADS)
            return posix_global<T>();
#else
            return std_global<T>();
#endif
        }
    } // namespace detail
    ASIO_INLINE_NAMESPACE_END
} // namespace xio

#endif // ASIO_DETAIL_GLOBAL_HPP
