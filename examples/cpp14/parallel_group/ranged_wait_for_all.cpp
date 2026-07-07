//
// ranged_wait_for_all.cpp
// ~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <xio/xio.h>
#include <xio/experimental/parallel_group.h>
#include <iostream>
#include <vector>

#ifdef ASIO_HAS_POSIX_STREAM_DESCRIPTOR

int main()
{
  xio::io_context ctx;

  xio::posix::stream_descriptor out(ctx, ::dup(STDOUT_FILENO));
  xio::posix::stream_descriptor err(ctx, ::dup(STDERR_FILENO));

  using op_type = decltype(
      out.async_write_some(xio::buffer("", 0))
    );

  std::vector<op_type> ops;

  ops.push_back(
      out.async_write_some(xio::buffer("first\r\n", 7))
    );

  ops.push_back(
      err.async_write_some(xio::buffer("second\r\n", 8))
    );

  xio::experimental::make_parallel_group(ops).async_wait(
      xio::experimental::wait_for_all(),
      [](
          std::vector<std::size_t> completion_order,
          std::vector<std::error_code> ec,
          std::vector<std::size_t> n
      )
      {
        for (std::size_t i = 0; i < completion_order.size(); ++i)
        {
          std::size_t idx = completion_order[i];
          std::cout << "operation " << idx << " finished: ";
          std::cout << ec[idx] << ", " << n[idx] << "\n";
        }
      }
    );

  ctx.run();
}

#else // defined(ASIO_HAS_POSIX_STREAM_DESCRIPTOR)
int main() {}
#endif // defined(ASIO_HAS_POSIX_STREAM_DESCRIPTOR)
