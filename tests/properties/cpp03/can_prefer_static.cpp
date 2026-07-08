//
// cpp03/can_prefer_static.cpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <xio/prefer.h>
#include <cassert>

template <int>
struct prop
{
  static const bool is_preferable = true;
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

namespace traits {

template<int N>
struct static_require<object<N>, prop<N> >
{
  static const bool is_valid = true;
};

} // namespace traits
} // namespace xio

int main()
{
  assert((xio::can_prefer<object<1>, prop<1> >::value));
  assert((xio::can_prefer<object<1>, prop<1>, prop<1> >::value));
  assert((xio::can_prefer<object<1>, prop<1>, prop<1>, prop<1> >::value));
  assert((xio::can_prefer<const object<1>, prop<1> >::value));
  assert((xio::can_prefer<const object<1>, prop<1>, prop<1> >::value));
  assert((xio::can_prefer<const object<1>, prop<1>, prop<1>, prop<1> >::value));
}
