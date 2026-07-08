//
// ip/detail/impl/endpoint.ipp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef XIO_IP_DETAIL_IMPL_ENDPOINT_IPP
#define XIO_IP_DETAIL_IMPL_ENDPOINT_IPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>
#include <cstring>
#if !defined(XIO_NO_IOSTREAM)
# include <sstream>
#endif // !defined(XIO_NO_IOSTREAM)
#include <xio/detail/socket_ops.h>
#include <xio/detail/throw_error.h>
#include <xio/error.h>
#include <xio/ip/detail/endpoint.h>

#include <xio/detail/push_options.h>

namespace xio {


    namespace ip {
        namespace detail {
            endpoint::endpoint() noexcept
                : data_() {
                data_.v4.sin_family = XIO_OS_DEF(AF_INET);
                data_.v4.sin_port = 0;
                data_.v4.sin_addr.s_addr = XIO_OS_DEF(INADDR_ANY);
            }

            endpoint::endpoint(int family, unsigned short port_num) noexcept
                : data_() {
                using namespace std; // For memcpy.
                if (family == XIO_OS_DEF(AF_INET)) {
                    data_.v4.sin_family = XIO_OS_DEF(AF_INET);
                    data_.v4.sin_port =
                            xio::detail::socket_ops::host_to_network_short(port_num);
                    data_.v4.sin_addr.s_addr = XIO_OS_DEF(INADDR_ANY);
                } else {
                    data_.v6.sin6_family = XIO_OS_DEF(AF_INET6);
                    data_.v6.sin6_port =
                            xio::detail::socket_ops::host_to_network_short(port_num);
                    data_.v6.sin6_flowinfo = 0;
                    data_.v6.sin6_addr.s6_addr[0] = 0;
                    data_.v6.sin6_addr.s6_addr[1] = 0;
                    data_.v6.sin6_addr.s6_addr[2] = 0;
                    data_.v6.sin6_addr.s6_addr[3] = 0;
                    data_.v6.sin6_addr.s6_addr[4] = 0;
                    data_.v6.sin6_addr.s6_addr[5] = 0;
                    data_.v6.sin6_addr.s6_addr[6] = 0;
                    data_.v6.sin6_addr.s6_addr[7] = 0;
                    data_.v6.sin6_addr.s6_addr[8] = 0;
                    data_.v6.sin6_addr.s6_addr[9] = 0;
                    data_.v6.sin6_addr.s6_addr[10] = 0;
                    data_.v6.sin6_addr.s6_addr[11] = 0;
                    data_.v6.sin6_addr.s6_addr[12] = 0;
                    data_.v6.sin6_addr.s6_addr[13] = 0;
                    data_.v6.sin6_addr.s6_addr[14] = 0;
                    data_.v6.sin6_addr.s6_addr[15] = 0;
                    data_.v6.sin6_scope_id = 0;
                }
            }

            endpoint::endpoint(const xio::ip::address &addr,
                               unsigned short port_num) noexcept
                : data_() {
                using namespace std; // For memcpy.
                if (addr.is_v4()) {
                    data_.v4.sin_family = XIO_OS_DEF(AF_INET);
                    data_.v4.sin_port =
                            xio::detail::socket_ops::host_to_network_short(port_num);
                    data_.v4.sin_addr.s_addr =
                            xio::detail::socket_ops::host_to_network_long(
                                addr.to_v4().to_uint());
                } else {
                    data_.v6.sin6_family = XIO_OS_DEF(AF_INET6);
                    data_.v6.sin6_port =
                            xio::detail::socket_ops::host_to_network_short(port_num);
                    data_.v6.sin6_flowinfo = 0;
                    xio::ip::address_v6 v6_addr = addr.to_v6();
                    xio::ip::address_v6::bytes_type bytes = v6_addr.to_bytes();
                    memcpy(data_.v6.sin6_addr.s6_addr, bytes.data(), 16);
                    data_.v6.sin6_scope_id =
                            static_cast<xio::detail::u_long_type>(
                                v6_addr.scope_id());
                }
            }

            void endpoint::resize(std::size_t new_size) {
                if (new_size > sizeof(xio::detail::sockaddr_storage_type)) {
                    xio::error_code ec(xio::error::invalid_argument);
                    xio::detail::throw_error(ec);
                }
            }

            unsigned short endpoint::port() const noexcept {
                if (is_v4()) {
                    return xio::detail::socket_ops::network_to_host_short(
                        data_.v4.sin_port);
                } else {
                    return xio::detail::socket_ops::network_to_host_short(
                        data_.v6.sin6_port);
                }
            }

            void endpoint::port(unsigned short port_num) noexcept {
                if (is_v4()) {
                    data_.v4.sin_port
                            = xio::detail::socket_ops::host_to_network_short(port_num);
                } else {
                    data_.v6.sin6_port
                            = xio::detail::socket_ops::host_to_network_short(port_num);
                }
            }

            xio::ip::address endpoint::address() const noexcept {
                using namespace std; // For memcpy.
                if (is_v4()) {
                    return xio::ip::address_v4(
                        xio::detail::socket_ops::network_to_host_long(
                            data_.v4.sin_addr.s_addr));
                } else {
                    xio::ip::address_v6::bytes_type bytes;
                    memcpy(bytes.data(), data_.v6.sin6_addr.s6_addr, 16);
                    return xio::ip::address_v6(bytes, data_.v6.sin6_scope_id);
                }
            }

            void endpoint::address(const xio::ip::address &addr) noexcept {
                endpoint tmp_endpoint(addr, port());
                data_ = tmp_endpoint.data_;
            }

            bool operator==(const endpoint &e1, const endpoint &e2) noexcept {
                return e1.address() == e2.address() && e1.port() == e2.port();
            }

            bool operator<(const endpoint &e1, const endpoint &e2) noexcept {
                if (e1.address() < e2.address())
                    return true;
                if (e1.address() != e2.address())
                    return false;
                return e1.port() < e2.port();
            }

#if !defined(XIO_NO_IOSTREAM)
            std::string endpoint::to_string() const {
                std::ostringstream tmp_os;
                tmp_os.imbue(std::locale::classic());
                if (is_v4())
                    tmp_os << address();
                else
                    tmp_os << '[' << address() << ']';
                tmp_os << ':' << port();

                return tmp_os.str();
            }
#endif // !defined(XIO_NO_IOSTREAM)
        } // namespace detail
    } // namespace ip

} // namespace xio

#include <xio/detail/pop_options.h>

#endif // XIO_IP_DETAIL_IMPL_ENDPOINT_IPP
