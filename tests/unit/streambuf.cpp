//
// streambuf.cpp
// ~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//



// Test that header file is self-contained.
#include <xio/streambuf.h>

#include <xio/buffer.h>
#include "unit_test.hpp"

void streambuf_test()
{
  xio::streambuf sb;

  sb.sputn("abcd", 4);

  ASIO_CHECK(sb.size() == 4);

  for (int i = 0; i < 100; ++i)
  {
    sb.consume(3);

    ASIO_CHECK(sb.size() == 1);

    char buf[1];
    sb.sgetn(buf, 1);

    ASIO_CHECK(sb.size() == 0);

    sb.sputn("ab", 2);

    ASIO_CHECK(sb.size() == 2);

    xio::buffer_copy(sb.prepare(10), xio::buffer("cd", 2));
    sb.commit(2);

    ASIO_CHECK(sb.size() == 4);
  }

  ASIO_CHECK(sb.size() == 4);

  sb.consume(4);

  ASIO_CHECK(sb.size() == 0);
}

ASIO_TEST_SUITE
(
  "streambuf",
  ASIO_TEST_CASE(streambuf_test)
)
