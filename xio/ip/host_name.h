//
// ip/host_name.hpp
// ~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_IP_HOST_NAME_HPP
#define ASIO_IP_HOST_NAME_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>
#include <string>
#include <xio/error_code.h>

#include <xio/detail/push_options.h>

namespace xio {
    ASIO_INLINE_NAMESPACE_BEGIN

    namespace ip {
        /// Get the current host name.
        ASIO_DECL std::string host_name();

        /// Get the current host name.
        ASIO_DECL std::string host_name(xio::error_code & ec);
    } // namespace ip
    ASIO_INLINE_NAMESPACE_END
} // namespace xio

#include <xio/detail/pop_options.h>

#if defined(ASIO_HEADER_ONLY)
# include "xio/ip/impl/host_name.ipp"
#endif // defined(ASIO_HEADER_ONLY)

#endif // ASIO_IP_HOST_NAME_HPP
