//
// ip/resolver_base.hpp
// ~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef XIO_IP_RESOLVER_BASE_HPP
#define XIO_IP_RESOLVER_BASE_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>
#include <xio/detail/socket_types.h>

#include <xio/detail/push_options.h>

namespace xio {


    namespace ip {
        /// The resolver_base class is used as a base for the basic_resolver class
/// templates to provide a common place to define the flag constants.
        class resolver_base {
        public:
            enum flags {
                canonical_name = XIO_OS_DEF(AI_CANONNAME),
                passive = XIO_OS_DEF(AI_PASSIVE),
                numeric_host = XIO_OS_DEF(AI_NUMERICHOST),
                numeric_service = XIO_OS_DEF(AI_NUMERICSERV),
                v4_mapped = XIO_OS_DEF(AI_V4MAPPED),
                all_matching = XIO_OS_DEF(AI_ALL),
                address_configured = XIO_OS_DEF(AI_ADDRCONFIG)
            };

            // Implement bitmask operations as shown in C++ Std [lib.bitmask.types].

            friend flags operator&(flags x, flags y) {
                return static_cast<flags>(
                    static_cast<unsigned int>(x) & static_cast<unsigned int>(y));
            }

            friend flags operator|(flags x, flags y) {
                return static_cast<flags>(
                    static_cast<unsigned int>(x) | static_cast<unsigned int>(y));
            }

            friend flags operator^(flags x, flags y) {
                return static_cast<flags>(
                    static_cast<unsigned int>(x) ^ static_cast<unsigned int>(y));
            }

            friend flags operator~(flags x) {
                return static_cast<flags>(~static_cast<unsigned int>(x));
            }

            friend flags &operator&=(flags &x, flags y) {
                x = x & y;
                return x;
            }

            friend flags &operator|=(flags &x, flags y) {
                x = x | y;
                return x;
            }

            friend flags &operator^=(flags &x, flags y) {
                x = x ^ y;
                return x;
            }

        protected:
            /// Protected destructor to prevent deletion through this type.
            ~resolver_base() {
            }
        };
    } // namespace ip

} // namespace xio

#include <xio/detail/pop_options.h>

#endif // XIO_IP_RESOLVER_BASE_HPP
