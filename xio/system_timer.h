//
// system_timer.hpp
// ~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef XIO_SYSTEM_TIMER_HPP
#define XIO_SYSTEM_TIMER_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>
#include <xio/basic_waitable_timer.h>
#include <chrono>

namespace xio {


    /// Typedef for a timer based on the system clock.
    /**
 * This typedef uses the C++11 @c &lt;chrono&gt; standard library facility, if
 * available. Otherwise, it may use the Boost.Chrono library. To explicitly
 * utilise Boost.Chrono, use the basic_waitable_timer template directly:
 * @code
 * typedef basic_waitable_timer<boost::std::chrono::system_clock> timer;
 * @endcode
 */
    typedef basic_waitable_timer<std::chrono::system_clock> system_timer;


} // namespace xio

#endif // XIO_SYSTEM_TIMER_HPP
