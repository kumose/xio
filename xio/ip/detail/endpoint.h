//
// ip/detail/endpoint.hpp
// ~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_IP_DETAIL_ENDPOINT_HPP
#define ASIO_IP_DETAIL_ENDPOINT_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>
#include <string>
#include <xio/detail/socket_types.h>
#include <xio/detail/winsock_init.h>
#include <xio/error_code.h>
#include <xio/ip/address.h>

#include <xio/detail/push_options.h>

namespace xio {
    ASIO_INLINE_NAMESPACE_BEGIN

    namespace ip {
        namespace detail {
            // Helper class for implementing an IP endpoint.
            class endpoint {
            public:
                // Default constructor.
                ASIO_DECL endpoint() noexcept;

                // Construct an endpoint using a family and port number.
                ASIO_DECL endpoint(int family,
                                   unsigned short port_num) noexcept;

                // Construct an endpoint using an address and port number.
                ASIO_DECL endpoint(const xio::ip::address &addr,
                                   unsigned short port_num) noexcept;

                // Copy constructor.
                endpoint(const endpoint &other) noexcept
                    : data_(other.data_) {
                }

                // Assign from another endpoint.
                endpoint &operator=(const endpoint &other) noexcept {
                    data_ = other.data_;
                    return *this;
                }

                // Get the underlying endpoint in the native type.
                xio::detail::socket_addr_type *data() noexcept {
                    return &data_.base;
                }

                // Get the underlying endpoint in the native type.
                const xio::detail::socket_addr_type *data() const noexcept {
                    return &data_.base;
                }

                // Get the underlying size of the endpoint in the native type.
                std::size_t size() const noexcept {
                    if (is_v4())
                        return sizeof(xio::detail::sockaddr_in4_type);
                    else
                        return sizeof(xio::detail::sockaddr_in6_type);
                }

                // Set the underlying size of the endpoint in the native type.
  ASIO_DECL void resize(std::size_t new_size);

                // Get the capacity of the endpoint in the native type.
                std::size_t capacity() const noexcept {
                    return sizeof(data_);
                }

                // Get the port associated with the endpoint.
  ASIO_DECL unsigned short port() const noexcept;

                // Set the port associated with the endpoint.
  ASIO_DECL void port(unsigned short port_num) noexcept;

                // Get the IP address associated with the endpoint.
                ASIO_DECL xio::ip::address address() const noexcept;

                // Set the IP address associated with the endpoint.
  ASIO_DECL void address(
                    const xio::ip::address &addr) noexcept;

                // Compare two endpoints for equality.
  ASIO_DECL friend bool operator==(const endpoint &e1,
                                   const endpoint &e2) noexcept;

                // Compare endpoints for ordering.
  ASIO_DECL friend bool operator<(const endpoint &e1,
                                  const endpoint &e2) noexcept;

                // Determine whether the endpoint is IPv4.
                bool is_v4() const noexcept {
                    return data_.base.sa_family == ASIO_OS_DEF(AF_INET);
                }

#if !defined(ASIO_NO_IOSTREAM)
                // Convert to a string.
                ASIO_DECL std::string to_string() const;
#endif // !defined(ASIO_NO_IOSTREAM)

            private:
                // The underlying IP socket address.
                union data_union {
                    xio::detail::socket_addr_type base;
                    xio::detail::sockaddr_in4_type v4;
                    xio::detail::sockaddr_in6_type v6;
                } data_;
            };
        } // namespace detail
    } // namespace ip
    ASIO_INLINE_NAMESPACE_END
} // namespace xio

#include <xio/detail/pop_options.h>

#if defined(ASIO_HEADER_ONLY)
# include "xio/ip/detail/impl/endpoint.ipp"
#endif // defined(ASIO_HEADER_ONLY)

#endif // ASIO_IP_DETAIL_ENDPOINT_HPP
