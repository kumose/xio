//
// ip/v6_only.hpp
// ~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef XIO_IP_V6_ONLY_HPP
#define XIO_IP_V6_ONLY_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>
#include <xio/detail/socket_option.h>

#include <xio/detail/push_options.h>

namespace xio {


    namespace ip {
        /// Socket option for determining whether an IPv6 socket supports IPv6
/// communication only.
        /**
 * Implements the IPPROTO_IPV6/IPV6_V6ONLY socket option.
 *
 * @par Examples
 * Setting the option:
 * @code
 * xio::ip::tcp::socket socket(my_context);
 * ...
 * xio::ip::v6_only option(true);
 * socket.set_option(option);
 * @endcode
 *
 * @par
 * Getting the current option value:
 * @code
 * xio::ip::tcp::socket socket(my_context);
 * ...
 * xio::ip::v6_only option;
 * socket.get_option(option);
 * bool v6_only = option.value();
 * @endcode
 *
 * @par Concepts:
 * GettableSocketOption, SettableSocketOption.
 */
#if defined(IPV6_V6ONLY)
        typedef xio::detail::socket_option::boolean<
            IPPROTO_IPV6, IPV6_V6ONLY> v6_only;
#else
        typedef xio::detail::socket_option::boolean<
            xio::detail::custom_socket_option_level,
            xio::detail::always_fail_option> v6_only;
#endif
    } // namespace ip

} // namespace xio

#include <xio/detail/pop_options.h>

#endif // XIO_IP_V6_ONLY_HPP
