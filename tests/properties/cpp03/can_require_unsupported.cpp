//
// cpp03/can_require_unsupported.cpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <xio/require.h>
#include <cassert>

template <int>
struct prop
{
};

template <int>
struct object
{
};

namespace xio {

template<int N, int M>
struct is_applicable_property<object<N>, prop<M> >
{
  static const bool value = true;
};

} // namespace xio

int main()
{
  assert((!xio::can_require<object<1>, prop<2> >::value));
  assert((!xio::can_require<object<1>, prop<2>, prop<3> >::value));
  assert((!xio::can_require<object<1>, prop<2>, prop<3>, prop<4> >::value));
  assert((!xio::can_require<const object<1>, prop<2> >::value));
  assert((!xio::can_require<const object<1>, prop<2>, prop<3> >::value));
  assert((!xio::can_require<const object<1>, prop<2>, prop<3>, prop<4> >::value));
}
