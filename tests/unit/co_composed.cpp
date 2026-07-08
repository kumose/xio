//
// co_composed.cpp
// ~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//



// Prevent link dependency on the Boost.System library.
#if !defined(BOOST_SYSTEM_NO_DEPRECATED)
#define BOOST_SYSTEM_NO_DEPRECATED
#endif // !defined(BOOST_SYSTEM_NO_DEPRECATED)

// Test that header file is self-contained.
#include <xio/co_composed.h>

#include "unit_test.hpp"

#if defined(XIO_HAS_CO_AWAIT)

#include <xio/bind_cancellation_slot.h>
#include <xio/deferred.h>
#include <xio/detached.h>
#include <xio/io_context.h>
#include <xio/post.h>

template <typename CompletionToken>
auto async_throw(CompletionToken&& token)
{
  return xio::async_initiate<CompletionToken, void()>(
      [](auto) { throw 42; }, token);
}

template <typename CompletionToken>
auto throw_first(CompletionToken&& token)
{
  return xio::async_initiate<CompletionToken, void()>(
      xio::co_composed(
        [](auto state) -> void
        {
          throw 42;
          co_yield state.complete();
        }), token);
}

void test_throw_first()
{
  try
  {
    throw_first(xio::detached);
    XIO_CHECK(0);
  }
  catch (int)
  {
  }
}

template <typename CompletionToken>
auto throw_after_await(xio::io_context& ctx, CompletionToken&& token)
{
  return xio::async_initiate<CompletionToken, void()>(
      xio::co_composed(
        [](auto state, xio::io_context& ctx) -> void
        {
          co_await xio::post(ctx, xio::deferred);
          throw 42;
          co_yield state.complete();
        }), token, std::ref(ctx));
}

void test_throw_after_await()
{
  try
  {
    xio::io_context ctx(1);
    throw_after_await(ctx, xio::detached);
    ctx.run();
    XIO_CHECK(0);
  }
  catch (int)
  {
  }
}

template <typename CompletionToken>
auto throw_in_first_suspend(CompletionToken&& token)
{
  return xio::async_initiate<CompletionToken, void()>(
      xio::co_composed(
        [](auto state) -> void
        {
          co_await async_throw(xio::deferred);
          co_yield state.complete();
        }), token);
}

void test_throw_in_first_suspend()
{
  try
  {
    throw_in_first_suspend(xio::detached);
    XIO_CHECK(0);
  }
  catch (int)
  {
  }
}

template <typename CompletionToken>
auto throw_in_suspend_after_await(
    xio::io_context& ctx, CompletionToken&& token)
{
  return xio::async_initiate<CompletionToken, void()>(
      xio::co_composed(
        [](auto state, xio::io_context& ctx) -> void
        {
          co_await xio::post(ctx, xio::deferred);
          co_await async_throw(xio::deferred);
          co_yield state.complete();
        }), token, std::ref(ctx));
}

void test_throw_in_suspend_after_await()
{
  try
  {
    xio::io_context ctx(1);
    throw_in_suspend_after_await(ctx, xio::detached);
    ctx.run();
    XIO_CHECK(0);
  }
  catch (int)
  {
  }
}

template <typename CompletionToken>
auto post_loop(xio::io_context& ctx, CompletionToken&& token)
{
  return xio::async_initiate<CompletionToken, void(int)>(
      xio::co_composed(
        [](auto state, xio::io_context& ctx) -> void
        {
          int i = 0;
          for (; i < 100; ++i)
            co_await xio::post(ctx, xio::deferred);
          co_yield state.complete(i);
        }, ctx), token, std::ref(ctx));
}

void test_post_loop()
{
  xio::io_context ctx(1);
  int count = 0;
  post_loop(ctx, [&](int i){ count = i; });
  ctx.run();
  XIO_CHECK(count == 100);
}

template <typename CompletionToken>
auto nested_post(xio::io_context& ctx, CompletionToken&& token)
{
  return xio::async_initiate<CompletionToken, void()>(
      xio::co_composed(
        [](auto state, xio::io_context& ctx) -> void
        {
          co_await xio::post(ctx, xio::deferred);
          co_yield state.complete();
        }, ctx), token, std::ref(ctx));
}

template <typename CompletionToken>
auto nested_post_loop(xio::io_context& ctx, CompletionToken&& token)
{
  return xio::async_initiate<CompletionToken, void(int)>(
      xio::co_composed(
        [](auto state, xio::io_context& ctx) -> void
        {
          int i = 0;
          for (; i < 100; ++i)
            co_await nested_post(ctx, xio::deferred);
          co_yield state.complete(i);
        }, ctx), token, std::ref(ctx));
}

void test_nested_post_loop()
{
  xio::io_context ctx(1);
  int count = 0;
  nested_post_loop(ctx, [&](int i){ count = i; });
  ctx.run();
  XIO_CHECK(count == 100);
}

template <typename CompletionToken>
auto post_loop_return_1_0(xio::io_context& ctx, CompletionToken&& token)
{
  return xio::async_initiate<CompletionToken, void()>(
      xio::co_composed<void()>(
        [](auto, xio::io_context& ctx) -> void
        {
          int i = 0;
          for (; i < 100; ++i)
            co_await xio::post(ctx, xio::deferred);
          co_return {};
        }, ctx), token, std::ref(ctx));
}

void test_post_loop_return_1_0()
{
  xio::io_context ctx(1);
  bool done = false;
  post_loop_return_1_0(ctx, [&]{ done = true; });
  ctx.run();
  XIO_CHECK(done);
}

template <typename CompletionToken>
auto post_loop_return_1_1(xio::io_context& ctx, CompletionToken&& token)
{
  return xio::async_initiate<CompletionToken, void(int)>(
      xio::co_composed<void(int)>(
        [](auto, xio::io_context& ctx) -> void
        {
          int i = 0;
          for (; i < 100; ++i)
            co_await xio::post(ctx, xio::deferred);
          co_return {i};
        }, ctx), token, std::ref(ctx));
}

void test_post_loop_return_1_1()
{
  xio::io_context ctx(1);
  int count = 0;
  post_loop_return_1_1(ctx, [&](int i){ count = i; });
  ctx.run();
  XIO_CHECK(count == 100);
}

template <typename CompletionToken>
auto post_loop_return_1_2(xio::io_context& ctx, CompletionToken&& token)
{
  return xio::async_initiate<CompletionToken, void(int, char)>(
      xio::co_composed<void(int, char)>(
        [](auto, xio::io_context& ctx) -> void
        {
          int i = 0;
          for (; i < 100; ++i)
            co_await xio::post(ctx, xio::deferred);
          co_return {i, 'A'};
        }, ctx), token, std::ref(ctx));
}

void test_post_loop_return_1_2()
{
  xio::io_context ctx(1);
  int count = 0;
  char ch = 0;
  post_loop_return_1_2(ctx, [&](int i, char c){ count = i, ch = c; });
  ctx.run();
  XIO_CHECK(count == 100);
  XIO_CHECK(ch == 'A');
}

template <typename CompletionToken>
auto post_loop_return_2(xio::io_context& ctx, CompletionToken&& token)
{
  return xio::async_initiate<CompletionToken, void(), void(int)>(
      xio::co_composed<void(), void(int)>(
        [](auto, xio::io_context& ctx) -> void
        {
          int i = 0;
          for (; i < 100; ++i)
            co_await xio::post(ctx, xio::deferred);
          co_return {i};
        }, ctx), token, std::ref(ctx));
}

void test_post_loop_return_2()
{
  xio::io_context ctx(1);
  int count = 0;
  post_loop_return_2(ctx, [&](int i = 0){ count = i; });
  ctx.run();
  XIO_CHECK(count == 100);
}

template <typename CompletionToken>
auto complete_on_cancel(xio::io_context& ctx, CompletionToken&& token)
{
  return xio::async_initiate<
    CompletionToken, void(xio::error_code, int)>(
      xio::co_composed<
        void(xio::error_code, int)>(
          [](auto state, xio::io_context& ctx) -> void
          {
            state.on_cancellation_complete_with(
                xio::error::invalid_argument, 42);
            int i = 0;
            for (; i < 100; ++i)
              co_await xio::post(ctx, xio::deferred);
            co_return {xio::error::eof, i};
          }, ctx), token, std::ref(ctx));
}

void test_complete_on_cancel()
{
  xio::io_context ctx(1);
  int count = 0;
  xio::error_code ec;
  xio::cancellation_signal cancel;

  complete_on_cancel(ctx,
      [&](xio::error_code e, int i)
      {
        ec = e;
        count = i;
      });

  ctx.run();

  XIO_CHECK(ec == xio::error::eof);
  XIO_CHECK(count == 100);

  complete_on_cancel(ctx,
      xio::bind_cancellation_slot(cancel.slot(),
        [&](xio::error_code e, int i)
        {
          ec = e;
          count = i;
        }));

  ctx.restart();
  ctx.run_one();
  cancel.emit(xio::cancellation_type::all);
  ctx.run();

  XIO_CHECK(ec == xio::error::invalid_argument);
  XIO_CHECK(count == 42);

  complete_on_cancel(ctx,
      xio::bind_cancellation_slot(cancel.slot(),
        [&](xio::error_code e, int i)
        {
          ec = e;
          count = i;
        }));

  ctx.restart();
  ctx.run();

  XIO_CHECK(ec == xio::error::eof);
  XIO_CHECK(count == 100);
}

template <typename CompletionToken>
auto complete_with_default_on_cancel(
    xio::io_context& ctx, CompletionToken&& token)
{
  return xio::async_initiate<
    CompletionToken, void(xio::error_code, int)>(
      xio::co_composed<
        void(xio::error_code, int)>(
          [](auto, xio::io_context& ctx) -> void
          {
            int i = 0;
            for (; i < 100; ++i)
              co_await xio::post(ctx, xio::deferred);
            co_return {xio::error::eof, i};
          }, ctx), token, std::ref(ctx));
}

void test_complete_with_default_on_cancel()
{
  xio::io_context ctx(1);
  int count = 0;
  xio::error_code ec;
  xio::cancellation_signal cancel;

  complete_with_default_on_cancel(ctx,
      [&](xio::error_code e, int i)
      {
        ec = e;
        count = i;
      });

  ctx.run();

  XIO_CHECK(ec == xio::error::eof);
  XIO_CHECK(count == 100);

  complete_with_default_on_cancel(ctx,
      xio::bind_cancellation_slot(cancel.slot(),
        [&](xio::error_code e, int i)
        {
          ec = e;
          count = i;
        }));

  ctx.restart();
  ctx.run_one();
  cancel.emit(xio::cancellation_type::all);
  ctx.run();

  XIO_CHECK(ec == xio::error::operation_aborted);
  XIO_CHECK(count == 0);

  complete_with_default_on_cancel(ctx,
      xio::bind_cancellation_slot(cancel.slot(),
        [&](xio::error_code e, int i)
        {
          ec = e;
          count = i;
        }));

  ctx.restart();
  ctx.run();

  XIO_CHECK(ec == xio::error::eof);
  XIO_CHECK(count == 100);
}

template <typename CompletionToken>
auto throw_on_cancel(xio::io_context& ctx, CompletionToken&& token)
{
  return xio::async_initiate<
    CompletionToken, void(xio::error_code, int)>(
      xio::co_composed<
        void(xio::error_code, int)>(
          [](auto state, xio::io_context& ctx) -> void
          {
            try
            {
              state.throw_if_cancelled(true);
              int i = 0;
              for (; i < 100; ++i)
                co_await xio::post(ctx, xio::deferred);
              co_return {xio::error::eof, i};
            }
            catch (...)
            {
              co_return {xio::error::invalid_argument, 42};
            }
          }, ctx), token, std::ref(ctx));
}

void test_throw_on_cancel()
{
  xio::io_context ctx(1);
  int count = 0;
  xio::error_code ec;
  xio::cancellation_signal cancel;

  throw_on_cancel(ctx,
      [&](xio::error_code e, int i)
      {
        ec = e;
        count = i;
      });

  ctx.run();

  XIO_CHECK(ec == xio::error::eof);
  XIO_CHECK(count == 100);

  throw_on_cancel(ctx,
      xio::bind_cancellation_slot(cancel.slot(),
        [&](xio::error_code e, int i)
        {
          ec = e;
          count = i;
        }));

  ctx.restart();
  ctx.run_one();
  cancel.emit(xio::cancellation_type::all);
  ctx.run();

  XIO_CHECK(ec == xio::error::invalid_argument);
  XIO_CHECK(count == 42);

  throw_on_cancel(ctx,
      xio::bind_cancellation_slot(cancel.slot(),
        [&](xio::error_code e, int i)
        {
          ec = e;
          count = i;
        }));

  ctx.restart();
  ctx.run();

  XIO_CHECK(ec == xio::error::eof);
  XIO_CHECK(count == 100);
}

void test_no_signatures_detached()
{
  xio::io_context ctx(1);
  int count1 = 0;
  int count2 = 0;

  xio::async_initiate(
      xio::co_composed(
        [](auto, xio::io_context& ctx, int& count1, int& count2) -> void
        {
          for (;;)
          {
            ++count1;
            co_await xio::post(ctx, xio::deferred);
            ++count2;
          }
        }), xio::detached, std::ref(ctx), std::ref(count1), std::ref(count2));

  assert(count1 == 1);
  assert(count2 == 0);

  ctx.run_one();

  assert(count1 == 2);
  assert(count2 == 1);

  ctx.restart();
  ctx.run_one();

  assert(count1 == 3);
  assert(count2 == 2);
}

XIO_TEST_SUITE
(
  "co_composed",
  XIO_TEST_CASE(test_throw_first)
  XIO_TEST_CASE(test_throw_after_await)
  XIO_TEST_CASE(test_throw_in_first_suspend)
  XIO_TEST_CASE(test_throw_in_suspend_after_await)
  XIO_TEST_CASE(test_post_loop)
  XIO_TEST_CASE(test_nested_post_loop)
  XIO_TEST_CASE(test_post_loop_return_1_0)
  XIO_TEST_CASE(test_post_loop_return_1_1)
  XIO_TEST_CASE(test_post_loop_return_1_2)
  XIO_TEST_CASE(test_post_loop_return_2)
  XIO_TEST_CASE(test_complete_on_cancel)
  XIO_TEST_CASE(test_complete_with_default_on_cancel)
  XIO_TEST_CASE(test_throw_on_cancel)
  XIO_TEST_CASE(test_no_signatures_detached)
)

#else // defined(XIO_HAS_CO_AWAIT)

XIO_TEST_SUITE
(
  "co_composed",
  XIO_TEST_CASE(null_test)
)

#endif // defined(XIO_HAS_CO_AWAIT)
