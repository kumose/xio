//
// local/connect_pair.hpp
// ~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef XIO_LOCAL_CONNECT_PAIR_HPP
#define XIO_LOCAL_CONNECT_PAIR_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>

#if defined(XIO_HAS_LOCAL_SOCKETS)
#include <xio/basic_socket.h>
#include <xio/detail/socket_ops.h>
#include <xio/detail/throw_error.h>
#include <xio/error.h>
#include <xio/local/basic_endpoint.h>

#include <xio/detail/push_options.h>

namespace xio {


    namespace local {
        /// Create a pair of connected sockets.
        template<typename Protocol, typename Executor1, typename Executor2>
        void connect_pair(basic_socket<Protocol, Executor1> & socket1,
                          basic_socket<Protocol, Executor2> & socket2);

        /// Create a pair of connected sockets.
        template<typename Protocol, typename Executor1, typename Executor2>
        XIO_SYNC_OP_VOID connect_pair(basic_socket<Protocol, Executor1> & socket1,
                                       basic_socket<Protocol, Executor2> & socket2, xio::error_code & ec);

        template<typename Protocol, typename Executor1, typename Executor2>
        inline void connect_pair(basic_socket<Protocol, Executor1> &socket1,
                                 basic_socket<Protocol, Executor2> &socket2) {
            xio::error_code ec;
            connect_pair(socket1, socket2, ec);
            xio::detail::throw_error(ec, "connect_pair");
        }

        template<typename Protocol, typename Executor1, typename Executor2>
        inline XIO_SYNC_OP_VOID connect_pair(
            basic_socket<Protocol, Executor1> &socket1,
            basic_socket<Protocol, Executor2> &socket2, xio::error_code &ec) {
            // Check that this function is only being used with a UNIX domain socket.
            xio::local::basic_endpoint<Protocol> *tmp
                    = static_cast<typename Protocol::endpoint *>(0);
            (void) tmp;

            Protocol protocol;
            xio::detail::socket_type sv[2];
            if (xio::detail::socket_ops::socketpair(protocol.family(),
                                                    protocol.type(), protocol.protocol(), sv, ec)
                == xio::detail::socket_error_retval)
                XIO_SYNC_OP_VOID_RETURN(ec);

            socket1.assign(protocol, sv[0], ec);
            if (ec) {
                xio::error_code temp_ec;
                xio::detail::socket_ops::state_type state[2] = {0, 0};
                xio::detail::socket_ops::close(sv[0], state[0], true, temp_ec);
                xio::detail::socket_ops::close(sv[1], state[1], true, temp_ec);
                XIO_SYNC_OP_VOID_RETURN(ec);
            }

            socket2.assign(protocol, sv[1], ec);
            if (ec) {
                xio::error_code temp_ec;
                socket1.close(temp_ec);
                xio::detail::socket_ops::state_type state = 0;
                xio::detail::socket_ops::close(sv[1], state, true, temp_ec);
                XIO_SYNC_OP_VOID_RETURN(ec);
            }

            XIO_SYNC_OP_VOID_RETURN(ec);
        }
    } // namespace local

} // namespace xio

#include <xio/detail/pop_options.h>

#endif // defined(XIO_HAS_LOCAL_SOCKETS)

#endif // XIO_LOCAL_CONNECT_PAIR_HPP
