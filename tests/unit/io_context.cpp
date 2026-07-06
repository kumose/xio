//
// io_context.cpp
// ~~~~~~~~~~~~~~
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
#include <xio/io_context.h>

#include <functional>
#include <sstream>
#include <xio/bind_executor.h>
#include <xio/dispatch.h>
#include <xio/post.h>
#include <xio/steady_timer.h>
#include <xio/thread.h>
#include "unit_test.hpp"

using namespace xio;
namespace bindns = std;

typedef steady_timer timer;
namespace chronons = xio::chrono;

void increment(int* count)
{
  ++(*count);
}

void decrement_to_zero(io_context* ioc, int* count)
{
  if (*count > 0)
  {
    --(*count);

    int before_value = *count;
    xio::post(*ioc, bindns::bind(decrement_to_zero, ioc, count));

    // Handler execution cannot nest, so count value should remain unchanged.
    ASIO_CHECK(*count == before_value);
  }
}

void nested_decrement_to_zero(io_context* ioc, int* count)
{
  if (*count > 0)
  {
    --(*count);

    xio::dispatch(*ioc,
        bindns::bind(nested_decrement_to_zero, ioc, count));

    // Handler execution is nested, so count value should now be zero.
    ASIO_CHECK(*count == 0);
  }
}

void sleep_increment(io_context* ioc, int* count)
{
  timer t(*ioc, chronons::seconds(2));
  t.wait();

  if (++(*count) < 3)
    xio::post(*ioc, bindns::bind(sleep_increment, ioc, count));
}

void start_sleep_increments(io_context* ioc, int* count)
{
  // Give all threads a chance to start.
  timer t(*ioc, chronons::seconds(2));
  t.wait();

  // Start the first of three increments.
  xio::post(*ioc, bindns::bind(sleep_increment, ioc, count));
}

void throw_exception()
{
  throw 1;
}

void io_context_run(io_context* ioc)
{
  ioc->run();
}

void io_context_test()
{
  io_context ioc;
  int count = 0;

  xio::post(ioc, bindns::bind(increment, &count));

  // No handlers can be called until run() is called.
  ASIO_CHECK(!ioc.stopped());
  ASIO_CHECK(count == 0);

  ioc.run();

  // The run() call will not return until all work has finished.
  ASIO_CHECK(ioc.stopped());
  ASIO_CHECK(count == 1);

  count = 0;
  ioc.restart();
  xio::post(ioc, bindns::bind(increment, &count));
  xio::post(ioc, bindns::bind(increment, &count));
  xio::post(ioc, bindns::bind(increment, &count));
  xio::post(ioc, bindns::bind(increment, &count));
  xio::post(ioc, bindns::bind(increment, &count));

  // No handlers can be called until run() is called.
  ASIO_CHECK(!ioc.stopped());
  ASIO_CHECK(count == 0);

  ioc.run();

  // The run() call will not return until all work has finished.
  ASIO_CHECK(ioc.stopped());
  ASIO_CHECK(count == 5);

  count = 0;
  ioc.restart();
  executor_work_guard<io_context::executor_type> w = make_work_guard(ioc);
  xio::post(ioc, bindns::bind(&io_context::stop, &ioc));
  ASIO_CHECK(!ioc.stopped());
  ioc.run();

  // The only operation executed should have been to stop run().
  ASIO_CHECK(ioc.stopped());
  ASIO_CHECK(count == 0);

  ioc.restart();
  xio::post(ioc, bindns::bind(increment, &count));
  w.reset();

  // No handlers can be called until run() is called.
  ASIO_CHECK(!ioc.stopped());
  ASIO_CHECK(count == 0);

  ioc.run();

  // The run() call will not return until all work has finished.
  ASIO_CHECK(ioc.stopped());
  ASIO_CHECK(count == 1);

  count = 10;
  ioc.restart();
  xio::post(ioc, bindns::bind(decrement_to_zero, &ioc, &count));

  // No handlers can be called until run() is called.
  ASIO_CHECK(!ioc.stopped());
  ASIO_CHECK(count == 10);

  ioc.run();

  // The run() call will not return until all work has finished.
  ASIO_CHECK(ioc.stopped());
  ASIO_CHECK(count == 0);

  count = 10;
  ioc.restart();
  xio::post(ioc, bindns::bind(nested_decrement_to_zero, &ioc, &count));

  // No handlers can be called until run() is called.
  ASIO_CHECK(!ioc.stopped());
  ASIO_CHECK(count == 10);

  ioc.run();

  // The run() call will not return until all work has finished.
  ASIO_CHECK(ioc.stopped());
  ASIO_CHECK(count == 0);

  count = 10;
  ioc.restart();
  xio::dispatch(ioc,
      bindns::bind(nested_decrement_to_zero, &ioc, &count));

  // No handlers can be called until run() is called, even though nested
  // delivery was specifically allowed in the previous call.
  ASIO_CHECK(!ioc.stopped());
  ASIO_CHECK(count == 10);

  ioc.run();

  // The run() call will not return until all work has finished.
  ASIO_CHECK(ioc.stopped());
  ASIO_CHECK(count == 0);

  count = 0;
  int count2 = 0;
  ioc.restart();
  ASIO_CHECK(!ioc.stopped());
  xio::post(ioc, bindns::bind(start_sleep_increments, &ioc, &count));
  xio::post(ioc, bindns::bind(start_sleep_increments, &ioc, &count2));
  thread thread1(bindns::bind(io_context_run, &ioc));
  thread thread2(bindns::bind(io_context_run, &ioc));
  thread1.join();
  thread2.join();

  // The run() calls will not return until all work has finished.
  ASIO_CHECK(ioc.stopped());
  ASIO_CHECK(count == 3);
  ASIO_CHECK(count2 == 3);

  count = 10;
  io_context ioc2;
  xio::dispatch(ioc, xio::bind_executor(ioc2,
        bindns::bind(decrement_to_zero, &ioc2, &count)));
  ioc.restart();
  ASIO_CHECK(!ioc.stopped());
  ioc.run();

  // No decrement_to_zero handlers can be called until run() is called on the
  // second io_context object.
  ASIO_CHECK(ioc.stopped());
  ASIO_CHECK(count == 10);

  ioc2.run();

  // The run() call will not return until all work has finished.
  ASIO_CHECK(count == 0);

  count = 0;
  int exception_count = 0;
  ioc.restart();
  xio::post(ioc, &throw_exception);
  xio::post(ioc, bindns::bind(increment, &count));
  xio::post(ioc, bindns::bind(increment, &count));
  xio::post(ioc, &throw_exception);
  xio::post(ioc, bindns::bind(increment, &count));

  // No handlers can be called until run() is called.
  ASIO_CHECK(!ioc.stopped());
  ASIO_CHECK(count == 0);
  ASIO_CHECK(exception_count == 0);

  for (;;)
  {
    try
    {
      ioc.run();
      break;
    }
    catch (int)
    {
      ++exception_count;
    }
  }

  // The run() calls will not return until all work has finished.
  ASIO_CHECK(ioc.stopped());
  ASIO_CHECK(count == 3);
  ASIO_CHECK(exception_count == 2);
}

class test_service : public xio::io_context::service
{
public:
  static xio::io_context::id id;

  test_service(xio::io_context& s)
    : xio::io_context::service(s)
  {
  }

private:
  void shutdown() override
  {
  }
};

xio::io_context::id test_service::id;

class test_context_service : public xio::execution_context::service
{
public:
  static xio::execution_context::id id;

  test_context_service(xio::execution_context& c, int value = 0)
    : xio::execution_context::service(c),
      value_(value)
  {
  }

  int get_value() const
  {
    return value_;
  }

private:
  void shutdown() override
  {
  }

  int value_;
};

xio::execution_context::id test_context_service::id;

class test_context_service_maker :
  public xio::execution_context::service_maker
{
public:
  void make(xio::execution_context& ctx) const override
  {
    (void)xio::make_service<test_context_service>(ctx, 42);
  }
};

void io_context_service_test()
{
  xio::io_context ioc1;
  xio::io_context ioc2;
  xio::io_context ioc3;

  // Implicit service registration.

  xio::use_service<test_service>(ioc1);

  ASIO_CHECK(xio::has_service<test_service>(ioc1));

  test_service* svc1 = new test_service(ioc1);
  try
  {
    xio::add_service(ioc1, svc1);
    ASIO_ERROR("add_service did not throw");
  }
  catch (xio::service_already_exists&)
  {
  }
  delete svc1;

  // Explicit service registration.

  test_service* svc2 = new test_service(ioc2);
  xio::add_service(ioc2, svc2);

  ASIO_CHECK(xio::has_service<test_service>(ioc2));
  ASIO_CHECK(&xio::use_service<test_service>(ioc2) == svc2);

  test_service* svc3 = new test_service(ioc2);
  try
  {
    xio::add_service(ioc2, svc3);
    ASIO_ERROR("add_service did not throw");
  }
  catch (xio::service_already_exists&)
  {
  }
  delete svc3;

  // Explicit registration with invalid owner.

  test_service* svc4 = new test_service(ioc2);
  try
  {
    xio::add_service(ioc3, svc4);
    ASIO_ERROR("add_service did not throw");
  }
  catch (xio::invalid_service_owner&)
  {
  }
  delete svc4;

  ASIO_CHECK(!xio::has_service<test_service>(ioc3));

  // Initial service registration.

  xio::io_context ioc4{test_context_service_maker{}};

  ASIO_CHECK(xio::has_service<test_context_service>(ioc4));
  ASIO_CHECK(xio::use_service<test_context_service>(ioc4).get_value()
      == 42);
}

void io_context_executor_query_test()
{
  io_context ioc;

  ASIO_CHECK(
      &xio::query(ioc.get_executor(),
        xio::execution::context)
      == &ioc);

  ASIO_CHECK(
      xio::query(ioc.get_executor(),
        xio::execution::blocking)
      == xio::execution::blocking.possibly);

  ASIO_CHECK(
      xio::query(ioc.get_executor(),
        xio::execution::blocking.possibly)
      == xio::execution::blocking.possibly);

  ASIO_CHECK(
      xio::query(ioc.get_executor(),
        xio::execution::outstanding_work)
      == xio::execution::outstanding_work.untracked);

  ASIO_CHECK(
      xio::query(ioc.get_executor(),
        xio::execution::outstanding_work.untracked)
      == xio::execution::outstanding_work.untracked);

  ASIO_CHECK(
      xio::query(ioc.get_executor(),
        xio::execution::relationship)
      == xio::execution::relationship.fork);

  ASIO_CHECK(
      xio::query(ioc.get_executor(),
        xio::execution::relationship.fork)
      == xio::execution::relationship.fork);

  ASIO_CHECK(
      xio::query(ioc.get_executor(),
        xio::execution::mapping)
      == xio::execution::mapping.thread);

  ASIO_CHECK(
      xio::query(ioc.get_executor(),
        xio::execution::inline_exception_handling)
      == xio::execution::inline_exception_handling.capture);

  ASIO_CHECK(
      xio::query(ioc.get_executor(),
        xio::execution::allocator)
      == std::allocator<void>());
}

void io_context_executor_execute_test()
{
  io_context ioc;
  int count = 0;

  ioc.get_executor().execute(bindns::bind(increment, &count));

  // No handlers can be called until run() is called.
  ASIO_CHECK(!ioc.stopped());
  ASIO_CHECK(count == 0);

  ioc.run();

  // The run() call will not return until all work has finished.
  ASIO_CHECK(ioc.stopped());
  ASIO_CHECK(count == 1);

  count = 0;
  ioc.restart();
  xio::require(ioc.get_executor(),
      xio::execution::blocking.possibly
    ).execute(bindns::bind(increment, &count));

  // No handlers can be called until run() is called.
  ASIO_CHECK(!ioc.stopped());
  ASIO_CHECK(count == 0);

  ioc.run();

  // The run() call will not return until all work has finished.
  ASIO_CHECK(ioc.stopped());
  ASIO_CHECK(count == 1);

  count = 0;
  ioc.restart();
  xio::require(ioc.get_executor(),
      xio::execution::blocking.never
    ).execute(bindns::bind(increment, &count));

  // No handlers can be called until run() is called.
  ASIO_CHECK(!ioc.stopped());
  ASIO_CHECK(count == 0);

  ioc.run();

  // The run() call will not return until all work has finished.
  ASIO_CHECK(ioc.stopped());
  ASIO_CHECK(count == 1);

  count = 0;
  ioc.restart();
  ASIO_CHECK(!ioc.stopped());

  xio::require(ioc.get_executor(),
      xio::execution::blocking.never,
      xio::execution::outstanding_work.tracked
    ).execute(bindns::bind(increment, &count));

  // No handlers can be called until run() is called.
  ASIO_CHECK(!ioc.stopped());
  ASIO_CHECK(count == 0);

  ioc.run();

  // The run() call will not return until all work has finished.
  ASIO_CHECK(ioc.stopped());
  ASIO_CHECK(count == 1);

  count = 0;
  ioc.restart();
  xio::require(ioc.get_executor(),
      xio::execution::blocking.never,
      xio::execution::outstanding_work.untracked
    ).execute(bindns::bind(increment, &count));

  // No handlers can be called until run() is called.
  ASIO_CHECK(!ioc.stopped());
  ASIO_CHECK(count == 0);

  ioc.run();

  // The run() call will not return until all work has finished.
  ASIO_CHECK(ioc.stopped());
  ASIO_CHECK(count == 1);

  count = 0;
  ioc.restart();
  xio::require(ioc.get_executor(),
      xio::execution::blocking.never,
      xio::execution::outstanding_work.untracked,
      xio::execution::relationship.fork
    ).execute(bindns::bind(increment, &count));

  // No handlers can be called until run() is called.
  ASIO_CHECK(!ioc.stopped());
  ASIO_CHECK(count == 0);

  ioc.run();

  // The run() call will not return until all work has finished.
  ASIO_CHECK(ioc.stopped());
  ASIO_CHECK(count == 1);

  count = 0;
  ioc.restart();
  xio::require(ioc.get_executor(),
      xio::execution::blocking.never,
      xio::execution::outstanding_work.untracked,
      xio::execution::relationship.continuation
    ).execute(bindns::bind(increment, &count));

  // No handlers can be called until run() is called.
  ASIO_CHECK(!ioc.stopped());
  ASIO_CHECK(count == 0);

  ioc.run();

  // The run() call will not return until all work has finished.
  ASIO_CHECK(ioc.stopped());
  ASIO_CHECK(count == 1);

  count = 0;
  ioc.restart();
  xio::prefer(
      xio::require(ioc.get_executor(),
        xio::execution::blocking.never,
        xio::execution::outstanding_work.untracked,
        xio::execution::relationship.continuation),
      xio::execution::allocator(std::allocator<void>())
    ).execute(bindns::bind(increment, &count));

  // No handlers can be called until run() is called.
  ASIO_CHECK(!ioc.stopped());
  ASIO_CHECK(count == 0);

  ioc.run();

  // The run() call will not return until all work has finished.
  ASIO_CHECK(ioc.stopped());
  ASIO_CHECK(count == 1);

  count = 0;
  ioc.restart();
  xio::prefer(
      xio::require(ioc.get_executor(),
        xio::execution::blocking.never,
        xio::execution::outstanding_work.untracked,
        xio::execution::relationship.continuation),
      xio::execution::allocator
    ).execute(bindns::bind(increment, &count));

  // No handlers can be called until run() is called.
  ASIO_CHECK(!ioc.stopped());
  ASIO_CHECK(count == 0);

  ioc.run();

  // The run() call will not return until all work has finished.
  ASIO_CHECK(ioc.stopped());
  ASIO_CHECK(count == 1);
}

template <typename T>
class custom_allocator
{
public:
  using value_type = T;

  custom_allocator(int* live_count, int* total_count)
    : live_count_(live_count),
      total_count_(total_count)
  {
  }

  template <typename U>
  custom_allocator(const custom_allocator<U>& other) noexcept
    : live_count_(other.live_count_),
      total_count_(other.total_count_)
  {
  }

  bool operator==(const custom_allocator& other) const noexcept
  {
    return &live_count_ == &other.live_count_ &&
      &total_count_ == &other.total_count_;;
  }

  bool operator!=(const custom_allocator& other) const noexcept
  {
    return &live_count_ != &other.live_count_ ||
      &total_count_ != &other.total_count_;
  }

  T* allocate(std::size_t n) const
  {
    ++(*live_count_);
    ++(*total_count_);
    return static_cast<T*>(::operator new(sizeof(T) * n));
  }

  void deallocate(T* p, std::size_t /*n*/) const
  {
    --(*live_count_);
    ::operator delete(p);
  }

private:
  template <typename> friend class custom_allocator;

  int* live_count_;
  int* total_count_;
};

void io_context_allocator_test()
{
  int live_count;
  int total_count;

  {
    live_count = 0;
    total_count = 0;
    io_context ioc1(std::allocator_arg,
        custom_allocator<int>(&live_count, &total_count));
    (void)ioc1;

    ASIO_CHECK(live_count > 0);
    ASIO_CHECK(total_count > 0);
  }

  ASIO_CHECK(live_count == 0);
  ASIO_CHECK(total_count > 0);

  {
    live_count = 0;
    total_count = 0;
    io_context ioc2(std::allocator_arg,
        custom_allocator<int>(&live_count, &total_count), 1);
    (void)ioc2;

    ASIO_CHECK(live_count > 0);
    ASIO_CHECK(total_count > 0);
  }

  ASIO_CHECK(live_count == 0);
  ASIO_CHECK(total_count > 0);

  {
    live_count = 0;
    total_count = 0;
    io_context ioc3(std::allocator_arg,
        custom_allocator<int>(&live_count, &total_count),
        xio::config_from_string(""));
    (void)ioc3;

    ASIO_CHECK(live_count > 0);
    ASIO_CHECK(total_count > 0);
  }

  ASIO_CHECK(live_count == 0);
  ASIO_CHECK(total_count > 0);
}

ASIO_TEST_SUITE
(
  "io_context",
  ASIO_TEST_CASE(io_context_test)
  ASIO_TEST_CASE(io_context_service_test)
  ASIO_TEST_CASE(io_context_executor_query_test)
  ASIO_TEST_CASE(io_context_executor_execute_test)
  ASIO_TEST_CASE(io_context_allocator_test)
)
