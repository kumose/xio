//
// echo_server.cpp
// ~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <xio/bind_executor.h>
#include <xio/io_context.h>
#include <xio/ip/tcp.h>
#include <xio/steady_timer.h>
#include <xio/strand.h>
#include <xio/write.h>
#include <chrono>
#include <iostream>
#include <memory>

using xio::ip::tcp;

class session : public std::enable_shared_from_this<session>
{
public:
  session(xio::io_context& io_context, tcp::socket socket)
    : socket_(std::move(socket)),
      timer_(io_context),
      strand_(socket_.get_executor())
  {
  }

  void start()
  {
    reset_timeout();
    do_read();
  }

private:
  void reset_timeout()
  {
    timer_.expires_after(std::chrono::seconds(10));
    auto self = shared_from_this();
    timer_.async_wait(xio::bind_executor(strand_,
        [this, self](const std::error_code& ec)
        {
          if (!ec && timer_.expiry() <= xio::steady_timer::clock_type::now())
            socket_.close();
        }));
  }

  void do_read()
  {
    auto self = shared_from_this();
    socket_.async_read_some(xio::buffer(data_),
        xio::bind_executor(strand_,
            [this, self](const std::error_code& ec, std::size_t length)
            {
              if (!ec)
              {
                reset_timeout();
                do_write(length);
              }
            }));
  }

  void do_write(std::size_t length)
  {
    auto self = shared_from_this();
    xio::async_write(socket_, xio::buffer(data_, length),
        xio::bind_executor(strand_,
            [this, self](const std::error_code& ec, std::size_t /*length*/)
            {
              if (!ec)
              {
                reset_timeout();
                do_read();
              }
            }));
  }

  tcp::socket socket_;
  xio::steady_timer timer_;
  xio::strand<tcp::socket::executor_type> strand_;
  enum { max_length = 128 };
  char data_[max_length];
};

class server
{
public:
  server(xio::io_context& io_context, unsigned short port)
    : acceptor_(io_context, tcp::endpoint(tcp::v4(), port)),
      io_context_(io_context)
  {
    do_accept();
  }

private:
  void do_accept()
  {
    acceptor_.async_accept(
        [this](const std::error_code& ec, tcp::socket socket)
        {
          if (!ec)
            std::make_shared<session>(io_context_, std::move(socket))->start();

          do_accept();
        });
  }

  tcp::acceptor acceptor_;
  xio::io_context& io_context_;
};

int main(int argc, char* argv[])
{
  try
  {
    if (argc != 2)
    {
      std::cerr << "Usage: echo_server <port>\n";
      return 1;
    }

    xio::io_context io_context;
    server s(io_context, static_cast<unsigned short>(std::atoi(argv[1])));
    io_context.run();
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}
