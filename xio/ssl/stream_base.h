//
// ssl/stream_base.hpp
// ~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef XIO_SSL_STREAM_BASE_HPP
#define XIO_SSL_STREAM_BASE_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>

#include <xio/detail/push_options.h>

namespace xio {


    namespace ssl {
        /// The stream_base class is used as a base for the xio::ssl::stream
/// class template so that we have a common place to define various enums.
        class stream_base {
        public:
            /// Different handshake types.
            enum handshake_type {
                /// Perform handshaking as a client.
                client,

                /// Perform handshaking as a server.
                server
            };

        protected:
            /// Protected destructor to prevent deletion through this type.
            ~stream_base() {
            }
        };
    } // namespace ssl

} // namespace xio

#include <xio/detail/pop_options.h>

#endif // XIO_SSL_STREAM_BASE_HPP
