//
// ip/multicast.hpp
// ~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef XIO_IP_MULTICAST_HPP
#define XIO_IP_MULTICAST_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>
#include <cstddef>
#include <xio/ip/detail/socket_option.h>

#include <xio/detail/push_options.h>

namespace xio {


    namespace ip {
        namespace multicast {
            /// Socket option to join a multicast group on a specified interface.
            /**
 * Implements the IPPROTO_IP/IP_ADD_MEMBERSHIP socket option.
 *
 * @par Examples
 * Setting the option to join a multicast group:
 * @code
 * xio::ip::udp::socket socket(my_context);
 * ...
 * xio::ip::address multicast_address =
 *   xio::ip::address::from_string("225.0.0.1");
 * xio::ip::multicast::join_group option(multicast_address);
 * socket.set_option(option);
 * @endcode
 *
 * @par Concepts:
 * SettableSocketOption.
 */
            typedef xio::ip::detail::socket_option::multicast_request<
                XIO_OS_DEF(IPPROTO_IP),
                XIO_OS_DEF(IP_ADD_MEMBERSHIP),
                XIO_OS_DEF(IPPROTO_IPV6),
                XIO_OS_DEF(IPV6_JOIN_GROUP)> join_group;

            /// Socket option to leave a multicast group on a specified interface.
            /**
 * Implements the IPPROTO_IP/IP_DROP_MEMBERSHIP socket option.
 *
 * @par Examples
 * Setting the option to leave a multicast group:
 * @code
 * xio::ip::udp::socket socket(my_context);
 * ...
 * xio::ip::address multicast_address =
 *   xio::ip::address::from_string("225.0.0.1");
 * xio::ip::multicast::leave_group option(multicast_address);
 * socket.set_option(option);
 * @endcode
 *
 * @par Concepts:
 * SettableSocketOption.
 */
            typedef xio::ip::detail::socket_option::multicast_request<
                XIO_OS_DEF(IPPROTO_IP),
                XIO_OS_DEF(IP_DROP_MEMBERSHIP),
                XIO_OS_DEF(IPPROTO_IPV6),
                XIO_OS_DEF(IPV6_LEAVE_GROUP)> leave_group;

            /// Socket option for local interface to use for outgoing multicast packets.
            /**
 * Implements the IPPROTO_IP/IP_MULTICAST_IF socket option.
 *
 * For IPv4, the outbound interface may be specified using an IPv4 address.
 * For IPv6, an interface index must be used, since an IPv6 address does not
 * uniquely identify a network interface.
 *
 * @par Examples
 * Setting the option:
 * @code
 * xio::ip::udp::socket socket(my_context);
 * ...
 * xio::ip::address_v4 local_interface =
 *   xio::ip::address_v4::from_string("1.2.3.4");
 * xio::ip::multicast::outbound_interface option(local_interface);
 * socket.set_option(option);
 * @endcode
 *
 * @par Concepts:
 * SettableSocketOption.
 */
            typedef xio::ip::detail::socket_option::network_interface<
                XIO_OS_DEF(IPPROTO_IP),
                XIO_OS_DEF(IP_MULTICAST_IF),
                XIO_OS_DEF(IPPROTO_IPV6),
                XIO_OS_DEF(IPV6_MULTICAST_IF)> outbound_interface;

            /// Socket option for time-to-live associated with outgoing multicast packets.
            /**
 * Implements the IPPROTO_IP/IP_MULTICAST_TTL socket option.
 *
 * @par Examples
 * Setting the option:
 * @code
 * xio::ip::udp::socket socket(my_context);
 * ...
 * xio::ip::multicast::hops option(4);
 * socket.set_option(option);
 * @endcode
 *
 * @par
 * Getting the current option value:
 * @code
 * xio::ip::udp::socket socket(my_context);
 * ...
 * xio::ip::multicast::hops option;
 * socket.get_option(option);
 * int ttl = option.value();
 * @endcode
 *
 * @par Concepts:
 * GettableSocketOption, SettableSocketOption.
 */
            typedef xio::ip::detail::socket_option::multicast_hops<
                XIO_OS_DEF(IPPROTO_IP),
                XIO_OS_DEF(IP_MULTICAST_TTL),
                XIO_OS_DEF(IPPROTO_IPV6),
                XIO_OS_DEF(IPV6_MULTICAST_HOPS)> hops;

            /// Socket option determining whether outgoing multicast packets will be
/// received on the same socket if it is a member of the multicast group.
            /**
 * Implements the IPPROTO_IP/IP_MULTICAST_LOOP socket option.
 *
 * @par Examples
 * Setting the option:
 * @code
 * xio::ip::udp::socket socket(my_context);
 * ...
 * xio::ip::multicast::enable_loopback option(true);
 * socket.set_option(option);
 * @endcode
 *
 * @par
 * Getting the current option value:
 * @code
 * xio::ip::udp::socket socket(my_context);
 * ...
 * xio::ip::multicast::enable_loopback option;
 * socket.get_option(option);
 * bool is_set = option.value();
 * @endcode
 *
 * @par Concepts:
 * GettableSocketOption, SettableSocketOption.
 */
            typedef xio::ip::detail::socket_option::multicast_enable_loopback<
                XIO_OS_DEF(IPPROTO_IP),
                XIO_OS_DEF(IP_MULTICAST_LOOP),
                XIO_OS_DEF(IPPROTO_IPV6),
                XIO_OS_DEF(IPV6_MULTICAST_LOOP)> enable_loopback;
        } // namespace multicast
    } // namespace ip

} // namespace xio

#include <xio/detail/pop_options.h>

#endif // XIO_IP_MULTICAST_HPP
