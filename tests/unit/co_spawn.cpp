//
// co_spawn.cpp
// ~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//



// Test that header file is self-contained.
#include <xio/co_spawn.h>

#include "unit_test.hpp"

#if defined(ASIO_HAS_CO_AWAIT)

#include <stdexcept>
#include <xio/any_completion_handler.h>
#include <xio/bind_cancellation_slot.h>
#include <xio/dispatch.h>
#include <xio/io_context.h>
#include <xio/use_awaitable.h>

xio::awaitable<void> void_returning_coroutine()
{
  co_return;
}

xio::awaitable<int> int_returning_coroutine()
{
  co_return 42;
}

void test_co_spawn_with_any_completion_handler()
{
  xio::io_context ctx;

  bool called = false;
  xio::co_spawn(ctx, void_returning_coroutine(),
      xio::any_completion_handler<void(std::exception_ptr)>(
        [&](std::exception_ptr)
        {
          called = true;
        }));

  ASIO_CHECK(!called);

  ctx.run();

  ASIO_CHECK(called);

  int result = 0;
  xio::co_spawn(ctx, int_returning_coroutine(),
      xio::any_completion_handler<void(std::exception_ptr, int)>(
        [&](std::exception_ptr, int i)
        {
          result = i;
        }));

  ASIO_CHECK(result == 0);

  ctx.restart();
  ctx.run();

  ASIO_CHECK(result == 42);
}

void test_co_spawn_immediate_cancel()
{
  xio::cancellation_signal sig;
  xio::io_context ctx;

  std::exception_ptr result = nullptr;
  bool called = false;
  xio::co_spawn(ctx, void_returning_coroutine(),
      xio::bind_cancellation_slot(sig.slot(),
        [&](std::exception_ptr e)
        {
          result = e;
          called = true;
        }));

  ASIO_CHECK(!called);
  ASIO_CHECK(result == nullptr);

  sig.emit(xio::cancellation_type::all);
  ctx.run();

  ASIO_CHECK(called);
  ASIO_CHECK(result != nullptr);

  result = nullptr;
  called = false;
  xio::co_spawn(ctx, int_returning_coroutine(),
      xio::bind_cancellation_slot(sig.slot(),
        [&](std::exception_ptr e, int i)
        {
          ASIO_CHECK(i != 42);
          result = e;
          called = true;
        }));

  ASIO_CHECK(!called);
  ASIO_CHECK(result == nullptr);

  sig.emit(xio::cancellation_type::all);
  ctx.restart();
  ctx.run();

  ASIO_CHECK(called);
  ASIO_CHECK(result != nullptr);
}

xio::awaitable<void> void_returning_dispatch_coroutine()
{
  co_await xio::dispatch(xio::use_awaitable);
  co_return;
}

xio::awaitable<int> int_returning_dispatch_coroutine()
{
  co_await xio::dispatch(xio::use_awaitable);
  co_return 42;
}

void test_co_spawn_with_immediate_completion_via_dispatch()
{
  xio::io_context ctx;

  bool called = false;
  xio::post(ctx,
      [&]
      {
        xio::co_spawn(ctx, void_returning_dispatch_coroutine(),
            [&](std::exception_ptr)
            {
              called = true;
            });

        ASIO_CHECK(!called);
      });

  ctx.run();

  ASIO_CHECK(called);

  int result = 0;
  xio::post(ctx,
      [&]
      {
        xio::co_spawn(ctx, int_returning_dispatch_coroutine(),
            [&](std::exception_ptr, int i)
            {
              result = i;
            });

        ASIO_CHECK(result == 0);
      });

  ctx.restart();
  ctx.run();

  ASIO_CHECK(result == 42);
}

ASIO_TEST_SUITE
(
  "co_spawn",
  ASIO_TEST_CASE(test_co_spawn_with_any_completion_handler)
  ASIO_TEST_CASE(test_co_spawn_immediate_cancel)
  ASIO_TEST_CASE(test_co_spawn_with_immediate_completion_via_dispatch)
)

#else // defined(ASIO_HAS_CO_AWAIT)

ASIO_TEST_SUITE
(
  "co_spawn",
  ASIO_TEST_CASE(null_test)
)

#endif // defined(ASIO_HAS_CO_AWAIT)
