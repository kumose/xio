//
// context_as.cpp
// ~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//



// Test that header file is self-contained.
#include <xio/execution/context_as.h>

#include <functional>
#include <xio/execution/any_executor.h>
#include <xio/io_context.h>
#include <xio/static_thread_pool.h>
#include "../unit_test.hpp"

using namespace xio;
namespace bindns = std;

void context_as_executor_query_test()
{
  static_thread_pool pool(1);

  XIO_CHECK(
      &xio::query(pool.executor(),
        execution::context_as_t<static_thread_pool&>())
        == &pool);

  execution::any_executor<
      execution::context_as_t<static_thread_pool&>
    > ex1 = pool.executor();

  XIO_CHECK(
      &xio::query(ex1,
        execution::context_as_t<static_thread_pool&>())
        == &pool);

  XIO_CHECK(
      &xio::query(ex1, execution::context)
        == &pool);

  XIO_CHECK(
      &xio::query(pool.executor(),
        execution::context_as_t<const static_thread_pool&>())
        == &pool);

  execution::any_executor<
      execution::context_as_t<const static_thread_pool&>
    > ex2 = pool.executor();

  XIO_CHECK(
      &xio::query(ex2,
        execution::context_as_t<const static_thread_pool&>())
        == &pool);

  XIO_CHECK(
      &xio::query(ex2, execution::context)
        == &pool);

  io_context io_ctx;

  XIO_CHECK(
      &xio::query(io_ctx.get_executor(),
        execution::context_as_t<io_context&>())
        == &io_ctx);

  execution::any_executor<
      execution::context_as_t<io_context&>
    > ex3 = io_ctx.get_executor();

  XIO_CHECK(
      &xio::query(ex3,
        execution::context_as_t<io_context&>())
        == &io_ctx);

  XIO_CHECK(
      &xio::query(ex3, execution::context)
        == &io_ctx);

  XIO_CHECK(
      &xio::query(io_ctx.get_executor(),
        execution::context_as_t<const io_context&>())
        == &io_ctx);

  execution::any_executor<
      execution::context_as_t<const io_context&>
    > ex4 = io_ctx.get_executor();

  XIO_CHECK(
      &xio::query(ex4,
        execution::context_as_t<const io_context&>())
        == &io_ctx);

  XIO_CHECK(
      &xio::query(ex4, execution::context)
        == &io_ctx);

  XIO_CHECK(
      &xio::query(io_ctx.get_executor(),
        execution::context_as_t<execution_context&>())
        == &io_ctx);

  execution::any_executor<
      execution::context_as_t<execution_context&>
    > ex5 = io_ctx.get_executor();

  XIO_CHECK(
      &xio::query(ex5,
        execution::context_as_t<execution_context&>())
        == &io_ctx);

  XIO_CHECK(
      &xio::query(ex5, execution::context)
        == &io_ctx);

  XIO_CHECK(
      &xio::query(io_ctx.get_executor(),
        execution::context_as_t<const execution_context&>())
        == &io_ctx);

  execution::any_executor<
      execution::context_as_t<const execution_context&>
    > ex6 = io_ctx.get_executor();

  XIO_CHECK(
      &xio::query(ex6,
        execution::context_as_t<const execution_context&>())
        == &io_ctx);

  XIO_CHECK(
      &xio::query(ex6, execution::context)
        == &io_ctx);
}

XIO_TEST_SUITE
(
  "context_as",
  XIO_TEST_CASE(context_as_executor_query_test)
)
