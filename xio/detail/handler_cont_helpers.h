//
// detail/handler_cont_helpers.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef XIO_DETAIL_HANDLER_CONT_HELPERS_HPP
#define XIO_DETAIL_HANDLER_CONT_HELPERS_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>
#include <xio/detail/memory.h>
#include <xio/handler_continuation_hook.h>

#include <xio/detail/push_options.h>

// Calls to xio_handler_is_continuation must be made from a namespace that
// does not contain overloads of this function. This namespace is defined here
// for that purpose.
namespace XIO_VERSIONED_NAME(handler_cont_helpers){

template <typename Context >
inline bool is_continuation(Context & context)
    {
#if !defined(XIO_HAS_HANDLER_HOOKS)
  return false;
#else
        using xio::xio_handler_is_continuation;
  return xio_handler_is_continuation(
            xio::detail::addressof(context));
#endif


    }

} // namespace XIO_VERSIONED_NAME(handler_cont_helpers)

#include <xio/detail/pop_options.h>

#endif // XIO_DETAIL_HANDLER_CONT_HELPERS_HPP
