//
// cpp11/can_query_member.cpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~
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
  constexpr int query(prop) const { return 123; }
};

namespace xio {

template<>
struct is_applicable_property<object, prop>
{
  static constexpr bool value = true;
};

} // namespace xio

int main()
{
  static_assert(xio::can_query<object, prop>::value, "");
  static_assert(xio::can_query<const object, prop>::value, "");
}
