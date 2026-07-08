//
// experimental/coro/use_coro.cpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2021-2023 Klemens D. Morgenstern
//                         (klemens dot morgenstern at gmx dot net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//



// Test that header file is self-contained.
#include <xio/experimental/use_coro.h>

#include <xio/steady_timer.h>
#include <iostream>
#include "../../unit_test.hpp"

using namespace xio::experimental;

namespace coro {

xio::experimental::coro<void(), int>
awaiter(xio::any_io_executor exec)
{
  xio::steady_timer timer{exec};
  co_await timer.async_wait(use_coro);
  co_return 42;
}

xio::experimental::coro<void() noexcept, int>
awaiter_noexcept(xio::any_io_executor exec)
{
  xio::steady_timer timer{exec};
  auto ec = co_await timer.async_wait(xio::deferred);
  XIO_CHECK(ec == xio::error_code{});
  co_return 42;
}

void stack_test2()
{
  bool done = false;
  xio::io_context ctx;

  auto k = awaiter(ctx.get_executor());
  auto k2 = awaiter_noexcept(ctx.get_executor());

  k.async_resume(
      [&](std::exception_ptr ex, int res)
      {
        XIO_CHECK(!ex);
        XIO_CHECK(res == 42);
        done = true;
      });

  k2.async_resume([&](int res)
       {
         XIO_CHECK(res == 42);
         done = true;
       });

  ctx.run();
  XIO_CHECK(done);
}

} // namespace coro

XIO_TEST_SUITE
(
  "coro/use_coro",
  XIO_TEST_CASE(::coro::stack_test2)
)
