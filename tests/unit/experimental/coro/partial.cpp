//
// experimental/coro/partial.cpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2021-2023 Klemens D. Morgenstern
//                         (klemens dot morgenstern at gmx dot net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//



// Test that header file is self-contained.
#include <xio/experimental/coro.h>

#include <xio/io_context.h>
#include "../../unit_test.hpp"

using namespace xio::experimental;

namespace coro {

void partial()
{
  xio::io_context ctx;
  bool ran = false;
  auto p = detail::post_coroutine(ctx, [&]{ran = true;}).handle;
  XIO_CHECK(!ran);
  p.resume();
  XIO_CHECK(!ran);
  ctx.run();
  XIO_CHECK(ran);
}

} // namespace coro

XIO_TEST_SUITE
(
  "coro/partial",
  XIO_TEST_CASE(::coro::partial)
)
