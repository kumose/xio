//
// main.cpp
// ~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "line_reader.hpp"
#include "sleep.hpp"
#include "stdin_line_reader.hpp"
#include <xio/co_spawn.h>
#include <xio/detached.h>
#include <xio/io_context.h>
#include <xio/use_awaitable.h>
#include <iostream>

xio::awaitable<void> do_read(line_reader& reader)
{
  for (int i = 0; i < 10; ++i)
  {
    std::cout << co_await reader.async_read_line("Enter something: ", xio::use_awaitable);
    co_await async_sleep(co_await xio::this_coro::executor, std::chrono::seconds(1), xio::use_awaitable);
  }
}

int main()
{
  xio::io_context ctx{1};
  stdin_line_reader reader{ctx.get_executor()};
  co_spawn(ctx, do_read(reader), xio::detached);
  ctx.run();
}
