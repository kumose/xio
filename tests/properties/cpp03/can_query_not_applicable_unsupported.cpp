//
// cpp03/can_query_not_applicable_unsupported.cpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
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

int main()
{
  assert((!xio::can_query<object, prop>::value));
  assert((!xio::can_query<const object, prop>::value));
}
