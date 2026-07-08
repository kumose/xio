//
// coroutine.cpp
// ~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//



// Test that header file is self-contained.
#include <xio/coroutine.h>

#include "unit_test.hpp"

// Must come after all other headers.
#include <xio/yield.h>

//------------------------------------------------------------------------------

// Coroutine completes via yield break.

void yield_break_coro(xio::coroutine& coro)
{
  reenter (coro)
  {
    yield return;
    yield break;
  }
}

void yield_break_test()
{
  xio::coroutine coro;
  XIO_CHECK(!coro.is_complete());
  yield_break_coro(coro);
  XIO_CHECK(!coro.is_complete());
  yield_break_coro(coro);
  XIO_CHECK(coro.is_complete());
}

//------------------------------------------------------------------------------

// Coroutine completes via return.

void return_coro(xio::coroutine& coro)
{
  reenter (coro)
  {
    return;
  }
}

void return_test()
{
  xio::coroutine coro;
  return_coro(coro);
  XIO_CHECK(coro.is_complete());
}

//------------------------------------------------------------------------------

// Coroutine completes via exception.

void exception_coro(xio::coroutine& coro)
{
  reenter (coro)
  {
    throw 1;
  }
}

void exception_test()
{
  xio::coroutine coro;
  try { exception_coro(coro); } catch (int) {}
  XIO_CHECK(coro.is_complete());
}

//------------------------------------------------------------------------------

// Coroutine completes by falling off the end.

void fall_off_end_coro(xio::coroutine& coro)
{
  reenter (coro)
  {
  }
}

void fall_off_end_test()
{
  xio::coroutine coro;
  fall_off_end_coro(coro);
  XIO_CHECK(coro.is_complete());
}

//------------------------------------------------------------------------------

XIO_TEST_SUITE
(
  "coroutine",
  XIO_TEST_CASE(yield_break_test)
  XIO_TEST_CASE(return_test)
  XIO_TEST_CASE(exception_test)
  XIO_TEST_CASE(fall_off_end_test)
)
