//
// experimental/coro/stack_test.cpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2021-2023 Klemens D. Morgenstern
//                         (klemens dot morgenstern at gmx dot net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//



// Test that header file is self-contained.
#include <xio/experimental/coro.h>

#include <xio/detached.h>
#include <xio/io_context.h>
#include <iostream>
#include "../../unit_test.hpp"

using namespace xio::experimental;

namespace coro {

xio::experimental::coro<int()>
  stack_generator(xio::any_io_executor, int i = 1)
{
  for (;;)
  {
    co_yield i;
    i *= 2;
  }
}

xio::experimental::coro<int(int)>
stack_accumulate(xio::any_io_executor exec)
{
  auto gen  = stack_generator(exec);
  int offset = 0;
  while (auto next = co_await gen) // 1, 2, 4, 8, ...
    offset  = co_yield *next + offset; // offset is delayed by one cycle
}

xio::experimental::coro<int>
main_stack_coro(xio::io_context&, bool & done)
{
  auto g = stack_accumulate(co_await xio::this_coro::executor);

  XIO_CHECK(g.is_open());
  XIO_CHECK(1    == (co_await g(1000)).value_or(-1));
  XIO_CHECK(2002 == (co_await g(2000)).value_or(-1));
  XIO_CHECK(3004 == (co_await g(3000)).value_or(-1));
  XIO_CHECK(4008 == (co_await g(4000)).value_or(-1));
  XIO_CHECK(5016 == (co_await g(5000)).value_or(-1));
  XIO_CHECK(6032 == (co_await g(6000)).value_or(-1));
  XIO_CHECK(7064 == (co_await g(7000)).value_or(-1));
  XIO_CHECK(8128 == (co_await g(8000)).value_or(-1));
  XIO_CHECK(9256 == (co_await g(9000)).value_or(-1));
  XIO_CHECK(511 == (co_await g(-1)).value_or(-1));
  done = true;
}

void stack_test()
{
  bool done = false;
  xio::io_context ctx;
  auto k = main_stack_coro(ctx, done);
  k.async_resume(xio::detached);
  ctx.run();
  XIO_CHECK(done);
}

} // namespace coro

XIO_TEST_SUITE
(
  "coro/stack_test",
  XIO_TEST_CASE(::coro::stack_test)
)
