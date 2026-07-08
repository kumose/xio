//
// ip/network_v6.hpp
// ~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
// Copyright (c) 2014 Oliver Kowalke (oliver dot kowalke at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef XIO_IP_NETWORK_V6_HPP
#define XIO_IP_NETWORK_V6_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>
#include <string>
#include <string_view>
#include <xio/error_code.h>
#include <xio/ip/address_v6_range.h>

#include <xio/detail/push_options.h>

namespace xio {
    namespace ip {
        /// Represents an IPv6 network.
        /**
 * The xio::ip::network_v6 class provides the ability to use and
 * manipulate IP version 6 networks.
 *
 * @par Thread Safety
 * @e Distinct @e objects: Safe.@n
 * @e Shared @e objects: Unsafe.
 */
        class network_v6 {
        public:
            /// Default constructor.
            network_v6() noexcept
                : address_(),
                  prefix_length_(0) {
            }

            /// Construct a network based on the specified address and prefix length.
            XIO_DECL network_v6(const address_v6 &addr,
                                unsigned short prefix_len);

            /// Copy constructor.
            network_v6(const network_v6 &other) noexcept
                : address_(other.address_),
                  prefix_length_(other.prefix_length_) {
            }

            /// Move constructor.
            network_v6(network_v6 &&other) noexcept
                : address_(static_cast<address_v6 &&>(other.address_)),
                  prefix_length_(other.prefix_length_) {
            }

            /// Assign from another network.
            network_v6 &operator=(const network_v6 &other) noexcept {
                address_ = other.address_;
                prefix_length_ = other.prefix_length_;
                return *this;
            }

            /// Move-assign from another network.
            network_v6 &operator=(network_v6 &&other) noexcept {
                address_ = static_cast<address_v6 &&>(other.address_);
                prefix_length_ = other.prefix_length_;
                return *this;
            }

            /// Obtain the address object specified when the network object was created.
            address_v6 address() const noexcept {
                return address_;
            }

            /// Obtain the prefix length that was specified when the network object was
  /// created.
            unsigned short prefix_length() const noexcept {
                return prefix_length_;
            }

            /// Obtain an address object that represents the network address.
            XIO_DECL address_v6 network() const noexcept;

            /// Obtain an address range corresponding to the hosts in the network.
            XIO_DECL address_v6_range hosts() const noexcept;

            /// Obtain the true network address, omitting any host bits.
            network_v6 canonical() const noexcept {
                return network_v6(network(), prefix_length());
            }

            /// Test if network is a valid host address.
            bool is_host() const noexcept {
                return prefix_length_ == 128;
            }

            /// Test if a network is a real subnet of another network.
            XIO_DECL bool is_subnet_of(const network_v6 &other) const;

            /// Get the network as an address in dotted decimal format.
            XIO_DECL std::string to_string() const;

            /// Get the network as an address in dotted decimal format.
            XIO_DECL std::string to_string(xio::error_code &ec) const;

            /// Compare two networks for equality.
            friend bool operator==(const network_v6 &a, const network_v6 &b) {
                return a.address_ == b.address_ && a.prefix_length_ == b.prefix_length_;
            }

            /// Compare two networks for inequality.
            friend bool operator!=(const network_v6 &a, const network_v6 &b) {
                return !(a == b);
            }

        private:
            address_v6 address_;
            unsigned short prefix_length_;
        };

        /// Create an IPv6 network from an address and prefix length.
        /**
 * @relates address_v6
 */
        inline network_v6 make_network_v6(
            const address_v6 &addr, unsigned short prefix_len) {
            return network_v6(addr, prefix_len);
        }

        /// Create an IPv6 network from a string containing IP address and prefix
/// length.
        /**
 * @relates network_v6
 */
        XIO_DECL network_v6 make_network_v6(const char *str);

        /// Create an IPv6 network from a string containing IP address and prefix
/// length.
        /**
 * @relates network_v6
 */
        XIO_DECL network_v6 make_network_v6(
            const char *str, xio::error_code &ec);

        /// Create an IPv6 network from a string containing IP address and prefix
/// length.
        /**
 * @relates network_v6
 */
        XIO_DECL network_v6 make_network_v6(const std::string &str);

        /// Create an IPv6 network from a string containing IP address and prefix
/// length.
        /**
 * @relates network_v6
 */
        XIO_DECL network_v6 make_network_v6(
            const std::string &str, xio::error_code &ec);


        /// Create an IPv6 network from a string containing IP address and prefix
        /// length.
        /**
         * @relates network_v6
         */
        XIO_DECL network_v6 make_network_v6(std::string_view str);

        /// Create an IPv6 network from a string containing IP address and prefix
        /// length.
        /**
         * @relates network_v6
         */
        XIO_DECL network_v6 make_network_v6(
            std::string_view str, xio::error_code &ec);


        /// Output a network as a string.
        /**
         * Used to output a human-readable string for a specified network.
         *
         * @param os The output stream to which the string will be written.
         *
         * @param net The network to be written.
         *
         * @return The output stream.
         *
         * @relates xio::ip::address_v6
         */
        template<typename Elem, typename Traits>
        std::basic_ostream<Elem, Traits> &operator<<(
            std::basic_ostream<Elem, Traits> &os, const network_v6 &net);
    } // namespace ip
} // namespace xio

#include <xio/detail/pop_options.h>

#include <xio/ip/impl/network_v6.h>


#endif // XIO_IP_NETWORK_V6_HPP
