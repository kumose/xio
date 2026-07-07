//
// prepend.cpp
// ~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//



// Test that header file is self-contained.
#include <xio/prepend.h>

#include <xio/bind_executor.h>
#include <xio/io_context.h>
#include <xio/post.h>
#include <xio/system_timer.h>
#include "unit_test.hpp"

void prepend_test()
{
  xio::io_context io1;
  xio::io_context io2;
  xio::system_timer timer1(io1);
  int count = 0;

  timer1.expires_after(std::chrono::seconds(0));
  timer1.async_wait(
      xio::prepend(
        xio::bind_executor(io2.get_executor(),
          [&count](int a, int b, xio::error_code)
          {
            ++count;
            ASIO_CHECK(a == 123);
            ASIO_CHECK(b == 321);
          }), 123, 321));

  ASIO_CHECK(count == 0);

  io1.run();

  ASIO_CHECK(count == 0);

  io2.run();

  ASIO_CHECK(count == 1);
}

ASIO_TEST_SUITE
(
  "prepend",
  ASIO_TEST_CASE(prepend_test)
)
