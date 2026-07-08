//
// executor.cpp
// ~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//



// Test that header file is self-contained.
#include <xio/execution/executor.h>

#include "../unit_test.hpp"

struct not_an_executor
{
};

struct executor
{
  executor()
  {
  }

  executor(const executor&) noexcept
  {
  }

  executor(executor&&) noexcept
  {
  }

  template <typename F>
  void execute(F&& f) const noexcept
  {
    (void)f;
  }

  bool operator==(const executor&) const noexcept
  {
    return true;
  }

  bool operator!=(const executor&) const noexcept
  {
    return false;
  }
};

namespace xio {
namespace traits {

#if !defined(XIO_HAS_DEDUCED_EXECUTE_MEMBER_TRAIT)

template <typename F>
struct execute_member<executor, F>
{
  static constexpr bool is_valid = true;
  static constexpr bool is_noexcept = true;
  typedef void result_type;
};

#endif // !defined(XIO_HAS_DEDUCED_SET_ERROR_MEMBER_TRAIT)
#if !defined(XIO_HAS_DEDUCED_EQUALITY_COMPARABLE_TRAIT)

template <>
struct equality_comparable<executor>
{
  static constexpr bool is_valid = true;
  static constexpr bool is_noexcept = true;
};

#endif // !defined(XIO_HAS_DEDUCED_EQUALITY_COMPARABLE_TRAIT)

} // namespace traits
} // namespace xio

void is_executor_test()
{
  XIO_CHECK((
      !xio::execution::is_executor<
        void
      >::value));

  XIO_CHECK((
      !xio::execution::is_executor<
        not_an_executor
      >::value));

  XIO_CHECK((
      xio::execution::is_executor<
        executor
      >::value));
}

XIO_TEST_SUITE
(
  "executor",
  XIO_TEST_CASE(is_executor_test)
)
