//
// ip/impl/basic_endpoint.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef XIO_IP_IMPL_BASIC_ENDPOINT_HPP
#define XIO_IP_IMPL_BASIC_ENDPOINT_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#if !defined(XIO_NO_IOSTREAM)

#include <xio/detail/throw_error.h>

#include <xio/detail/push_options.h>

namespace xio {


    namespace ip {
        template<typename Elem, typename Traits, typename InternetProtocol>
        std::basic_ostream<Elem, Traits> &operator<<(
            std::basic_ostream<Elem, Traits> &os,
            const basic_endpoint<InternetProtocol> &endpoint) {
            xio::ip::detail::endpoint tmp_ep(endpoint.address(), endpoint.port());
            return os << tmp_ep.to_string().c_str();
        }
    } // namespace ip

} // namespace xio

#include <xio/detail/pop_options.h>

#endif // !defined(XIO_NO_IOSTREAM)

#endif // XIO_IP_IMPL_BASIC_ENDPOINT_HPP
