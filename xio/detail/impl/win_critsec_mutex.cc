//
// detail/impl/win_critsec_mutex.ipp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef XIO_DETAIL_IMPL_WIN_CRITSEC_MUTEX_IPP
#define XIO_DETAIL_IMPL_WIN_CRITSEC_MUTEX_IPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>

#if defined(XIO_WINDOWS)

#include <xio/detail/throw_error.h>
#include <xio/detail/win_critsec_mutex.h>
#include <xio/error.h>

#include <xio/detail/push_options.h>

namespace xio {


    namespace detail {
        win_critsec_mutex::win_critsec_mutex() {
            int error = do_init();
            xio::error_code ec(error,
                               xio::error::get_system_category());
            xio::detail::throw_error(ec, "mutex");
        }

        int win_critsec_mutex::do_init() {


#if defined(__MINGW32__)
// Not sure if MinGW supports structured exception handling, so for now
// we'll just call the Windows API and hope.
# if defined(UNDER_CE)
::InitializeCriticalSection (&crit_section_);
# elif defined(XIO_WINDOWS_APP)
if (!::InitializeCriticalSectionEx (&crit_section_, 0, 0))
    return ::GetLastError();
# else
if (!::InitializeCriticalSectionAndSpinCount (&crit_section_, 0x80000000))
    return ::GetLastError();
# endif
return 0;
#else
__try
  {
# if defined(UNDER_CE)
::InitializeCriticalSection (&crit_section_);
# elif defined(XIO_WINDOWS_APP)
if (!::InitializeCriticalSectionEx (&crit_section_, 0, 0))
      return ::GetLastError();
# else
if (!::InitializeCriticalSectionAndSpinCount (&crit_section_, 0x80000000))
      return ::GetLastError();
# endif
}
  __except(GetExceptionCode()== STATUS_NO_MEMORY? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
  {
    return ERROR_OUTOFMEMORY;
  }

  return 0;
#endif
}

} // namespace detail
} // namespace xio

#include <xio/detail/pop_options.h>

#endif // defined(XIO_WINDOWS)

#endif // XIO_DETAIL_IMPL_WIN_CRITSEC_MUTEX_IPP
