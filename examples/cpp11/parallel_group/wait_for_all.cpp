//
// wait_for_all.cpp
// ~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <xio/xio.h>
#include <xio/experimental/parallel_group.h>
#include <iostream>

#ifdef ASIO_HAS_POSIX_STREAM_DESCRIPTOR

int main()
{
  xio::io_context ctx;

  xio::posix::stream_descriptor in(ctx, ::dup(STDIN_FILENO));
  xio::steady_timer timer(ctx, std::chrono::seconds(5));

  char data[1024];

  xio::experimental::make_parallel_group(
      in.async_read_some(xio::buffer(data)),
      timer.async_wait()
    ).async_wait(
      xio::experimental::wait_for_all(),
      [](
          std::array<std::size_t, 2> completion_order,
          std::error_code ec1, std::size_t n1,
          std::error_code ec2
      )
      {
        switch (completion_order[0])
        {
        case 0:
          {
            std::cout << "descriptor finished: " << ec1 << ", " << n1 << "\n";
          }
          break;
        case 1:
          {
            std::cout << "timer finished: " << ec2 << "\n";
          }
          break;
        }
      }
    );

  ctx.run();
}

#else // defined(ASIO_HAS_POSIX_STREAM_DESCRIPTOR)
int main() {}
#endif // defined(ASIO_HAS_POSIX_STREAM_DESCRIPTOR)
