//
// ip/impl/address.ipp
// ~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_IP_IMPL_ADDRESS_IPP
#define ASIO_IP_IMPL_ADDRESS_IPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>
#include <typeinfo>
#include <xio/detail/throw_error.h>
#include <xio/detail/throw_exception.h>
#include <xio/error.h>
#include <xio/ip/address.h>
#include <xio/ip/bad_address_cast.h>
#include <xio/system_error.h>

#include <xio/detail/push_options.h>

namespace xio {


    namespace ip {
        address::address() noexcept
            : type_(ipv4),
              ipv4_address_(),
              ipv6_address_() {
        }

        address::address(
            const xio::ip::address_v4 &ipv4_address) noexcept
            : type_(ipv4),
              ipv4_address_(ipv4_address),
              ipv6_address_() {
        }

        address::address(
            const xio::ip::address_v6 &ipv6_address) noexcept
            : type_(ipv6),
              ipv4_address_(),
              ipv6_address_(ipv6_address) {
        }

        address::address(const address &other) noexcept
            : type_(other.type_),
              ipv4_address_(other.ipv4_address_),
              ipv6_address_(other.ipv6_address_) {
        }

        address::address(address &&other) noexcept
            : type_(other.type_),
              ipv4_address_(other.ipv4_address_),
              ipv6_address_(other.ipv6_address_) {
        }

        address &address::operator=(const address &other) noexcept {
            type_ = other.type_;
            ipv4_address_ = other.ipv4_address_;
            ipv6_address_ = other.ipv6_address_;
            return *this;
        }

        address &address::operator=(address &&other) noexcept {
            type_ = other.type_;
            ipv4_address_ = other.ipv4_address_;
            ipv6_address_ = other.ipv6_address_;
            return *this;
        }

        address &address::operator=(
            const xio::ip::address_v4 &ipv4_address) noexcept {
            type_ = ipv4;
            ipv4_address_ = ipv4_address;
            ipv6_address_ = xio::ip::address_v6();
            return *this;
        }

        address &address::operator=(
            const xio::ip::address_v6 &ipv6_address) noexcept {
            type_ = ipv6;
            ipv4_address_ = xio::ip::address_v4();
            ipv6_address_ = ipv6_address;
            return *this;
        }

        address make_address(const char *str) {
            xio::error_code ec;
            address addr = make_address(str, ec);
            xio::detail::throw_error(ec);
            return addr;
        }

        address make_address(const char *str,
                             xio::error_code &ec) noexcept {
            xio::ip::address_v6 ipv6_address =
                    xio::ip::make_address_v6(str, ec);
            if (!ec)
                return address(ipv6_address);

            xio::ip::address_v4 ipv4_address =
                    xio::ip::make_address_v4(str, ec);
            if (!ec)
                return address(ipv4_address);

            return address();
        }

        address make_address(const std::string &str) {
            return make_address(str.c_str());
        }

        address make_address(const std::string &str,
                             xio::error_code &ec) noexcept {
            return make_address(str.c_str(), ec);
        }


        address make_address(string_view str) {
            return make_address(static_cast<std::string>(str));
        }

        address make_address(string_view str,
                             xio::error_code &ec) noexcept {
            return make_address(static_cast<std::string>(str), ec);
        }


        xio::ip::address_v4 address::to_v4() const {
            if (type_ != ipv4) {
                bad_address_cast ex;
                xio::detail::throw_exception(ex);
            }
            return ipv4_address_;
        }

        xio::ip::address_v6 address::to_v6() const {
            if (type_ != ipv6) {
                bad_address_cast ex;
                xio::detail::throw_exception(ex);
            }
            return ipv6_address_;
        }

        std::string address::to_string() const {
            if (type_ == ipv6)
                return ipv6_address_.to_string();
            return ipv4_address_.to_string();
        }

        bool address::is_loopback() const noexcept {
            return (type_ == ipv4)
                       ? ipv4_address_.is_loopback()
                       : ipv6_address_.is_loopback();
        }

        bool address::is_unspecified() const noexcept {
            return (type_ == ipv4)
                       ? ipv4_address_.is_unspecified()
                       : ipv6_address_.is_unspecified();
        }

        bool address::is_multicast() const noexcept {
            return (type_ == ipv4)
                       ? ipv4_address_.is_multicast()
                       : ipv6_address_.is_multicast();
        }

        bool operator==(const address &a1, const address &a2) noexcept {
            if (a1.type_ != a2.type_)
                return false;
            if (a1.type_ == address::ipv6)
                return a1.ipv6_address_ == a2.ipv6_address_;
            return a1.ipv4_address_ == a2.ipv4_address_;
        }

        bool operator<(const address &a1, const address &a2) noexcept {
            if (a1.type_ < a2.type_)
                return true;
            if (a1.type_ > a2.type_)
                return false;
            if (a1.type_ == address::ipv6)
                return a1.ipv6_address_ < a2.ipv6_address_;
            return a1.ipv4_address_ < a2.ipv4_address_;
        }
    } // namespace ip

} // namespace xio

#include <xio/detail/pop_options.h>

#endif // ASIO_IP_IMPL_ADDRESS_IPP
