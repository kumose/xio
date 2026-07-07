//
// local/stream_protocol.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_LOCAL_STREAM_PROTOCOL_HPP
#define ASIO_LOCAL_STREAM_PROTOCOL_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>

#if defined(ASIO_HAS_LOCAL_SOCKETS) \
  || defined(GENERATING_DOCUMENTATION)

#include <xio/basic_socket_acceptor.h>
#include <xio/basic_socket_iostream.h>
#include <xio/basic_stream_socket.h>
#include <xio/detail/socket_types.h>
#include <xio/local/basic_endpoint.h>

#include <xio/detail/push_options.h>

namespace xio {


    namespace local {
        /// Encapsulates the flags needed for stream-oriented UNIX sockets.
        /**
 * The xio::local::stream_protocol class contains flags necessary for
 * stream-oriented UNIX domain sockets.
 *
 * @par Thread Safety
 * @e Distinct @e objects: Safe.@n
 * @e Shared @e objects: Safe.
 *
 * @par Concepts:
 * Protocol.
 */
        class stream_protocol {
        public:
            /// Obtain an identifier for the type of the protocol.
            int type() const noexcept {
                return SOCK_STREAM;
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
            typedef basic_endpoint<stream_protocol> endpoint;

            /// The UNIX domain socket type.
            typedef basic_stream_socket<stream_protocol> socket;

            /// The UNIX domain acceptor type.
            typedef basic_socket_acceptor<stream_protocol> acceptor;

#if !defined(ASIO_NO_IOSTREAM)
/// The UNIX domain iostream type.
typedef basic_socket_iostream<stream_protocol> iostream;
#endif // !defined(ASIO_NO_IOSTREAM)
};

} // namespace local
} // namespace xio

#include <xio/detail/pop_options.h>

#endif // defined(ASIO_HAS_LOCAL_SOCKETS)
//   || defined(GENERATING_DOCUMENTATION)

#endif // ASIO_LOCAL_STREAM_PROTOCOL_HPP
