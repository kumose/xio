//
// packaged_task.hpp
// ~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_PACKAGED_TASK_HPP
#define ASIO_PACKAGED_TASK_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>
#include <future>

#include <xio/async_result.h>
#include <xio/detail/type_traits.h>

#include <xio/detail/push_options.h>

namespace xio {


    /// Partial specialisation of @c async_result for @c std::packaged_task.


    template<typename Result, typename... Args, typename Signature>
    class async_result<std::packaged_task < Result(Args...)>
    ,
    Signature
    >
{
public:
  /// The packaged task is the concrete completion handler type.
  typedef std::packaged_task<Result(Args...)> completion_handler_type;

  /// The return type of the initiating function is the future obtained from
  /// the packaged task.
  typedef std::future<Result> return_type;

  /// The constructor extracts the future from the packaged task.
  explicit async_result(completion_handler_type& h)
    : future_(h.get_future())
  {
  }

  /// Returns the packaged task's future.
  return_type get()
  {
    return std::move(future_);
  }

private:
  return_type future_;
};


} // namespace xio

#include <xio/detail/pop_options.h>

#endif // ASIO_PACKAGED_TASK_HPP
