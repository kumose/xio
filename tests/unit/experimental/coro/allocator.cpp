//
// experimental/coro/partial.cpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2021-2023 Klemens D. Morgenstern
//                         (klemens dot morgenstern at gmx dot net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//



// Test that header file is self-contained.
#include <xio/experimental/coro.h>

#include <vector>
#include <xio/io_context.h>
#include "../../unit_test.hpp"

namespace exp =  xio::experimental;

namespace coro {

template<typename Value = void>
struct tracked_allocator
{
  using value_type = Value;
  std::vector<std::pair<void*, std::size_t>> &allocs, &deallocs;

  tracked_allocator(std::vector<std::pair<void*, std::size_t>>& allocs,
                    std::vector<std::pair<void*, std::size_t>>& deallocs) : allocs(allocs), deallocs(deallocs) {}

  template<typename T>
  tracked_allocator(const tracked_allocator<T>& a) : allocs(a.allocs), deallocs(a.deallocs) {}

  value_type* allocate(std::size_t n)
  {
    auto p = new char[n * sizeof(Value)];
    allocs.emplace_back(p, n);
    return reinterpret_cast<value_type*>(p);
  }

  void deallocate(void* p, std::size_t n)
  {
    deallocs.emplace_back(p, n);
    delete[] static_cast<char*>(p);
//    XIO_CHECK(allocs.back() == deallocs.back());
  }

  bool operator==(const tracked_allocator& rhs) const
  {
    return &allocs == &rhs.allocs
           && &deallocs == &rhs.deallocs;
  }
};

exp::coro<void, void, xio::any_io_executor, tracked_allocator<void>>
        alloc_test_impl(xio::io_context& ctx, int, std::allocator_arg_t, tracked_allocator<void> ta, double)
{
  co_return ;
}

void alloc_test()
{
  std::vector<std::pair<void*, std::size_t>> allocs, deallocs;
  xio::io_context ctx;
  bool ran = false;

  {
    auto pp = alloc_test_impl(ctx, 42, std::allocator_arg, {allocs, deallocs}, 42.);

    XIO_CHECK(allocs.size()  == 1u);
    XIO_CHECK(deallocs.empty());

    pp.async_resume([&](auto e){ran = true; XIO_CHECK(!e);});
    ctx.run();
    XIO_CHECK(deallocs.size() == 0u);
  }
  ctx.restart();
  ctx.run();
  XIO_CHECK(deallocs.size() == 1u);
  XIO_CHECK(allocs == deallocs);

  XIO_CHECK(ran);

  ran = false;

  auto p = xio::experimental::detail::post_coroutine(
          ctx,
          xio::bind_allocator(tracked_allocator{allocs, deallocs}, [&]{ran = true;})).handle;
  XIO_CHECK(allocs.size()  == 2u);
  XIO_CHECK(deallocs.size() == 1u);
  p.resume();
  XIO_CHECK(allocs.size()  == 3u);
  XIO_CHECK(deallocs.size() == 2u);
  ctx.restart();
  ctx.run();

  XIO_CHECK(allocs == deallocs);
}

} // namespace coro

XIO_TEST_SUITE
(
  "coro/allocate",
  XIO_TEST_CASE(::coro::alloc_test)
)
