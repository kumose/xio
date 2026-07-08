//
// cpp11/can_query_not_applicable_static.cpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <xio/query.h>
#include <cassert>

struct prop
{
};

struct object
{
};

namespace xio {
namespace traits {

template<>
struct static_query<object, prop>
{
  static constexpr bool is_valid = true;
  static constexpr bool is_noexcept = true;
  typedef int result_type;
  static constexpr int value() { return 123; }
};

} // namespace traits
} // namespace xio

int main()
{
  static_assert(!xio::can_query<object, prop>::value, "");
  static_assert(!xio::can_query<const object, prop>::value, "");
}
