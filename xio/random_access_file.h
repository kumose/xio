//
// random_access_file.hpp
// ~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef XIO_RANDOM_ACCESS_FILE_HPP
#define XIO_RANDOM_ACCESS_FILE_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>

#if defined(XIO_HAS_FILE)

#include <xio/basic_random_access_file.h>

namespace xio {


    /// Typedef for the typical usage of a random-access file.
    typedef basic_random_access_file<> random_access_file;


} // namespace xio

#endif // defined(XIO_HAS_FILE)

#endif // XIO_RANDOM_ACCESS_FILE_HPP
