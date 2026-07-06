//
// streambuf.hpp
// ~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_STREAMBUF_HPP
#define ASIO_STREAMBUF_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>

#if !defined(ASIO_NO_IOSTREAM)

#include <xio/basic_streambuf.h>

namespace xio {
    ASIO_INLINE_NAMESPACE_BEGIN

    /// Typedef for the typical usage of basic_streambuf.
    typedef basic_streambuf<> streambuf;

    ASIO_INLINE_NAMESPACE_END
} // namespace xio

#endif // !defined(ASIO_NO_IOSTREAM)

#endif // ASIO_STREAMBUF_HPP
