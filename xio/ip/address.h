//
// ip/address.hpp
// ~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_IP_ADDRESS_HPP
#define ASIO_IP_ADDRESS_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>
#include <functional>
#include <string>
#include <xio/detail/throw_exception.h>
#include <string_view>
#include <xio/detail/type_traits.h>
#include <xio/error_code.h>
#include <xio/ip/address_v4.h>
#include <xio/ip/address_v6.h>
#include <xio/ip/bad_address_cast.h>

#if !defined(ASIO_NO_IOSTREAM)
# include <iosfwd>
#endif // !defined(ASIO_NO_IOSTREAM)

#include <xio/detail/push_options.h>

namespace xio {
    namespace ip {
        /// Implements version-independent IP addresses.
        /**
 * The xio::ip::address class provides the ability to use either IP
 * version 4 or version 6 addresses.
 *
 * @par Thread Safety
 * @e Distinct @e objects: Safe.@n
 * @e Shared @e objects: Unsafe.
 */
        class address {
        public:
            /// Default constructor.
            ASIO_DECL address() noexcept;

            /// Construct an address from an IPv4 address.
            ASIO_DECL address(
                const xio::ip::address_v4 &ipv4_address) noexcept;

            /// Construct an address from an IPv6 address.
            ASIO_DECL address(
                const xio::ip::address_v6 &ipv6_address) noexcept;

            /// Copy constructor.
            ASIO_DECL address(const address &other) noexcept;

            /// Move constructor.
            ASIO_DECL address(address &&other) noexcept;

            /// Assign from another address.
            ASIO_DECL address &operator=(const address &other) noexcept;

            /// Move-assign from another address.
            ASIO_DECL address &operator=(address &&other) noexcept;

            /// Assign from an IPv4 address.
            ASIO_DECL address &operator=(
                const xio::ip::address_v4 &ipv4_address) noexcept;

            /// Assign from an IPv6 address.
            ASIO_DECL address &operator=(
                const xio::ip::address_v6 &ipv6_address) noexcept;

            /// Get whether the address is an IP version 4 address.
            bool is_v4() const noexcept {
                return type_ == ipv4;
            }

            /// Get whether the address is an IP version 6 address.
            bool is_v6() const noexcept {
                return type_ == ipv6;
            }

            /// Get the address as an IP version 4 address.
            ASIO_DECL xio::ip::address_v4 to_v4() const;

            /// Get the address as an IP version 6 address.
            ASIO_DECL xio::ip::address_v6 to_v6() const;

            /// Get the address as a string.
            ASIO_DECL std::string to_string() const;

            /// Determine whether the address is a loopback address.
            ASIO_DECL bool is_loopback() const noexcept;

            /// Determine whether the address is unspecified.
            ASIO_DECL bool is_unspecified() const noexcept;

            /// Determine whether the address is a multicast address.
            ASIO_DECL bool is_multicast() const noexcept;

            /// Compare two addresses for equality.
            ASIO_DECL friend bool operator==(const address &a1,
                                             const address &a2) noexcept;

            /// Compare two addresses for inequality.
            friend bool operator!=(const address &a1,
                                   const address &a2) noexcept {
                return !(a1 == a2);
            }

            /// Compare addresses for ordering.
            ASIO_DECL friend bool operator<(const address &a1,
                                            const address &a2) noexcept;

            /// Compare addresses for ordering.
            friend bool operator>(const address &a1,
                                  const address &a2) noexcept {
                return a2 < a1;
            }

            /// Compare addresses for ordering.
            friend bool operator<=(const address &a1,
                                   const address &a2) noexcept {
                return !(a2 < a1);
            }

            /// Compare addresses for ordering.
            friend bool operator>=(const address &a1,
                                   const address &a2) noexcept {
                return !(a1 < a2);
            }

        private:
            // The type of the address.
            enum { ipv4, ipv6 } type_;

            // The underlying IPv4 address.
            xio::ip::address_v4 ipv4_address_;

            // The underlying IPv6 address.
            xio::ip::address_v6 ipv6_address_;
        };

        /// Create an address from an IPv4 address string in dotted decimal form,
/// or from an IPv6 address in hexadecimal notation.
        /**
 * @relates address
 */
        ASIO_DECL address make_address(const char *str);

        /// Create an address from an IPv4 address string in dotted decimal form,
/// or from an IPv6 address in hexadecimal notation.
        /**
 * @relates address
 */
        ASIO_DECL address make_address(const char *str,
                                       xio::error_code &ec) noexcept;

        /// Create an address from an IPv4 address string in dotted decimal form,
/// or from an IPv6 address in hexadecimal notation.
        /**
 * @relates address
 */
        ASIO_DECL address make_address(const std::string &str);

        /// Create an address from an IPv4 address string in dotted decimal form,
/// or from an IPv6 address in hexadecimal notation.
        /**
 * @relates address
 */
        ASIO_DECL address make_address(const std::string &str,
                                       xio::error_code &ec) noexcept;


        /// Create an address from an IPv4 address string in dotted decimal form,
/// or from an IPv6 address in hexadecimal notation.
        /**
 * @relates address
 */
        ASIO_DECL address make_address(std::string_view str);

        /// Create an address from an IPv4 address string in dotted decimal form,
/// or from an IPv6 address in hexadecimal notation.
        /**
 * @relates address
 */
        ASIO_DECL address make_address(std::string_view str,
                                       xio::error_code &ec) noexcept;


#if !defined(ASIO_NO_IOSTREAM)

        /// Output an address as a string.
        /**
 * Used to output a human-readable string for a specified address.
 *
 * @param os The output stream to which the string will be written.
 *
 * @param addr The address to be written.
 *
 * @return The output stream.
 *
 * @relates xio::ip::address
 */
        template<typename Elem, typename Traits>
        std::basic_ostream<Elem, Traits> &operator<<(
            std::basic_ostream<Elem, Traits> &os, const address &addr);

#endif // !defined(ASIO_NO_IOSTREAM)
    } // namespace ip
} // namespace xio

namespace std {
    template<>
    struct hash<xio::ip::address> {
        std::size_t operator()(const xio::ip::address &addr)
        const noexcept {
            return addr.is_v4()
                       ? std::hash<xio::ip::address_v4>()(addr.to_v4())
                       : std::hash<xio::ip::address_v6>()(addr.to_v6());
        }
    };
} // namespace std

#include <xio/detail/pop_options.h>

#include <xio/ip/impl/address.h>

#endif // ASIO_IP_ADDRESS_HPP
