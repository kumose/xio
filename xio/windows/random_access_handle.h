//
// windows/random_access_handle.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef XIO_WINDOWS_RANDOM_ACCESS_HANDLE_HPP
#define XIO_WINDOWS_RANDOM_ACCESS_HANDLE_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>

#if defined(XIO_HAS_WINDOWS_RANDOM_ACCESS_HANDLE)

#include <xio/windows/basic_random_access_handle.h>

namespace xio {


    namespace windows {
        /// Typedef for the typical usage of a random-access handle.
        typedef basic_random_access_handle<> random_access_handle;
    } // namespace windows

} // namespace xio

#endif // defined(XIO_HAS_WINDOWS_RANDOM_ACCESS_HANDLE)

#endif // XIO_WINDOWS_RANDOM_ACCESS_HANDLE_HPP
