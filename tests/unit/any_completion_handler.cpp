//
// any_completion_handler.cpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//



// Test that header file is self-contained.
#include <xio/any_completion_handler.h>

#include "unit_test.hpp"

#include <functional>
#include <xio/bind_allocator.h>
#include <xio/bind_cancellation_slot.h>
#include <xio/bind_executor.h>
#include <xio/bind_immediate_executor.h>
#include <xio/error.h>
#include <xio/inline_executor.h>
#include <xio/thread_pool.h>

namespace bindns = std;

void increment(int* count)
{
  ++(*count);
}

void any_completion_handler_construction_test()
{
  int count = 0;
  std::nullptr_t null_ptr = std::nullptr_t();

  xio::any_completion_handler<void()> h1;

  XIO_CHECK(!h1);
  XIO_CHECK(h1 == null_ptr);

  xio::any_completion_handler<void()> h2(null_ptr);

  XIO_CHECK(!h2);
  XIO_CHECK(h2 == null_ptr);

  xio::any_completion_handler<void()> h3(
      bindns::bind(&increment, &count));

  XIO_CHECK(!!h3);
  XIO_CHECK(h3 != null_ptr);

  xio::any_completion_handler<void()> h4(std::move(h1));

  XIO_CHECK(!h4);
  XIO_CHECK(h4 == null_ptr);
  XIO_CHECK(!h1);
  XIO_CHECK(h1 == null_ptr);

  xio::any_completion_handler<void()> h5(std::move(h3));

  XIO_CHECK(!!h5);
  XIO_CHECK(h5 != null_ptr);
  XIO_CHECK(!h3);
  XIO_CHECK(h3 == null_ptr);

  xio::any_completion_handler<void()> h6(std::move(h5));

  XIO_CHECK(!!h6);
  XIO_CHECK(h6 != null_ptr);
  XIO_CHECK(!h5);
  XIO_CHECK(h5 == null_ptr);
}

void any_completion_handler_assignment_test()
{
  int count = 0;
  std::nullptr_t null_ptr = std::nullptr_t();

  xio::any_completion_handler<void()> h1;

  xio::any_completion_handler<void()> h2;
  h2 = null_ptr;

  XIO_CHECK(!h2);

  xio::any_completion_handler<void()> h3;
  h3 = bindns::bind(&increment, &count);

  XIO_CHECK(!!h3);

  xio::any_completion_handler<void()> h4;
  h4 = std::move(h1);

  XIO_CHECK(!h4);
  XIO_CHECK(!h1);

  h4 = std::move(h3);

  XIO_CHECK(!!h4);
  XIO_CHECK(!h3);
}

template <typename T>
class handler_allocator
{
public:
  using value_type = T;

  explicit handler_allocator(int* count)
    : count_(count)
  {
  }

  template <typename U>
  handler_allocator(const handler_allocator<U>& other) noexcept
    : count_(other.count_)
  {
  }

  bool operator==(const handler_allocator& other) const noexcept
  {
    return &count_ == &other.count_;
  }

  bool operator!=(const handler_allocator& other) const noexcept
  {
    return &count_ != &other.count_;
  }

  T* allocate(std::size_t n) const
  {
    ++(*count_);
    return static_cast<T*>(::operator new(sizeof(T) * n));
  }

  void deallocate(T* p, std::size_t /*n*/) const
  {
    ::operator delete(p);
  }

private:
  template <typename> friend class handler_allocator;

  int* count_;
};

class cancel_handler
{
public:
  explicit cancel_handler(int* count)
    : count_(count)
  {
  }

  void operator()(xio::cancellation_type_t)
  {
    ++(*count_);
  }

private:
  int* count_;
};

void any_completion_handler_associator_test()
{
  typedef xio::any_completion_handler<void()> handler_type;

  int count = 0;
  int alloc_count = 0;
  int cancel_count = 0;
  xio::thread_pool pool(1);
  xio::cancellation_signal sig;

  xio::any_completion_handler<void()> h1(
      xio::bind_allocator(handler_allocator<char>(&alloc_count),
        xio::bind_cancellation_slot(sig.slot(),
          xio::bind_executor(pool.get_executor(),
            xio::bind_immediate_executor(xio::inline_executor(),
              bindns::bind(&increment, &count))))));

  XIO_CHECK(alloc_count == 1);

  XIO_REBIND_ALLOC(xio::associated_allocator<handler_type>::type,
      char) alloc1(xio::get_associated_allocator(h1));
  alloc1.deallocate(alloc1.allocate(1), 1);

  XIO_CHECK(alloc_count == 2);

  xio::associated_cancellation_slot<handler_type>::type slot1
    = xio::get_associated_cancellation_slot(h1);

  XIO_CHECK(slot1.is_connected());

  slot1.emplace<cancel_handler>(&cancel_count);

  XIO_CHECK(cancel_count == 0);

  sig.emit(xio::cancellation_type::terminal);

  XIO_CHECK(cancel_count == 1);

  xio::associated_executor<handler_type>::type ex1
    = xio::get_associated_executor(h1);

  XIO_CHECK(ex1 == pool.get_executor());

  xio::associated_immediate_executor<
    handler_type, xio::thread_pool::executor_type>::type ex2
      = xio::get_associated_immediate_executor(h1, pool.get_executor());

  XIO_CHECK(ex2 == xio::inline_executor());
}

void increment_with_error(xio::error_code ec,
    xio::error_code* out_ec, int* count)
{
  *out_ec = ec;
  ++(*count);
}

void any_completion_handler_invocation_test()
{
  int count = 0;
  xio::error_code ec;

  xio::any_completion_handler<void()> h1(
      bindns::bind(&increment, &count));

  XIO_CHECK(count == 0);

  std::move(h1)();

  XIO_CHECK(count == 1);

  xio::any_completion_handler<void(xio::error_code)> h2(
      bindns::bind(&increment_with_error,
        bindns::placeholders::_1, &ec, &count));

  XIO_CHECK(!ec);
  XIO_CHECK(count == 1);

  std::move(h2)(xio::error::eof);

  XIO_CHECK(ec == xio::error::eof);
  XIO_CHECK(count == 2);
}

XIO_TEST_SUITE
(
  "any_completion_handler",
  XIO_TEST_CASE(any_completion_handler_construction_test)
  XIO_TEST_CASE(any_completion_handler_assignment_test)
  XIO_TEST_CASE(any_completion_handler_associator_test)
  XIO_TEST_CASE(any_completion_handler_invocation_test)
)
