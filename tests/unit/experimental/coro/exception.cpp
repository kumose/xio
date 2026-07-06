//
// experimental/coro/exception.cpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2021-2023 Klemens D. Morgenstern
//                         (klemens dot morgenstern at gmx dot net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

// Disable autolinking for unit tests.
#if !defined(BOOST_ALL_NO_LIB)
#define BOOST_ALL_NO_LIB 1
#endif // !defined(BOOST_ALL_NO_LIB)

// Test that header file is self-contained.
#include <xio/experimental/coro.h>

#include <xio/co_spawn.h>
#include <xio/detached.h>
#include <xio/io_context.h>
#include <xio/use_awaitable.h>
#include <xio/awaitable.h>
#include "../../unit_test.hpp"

using namespace xio::experimental;

namespace coro {

template <typename Func>
struct on_scope_exit
{
	Func func;

  static_assert(noexcept(func()));

  on_scope_exit(const Func &f)
    : func(static_cast< Func && >(f))
	{
	}

	on_scope_exit(Func &&f)
    : func(f)
	{
	}

  on_scope_exit(const on_scope_exit &) = delete;

	~on_scope_exit()
  {
    func();
  }
};

xio::experimental::coro<int> throwing_generator(
  xio::any_io_executor, int &last, bool &destroyed)
{
  on_scope_exit x = [&]() noexcept { destroyed = true; };
  (void)x;

  int i = 0;
  while (i < 3)
  {
    last = ++i;
    co_yield last;
  }

  throw std::runtime_error("throwing-generator");
}

xio::awaitable<void> throwing_generator_test()
{
  int val = 0;
  bool destr = false;
  {
    auto gi = throwing_generator(
        co_await xio::this_coro::executor,
        val, destr);
    bool caught = false;
    try
    {
      for (int i = 0; i < 10; i++)
      {
        ASIO_CHECK(val == i);
        const auto next = co_await gi.async_resume(xio::use_awaitable);
        ASIO_CHECK(next);
        ASIO_CHECK(val == *next);
        ASIO_CHECK(val == i + 1);
      }
    }
    catch (std::runtime_error &err)
    {
      caught = true;
      using std::operator ""sv;
      ASIO_CHECK(err.what() == "throwing-generator"sv);
    }
    ASIO_CHECK(val == 3);
    ASIO_CHECK(caught);
  }
  ASIO_CHECK(destr);
};

void run_throwing_generator_test()
{
  xio::io_context ctx;
  xio::co_spawn(ctx, throwing_generator_test(), xio::detached);
  ctx.run();
}

xio::experimental::coro<int(int)> throwing_stacked(
    xio::any_io_executor exec, int &val,
    bool &destroyed_inner, bool &destroyed)
{
  ASIO_CHECK((co_await xio::this_coro::throw_if_cancelled()));

  on_scope_exit x = [&]() noexcept { destroyed = true; };
  (void)x;

  auto gen = throwing_generator(exec, val, destroyed_inner);
  while (auto next = co_await gen) // 1, 2, 4, 8, ...
    ASIO_CHECK(42 ==(co_yield *next)); // offset is delayed by one cycle
}

xio::awaitable<void> throwing_generator_stacked_test()
{
  int val = 0;
  bool destr = false, destr_inner = false;
  {
    auto gi = throwing_stacked(
        co_await xio::this_coro::executor,
        val, destr, destr_inner);
    bool caught = false;
    try
    {
      for (int i = 0; i < 10; i++)
      {
        ASIO_CHECK(val == i);
        const auto next =
          co_await gi.async_resume(42, xio::use_awaitable);
        ASIO_CHECK(next);
        ASIO_CHECK(val == *next);
        ASIO_CHECK(val == i + 1);
      }
    }
    catch (std::runtime_error &err)
    {
      caught = true;
      using std::operator ""sv;
      ASIO_CHECK(err.what() == "throwing-generator"sv);
    }
    ASIO_CHECK(val == 3);
    ASIO_CHECK(caught);
  }
  ASIO_CHECK(destr);
  ASIO_CHECK(destr_inner);
};

void run_throwing_generator_stacked_test()
{
  xio::io_context ctx;
  xio::co_spawn(ctx,
      throwing_generator_stacked_test,
      xio::detached);
  ctx.run();
}

} // namespace coro

ASIO_TEST_SUITE
(
  "coro/exception",
  ASIO_TEST_CASE(::coro::run_throwing_generator_stacked_test)
  ASIO_TEST_CASE(::coro::run_throwing_generator_test)
)
