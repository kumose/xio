//
// sleep.hpp
// ~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef SLEEP_HPP
#define SLEEP_HPP

#include <xio/any_completion_handler.h>
#include <xio/any_io_executor.h>
#include <xio/async_result.h>
#include <xio/error.h>
#include <chrono>

void async_sleep_impl(
    xio::any_completion_handler<void(std::error_code)> handler,
    xio::any_io_executor ex, std::chrono::nanoseconds duration);

template <typename CompletionToken>
inline auto async_sleep(xio::any_io_executor ex,
    std::chrono::nanoseconds duration, CompletionToken&& token)
  -> decltype(
      xio::async_initiate<CompletionToken, void(std::error_code)>(
        async_sleep_impl, token, std::move(ex), duration))
{
  return xio::async_initiate<CompletionToken, void(std::error_code)>(
      async_sleep_impl, token, std::move(ex), duration);
}

#endif // SLEEP_HPP
