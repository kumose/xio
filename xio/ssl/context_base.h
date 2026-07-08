//
// ssl/context_base.hpp
// ~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef XIO_SSL_CONTEXT_BASE_HPP
#define XIO_SSL_CONTEXT_BASE_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>
#include <xio/ssl/detail/openssl_types.h>

#include <xio/detail/push_options.h>

namespace xio {


    namespace ssl {
        /// The context_base class is used as a base for the basic_context class
        /// template so that we have a common place to define various enums.
        class context_base {
        public:
            /// Different methods supported by a context.
            enum method {
                /// Generic SSL version 2.
                sslv2,

                /// SSL version 2 client.
                sslv2_client,

                /// SSL version 2 server.
                sslv2_server,

                /// Generic SSL version 3.
                sslv3,

                /// SSL version 3 client.
                sslv3_client,

                /// SSL version 3 server.
                sslv3_server,

                /// Generic TLS version 1.
                tlsv1,

                /// TLS version 1 client.
                tlsv1_client,

                /// TLS version 1 server.
                tlsv1_server,

                /// Generic SSL/TLS.
                sslv23,

                /// SSL/TLS client.
                sslv23_client,

                /// SSL/TLS server.
                sslv23_server,

                /// Generic TLS version 1.1.
                tlsv11,

                /// TLS version 1.1 client.
                tlsv11_client,

                /// TLS version 1.1 server.
                tlsv11_server,

                /// Generic TLS version 1.2.
                tlsv12,

                /// TLS version 1.2 client.
                tlsv12_client,

                /// TLS version 1.2 server.
                tlsv12_server,

                /// Generic TLS version 1.3.
                tlsv13,

                /// TLS version 1.3 client.
                tlsv13_client,

                /// TLS version 1.3 server.
                tlsv13_server,

                /// Generic TLS.
                tls,

                /// TLS client.
                tls_client,

                /// TLS server.
                tls_server
            };

            /// Bitmask type for SSL options.
            typedef uint64_t options;

            XIO_STATIC_CONSTANT(uint64_t, default_workarounds = SSL_OP_ALL);

            XIO_STATIC_CONSTANT(uint64_t, single_dh_use = SSL_OP_SINGLE_DH_USE);

            XIO_STATIC_CONSTANT(uint64_t, no_sslv2 = SSL_OP_NO_SSLv2);

            XIO_STATIC_CONSTANT(uint64_t, no_sslv3 = SSL_OP_NO_SSLv3);

            XIO_STATIC_CONSTANT(uint64_t, no_tlsv1 = SSL_OP_NO_TLSv1);
# if defined(SSL_OP_NO_TLSv1_1)
            XIO_STATIC_CONSTANT(uint64_t, no_tlsv1_1 = SSL_OP_NO_TLSv1_1);
# else // defined(SSL_OP_NO_TLSv1_1)
            XIO_STATIC_CONSTANT(uint64_t, no_tlsv1_1 = 0x10000000L);
# endif // defined(SSL_OP_NO_TLSv1_1)
# if defined(SSL_OP_NO_TLSv1_2)
            XIO_STATIC_CONSTANT(uint64_t, no_tlsv1_2 = SSL_OP_NO_TLSv1_2);
# else // defined(SSL_OP_NO_TLSv1_2)
            XIO_STATIC_CONSTANT(uint64_t, no_tlsv1_2 = 0x08000000L);
# endif // defined(SSL_OP_NO_TLSv1_2)
# if defined(SSL_OP_NO_TLSv1_3)
            XIO_STATIC_CONSTANT(uint64_t, no_tlsv1_3 = SSL_OP_NO_TLSv1_3);
# else // defined(SSL_OP_NO_TLSv1_3)
            XIO_STATIC_CONSTANT(uint64_t, no_tlsv1_3 = 0x20000000L);
# endif // defined(SSL_OP_NO_TLSv1_3)
# if defined(SSL_OP_NO_COMPRESSION)
            XIO_STATIC_CONSTANT(uint64_t, no_compression = SSL_OP_NO_COMPRESSION);
# else // defined(SSL_OP_NO_COMPRESSION)
            XIO_STATIC_CONSTANT(uint64_t, no_compression = 0x20000L);
# endif // defined(SSL_OP_NO_COMPRESSION)

            /// File format types.
            enum file_format {
                /// ASN.1 file.
                asn1,

                /// PEM file.
                pem
            };

            // The following types and constants are preserved for backward compatibility.
            // New programs should use the equivalents of the same names that are defined
            // in the xio::ssl namespace.
            typedef int verify_mode;

            XIO_STATIC_CONSTANT(int, verify_none = SSL_VERIFY_NONE);

            XIO_STATIC_CONSTANT(int, verify_peer = SSL_VERIFY_PEER);

            XIO_STATIC_CONSTANT(int,
                                 verify_fail_if_no_peer_cert = SSL_VERIFY_FAIL_IF_NO_PEER_CERT);

            XIO_STATIC_CONSTANT(int, verify_client_once = SSL_VERIFY_CLIENT_ONCE);

            /// Purpose of PEM password.
            enum password_purpose {
                /// The password is needed for reading/decryption.
                for_reading,

                /// The password is needed for writing/encryption.
                for_writing
            };

        protected:
            /// Protected destructor to prevent deletion through this type.
            ~context_base() {
            }
        };
    } // namespace ssl

} // namespace xio

#include <xio/detail/pop_options.h>

#endif // XIO_SSL_CONTEXT_BASE_HPP
