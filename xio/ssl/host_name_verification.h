//
// ssl/host_name_verification.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef XIO_SSL_HOST_NAME_VERIFICATION_HPP
#define XIO_SSL_HOST_NAME_VERIFICATION_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>

#include <string>
#include <xio/ssl/detail/openssl_types.h>
#include <xio/ssl/verify_context.h>

#include <xio/detail/push_options.h>

namespace xio {


    namespace ssl {
        /// Verifies a certificate against a host_name according to the rules described
/// in RFC 6125.
        /**
 * @par Example
 * The following example shows how to synchronously open a secure connection to
 * a given host name:
 * @code
 * using xio::ip::tcp;
 * namespace ssl = xio::ssl;
 * typedef ssl::stream<tcp::socket> ssl_socket;
 *
 * // Create a context that uses the default paths for finding CA certificates.
 * ssl::context ctx(ssl::context::sslv23);
 * ctx.set_default_verify_paths();
 *
 * // Open a socket and connect it to the remote host.
 * xio::io_context io_context;
 * ssl_socket sock(io_context, ctx);
 * tcp::resolver resolver(io_context);
 * tcp::resolver::query query("host.name", "https");
 * xio::connect(sock.lowest_layer(), resolver.resolve(query));
 * sock.lowest_layer().set_option(tcp::no_delay(true));
 *
 * // Perform SSL handshake and verify the remote host's certificate.
 * sock.set_verify_mode(ssl::verify_peer);
 * sock.set_verify_callback(ssl::host_name_verification("host.name"));
 * sock.handshake(ssl_socket::client);
 *
 * // ... read and write as normal ...
 * @endcode
 */
        class host_name_verification {
        public:
            /// The type of the function object's result.
            typedef bool result_type;

            /// Constructor.
            explicit host_name_verification(const std::string &host)
                : host_(host) {
            }

            /// Perform certificate verification.
            XIO_DECL bool operator()(bool preverified, verify_context &ctx) const;

        private:
            // Helper function to check a host name against an IPv4 address
            // The host name to be checked.
            std::string host_;
        };
    } // namespace ssl

} // namespace xio

#include <xio/detail/pop_options.h>

#endif // XIO_SSL_HOST_NAME_VERIFICATION_HPP
