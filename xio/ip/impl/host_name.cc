//
// ip/impl/host_name.ipp
// ~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef XIO_IP_IMPL_HOST_NAME_IPP
#define XIO_IP_IMPL_HOST_NAME_IPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>
#include <xio/detail/socket_ops.h>
#include <xio/detail/throw_error.h>
#include <xio/detail/winsock_init.h>
#include <xio/ip/host_name.h>

#include <xio/detail/push_options.h>

namespace xio {


    namespace ip {
        std::string host_name() {
            char name[1024];
            xio::error_code ec;
            if (xio::detail::socket_ops::gethostname(name, sizeof(name), ec) != 0) {
                xio::detail::throw_error(ec);
                return std::string();
            }
            return std::string(name);
        }

        std::string host_name(xio::error_code &ec) {
            char name[1024];
            if (xio::detail::socket_ops::gethostname(name, sizeof(name), ec) != 0)
                return std::string();
            return std::string(name);
        }
    } // namespace ip

} // namespace xio

#include <xio/detail/pop_options.h>

#endif // XIO_IP_IMPL_HOST_NAME_IPP
