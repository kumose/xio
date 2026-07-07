//
// system_timer.cpp
// ~~~~~~~~~~~~~~~~
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
#include <xio/system_timer.h>

#include <functional>
#include <xio/bind_cancellation_slot.h>
#include <xio/cancellation_signal.h>
#include <xio/executor_work_guard.h>
#include <xio/io_context.h>
#include <xio/thread.h>
#include "unit_test.hpp"

namespace bindns = std;

void increment(int* count)
{
  ++(*count);
}

void decrement_to_zero(xio::system_timer* t, int* count)
{
  if (*count > 0)
  {
    --(*count);

    int before_value = *count;

    t->expires_at(t->expiry() + xio::chrono::seconds(1));
    t->async_wait(bindns::bind(decrement_to_zero, t, count));

    // Completion cannot nest, so count value should remain unchanged.
    ASIO_CHECK(*count == before_value);
  }
}

void increment_if_not_cancelled(int* count,
    const xio::error_code& ec)
{
  if (!ec)
    ++(*count);
}

void cancel_timer(xio::system_timer* t)
{
  std::size_t num_cancelled = t->cancel();
  ASIO_CHECK(num_cancelled == 1);
}

void cancel_one_timer(xio::system_timer* t)
{
  std::size_t num_cancelled = t->cancel_one();
  ASIO_CHECK(num_cancelled == 1);
}

xio::system_timer::time_point now()
{
  return xio::system_timer::clock_type::now();
}

void system_timer_test()
{
  using xio::chrono::seconds;
  using xio::chrono::microseconds;
  using bindns::placeholders::_1;
  using bindns::placeholders::_2;

  xio::io_context ioc;
  const xio::io_context::executor_type ioc_ex = ioc.get_executor();
  int count = 0;

  xio::system_timer::time_point start = now();

  xio::system_timer t1(ioc, seconds(1));
  t1.wait();

  // The timer must block until after its expiry time.
  xio::system_timer::time_point end = now();
  xio::system_timer::time_point expected_end = start + seconds(1);
  ASIO_CHECK(expected_end < end || expected_end == end);

  start = now();

  xio::system_timer t2(ioc_ex, seconds(1) + microseconds(500000));
  t2.wait();

  // The timer must block until after its expiry time.
  end = now();
  expected_end = start + seconds(1) + microseconds(500000);
  ASIO_CHECK(expected_end < end || expected_end == end);

  t2.expires_at(t2.expiry() + seconds(1));
  t2.wait();

  // The timer must block until after its expiry time.
  end = now();
  expected_end += seconds(1);
  ASIO_CHECK(expected_end < end || expected_end == end);

  start = now();

  t2.expires_after(seconds(1) + microseconds(200000));
  t2.wait();

  // The timer must block until after its expiry time.
  end = now();
  expected_end = start + seconds(1) + microseconds(200000);
  ASIO_CHECK(expected_end < end || expected_end == end);

  start = now();

  xio::system_timer t3(ioc, seconds(5));
  t3.async_wait(bindns::bind(increment, &count));

  // No completions can be delivered until run() is called.
  ASIO_CHECK(count == 0);

  ioc.run();

  // The run() call will not return until all operations have finished, and
  // this should not be until after the timer's expiry time.
  ASIO_CHECK(count == 1);
  end = now();
  expected_end = start + seconds(1);
  ASIO_CHECK(expected_end < end || expected_end == end);

  count = 3;
  start = now();

  xio::system_timer t4(ioc, seconds(1));
  t4.async_wait(bindns::bind(decrement_to_zero, &t4, &count));

  // No completions can be delivered until run() is called.
  ASIO_CHECK(count == 3);

  ioc.restart();
  ioc.run();

  // The run() call will not return until all operations have finished, and
  // this should not be until after the timer's final expiry time.
  ASIO_CHECK(count == 0);
  end = now();
  expected_end = start + seconds(3);
  ASIO_CHECK(expected_end < end || expected_end == end);

  count = 0;
  start = now();

  xio::system_timer t5(ioc, seconds(10));
  t5.async_wait(bindns::bind(increment_if_not_cancelled, &count, _1));
  xio::system_timer t6(ioc, seconds(1));
  t6.async_wait(bindns::bind(cancel_timer, &t5));

  // No completions can be delivered until run() is called.
  ASIO_CHECK(count == 0);

  ioc.restart();
  ioc.run();

  // The timer should have been cancelled, so count should not have changed.
  // The total run time should not have been much more than 1 second (and
  // certainly far less than 10 seconds).
  ASIO_CHECK(count == 0);
  end = now();
  expected_end = start + seconds(2);
  ASIO_CHECK(end < expected_end);

  // Wait on the timer again without cancelling it. This time the asynchronous
  // wait should run to completion and increment the counter.
  t5.async_wait(bindns::bind(increment_if_not_cancelled, &count, _1));

  ioc.restart();
  ioc.run();

  // The timer should not have been cancelled, so count should have changed.
  // The total time since the timer was created should be more than 10 seconds.
  ASIO_CHECK(count == 1);
  end = now();
  expected_end = start + seconds(10);
  ASIO_CHECK(expected_end < end || expected_end == end);

  count = 0;
  start = now();

  // Start two waits on a timer, one of which will be cancelled. The one
  // which is not cancelled should still run to completion and increment the
  // counter.
  xio::system_timer t7(ioc, seconds(3));
  t7.async_wait(bindns::bind(increment_if_not_cancelled, &count, _1));
  t7.async_wait(bindns::bind(increment_if_not_cancelled, &count, _1));
  xio::system_timer t8(ioc, seconds(1));
  t8.async_wait(bindns::bind(cancel_one_timer, &t7));

  ioc.restart();
  ioc.run();

  // One of the waits should not have been cancelled, so count should have
  // changed. The total time since the timer was created should be more than 3
  // seconds.
  ASIO_CHECK(count == 1);
  end = now();
  expected_end = start + seconds(3);
  ASIO_CHECK(expected_end < end || expected_end == end);
}

struct timer_handler
{
  timer_handler() {}
  void operator()(const xio::error_code&) {}
  timer_handler(timer_handler&&) {}
private:
  timer_handler(const timer_handler&);
};

void system_timer_cancel_test()
{
  static xio::io_context io_context;
  struct timer
  {
    xio::system_timer t;
    timer() : t(io_context)
    {
      t.expires_at((xio::system_timer::time_point::max)());
    }
  } timers[50];

  timers[2].t.async_wait(timer_handler());
  timers[41].t.async_wait(timer_handler());
  for (int i = 10; i < 20; ++i)
    timers[i].t.async_wait(timer_handler());

  ASIO_CHECK(timers[2].t.cancel() == 1);
  ASIO_CHECK(timers[41].t.cancel() == 1);
  for (int i = 10; i < 20; ++i)
    ASIO_CHECK(timers[i].t.cancel() == 1);
}

struct custom_allocation_timer_handler
{
  custom_allocation_timer_handler(int* count) : count_(count) {}
  void operator()(const xio::error_code&) {}
  int* count_;

  template <typename T>
  struct allocator
  {
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;
    typedef T* pointer;
    typedef const T* const_pointer;
    typedef T& reference;
    typedef const T& const_reference;
    typedef T value_type;

    template <typename U>
    struct rebind
    {
      typedef allocator<U> other;
    };

    explicit allocator(int* count) noexcept
      : count_(count)
    {
    }

    allocator(const allocator& other) noexcept
      : count_(other.count_)
    {
    }

    template <typename U>
    allocator(const allocator<U>& other) noexcept
      : count_(other.count_)
    {
    }

    pointer allocate(size_type n, const void* = 0)
    {
      ++(*count_);
      return static_cast<T*>(::operator new(sizeof(T) * n));
    }

    void deallocate(pointer p, size_type)
    {
      --(*count_);
      ::operator delete(p);
    }

    size_type max_size() const
    {
      return ~size_type(0);
    }

    void construct(pointer p, const T& v)
    {
      new (p) T(v);
    }

    void destroy(pointer p)
    {
      p->~T();
    }

    int* count_;
  };

  typedef allocator<int> allocator_type;

  allocator_type get_allocator() const noexcept
  {
    return allocator_type(count_);
  }
};

void system_timer_custom_allocation_test()
{
  static xio::io_context io_context;
  struct timer
  {
    xio::system_timer t;
    timer() : t(io_context) {}
  } timers[100];

  int allocation_count = 0;

  for (int i = 0; i < 50; ++i)
  {
    timers[i].t.expires_at((xio::system_timer::time_point::max)());
    timers[i].t.async_wait(custom_allocation_timer_handler(&allocation_count));
  }

  for (int i = 50; i < 100; ++i)
  {
    timers[i].t.expires_at((xio::system_timer::time_point::min)());
    timers[i].t.async_wait(custom_allocation_timer_handler(&allocation_count));
  }

  for (int i = 0; i < 50; ++i)
    timers[i].t.cancel();

  io_context.run();

  ASIO_CHECK(allocation_count == 0);
}

void io_context_run(xio::io_context* ioc)
{
  ioc->run();
}

void system_timer_thread_test()
{
  xio::io_context ioc;
  xio::executor_work_guard<xio::io_context::executor_type> work
    = xio::make_work_guard(ioc);
  xio::system_timer t1(ioc);
  xio::system_timer t2(ioc);
  int count = 0;

  xio::thread th(bindns::bind(io_context_run, &ioc));

  t2.expires_after(xio::chrono::seconds(2));
  t2.wait();

  t1.expires_after(xio::chrono::seconds(2));
  t1.async_wait(bindns::bind(increment, &count));

  t2.expires_after(xio::chrono::seconds(4));
  t2.wait();

  ioc.stop();
  th.join();

  ASIO_CHECK(count == 1);
}

xio::system_timer make_timer(xio::io_context& ioc, int* count)
{
  xio::system_timer t(ioc);
  t.expires_after(xio::chrono::seconds(1));
  t.async_wait(bindns::bind(increment, count));
  return t;
}

typedef xio::basic_waitable_timer<
    xio::system_timer::clock_type,
    xio::system_timer::traits_type,
    xio::io_context::executor_type> io_context_system_timer;

io_context_system_timer make_convertible_timer(xio::io_context& ioc, int* count)
{
  io_context_system_timer t(ioc);
  t.expires_after(xio::chrono::seconds(1));
  t.async_wait(bindns::bind(increment, count));
  return t;
}

void system_timer_move_test()
{
  xio::io_context io_context1;
  xio::io_context io_context2;
  int count = 0;

  xio::system_timer t1 = make_timer(io_context1, &count);
  xio::system_timer t2 = make_timer(io_context2, &count);
  xio::system_timer t3 = std::move(t1);

  t2 = std::move(t1);

  io_context2.run();

  ASIO_CHECK(count == 1);

  io_context1.run();

  ASIO_CHECK(count == 2);

  xio::system_timer t4 = make_convertible_timer(io_context1, &count);
  xio::system_timer t5 = make_convertible_timer(io_context2, &count);
  xio::system_timer t6 = std::move(t4);

  t2 = std::move(t4);

  io_context2.restart();
  io_context2.run();

  ASIO_CHECK(count == 3);

  io_context1.restart();
  io_context1.run();

  ASIO_CHECK(count == 4);
}

void system_timer_op_cancel_test()
{
  xio::cancellation_signal cancel_signal;
  xio::io_context ioc;
  int count = 0;

  xio::system_timer timer(ioc, xio::chrono::seconds(10));

  timer.async_wait(bindns::bind(increment, &count));

  timer.async_wait(
      xio::bind_cancellation_slot(
        cancel_signal.slot(),
        bindns::bind(increment, &count)));

  timer.async_wait(bindns::bind(increment, &count));

  ioc.poll();

  ASIO_CHECK(count == 0);
  ASIO_CHECK(!ioc.stopped());

  cancel_signal.emit(xio::cancellation_type::all);

  ioc.run_one();
  ioc.poll();

  ASIO_CHECK(count == 1);
  ASIO_CHECK(!ioc.stopped());

  timer.cancel();

  ioc.run();

  ASIO_CHECK(count == 3);
  ASIO_CHECK(ioc.stopped());
}

ASIO_TEST_SUITE
(
  "system_timer",
  ASIO_TEST_CASE(system_timer_test)
  ASIO_TEST_CASE(system_timer_cancel_test)
  ASIO_TEST_CASE(system_timer_custom_allocation_test)
  ASIO_TEST_CASE(system_timer_thread_test)
  ASIO_TEST_CASE(system_timer_move_test)
  ASIO_TEST_CASE(system_timer_op_cancel_test)
)
