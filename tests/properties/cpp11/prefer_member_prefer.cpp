//
// cpp11/prefer_member_prefer.cpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
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
  static constexpr bool is_preferable = true;
};

template <int>
struct object
{
  template <int N>
  constexpr object<N> prefer(prop<N>) const
  {
    return object<N>();
  }
};

namespace xio {

template<int N, int M>
struct is_applicable_property<object<N>, prop<M> >
{
  static constexpr bool value = true;
};

} // namespace xio

int main()
{
  object<1> o1 = {};
  object<2> o2 = xio::prefer(o1, prop<2>());
  object<3> o3 = xio::prefer(o1, prop<2>(), prop<3>());
  object<4> o4 = xio::prefer(o1, prop<2>(), prop<3>(), prop<4>());
  (void)o2;
  (void)o3;
  (void)o4;

  const object<1> o5 = {};
  object<2> o6 = xio::prefer(o5, prop<2>());
  object<3> o7 = xio::prefer(o5, prop<2>(), prop<3>());
  object<4> o8 = xio::prefer(o5, prop<2>(), prop<3>(), prop<4>());
  (void)o6;
  (void)o7;
  (void)o8;

  constexpr object<2> o9 = xio::prefer(object<1>(), prop<2>());
  constexpr object<3> o10 = xio::prefer(object<1>(), prop<2>(), prop<3>());
  constexpr object<4> o11 = xio::prefer(object<1>(), prop<2>(), prop<3>(), prop<4>());
  (void)o9;
  (void)o10;
  (void)o11;
}
