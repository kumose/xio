//
// posix/stream_descriptor.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef XIO_POSIX_STREAM_DESCRIPTOR_HPP
#define XIO_POSIX_STREAM_DESCRIPTOR_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>

#if defined(XIO_HAS_POSIX_STREAM_DESCRIPTOR)

#include <xio/posix/basic_stream_descriptor.h>

namespace xio {


    namespace posix {
        /// Typedef for the typical usage of a stream-oriented descriptor.
        typedef basic_stream_descriptor<> stream_descriptor;
    } // namespace posix

} // namespace xio

#endif // defined(XIO_HAS_POSIX_STREAM_DESCRIPTOR)

#endif // XIO_POSIX_STREAM_DESCRIPTOR_HPP
