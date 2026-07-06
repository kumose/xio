//
// detail/handler_cont_helpers.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_DETAIL_HANDLER_CONT_HELPERS_HPP
#define ASIO_DETAIL_HANDLER_CONT_HELPERS_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>
#include <xio/detail/memory.h>
#include <xio/handler_continuation_hook.h>

#include <xio/detail/push_options.h>

// Calls to asio_handler_is_continuation must be made from a namespace that
// does not contain overloads of this function. This namespace is defined here
// for that purpose.
namespace ASIO_VERSIONED_NAME(handler_cont_helpers){

template <typename Context >
inline bool is_continuation(Context & context)
    {
#if !defined(ASIO_HAS_HANDLER_HOOKS)
  return false;
#else
        using xio::asio_handler_is_continuation;
  return asio_handler_is_continuation(
            xio::detail::addressof(context));
#endif

    }

} // namespace ASIO_VERSIONED_NAME(handler_cont_helpers)

#include <xio/detail/pop_options.h>

#endif // ASIO_DETAIL_HANDLER_CONT_HELPERS_HPP
