//
// detail/fd_set_adapter.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef XIO_DETAIL_FD_SET_ADAPTER_HPP
#define XIO_DETAIL_FD_SET_ADAPTER_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>

#if !defined(XIO_WINDOWS_RUNTIME)

#include <xio/detail/posix_fd_set_adapter.h>
#include <xio/detail/win_fd_set_adapter.h>

namespace xio {


    namespace detail {
#if defined(XIO_WINDOWS) || defined(XIO_CYGWIN_W32_SOCKETS)
        typedef win_fd_set_adapter fd_set_adapter;
#else
        typedef posix_fd_set_adapter fd_set_adapter;
#endif
    } // namespace detail

} // namespace xio

#endif // !defined(XIO_WINDOWS_RUNTIME)

#endif // XIO_DETAIL_FD_SET_ADAPTER_HPP
