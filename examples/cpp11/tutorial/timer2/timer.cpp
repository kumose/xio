//
// timer.cpp
// ~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <iostream>
#include <xio/xio.h>

void print(const std::error_code& /*e*/)
{
  std::cout << "Hello, world!" << std::endl;
}

int main()
{
  xio::io_context io;

  xio::steady_timer t(io, std::chrono::seconds(5));
  t.async_wait(&print);

  io.run();

  return 0;
}
