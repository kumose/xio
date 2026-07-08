//
// streambuf.hpp
// ~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef XIO_STREAMBUF_HPP
#define XIO_STREAMBUF_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>
#include <xio/basic_streambuf.h>

namespace xio {


    /// Typedef for the typical usage of basic_streambuf.
    typedef basic_streambuf<> streambuf;


} // namespace xio


#endif // XIO_STREAMBUF_HPP
