//
// system_executor.cpp
// ~~~~~~~~~~~~~~~~~~~
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
#include <xio/system_executor.h>

#include <functional>
#include <xio/dispatch.h>
#include <xio/post.h>
#include "unit_test.hpp"

using namespace xio;

namespace bindns = std;

void increment(xio::detail::atomic_count* count)
{
  ++(*count);
}

void system_executor_query_test()
{
  XIO_CHECK(
      &xio::query(system_executor(),
        xio::execution::context)
      != static_cast<const system_context*>(0));

  XIO_CHECK(
      xio::query(system_executor(),
        xio::execution::blocking)
      == xio::execution::blocking.possibly);

  XIO_CHECK(
      xio::query(system_executor(),
        xio::execution::blocking.possibly)
      == xio::execution::blocking.possibly);

  XIO_CHECK(
      xio::query(system_executor(),
        xio::execution::outstanding_work)
      == xio::execution::outstanding_work.untracked);

  XIO_CHECK(
      xio::query(system_executor(),
        xio::execution::outstanding_work.untracked)
      == xio::execution::outstanding_work.untracked);

  XIO_CHECK(
      xio::query(system_executor(),
        xio::execution::relationship)
      == xio::execution::relationship.fork);

  XIO_CHECK(
      xio::query(system_executor(),
        xio::execution::relationship.fork)
      == xio::execution::relationship.fork);

  XIO_CHECK(
      xio::query(system_executor(),
        xio::execution::mapping)
      == xio::execution::mapping.thread);

  XIO_CHECK(
      xio::query(system_executor(),
        xio::execution::inline_exception_handling)
      == xio::execution::inline_exception_handling.terminate);

  XIO_CHECK(
      xio::query(system_executor(),
        xio::execution::allocator)
      == std::allocator<void>());
}

void system_executor_execute_test()
{
  xio::detail::atomic_count count(0);

  system_executor().execute(bindns::bind(increment, &count));

  xio::require(system_executor(),
      xio::execution::blocking.possibly
    ).execute(bindns::bind(increment, &count));

  xio::require(system_executor(),
      xio::execution::blocking.always
    ).execute(bindns::bind(increment, &count));

  xio::require(system_executor(),
      xio::execution::blocking.never
    ).execute(bindns::bind(increment, &count));

  xio::require(system_executor(),
      xio::execution::blocking.never,
      xio::execution::outstanding_work.untracked
    ).execute(bindns::bind(increment, &count));

  xio::require(system_executor(),
      xio::execution::blocking.never,
      xio::execution::outstanding_work.untracked,
      xio::execution::relationship.fork
    ).execute(bindns::bind(increment, &count));

  xio::require(system_executor(),
      xio::execution::blocking.never,
      xio::execution::outstanding_work.untracked,
      xio::execution::relationship.continuation
    ).execute(bindns::bind(increment, &count));

  xio::prefer(
      xio::require(system_executor(),
        xio::execution::blocking.never,
        xio::execution::outstanding_work.untracked,
        xio::execution::relationship.continuation),
      xio::execution::allocator(std::allocator<void>())
    ).execute(bindns::bind(increment, &count));

  xio::prefer(
      xio::require(system_executor(),
        xio::execution::blocking.never,
        xio::execution::outstanding_work.untracked,
        xio::execution::relationship.continuation),
      xio::execution::allocator
    ).execute(bindns::bind(increment, &count));

  xio::query(system_executor(), execution::context).join();

  XIO_CHECK(count == 9);
}

XIO_TEST_SUITE
(
  "system_executor",
  XIO_TEST_CASE(system_executor_query_test)
  XIO_TEST_CASE(system_executor_execute_test)
)
