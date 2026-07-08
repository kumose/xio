//
// ip/host_name.hpp
// ~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef XIO_IP_HOST_NAME_HPP
#define XIO_IP_HOST_NAME_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>
#include <string>
#include <xio/error_code.h>

#include <xio/detail/push_options.h>

namespace xio {


    namespace ip {
        /// Get the current host name.
        XIO_DECL std::string host_name();

        /// Get the current host name.
        XIO_DECL std::string host_name(xio::error_code & ec);
    } // namespace ip

} // namespace xio

#include <xio/detail/pop_options.h>


#endif // XIO_IP_HOST_NAME_HPP
