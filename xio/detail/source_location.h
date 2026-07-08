//
// detail/source_location.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef XIO_DETAIL_SOURCE_LOCATION_HPP
#define XIO_DETAIL_SOURCE_LOCATION_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>

#if defined(XIO_HAS_SOURCE_LOCATION)

#if defined(XIO_HAS_STD_SOURCE_LOCATION)
# include <source_location>
#elif defined(XIO_HAS_STD_EXPERIMENTAL_SOURCE_LOCATION)
# include <experimental/source_location>
#else // defined(XIO_HAS_STD_EXPERIMENTAL_SOURCE_LOCATION)
# error XIO_HAS_SOURCE_LOCATION is set \
  but no source_location is available
#endif // defined(XIO_HAS_STD_EXPERIMENTAL_SOURCE_LOCATION)

namespace xio {


    namespace detail {



#if defined(XIO_HAS_STD_SOURCE_LOCATION)
using std::source_location;
#elif defined(XIO_HAS_STD_EXPERIMENTAL_SOURCE_LOCATION)
using std::experimental::source_location;
#endif // defined(XIO_HAS_STD_EXPERIMENTAL_SOURCE_LOCATION)

} // namespace detail
} // namespace xio

#endif // defined(XIO_HAS_SOURCE_LOCATION)

#endif // XIO_DETAIL_SOURCE_LOCATION_HPP
