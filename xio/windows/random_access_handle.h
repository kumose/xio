//
// windows/random_access_handle.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_WINDOWS_RANDOM_ACCESS_HANDLE_HPP
#define ASIO_WINDOWS_RANDOM_ACCESS_HANDLE_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>

#if defined(ASIO_HAS_WINDOWS_RANDOM_ACCESS_HANDLE) \
  || defined(GENERATING_DOCUMENTATION)

#include <xio/windows/basic_random_access_handle.h>

namespace xio {
    ASIO_INLINE_NAMESPACE_BEGIN

    namespace windows {
        /// Typedef for the typical usage of a random-access handle.
        typedef basic_random_access_handle<> random_access_handle;
    } // namespace windows
    ASIO_INLINE_NAMESPACE_END
} // namespace xio

#endif // defined(ASIO_HAS_WINDOWS_RANDOM_ACCESS_HANDLE)
//   || defined(GENERATING_DOCUMENTATION)

#endif // ASIO_WINDOWS_RANDOM_ACCESS_HANDLE_HPP
