//
// detail/win_event.ipp
// ~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef XIO_DETAIL_IMPL_WIN_EVENT_IPP
#define XIO_DETAIL_IMPL_WIN_EVENT_IPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>

#if defined(XIO_WINDOWS)

#include <xio/detail/throw_error.h>
#include <xio/detail/win_event.h>
#include <xio/error.h>

#include <xio/detail/push_options.h>

namespace xio {


    namespace detail {
        win_event::win_event()
            : state_(0) {


#if defined(XIO_WINDOWS_APP)
events_ [0] = ::CreateEventExW (0, 0,
CREATE_EVENT_MANUAL_RESET, EVENT_ALL_ACCESS);
#else // defined(XIO_WINDOWS_APP)
events_ [0] = ::CreateEventW (0, true, false, 0);
#endif // defined(XIO_WINDOWS_APP)
if (!events_ [0])
  {
    DWORD last_error = ::GetLastError();
    xio::error_code ec(last_error,
        xio::error::get_system_category());
    xio::detail::throw_error(ec, "event");
  }

#if defined(XIO_WINDOWS_APP)
events_ [1] = ::CreateEventExW (0, 0, 0, EVENT_ALL_ACCESS);
#else // defined(XIO_WINDOWS_APP)
events_ [1] = ::CreateEventW (0, false, false, 0);
#endif // defined(XIO_WINDOWS_APP)
if (!events_ [1])
  {
    DWORD last_error = ::GetLastError();
    ::CloseHandle(events_[0]);
    xio::error_code ec(last_error,
        xio::error::get_system_category());
    xio::detail::throw_error(ec, "event");
  }
}

win_event::~win_event() {
    ::CloseHandle(events_[0]);
    ::CloseHandle(events_[1]);
}

} // namespace detail
} // namespace xio

#include <xio/detail/pop_options.h>

#endif // defined(XIO_WINDOWS)

#endif // XIO_DETAIL_IMPL_WIN_EVENT_IPP
