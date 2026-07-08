//
// echo_server.cpp
// ~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <xio/co_spawn.h>
#include <xio/detached.h>
#include <xio/io_context.h>
#include <xio/ip/tcp.h>
#include <xio/signal_set.h>
#include <xio/write.h>
#include <cstdio>

using xio::ip::tcp;
using xio::awaitable;
using xio::co_spawn;
using xio::detached;
using xio::use_awaitable;
namespace this_coro = xio::this_coro;

#if defined(XIO_ENABLE_HANDLER_TRACKING)
# define use_awaitable \
  xio::use_awaitable_t(__FILE__, __LINE__, __PRETTY_FUNCTION__)
#endif

awaitable<void> echo(tcp::socket socket)
{
  try
  {
    char data[1024];
    for (;;)
    {
      std::size_t n = co_await socket.async_read_some(xio::buffer(data), use_awaitable);
      co_await async_write(socket, xio::buffer(data, n), use_awaitable);
    }
  }
  catch (std::exception& e)
  {
    std::printf("echo Exception: %s\n", e.what());
  }
}

awaitable<void> listener()
{
  auto executor = co_await this_coro::executor;
  tcp::acceptor acceptor(executor, {tcp::v4(), 55555});
  for (;;)
  {
    tcp::socket socket = co_await acceptor.async_accept(use_awaitable);
    co_spawn(executor, echo(std::move(socket)), detached);
  }
}

int main()
{
  try
  {
    xio::io_context io_context(1);

    xio::signal_set signals(io_context, SIGINT, SIGTERM);
    signals.async_wait([&](auto, auto){ io_context.stop(); });

    co_spawn(io_context, listener(), detached);

    io_context.run();
  }
  catch (std::exception& e)
  {
    std::printf("Exception: %s\n", e.what());
  }
}
