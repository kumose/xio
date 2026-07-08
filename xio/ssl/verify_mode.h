//
// ssl/verify_mode.hpp
// ~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef XIO_SSL_VERIFY_MODE_HPP
#define XIO_SSL_VERIFY_MODE_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>
#include <xio/ssl/detail/openssl_types.h>

#include <xio/detail/push_options.h>

namespace xio {
    namespace ssl {
        /// Bitmask type for peer verification.
        /**
         * Possible values are:
         *
         * @li @ref verify_none
         * @li @ref verify_peer
         * @li @ref verify_fail_if_no_peer_cert
         * @li @ref verify_client_once
         */
        typedef int verify_mode;
        /// No verification.
        const int verify_none = SSL_VERIFY_NONE;
        /// Verify the peer.
        const int verify_peer = SSL_VERIFY_PEER;
        /// Fail verification if the peer has no certificate. Ignored unless
        /// @ref verify_peer is set.
        const int verify_fail_if_no_peer_cert = SSL_VERIFY_FAIL_IF_NO_PEER_CERT;

        /// Do not request client certificate on renegotiation. Ignored unless
        /// @ref verify_peer is set.
        const int verify_client_once = SSL_VERIFY_CLIENT_ONCE;
    } // namespace ssl
} // namespace xio

#include <xio/detail/pop_options.h>

#endif // XIO_SSL_VERIFY_MODE_HPP
