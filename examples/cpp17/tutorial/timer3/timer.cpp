//
// timer.cpp
// ~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <functional>
#include <iostream>
#include <xio/xio.h>

void print(const std::error_code& /*e*/,
    xio::steady_timer* t, int* count)
{
  if (*count < 5)
  {
    std::cout << *count << std::endl;
    ++(*count);

    t->expires_at(t->expiry() + std::chrono::seconds(1));
    t->async_wait(std::bind(print,
          xio::placeholders::error, t, count));
  }
}

int main()
{
  xio::io_context io;

  int count = 0;
  xio::steady_timer t(io, std::chrono::seconds(1));
  t.async_wait(std::bind(print,
        xio::placeholders::error, &t, &count));

  io.run();

  std::cout << "Final count is " << count << std::endl;

  return 0;
}
