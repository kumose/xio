//
// basic_streambuf_fwd.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef XIO_BASIC_STREAMBUF_FWD_HPP
#define XIO_BASIC_STREAMBUF_FWD_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>

#if !defined(XIO_NO_IOSTREAM)

#include <memory>

namespace xio {

    template<typename Allocator = std::allocator<char> >
    class basic_streambuf;

    template<typename Allocator = std::allocator<char> >
    class basic_streambuf_ref;


} // namespace xio

#endif // !defined(XIO_NO_IOSTREAM)

#endif // XIO_BASIC_STREAMBUF_FWD_HPP
