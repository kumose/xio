//
// local/datagram_protocol.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef XIO_LOCAL_DATAGRAM_PROTOCOL_HPP
#define XIO_LOCAL_DATAGRAM_PROTOCOL_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>

#if defined(XIO_HAS_LOCAL_SOCKETS)

#include <xio/basic_datagram_socket.h>
#include <xio/detail/socket_types.h>
#include <xio/local/basic_endpoint.h>

#include <xio/detail/push_options.h>

namespace xio {


    namespace local {
        /// Encapsulates the flags needed for datagram-oriented UNIX sockets.
        /**
 * The xio::local::datagram_protocol class contains flags necessary for
 * datagram-oriented UNIX domain sockets.
 *
 * @par Thread Safety
 * @e Distinct @e objects: Safe.@n
 * @e Shared @e objects: Safe.
 *
 * @par Concepts:
 * Protocol.
 */
        class datagram_protocol {
        public:
            /// Obtain an identifier for the type of the protocol.
            int type() const noexcept {
                return SOCK_DGRAM;
            }

            /// Obtain an identifier for the protocol.
            int protocol() const noexcept {
                return 0;
            }

            /// Obtain an identifier for the protocol family.
            int family() const noexcept {
                return AF_UNIX;
            }

            /// The type of a UNIX domain endpoint.
            typedef basic_endpoint<datagram_protocol> endpoint;

            /// The UNIX domain socket type.
            typedef basic_datagram_socket<datagram_protocol> socket;
        };
    } // namespace local

} // namespace xio

#include <xio/detail/pop_options.h>

#endif // defined(XIO_HAS_LOCAL_SOCKETS)

#endif // XIO_LOCAL_DATAGRAM_PROTOCOL_HPP
