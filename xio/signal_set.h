//
// signal_set.hpp
// ~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef XIO_SIGNAL_SET_HPP
#define XIO_SIGNAL_SET_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>
#include <xio/basic_signal_set.h>

namespace xio {


    /// Typedef for the typical usage of a signal set.
    typedef basic_signal_set<> signal_set;


} // namespace xio

#endif // XIO_SIGNAL_SET_HPP
