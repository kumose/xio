//
// recycling_allocator.cpp
// ~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//



// Test that header file is self-contained.
#include <xio/recycling_allocator.h>

#include "unit_test.hpp"
#include <vector>
#include <xio/detail/type_traits.h>

void recycling_allocator_test()
{
  ASIO_CHECK((
      std::is_same<
        xio::recycling_allocator<int>::value_type,
        int
      >::value));

  ASIO_CHECK((
      std::is_same<
        xio::recycling_allocator<void>::value_type,
        void
      >::value));

  ASIO_CHECK((
      std::is_same<
        xio::recycling_allocator<int>::rebind<char>::other,
        xio::recycling_allocator<char>
      >::value));

  ASIO_CHECK((
      std::is_same<
        xio::recycling_allocator<void>::rebind<char>::other,
        xio::recycling_allocator<char>
      >::value));

  xio::recycling_allocator<int> a1;
  xio::recycling_allocator<int> a2(a1);

  ASIO_CHECK(a1 == a2);
  ASIO_CHECK(!(a1 != a2));

  xio::recycling_allocator<void> a3;
  xio::recycling_allocator<void> a4(a3);

  ASIO_CHECK(a3 == a4);
  ASIO_CHECK(!(a3 != a4));

  xio::recycling_allocator<int> a5(a4);
  (void)a5;

  xio::recycling_allocator<void> a6(a5);
  (void)a6;

  int* p = a1.allocate(42);
  ASIO_CHECK(p != 0);

  a1.deallocate(p, 42);

  std::vector<int, xio::recycling_allocator<int> > v(42);
  ASIO_CHECK(v.size() == 42);
}

ASIO_TEST_SUITE
(
  "recycling_allocator",
  ASIO_TEST_CASE(recycling_allocator_test)
)
