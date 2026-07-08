//
// bind_cancellation_slot.cpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//



// Test that header file is self-contained.
#include <xio/bind_cancellation_slot.h>

#include <functional>
#include <xio/cancellation_signal.h>
#include <xio/io_context.h>
#include <xio/steady_timer.h>
#include "unit_test.hpp"

using namespace xio;
namespace bindns = std;
typedef steady_timer timer;
namespace chronons = std::chrono;

void increment_on_cancel(int* count, const xio::error_code& error)
{
  if (error == xio::error::operation_aborted)
    ++(*count);
}

void bind_cancellation_slot_to_function_object_test()
{
  io_context ioc;
  cancellation_signal sig;

  int count = 0;

  timer t(ioc, chronons::seconds(5));
  t.async_wait(
      bind_cancellation_slot(sig.slot(),
        bindns::bind(&increment_on_cancel,
          &count, bindns::placeholders::_1)));

  ioc.poll();

  XIO_CHECK(count == 0);

  sig.emit(xio::cancellation_type::all);

  ioc.run();

  XIO_CHECK(count == 1);

  t.async_wait(
      bind_cancellation_slot(sig.slot(),
        bind_cancellation_slot(sig.slot(),
          bindns::bind(&increment_on_cancel,
            &count, bindns::placeholders::_1))));

  ioc.restart();
  ioc.poll();

  XIO_CHECK(count == 1);

  sig.emit(xio::cancellation_type::all);

  ioc.run();

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

  void operator()(xio::error_code error)
  {
    increment_on_cancel(count, error);
  }

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

void bind_cancellation_slot_to_completion_token_v1_test()
{
  io_context ioc;
  cancellation_signal sig;

  int count = 0;

  timer t(ioc, chronons::seconds(5));
  t.async_wait(
      bind_cancellation_slot(sig.slot(),
        incrementer_token_v1(&count)));

  ioc.poll();

  XIO_CHECK(count == 0);

  sig.emit(xio::cancellation_type::all);

  ioc.run();

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
    initiation(
        bindns::bind(&increment_on_cancel,
          token.count, bindns::placeholders::_1),
        static_cast<Args&&>(args)...);
  }
};

} // namespace xio

void bind_cancellation_slot_to_completion_token_v2_test()
{
  io_context ioc;
  cancellation_signal sig;

  int count = 0;

  timer t(ioc, chronons::seconds(5));
  t.async_wait(
      bind_cancellation_slot(sig.slot(),
        incrementer_token_v2(&count)));

  ioc.poll();

  XIO_CHECK(count == 0);

  sig.emit(xio::cancellation_type::all);

  ioc.run();

  XIO_CHECK(count == 1);
}

void partial_bind_cancellation_slot()
{
  io_context ioc;
  cancellation_signal sig;

  int count = 0;

  timer t(ioc, chronons::seconds(5));
  t.async_wait(bind_cancellation_slot(sig.slot()))(
      bindns::bind(&increment_on_cancel,
        &count, bindns::placeholders::_1));

  ioc.poll();

  XIO_CHECK(count == 0);

  sig.emit(xio::cancellation_type::all);

  ioc.run();

  XIO_CHECK(count == 1);

  t.async_wait()(
      bind_cancellation_slot(sig.slot()))(
        incrementer_token_v2(&count));

  ioc.restart();
  ioc.poll();

  XIO_CHECK(count == 1);

  sig.emit(xio::cancellation_type::all);

  ioc.run();

  XIO_CHECK(count == 2);
}

XIO_TEST_SUITE
(
  "bind_cancellation_slot",
  XIO_TEST_CASE(bind_cancellation_slot_to_function_object_test)
  XIO_TEST_CASE(bind_cancellation_slot_to_completion_token_v1_test)
  XIO_TEST_CASE(bind_cancellation_slot_to_completion_token_v2_test)
  XIO_TEST_CASE(partial_bind_cancellation_slot)
)
