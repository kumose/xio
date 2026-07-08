//
// readable_pipe.hpp
// ~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef XIO_READABLE_PIPE_HPP
#define XIO_READABLE_PIPE_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>

#if defined(XIO_HAS_PIPE)
#include <xio/basic_readable_pipe.h>

namespace xio {


    /// Typedef for the typical usage of a readable pipe.
    typedef basic_readable_pipe<> readable_pipe;


} // namespace xio

#endif // defined(XIO_HAS_PIPE)

#endif // XIO_READABLE_PIPE_HPP
