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
#include <xio/coroutine.h>
#include <xio/detached.h>
#include <xio/io_context.h>
#include <xio/use_awaitable.h>
#include <iostream>

#include <xio/yield.h>

class read_loop : xio::coroutine
{
public:
  read_loop(xio::any_io_executor e, line_reader& r)
    : executor(std::move(e)),
      reader(r)
  {
  }

  void operator()(std::error_code ec = {}, std::string line = {})
  {
    reenter (this)
    {
      for (i = 0; i < 10; ++i)
      {
        yield reader.async_read_line("Enter something: ", *this);
        if (ec) break;
        std::cout << line;
        yield async_sleep(executor, std::chrono::seconds(1), *this);
        if (ec) break;
      }
    }
  }

private:
  xio::any_io_executor executor;
  line_reader& reader;
  int i;
};

#include <xio/unyield.h>

int main()
{
  xio::io_context ctx{1};
  stdin_line_reader reader{ctx.get_executor()};
  read_loop(ctx.get_executor(), reader)();
  ctx.run();
}
