//
// experimental/coro/cancel.cpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2021-2023 Klemens D. Morgenstern
//                         (klemens dot morgenstern at gmx dot net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//



// Test that header file is self-contained.
#include <xio/experimental/coro.h>
#include <iostream>
#include <xio/bind_cancellation_slot.h>
#include <xio/io_context.h>
#include <xio/steady_timer.h>
#include <xio/this_coro.h>
#include "../../unit_test.hpp"

using namespace xio::experimental;
namespace this_coro = xio::this_coro;

namespace coro {

auto coro_simple_cancel_impl(xio::io_context& ) noexcept
  -> xio::experimental::coro<void() noexcept, xio::error_code>
{
    ASIO_CHECK(
        !(co_await this_coro::cancellation_state).cancelled());

    xio::steady_timer timer{
        co_await this_coro::executor,
        std::chrono::seconds(1)};

    ASIO_CHECK(
        !(co_await this_coro::cancellation_state).cancelled());

    auto ec = co_await timer;

    ASIO_CHECK(
        (co_await this_coro::cancellation_state).cancelled());

    co_return ec;
}

void coro_simple_cancel()
{
  xio::io_context ctx;
  xio::cancellation_signal sig;

  auto k = coro_simple_cancel_impl(ctx);

  xio::error_code res_ec;
  k.async_resume(
      xio::bind_cancellation_slot(sig.slot(),
        [&](xio::error_code ec) {res_ec = ec;}));
  xio::post(ctx, [&]{sig.emit(xio::cancellation_type::all);});

  ASIO_CHECK(!res_ec);

  ctx.run();

  ASIO_CHECK(res_ec == xio::error::operation_aborted);
}

auto coro_throw_cancel_impl(xio::io_context& )
  -> xio::experimental::coro<void() , void>
{
    xio::steady_timer timer{
        co_await this_coro::executor,
        std::chrono::seconds(1)};
    co_await timer;
}

void coro_throw_cancel()
{
  xio::io_context ctx;
  xio::cancellation_signal sig;

  auto k = coro_throw_cancel_impl(ctx);

  std::exception_ptr res_ex;
  k.async_resume(
      xio::bind_cancellation_slot(sig.slot(),
        [&](std::exception_ptr ex) {res_ex = ex;}));
  xio::post(ctx, [&]{sig.emit(xio::cancellation_type::all);});

  ASIO_CHECK(!res_ex);

  ctx.run();

  ASIO_CHECK(res_ex);
  try
  {
    if (res_ex)
      std::rethrow_exception(res_ex);
  }
  catch (xio::system_error& se)
  {
    ASIO_CHECK(se.code() == xio::error::operation_aborted);
  }
}

auto coro_simple_cancel_nested_k(xio::io_context&, int& cnt) noexcept
  -> xio::experimental::coro<
      void() noexcept,
      xio::error_code>
{
  xio::steady_timer timer{
          co_await this_coro::executor,
          std::chrono::milliseconds(100)};

  ASIO_CHECK(!(co_await this_coro::cancellation_state).cancelled());
  auto ec = co_await timer;
  cnt++;
  ASIO_CHECK((co_await this_coro::cancellation_state).cancelled());

  co_return ec;
}

auto coro_simple_cancel_nested_kouter(
    xio::io_context& ctx, int& cnt) noexcept
  -> xio::experimental::coro<
      xio::error_code() noexcept,
      xio::error_code>
{
    ASIO_CHECK(cnt == 0);
    co_yield co_await coro_simple_cancel_nested_k(ctx, cnt);
    ASIO_CHECK(cnt == 1);
    auto ec = co_await coro_simple_cancel_nested_k(ctx, cnt);
    ASIO_CHECK(cnt == 2);
    co_return ec;
}

void coro_simple_cancel_nested()
{
  xio::io_context ctx;
  xio::cancellation_signal sig;

  int cnt = 0;
  auto kouter = coro_simple_cancel_nested_kouter(ctx, cnt);

  xio::error_code res_ec;
  kouter.async_resume(
      xio::bind_cancellation_slot(sig.slot(),
        [&](xio::error_code ec) {res_ec = ec;}));
  xio::post(ctx, [&]{sig.emit(xio::cancellation_type::all);});
  ASIO_CHECK(!res_ec);
  ctx.run();
  ASIO_CHECK(res_ec == xio::error::operation_aborted);

  ctx.restart();
  res_ec = {};
  kouter.async_resume(
      xio::bind_cancellation_slot(sig.slot(),
        [&](xio::error_code ec) {res_ec = ec;}));
  xio::post(ctx, [&]{sig.emit(xio::cancellation_type::all);});
  ASIO_CHECK(!res_ec);
  ctx.run();
  ASIO_CHECK(res_ec == xio::error::operation_aborted);
  ASIO_CHECK(cnt == 2);
}

} // namespace coro

ASIO_TEST_SUITE
(
  "coro/cancel",
  ASIO_TEST_CASE(::coro::coro_simple_cancel)
  ASIO_TEST_CASE(::coro::coro_throw_cancel)
  ASIO_TEST_CASE(::coro::coro_simple_cancel_nested)
)
