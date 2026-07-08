//
// local/seq_packet_protocol.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef XIO_LOCAL_SEQ_PACKET_PROTOCOL_HPP
#define XIO_LOCAL_SEQ_PACKET_PROTOCOL_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>

#if defined(XIO_HAS_LOCAL_SOCKETS)

#include <xio/basic_socket_acceptor.h>
#include <xio/basic_seq_packet_socket.h>
#include <xio/detail/socket_types.h>
#include <xio/local/basic_endpoint.h>

#include <xio/detail/push_options.h>

namespace xio {


    namespace local {
        /// Encapsulates the flags needed for seq_packet UNIX sockets.
        /**
 * The xio::local::seq_packet_protocol class contains flags necessary
 * for sequenced packet UNIX domain sockets.
 *
 * @par Thread Safety
 * @e Distinct @e objects: Safe.@n
 * @e Shared @e objects: Safe.
 *
 * @par Concepts:
 * Protocol.
 */
        class seq_packet_protocol {
        public:
            /// Obtain an identifier for the type of the protocol.
            int type() const noexcept {
                return SOCK_SEQPACKET;
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
            typedef basic_endpoint<seq_packet_protocol> endpoint;

            /// The UNIX domain socket type.
            typedef basic_seq_packet_socket<seq_packet_protocol> socket;

            /// The UNIX domain acceptor type.
            typedef basic_socket_acceptor<seq_packet_protocol> acceptor;
        };
    } // namespace local

} // namespace xio

#include <xio/detail/pop_options.h>

#endif // defined(XIO_HAS_LOCAL_SOCKETS)

#endif // XIO_LOCAL_SEQ_PACKET_PROTOCOL_HPP
