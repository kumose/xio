//
// sleep.cpp
// ~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "sleep.hpp"
#include <xio/consign.h>
#include <xio/steady_timer.h>
#include <memory>

void async_sleep_impl(
    xio::any_completion_handler<void(std::error_code)> handler,
    xio::any_io_executor ex, std::chrono::nanoseconds duration)
{
  auto timer = std::make_shared<xio::steady_timer>(ex, duration);
  timer->async_wait(xio::consign(std::move(handler), timer));
}
