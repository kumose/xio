//
// local/detail/impl/endpoint.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
// Derived from a public domain implementation written by Daniel Casimiro.
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_LOCAL_DETAIL_IMPL_ENDPOINT_IPP
#define ASIO_LOCAL_DETAIL_IMPL_ENDPOINT_IPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>

#if defined(ASIO_HAS_LOCAL_SOCKETS)

#include <cstring>
#include <xio/detail/socket_ops.h>
#include <xio/detail/throw_error.h>
#include <xio/error.h>
#include <xio/local/detail/endpoint.h>

#include <xio/detail/push_options.h>

namespace xio {


    namespace local {
        namespace detail {
            endpoint::endpoint() noexcept {
                init("", 0);
            }

            endpoint::endpoint(const char *path_name) {
                using namespace std; // For strlen.
                init(path_name, strlen(path_name));
            }

            endpoint::endpoint(const std::string &path_name) {
                init(path_name.data(), path_name.length());
            }

endpoint::endpoint(std::string_view path_name) {
    init(path_name.data(), path_name.length());
}

void endpoint::resize(std::size_t new_size) {
    if (new_size > sizeof(xio::detail::sockaddr_un_type)) {
        xio::error_code ec(xio::error::invalid_argument);
        xio::detail::throw_error(ec);
    } else if (new_size == 0) {
        path_length_ = 0;
    } else {
        path_length_ = new_size
                       - offsetof(xio::detail::sockaddr_un_type, sun_path);

        // The path returned by the operating system may be NUL-terminated.
        if (path_length_ > 0 && data_.local.sun_path[path_length_ - 1] == 0)
            --path_length_;
    }
}

std::string endpoint::path() const {
    return std::string(data_.local.sun_path, path_length_);
}

void endpoint::path(const char *p) {
    using namespace std; // For strlen.
    init(p, strlen(p));
}

void endpoint::path(const std::string &p) {
    init(p.data(), p.length());
}

bool operator==(const endpoint &e1, const endpoint &e2) noexcept {
    return e1.path() == e2.path();
}

bool operator<(const endpoint &e1, const endpoint &e2) noexcept {
    return e1.path() < e2.path();
}

void endpoint::init(const char *path_name, std::size_t path_length) {
    if (path_length > sizeof(data_.local.sun_path) - 1) {
        // The buffer is not large enough to store this address.
        xio::error_code ec(xio::error::name_too_long);
        xio::detail::throw_error(ec);
    }

    using namespace std; // For memset and memcpy.
    memset(&data_.local, 0, sizeof(xio::detail::sockaddr_un_type));
    data_.local.sun_family = AF_UNIX;
    if (path_length > 0)
        memcpy(data_.local.sun_path, path_name, path_length);
    path_length_ = path_length;
}

} // namespace detail
} // namespace local
} // namespace xio

#include <xio/detail/pop_options.h>

#endif // defined(ASIO_HAS_LOCAL_SOCKETS)

#endif // ASIO_LOCAL_DETAIL_IMPL_ENDPOINT_IPP
