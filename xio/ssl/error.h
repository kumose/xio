//
// ssl/error.hpp
// ~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef XIO_SSL_ERROR_HPP
#define XIO_SSL_ERROR_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>
#include <xio/error_code.h>
#include <xio/ssl/detail/openssl_types.h>

#include <xio/detail/push_options.h>

namespace xio {


    namespace error {
        enum ssl_errors {
            // Error numbers are those produced by openssl.
        };

        extern
        XIO_DECL
        const xio::error_category &get_ssl_category();

        inline const xio::error_category &
                ssl_category XIO_UNUSED_VARIABLE
                = xio::error::get_ssl_category();
    } // namespace error
    namespace ssl {
        namespace error {
            enum stream_errors {
# if (OPENSSL_VERSION_NUMBER < 0x10100000L) \
    && !defined(OPENSSL_IS_BORINGSSL) \
    && !defined(XIO_USE_WOLFSSL)
                stream_truncated= ERR_PACK(ERR_LIB_SSL, 0, SSL_R_SHORT_READ),
# else
                stream_truncated = 1,
# endif
                unspecified_system_error = 2,
                unexpected_result = 3
            };

            extern
            XIO_DECL
            const xio::error_category &get_stream_category();

            inline const xio::error_category &
                    stream_category XIO_UNUSED_VARIABLE
                    = xio::ssl::error::get_stream_category();
        } // namespace error
    } // namespace ssl

} // namespace xio

namespace std {
    template<>
    struct is_error_code_enum<xio::error::ssl_errors> {
        static const bool value = true;
    };

    template<>
    struct is_error_code_enum<xio::ssl::error::stream_errors> {
        static const bool value = true;
    };
} // namespace std

namespace xio {


    namespace error {
        inline xio::error_code make_error_code(ssl_errors e) {
            return xio::error_code(
                static_cast<int>(e), get_ssl_category());
        }
    } // namespace error
    namespace ssl {
        namespace error {
            inline xio::error_code make_error_code(stream_errors e) {
                return xio::error_code(
                    static_cast<int>(e), get_stream_category());
            }
        } // namespace error
    } // namespace ssl

} // namespace xio

#include <xio/detail/pop_options.h>


#endif // XIO_SSL_ERROR_HPP
