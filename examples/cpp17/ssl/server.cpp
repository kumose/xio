//
// server.cpp
// ~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <cstdlib>
#include <functional>
#include <iostream>
#include <xio/xio.h>
#include <xio/ssl.h>

using xio::ip::tcp;

class session : public std::enable_shared_from_this<session>
{
public:
  session(xio::ssl::stream<tcp::socket> socket)
    : socket_(std::move(socket))
  {
  }

  void start()
  {
    do_handshake();
  }

private:
  void do_handshake()
  {
    auto self(shared_from_this());
    socket_.async_handshake(xio::ssl::stream_base::server,
        [this, self](const std::error_code& error)
        {
          if (!error)
          {
            do_read();
          }
        });
  }

  void do_read()
  {
    auto self(shared_from_this());
    socket_.async_read_some(xio::buffer(data_),
        [this, self](const std::error_code& ec, std::size_t length)
        {
          if (!ec)
          {
            do_write(length);
          }
        });
  }

  void do_write(std::size_t length)
  {
    auto self(shared_from_this());
    xio::async_write(socket_, xio::buffer(data_, length),
        [this, self](const std::error_code& ec,
          std::size_t /*length*/)
        {
          if (!ec)
          {
            do_read();
          }
        });
  }

  xio::ssl::stream<tcp::socket> socket_;
  char data_[1024];
};

class server
{
public:
  server(xio::io_context& io_context, unsigned short port)
    : acceptor_(io_context, tcp::endpoint(tcp::v4(), port)),
      context_(xio::ssl::context::sslv23)
  {
    context_.set_options(
        xio::ssl::context::default_workarounds
        | xio::ssl::context::no_sslv2
        | xio::ssl::context::single_dh_use);
    context_.set_password_callback(std::bind(&server::get_password, this));
    context_.use_certificate_chain_file("server.pem");
    context_.use_private_key_file("server.pem", xio::ssl::context::pem);
    context_.use_tmp_dh_file("dh4096.pem");

    do_accept();
  }

private:
  std::string get_password() const
  {
    return "test";
  }

  void do_accept()
  {
    acceptor_.async_accept(
        [this](const std::error_code& error, tcp::socket socket)
        {
          if (!error)
          {
            std::make_shared<session>(
                xio::ssl::stream<tcp::socket>(
                  std::move(socket), context_))->start();
          }

          do_accept();
        });
  }

  tcp::acceptor acceptor_;
  xio::ssl::context context_;
};

int main(int argc, char* argv[])
{
  try
  {
    if (argc != 2)
    {
      std::cerr << "Usage: server <port>\n";
      return 1;
    }

    xio::io_context io_context;

    using namespace std; // For atoi.
    server s(io_context, atoi(argv[1]));

    io_context.run();
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}
