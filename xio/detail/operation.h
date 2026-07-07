//
// detail/operation.hpp
// ~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_DETAIL_OPERATION_HPP
#define ASIO_DETAIL_OPERATION_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>

#if defined(ASIO_HAS_IOCP)
#include <xio/detail/win_iocp_operation.h>
#else
#include <xio/detail/scheduler_operation.h>
#endif

namespace xio {


    namespace detail {
#if defined(ASIO_HAS_IOCP)
        typedef win_iocp_operation operation;
#else
        typedef scheduler_operation operation;
#endif
    } // namespace detail

} // namespace xio

#endif // ASIO_DETAIL_OPERATION_HPP
