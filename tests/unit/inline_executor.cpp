//
// inline_executor.cpp
// ~~~~~~~~~~~~~~~~~~~
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

// Prevent link dependency on the Boost.System library.
#if !defined(BOOST_SYSTEM_NO_DEPRECATED)
#define BOOST_SYSTEM_NO_DEPRECATED
#endif // !defined(BOOST_SYSTEM_NO_DEPRECATED)

// Test that header file is self-contained.
#include <xio/inline_executor.h>

#include <functional>
#include <xio/any_completion_executor.h>
#include <xio/dispatch.h>
#include "unit_test.hpp"

using namespace xio;

namespace bindns = std;

void increment(xio::detail::atomic_count* count)
{
  ++(*count);
}

void inline_executor_query_test()
{
  ASIO_CHECK(
      xio::query(inline_executor(),
        xio::execution::blocking)
      == xio::execution::blocking.always);

  ASIO_CHECK(
      xio::query(inline_executor(),
        xio::execution::blocking.possibly)
      == xio::execution::blocking.always);

  ASIO_CHECK(
      xio::query(inline_executor(),
        xio::execution::outstanding_work)
      == xio::execution::outstanding_work.untracked);

  ASIO_CHECK(
      xio::query(inline_executor(),
        xio::execution::outstanding_work.untracked)
      == xio::execution::outstanding_work.untracked);

  ASIO_CHECK(
      xio::query(inline_executor(),
        xio::execution::relationship)
      == xio::execution::relationship.fork);

  ASIO_CHECK(
      xio::query(inline_executor(),
        xio::execution::relationship.fork)
      == xio::execution::relationship.fork);

  ASIO_CHECK(
      xio::query(inline_executor(),
        xio::execution::mapping)
      == xio::execution::mapping.thread);

  ASIO_CHECK(
      xio::query(inline_executor(),
        xio::execution::inline_exception_handling)
      == xio::execution::inline_exception_handling.propagate);

  ASIO_CHECK(
      xio::query(inline_executor(),
        xio::execution::inline_exception_handling.propagate)
      == xio::execution::inline_exception_handling.propagate);

  ASIO_CHECK(
      xio::query(
        xio::require(inline_executor(),
          xio::execution::inline_exception_handling.terminate),
        xio::execution::inline_exception_handling)
      == xio::execution::inline_exception_handling.terminate);

  ASIO_CHECK(
      xio::query(
        xio::require(inline_executor(),
          xio::execution::inline_exception_handling.terminate),
        xio::execution::inline_exception_handling.propagate)
      == xio::execution::inline_exception_handling.terminate);
}

void inline_executor_execute_test()
{
  xio::detail::atomic_count count(0);

  inline_executor().execute(bindns::bind(increment, &count));

  xio::require(inline_executor(),
      xio::execution::blocking.always
    ).execute(bindns::bind(increment, &count));

  xio::prefer(inline_executor(),
      xio::execution::blocking.possibly
    ).execute(bindns::bind(increment, &count));

  xio::any_completion_executor ex = inline_executor();

  ex.execute(bindns::bind(increment, &count));

  ASIO_CHECK(count == 4);
}

void inline_executor_dispatch_test()
{
  xio::detail::atomic_count count(0);

  xio::dispatch(inline_executor(),
      bindns::bind(increment, &count));

  xio::dispatch(
      xio::require(inline_executor(),
        xio::execution::inline_exception_handling.terminate),
      bindns::bind(increment, &count));

  ASIO_CHECK(count == 2);
}

void throw_exception()
{
  throw 42;
}

void inline_executor_exception_test()
{
#if !defined(ASIO_NO_EXCEPTIONS)
  bool caught = false;

  try
  {
    inline_executor().execute(throw_exception);
  }
  catch (...)
  {
    caught = true;
  }

  ASIO_CHECK(caught);
#endif // !defined(ASIO_NO_EXCEPTIONS)
}

ASIO_TEST_SUITE
(
  "inline_executor",
  ASIO_TEST_CASE(inline_executor_query_test)
  ASIO_TEST_CASE(inline_executor_execute_test)
  ASIO_TEST_CASE(inline_executor_dispatch_test)
  ASIO_TEST_CASE(inline_executor_exception_test)
)
