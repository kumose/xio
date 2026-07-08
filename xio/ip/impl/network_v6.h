//
// ip/impl/network_v6.hpp
// ~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef XIO_IP_IMPL_NETWORK_V6_HPP
#define XIO_IP_IMPL_NETWORK_V6_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#if !defined(XIO_NO_IOSTREAM)

#include <xio/detail/throw_error.h>

#include <xio/detail/push_options.h>

namespace xio {


    namespace ip {
        template<typename Elem, typename Traits>
        std::basic_ostream<Elem, Traits> &operator<<(
            std::basic_ostream<Elem, Traits> &os, const network_v6 &addr) {
            xio::error_code ec;
            std::string s = addr.to_string(ec);
            if (ec) {
                if (os.exceptions() & std::basic_ostream<Elem, Traits>::failbit)
                    xio::detail::throw_error(ec);
                else
                    os.setstate(std::basic_ostream<Elem, Traits>::failbit);
            } else
                for (std::string::iterator i = s.begin(); i != s.end(); ++i)
                    os << os.widen(*i);
            return os;
        }
    } // namespace ip

} // namespace xio

#include <xio/detail/pop_options.h>

#endif // !defined(XIO_NO_IOSTREAM)

#endif // XIO_IP_IMPL_NETWORK_V6_HPP
