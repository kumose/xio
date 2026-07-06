//
// thread_pool.cpp
// ~~~~~~~~~~~~~~~
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
#include <xio/thread_pool.h>

#include <functional>
#include <xio/dispatch.h>
#include <xio/post.h>
#include "unit_test.hpp"

using namespace xio;
namespace bindns = std;

void increment(int* count)
{
  ++(*count);
}

void decrement_to_zero(thread_pool* pool, int* count)
{
  if (*count > 0)
  {
    --(*count);

    int before_value = *count;
    xio::post(*pool, bindns::bind(decrement_to_zero, pool, count));

    // Handler execution cannot nest, so count value should remain unchanged.
    ASIO_CHECK(*count == before_value);
  }
}

void nested_decrement_to_zero(thread_pool* pool, int* count)
{
  if (*count > 0)
  {
    --(*count);

    xio::dispatch(*pool,
        bindns::bind(nested_decrement_to_zero, pool, count));

    // Handler execution is nested, so count value should now be zero.
    ASIO_CHECK(*count == 0);
  }
}

void thread_pool_test()
{
  thread_pool pool(1);

  int count1 = 0;
  xio::post(pool, bindns::bind(increment, &count1));

  int count2 = 10;
  xio::post(pool, bindns::bind(decrement_to_zero, &pool, &count2));

  int count3 = 10;
  xio::post(pool, bindns::bind(nested_decrement_to_zero, &pool, &count3));

  pool.wait();

  ASIO_CHECK(count1 == 1);
  ASIO_CHECK(count2 == 0);
  ASIO_CHECK(count3 == 0);
}

class test_service : public xio::execution_context::service
{
public:
#if defined(ASIO_NO_TYPEID)
  static xio::execution_context::id id;
#endif // defined(ASIO_NO_TYPEID)

  typedef test_service key_type;

  test_service(xio::execution_context& ctx)
    : xio::execution_context::service(ctx)
  {
  }

private:
  virtual void shutdown() {}
};

#if defined(ASIO_NO_TYPEID)
xio::execution_context::id test_service::id;
#endif // defined(ASIO_NO_TYPEID)

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
  virtual void shutdown() {}

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

void thread_pool_service_test()
{
  xio::thread_pool pool1(1);
  xio::thread_pool pool2(1);
  xio::thread_pool pool3(1);

  // Implicit service registration.

  xio::use_service<test_service>(pool1);

  ASIO_CHECK(xio::has_service<test_service>(pool1));

  test_service* svc1 = new test_service(pool1);
  try
  {
    xio::add_service(pool1, svc1);
    ASIO_ERROR("add_service did not throw");
  }
  catch (xio::service_already_exists&)
  {
  }
  delete svc1;

  // Explicit service registration.

  test_service& svc2 = xio::make_service<test_service>(pool2);

  ASIO_CHECK(xio::has_service<test_service>(pool2));
  ASIO_CHECK(&xio::use_service<test_service>(pool2) == &svc2);

  test_service* svc3 = new test_service(pool2);
  try
  {
    xio::add_service(pool2, svc3);
    ASIO_ERROR("add_service did not throw");
  }
  catch (xio::service_already_exists&)
  {
  }
  delete svc3;

  // Explicit registration with invalid owner.

  test_service* svc4 = new test_service(pool2);
  try
  {
    xio::add_service(pool3, svc4);
    ASIO_ERROR("add_service did not throw");
  }
  catch (xio::invalid_service_owner&)
  {
  }
  delete svc4;

  ASIO_CHECK(!xio::has_service<test_service>(pool3));

  // Initial service registration.

  xio::thread_pool pool4{1, test_context_service_maker{}};

  ASIO_CHECK(xio::has_service<test_context_service>(pool4));
  ASIO_CHECK(xio::use_service<test_context_service>(pool4).get_value()
      == 42);
}

void thread_pool_executor_query_test()
{
  thread_pool pool(1);

  ASIO_CHECK(
      &xio::query(pool.executor(),
        xio::execution::context)
      == &pool);

  ASIO_CHECK(
      xio::query(pool.executor(),
        xio::execution::blocking)
      == xio::execution::blocking.possibly);

  ASIO_CHECK(
      xio::query(pool.executor(),
        xio::execution::blocking.possibly)
      == xio::execution::blocking.possibly);

  ASIO_CHECK(
      xio::query(pool.executor(),
        xio::execution::outstanding_work)
      == xio::execution::outstanding_work.untracked);

  ASIO_CHECK(
      xio::query(pool.executor(),
        xio::execution::outstanding_work.untracked)
      == xio::execution::outstanding_work.untracked);

  ASIO_CHECK(
      xio::query(pool.executor(),
        xio::execution::relationship)
      == xio::execution::relationship.fork);

  ASIO_CHECK(
      xio::query(pool.executor(),
        xio::execution::relationship.fork)
      == xio::execution::relationship.fork);

  ASIO_CHECK(
      xio::query(pool.executor(),
        xio::execution::mapping)
      == xio::execution::mapping.thread);

  ASIO_CHECK(
      xio::query(pool.executor(),
        xio::execution::inline_exception_handling)
      == xio::execution::inline_exception_handling.terminate);

  ASIO_CHECK(
      xio::query(pool.executor(),
        xio::execution::allocator)
      == std::allocator<void>());

  ASIO_CHECK(
      xio::query(pool.executor(),
        xio::execution::occupancy)
      == 1);
}

void thread_pool_executor_execute_test()
{
  int count = 0;
  thread_pool pool(1);

  pool.executor().execute(bindns::bind(increment, &count));

  xio::require(pool.executor(),
      xio::execution::blocking.possibly
    ).execute(bindns::bind(increment, &count));

  xio::require(pool.executor(),
      xio::execution::blocking.always
    ).execute(bindns::bind(increment, &count));

  xio::require(pool.executor(),
      xio::execution::blocking.never
    ).execute(bindns::bind(increment, &count));

  xio::require(pool.executor(),
      xio::execution::blocking.never,
      xio::execution::outstanding_work.tracked
    ).execute(bindns::bind(increment, &count));

  xio::require(pool.executor(),
      xio::execution::blocking.never,
      xio::execution::outstanding_work.untracked
    ).execute(bindns::bind(increment, &count));

  xio::require(pool.executor(),
      xio::execution::blocking.never,
      xio::execution::outstanding_work.untracked,
      xio::execution::relationship.fork
    ).execute(bindns::bind(increment, &count));

  xio::require(pool.executor(),
      xio::execution::blocking.never,
      xio::execution::outstanding_work.untracked,
      xio::execution::relationship.continuation
    ).execute(bindns::bind(increment, &count));

  xio::prefer(
      xio::require(pool.executor(),
        xio::execution::blocking.never,
        xio::execution::outstanding_work.untracked,
        xio::execution::relationship.continuation),
      xio::execution::allocator(std::allocator<void>())
    ).execute(bindns::bind(increment, &count));

  xio::prefer(
      xio::require(pool.executor(),
        xio::execution::blocking.never,
        xio::execution::outstanding_work.untracked,
        xio::execution::relationship.continuation),
      xio::execution::allocator
    ).execute(bindns::bind(increment, &count));

  pool.wait();

  ASIO_CHECK(count == 10);
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

void thread_pool_allocator_test()
{
  int live_count;
  int total_count;

#if !defined(ASIO_NO_TS_EXECUTORS)
  {
    live_count = 0;
    total_count = 0;
    thread_pool pool1(std::allocator_arg,
        custom_allocator<int>(&live_count, &total_count));
    (void)pool1;

    ASIO_CHECK(live_count > 0);
    ASIO_CHECK(total_count > 0);
  }

  ASIO_CHECK(live_count == 0);
  ASIO_CHECK(total_count > 0);
#endif // !defined(ASIO_NO_TS_EXECUTORS)

  {
    live_count = 0;
    total_count = 0;
    thread_pool pool2(std::allocator_arg,
        custom_allocator<int>(&live_count, &total_count), 1);
    (void)pool2;

    ASIO_CHECK(live_count > 0);
    ASIO_CHECK(total_count > 0);
  }

  ASIO_CHECK(live_count == 0);
  ASIO_CHECK(total_count > 0);

  {
    live_count = 0;
    total_count = 0;
    thread_pool pool3(std::allocator_arg,
        custom_allocator<int>(&live_count, &total_count), 1,
        xio::config_from_string(""));
    (void)pool3;

    ASIO_CHECK(live_count > 0);
    ASIO_CHECK(total_count > 0);
  }

  ASIO_CHECK(live_count == 0);
  ASIO_CHECK(total_count > 0);
}

ASIO_TEST_SUITE
(
  "thread_pool",
  ASIO_TEST_CASE(thread_pool_test)
  ASIO_TEST_CASE(thread_pool_service_test)
  ASIO_TEST_CASE(thread_pool_executor_query_test)
  ASIO_TEST_CASE(thread_pool_executor_execute_test)
  ASIO_TEST_CASE(thread_pool_allocator_test)
)
