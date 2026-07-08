//
// bind_executor.cpp
// ~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//



// Test that header file is self-contained.
#include <xio/bind_executor.h>

#include <functional>
#include <xio/inline_executor.h>
#include <xio/io_context.h>
#include <xio/steady_timer.h>
#include "unit_test.hpp"

using namespace xio;
namespace bindns = std;
typedef steady_timer timer;
namespace chronons = std::chrono;

void increment(int* count)
{
  ++(*count);
}

void bind_executor_to_function_object_test()
{
  io_context ioc1;
  io_context ioc2;

  int count = 0;

  timer t(ioc1, chronons::seconds(1));
  t.async_wait(
      bind_executor(
        ioc2.get_executor(),
        bindns::bind(&increment, &count)));

  ioc1.run();

  XIO_CHECK(count == 0);

  ioc2.run();

  XIO_CHECK(count == 1);

  t.async_wait(
      bind_executor(
        ioc2.get_executor(),
        bind_executor(
          xio::inline_executor(),
          bindns::bind(&increment, &count))));

  ioc1.restart();
  ioc1.run();

  XIO_CHECK(count == 1);

  ioc2.restart();
  ioc2.run();

  XIO_CHECK(count == 2);
}

struct incrementer_token_v1
{
  explicit incrementer_token_v1(int* c) : count(c) {}
  int* count;
};

struct incrementer_handler_v1
{
  explicit incrementer_handler_v1(incrementer_token_v1 t) : count(t.count) {}
  void operator()(xio::error_code){ increment(count); }
  int* count;
};

namespace xio {

template <>
class async_result<incrementer_token_v1, void(xio::error_code)>
{
public:
  typedef incrementer_handler_v1 completion_handler_type;
  typedef void return_type;
  explicit async_result(completion_handler_type&) {}
  return_type get() {}
};

} // namespace xio

void bind_executor_to_completion_token_v1_test()
{
  io_context ioc1;
  io_context ioc2;

  int count = 0;

  timer t(ioc1, chronons::seconds(1));
  t.async_wait(
      bind_executor(
        ioc2.get_executor(),
        incrementer_token_v1(&count)));

  ioc1.run();

  XIO_CHECK(count == 0);

  ioc2.run();

  XIO_CHECK(count == 1);
}

struct incrementer_token_v2
{
  explicit incrementer_token_v2(int* c) : count(c) {}
  int* count;
};

namespace xio {

template <>
class async_result<incrementer_token_v2, void(xio::error_code)>
{
public:
#if !defined(XIO_HAS_RETURN_TYPE_DEDUCTION)
  typedef void return_type;
#endif // !defined(XIO_HAS_RETURN_TYPE_DEDUCTION)

  template <typename Initiation, typename... Args>
  static void initiate(Initiation initiation,
      incrementer_token_v2 token, Args&&... args)
  {
    initiation(bindns::bind(&increment, token.count),
        static_cast<Args&&>(args)...);
  }
};

} // namespace xio

void bind_executor_to_completion_token_v2_test()
{
  io_context ioc1;
  io_context ioc2;

  int count = 0;

  timer t(ioc1, chronons::seconds(1));
  t.async_wait(
      bind_executor(
        ioc2.get_executor(),
        incrementer_token_v2(&count)));

  ioc1.run();

  XIO_CHECK(count == 0);

  ioc2.run();

  XIO_CHECK(count == 1);
}

void partial_bind_executor_test()
{
  io_context ioc1;
  io_context ioc2;

  int count = 0;

  timer t(ioc1, chronons::seconds(1));
  t.async_wait(bind_executor(ioc2.get_executor()))(
      bindns::bind(&increment, &count));

  ioc1.run();

  XIO_CHECK(count == 0);

  ioc2.run();

  XIO_CHECK(count == 1);

  t.expires_after(chronons::seconds(1));
  t.async_wait()(
      bind_executor(ioc2.get_executor()))(
        incrementer_token_v2(&count));

  ioc1.restart();
  ioc1.run();

  XIO_CHECK(count == 1);

  ioc2.restart();
  ioc2.run();

  XIO_CHECK(count == 2);
}

XIO_TEST_SUITE
(
  "bind_executor",
  XIO_TEST_CASE(bind_executor_to_function_object_test)
  XIO_TEST_CASE(bind_executor_to_completion_token_v1_test)
  XIO_TEST_CASE(bind_executor_to_completion_token_v2_test)
  XIO_TEST_CASE(partial_bind_executor_test)
)
