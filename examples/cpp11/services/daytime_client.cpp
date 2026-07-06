//
// daytime_client.cpp
// ~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <asio.hpp>
#include <functional>
#include <iostream>
#include "logger.hpp"

using xio::ip::tcp;

char read_buffer[1024];

void read_handler(const xio::error_code& e,
    std::size_t bytes_transferred, tcp::socket* s)
{
  if (!e)
  {
    std::cout.write(read_buffer, bytes_transferred);

    s->async_read_some(xio::buffer(read_buffer),
        std::bind(read_handler, xio::placeholders::error,
          xio::placeholders::bytes_transferred, s));
  }
  else
  {
    xio::execution_context& context = xio::query(
        s->get_executor(), xio::execution::context);
    services::logger logger(context, "read_handler");

    std::string msg = "Read error: ";
    msg += e.message();
    logger.log(msg);
  }
}

void connect_handler(const xio::error_code& e, tcp::socket* s)
{
  xio::execution_context& context = xio::query(
      s->get_executor(), xio::execution::context);
  services::logger logger(context, "connect_handler");

  if (!e)
  {
    logger.log("Connection established");

    s->async_read_some(xio::buffer(read_buffer),
        std::bind(read_handler, xio::placeholders::error,
          xio::placeholders::bytes_transferred, s));
  }
  else
  {
    std::string msg = "Unable to establish connection: ";
    msg += e.message();
    logger.log(msg);
  }
}

int main(int argc, char* argv[])
{
  try
  {
    if (argc != 2)
    {
      std::cerr << "Usage: daytime_client <host>" << std::endl;
      return 1;
    }

    xio::io_context io_context;

    // Set the name of the file that all logger instances will use.
    services::logger logger(io_context, "");
    logger.use_file("log.txt");

    // Resolve the address corresponding to the given host.
    tcp::resolver resolver(io_context);
    tcp::resolver::results_type endpoints =
      resolver.resolve(argv[1], "daytime");

    // Start an asynchronous connect.
    tcp::socket socket(io_context);
    xio::async_connect(socket, endpoints,
        std::bind(connect_handler,
          xio::placeholders::error, &socket));

    // Run the io_context until all operations have finished.
    io_context.run();
  }
  catch (std::exception& e)
  {
    std::cerr << e.what() << std::endl;
  }

  return 0;
}
