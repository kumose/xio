//
// experimental/parallel_group.cpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

// Disable autolinking for unit tests.
#if !defined(BOOST_ALL_NO_LIB)
#define BOOST_ALL_NO_LIB 1
#endif // !defined(BOOST_ALL_NO_LIB)

// Test that header file is self-contained.
#include <xio/experimental/parallel_group.h>

#include <array>
#include <xio/bind_cancellation_slot.h>
#include <xio/cancellation_signal.h>
#include <xio/deferred.h>
#include <xio/error.h>
#include <xio/io_context.h>
#include <xio/steady_timer.h>
#include "../unit_test.hpp"

struct wait_for_cancel_filter
{
  xio::steady_timer* timer_;
  xio::cancellation_type_t react_to_;

  void operator()(xio::cancellation_type_t type)
  {
    if (!!(type & react_to_))
      timer_->cancel();
  }
};

struct wait_for_cancel_initiation
{
  xio::cancellation_type_t react_to_;

  template <typename Handler>
  void operator()(Handler&& handler, xio::steady_timer* timer) const
  {
    auto slot = xio::get_associated_cancellation_slot(handler);
    timer->expires_after(xio::chrono::hours(1));
    if (slot.is_connected())
      slot.assign(wait_for_cancel_filter{timer, react_to_});

    timer->async_wait(
        xio::bind_cancellation_slot(xio::cancellation_slot(),
          static_cast<Handler&&>(handler)));
  }
};

template <typename CompletionToken = xio::deferred_t>
auto async_wait_for_cancel(xio::steady_timer& timer,
    xio::cancellation_type_t react_to,
    CompletionToken&& token = xio::deferred_t())
  -> decltype(
      xio::async_initiate<CompletionToken, void(xio::error_code)>(
        wait_for_cancel_initiation{react_to}, token, &timer))
{
  return xio::async_initiate<CompletionToken, void(xio::error_code)>(
      wait_for_cancel_initiation{react_to}, token, &timer);
}

void non_terminal_group_cancellation_test()
{
  xio::io_context ioc;
  xio::steady_timer t0(ioc);
  xio::steady_timer t1(ioc);
  xio::cancellation_signal cancel_signal;

  int called = 0;
  std::array<std::size_t, 2> order = {{}};
  xio::error_code ec0 = xio::error::would_block;
  xio::error_code ec1 = xio::error::would_block;

  xio::experimental::make_parallel_group(
      async_wait_for_cancel(t0, xio::cancellation_type::partial),
      async_wait_for_cancel(t1, xio::cancellation_type::terminal)
    ).async_wait(
        xio::experimental::wait_for_one(),
        xio::bind_cancellation_slot(
          cancel_signal.slot(),
          [&](std::array<std::size_t, 2> o,
            xio::error_code e0, xio::error_code e1)
          {
            ++called;
            order = o;
            ec0 = e0;
            ec1 = e1;
          }
        )
      );

  cancel_signal.emit(xio::cancellation_type::partial);

  ioc.run_for(xio::chrono::seconds(5));

  ASIO_CHECK(called == 1);
  ASIO_CHECK(order[0] == 0);
  ASIO_CHECK(order[1] == 1);
  ASIO_CHECK(ec0 == xio::error::operation_aborted);
  ASIO_CHECK(ec1 == xio::error::operation_aborted);
}

ASIO_TEST_SUITE
(
  "experimental/parallel_group",
  ASIO_TEST_CASE(non_terminal_group_cancellation_test)
)
