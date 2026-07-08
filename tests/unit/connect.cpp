//
// connect.cpp
// ~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//



// Test that header file is self-contained.
#include <xio/connect.h>

#include <functional>
#include <vector>
#include <xio/detail/thread.h>
#include <xio/ip/tcp.h>
#include "unit_test.hpp"

namespace bindns = std;
using bindns::placeholders::_1;
using bindns::placeholders::_2;

class connection_sink
{
public:
  connection_sink()
    : acceptor_(io_context_,
        xio::ip::tcp::endpoint(
          xio::ip::address_v4::loopback(), 0)),
      target_endpoint_(acceptor_.local_endpoint()),
      socket_(io_context_),
      thread_(bindns::bind(&connection_sink::run, this))
  {
  }

  ~connection_sink()
  {
    io_context_.stop();
    thread_.join();
  }

  xio::ip::tcp::endpoint target_endpoint()
  {
    return target_endpoint_;
  }

private:
  void run()
  {
    io_context_.run();
  }

  void handle_accept()
  {
    socket_.close();
    acceptor_.async_accept(socket_,
        bindns::bind(&connection_sink::handle_accept, this));
  }

  xio::io_context io_context_;
  xio::ip::tcp::acceptor acceptor_;
  xio::ip::tcp::endpoint target_endpoint_;
  xio::ip::tcp::socket socket_;
  xio::detail::thread thread_;
};

bool true_cond_1(const xio::error_code& /*ec*/,
    const xio::ip::tcp::endpoint& /*endpoint*/)
{
  return true;
}

struct true_cond_2
{
  template <typename Endpoint>
  bool operator()(const xio::error_code& /*ec*/,
      const Endpoint& /*endpoint*/)
  {
    return true;
  }
};

std::vector<xio::ip::tcp::endpoint>::const_iterator legacy_true_cond_1(
    const xio::error_code& /*ec*/,
    std::vector<xio::ip::tcp::endpoint>::const_iterator next)
{
  return next;
}

struct legacy_true_cond_2
{
  template <typename Iterator>
  Iterator operator()(const xio::error_code& /*ec*/, Iterator next)
  {
    return next;
  }
};

bool false_cond(const xio::error_code& /*ec*/,
    const xio::ip::tcp::endpoint& /*endpoint*/)
{
  return false;
}

void range_handler(const xio::error_code& ec,
    const xio::ip::tcp::endpoint& endpoint,
    xio::error_code* out_ec,
    xio::ip::tcp::endpoint* out_endpoint)
{
  *out_ec = ec;
  *out_endpoint = endpoint;
}

void iter_handler(const xio::error_code& ec,
    std::vector<xio::ip::tcp::endpoint>::const_iterator iter,
    xio::error_code* out_ec,
    std::vector<xio::ip::tcp::endpoint>::const_iterator* out_iter)
{
  *out_ec = ec;
  *out_iter = iter;
}

void test_connect_range()
{
  connection_sink sink;
  xio::io_context io_context;
  xio::ip::tcp::socket socket(io_context);
  std::vector<xio::ip::tcp::endpoint> endpoints;
  xio::ip::tcp::endpoint result;

  try
  {
    result = xio::connect(socket, endpoints);
    XIO_CHECK(false);
  }
  catch (std::system_error& e)
  {
    XIO_CHECK(e.code() == xio::error::not_found);
  }

  endpoints.push_back(sink.target_endpoint());

  result = xio::connect(socket, endpoints);
  XIO_CHECK(result == endpoints[0]);

  endpoints.push_back(sink.target_endpoint());

  result = xio::connect(socket, endpoints);
  XIO_CHECK(result == endpoints[0]);

  endpoints.insert(endpoints.begin(), xio::ip::tcp::endpoint());

  result = xio::connect(socket, endpoints);
  XIO_CHECK(result == endpoints[1]);
}

void test_connect_range_ec()
{
  connection_sink sink;
  xio::io_context io_context;
  xio::ip::tcp::socket socket(io_context);
  std::vector<xio::ip::tcp::endpoint> endpoints;
  xio::ip::tcp::endpoint result;
  xio::error_code ec;

  result = xio::connect(socket, endpoints, ec);
  XIO_CHECK(result == xio::ip::tcp::endpoint());
  XIO_CHECK(ec == xio::error::not_found);

  endpoints.push_back(sink.target_endpoint());

  result = xio::connect(socket, endpoints, ec);
  XIO_CHECK(result == endpoints[0]);
  XIO_CHECK(!ec);

  endpoints.push_back(sink.target_endpoint());

  result = xio::connect(socket, endpoints, ec);
  XIO_CHECK(result == endpoints[0]);
  XIO_CHECK(!ec);

  endpoints.insert(endpoints.begin(), xio::ip::tcp::endpoint());

  result = xio::connect(socket, endpoints, ec);
  XIO_CHECK(result == endpoints[1]);
  XIO_CHECK(!ec);
}

void test_connect_range_cond()
{
  connection_sink sink;
  xio::io_context io_context;
  xio::ip::tcp::socket socket(io_context);
  std::vector<xio::ip::tcp::endpoint> endpoints;
  xio::ip::tcp::endpoint result;

  try
  {
    result = xio::connect(socket, endpoints, true_cond_1);
    XIO_CHECK(false);
  }
  catch (std::system_error& e)
  {
    XIO_CHECK(e.code() == xio::error::not_found);
  }

  try
  {
    result = xio::connect(socket, endpoints, true_cond_2());
    XIO_CHECK(false);
  }
  catch (std::system_error& e)
  {
    XIO_CHECK(e.code() == xio::error::not_found);
  }

  try
  {
    result = xio::connect(socket, endpoints, legacy_true_cond_1);
    XIO_CHECK(false);
  }
  catch (std::system_error& e)
  {
    XIO_CHECK(e.code() == xio::error::not_found);
  }

  try
  {
    result = xio::connect(socket, endpoints, legacy_true_cond_2());
    XIO_CHECK(false);
  }
  catch (std::system_error& e)
  {
    XIO_CHECK(e.code() == xio::error::not_found);
  }

  try
  {
    result = xio::connect(socket, endpoints, false_cond);
    XIO_CHECK(false);
  }
  catch (std::system_error& e)
  {
    XIO_CHECK(e.code() == xio::error::not_found);
  }

  endpoints.push_back(sink.target_endpoint());

  result = xio::connect(socket, endpoints, true_cond_1);
  XIO_CHECK(result == endpoints[0]);

  result = xio::connect(socket, endpoints, true_cond_2());
  XIO_CHECK(result == endpoints[0]);

  result = xio::connect(socket, endpoints, legacy_true_cond_1);
  XIO_CHECK(result == endpoints[0]);

  result = xio::connect(socket, endpoints, legacy_true_cond_2());
  XIO_CHECK(result == endpoints[0]);

  try
  {
    result = xio::connect(socket, endpoints, false_cond);
    XIO_CHECK(false);
  }
  catch (std::system_error& e)
  {
    XIO_CHECK(e.code() == xio::error::not_found);
  }

  endpoints.push_back(sink.target_endpoint());

  result = xio::connect(socket, endpoints, true_cond_1);
  XIO_CHECK(result == endpoints[0]);

  result = xio::connect(socket, endpoints, true_cond_2());
  XIO_CHECK(result == endpoints[0]);

  result = xio::connect(socket, endpoints, legacy_true_cond_1);
  XIO_CHECK(result == endpoints[0]);

  result = xio::connect(socket, endpoints, legacy_true_cond_2());
  XIO_CHECK(result == endpoints[0]);

  try
  {
    result = xio::connect(socket, endpoints, false_cond);
    XIO_CHECK(false);
  }
  catch (std::system_error& e)
  {
    XIO_CHECK(e.code() == xio::error::not_found);
  }

  endpoints.insert(endpoints.begin(), xio::ip::tcp::endpoint());

  result = xio::connect(socket, endpoints, true_cond_1);
  XIO_CHECK(result == endpoints[1]);

  result = xio::connect(socket, endpoints, true_cond_2());
  XIO_CHECK(result == endpoints[1]);

  result = xio::connect(socket, endpoints, legacy_true_cond_1);
  XIO_CHECK(result == endpoints[1]);

  result = xio::connect(socket, endpoints, legacy_true_cond_2());
  XIO_CHECK(result == endpoints[1]);

  try
  {
    result = xio::connect(socket, endpoints, false_cond);
    XIO_CHECK(false);
  }
  catch (std::system_error& e)
  {
    XIO_CHECK(e.code() == xio::error::not_found);
  }
}

void test_connect_range_cond_ec()
{
  connection_sink sink;
  xio::io_context io_context;
  xio::ip::tcp::socket socket(io_context);
  std::vector<xio::ip::tcp::endpoint> endpoints;
  xio::ip::tcp::endpoint result;
  xio::error_code ec;

  result = xio::connect(socket, endpoints, true_cond_1, ec);
  XIO_CHECK(result == xio::ip::tcp::endpoint());
  XIO_CHECK(ec == xio::error::not_found);

  result = xio::connect(socket, endpoints, true_cond_2(), ec);
  XIO_CHECK(result == xio::ip::tcp::endpoint());
  XIO_CHECK(ec == xio::error::not_found);

  result = xio::connect(socket, endpoints, legacy_true_cond_1, ec);
  XIO_CHECK(result == xio::ip::tcp::endpoint());
  XIO_CHECK(ec == xio::error::not_found);

  result = xio::connect(socket, endpoints, legacy_true_cond_2(), ec);
  XIO_CHECK(result == xio::ip::tcp::endpoint());
  XIO_CHECK(ec == xio::error::not_found);

  result = xio::connect(socket, endpoints, false_cond, ec);
  XIO_CHECK(result == xio::ip::tcp::endpoint());
  XIO_CHECK(ec == xio::error::not_found);

  endpoints.push_back(sink.target_endpoint());

  result = xio::connect(socket, endpoints, true_cond_1, ec);
  XIO_CHECK(result == endpoints[0]);
  XIO_CHECK(!ec);

  result = xio::connect(socket, endpoints, true_cond_2(), ec);
  XIO_CHECK(result == endpoints[0]);
  XIO_CHECK(!ec);

  result = xio::connect(socket, endpoints, legacy_true_cond_1, ec);
  XIO_CHECK(result == endpoints[0]);
  XIO_CHECK(!ec);

  result = xio::connect(socket, endpoints, legacy_true_cond_2(), ec);
  XIO_CHECK(result == endpoints[0]);
  XIO_CHECK(!ec);

  result = xio::connect(socket, endpoints, false_cond, ec);
  XIO_CHECK(result == xio::ip::tcp::endpoint());
  XIO_CHECK(ec == xio::error::not_found);

  endpoints.push_back(sink.target_endpoint());

  result = xio::connect(socket, endpoints, true_cond_1, ec);
  XIO_CHECK(result == endpoints[0]);
  XIO_CHECK(!ec);

  result = xio::connect(socket, endpoints, true_cond_2(), ec);
  XIO_CHECK(result == endpoints[0]);
  XIO_CHECK(!ec);

  result = xio::connect(socket, endpoints, legacy_true_cond_1, ec);
  XIO_CHECK(result == endpoints[0]);
  XIO_CHECK(!ec);

  result = xio::connect(socket, endpoints, legacy_true_cond_2(), ec);
  XIO_CHECK(result == endpoints[0]);
  XIO_CHECK(!ec);

  result = xio::connect(socket, endpoints, false_cond, ec);
  XIO_CHECK(result == xio::ip::tcp::endpoint());
  XIO_CHECK(ec == xio::error::not_found);

  endpoints.insert(endpoints.begin(), xio::ip::tcp::endpoint());

  result = xio::connect(socket, endpoints, true_cond_1, ec);
  XIO_CHECK(result == endpoints[1]);
  XIO_CHECK(!ec);

  result = xio::connect(socket, endpoints, true_cond_2(), ec);
  XIO_CHECK(result == endpoints[1]);
  XIO_CHECK(!ec);

  result = xio::connect(socket, endpoints, legacy_true_cond_1, ec);
  XIO_CHECK(result == endpoints[1]);
  XIO_CHECK(!ec);

  result = xio::connect(socket, endpoints, legacy_true_cond_2(), ec);
  XIO_CHECK(result == endpoints[1]);
  XIO_CHECK(!ec);

  result = xio::connect(socket, endpoints, false_cond, ec);
  XIO_CHECK(result == xio::ip::tcp::endpoint());
  XIO_CHECK(ec == xio::error::not_found);
}

void test_connect_iter()
{
  connection_sink sink;
  xio::io_context io_context;
  xio::ip::tcp::socket socket(io_context);
  std::vector<xio::ip::tcp::endpoint> endpoints;
  const std::vector<xio::ip::tcp::endpoint>& cendpoints = endpoints;
  std::vector<xio::ip::tcp::endpoint>::const_iterator result;

  try
  {
    result = xio::connect(socket, cendpoints.begin(), cendpoints.end());
    XIO_CHECK(false);
  }
  catch (std::system_error& e)
  {
    XIO_CHECK(e.code() == xio::error::not_found);
  }

  endpoints.push_back(sink.target_endpoint());

  result = xio::connect(socket, cendpoints.begin(), cendpoints.end());
  XIO_CHECK(result == cendpoints.begin());

  endpoints.push_back(sink.target_endpoint());

  result = xio::connect(socket, cendpoints.begin(), cendpoints.end());
  XIO_CHECK(result == cendpoints.begin());

  endpoints.insert(endpoints.begin(), xio::ip::tcp::endpoint());

  result = xio::connect(socket, cendpoints.begin(), cendpoints.end());
  XIO_CHECK(result == cendpoints.begin() + 1);
}

void test_connect_iter_ec()
{
  connection_sink sink;
  xio::io_context io_context;
  xio::ip::tcp::socket socket(io_context);
  std::vector<xio::ip::tcp::endpoint> endpoints;
  const std::vector<xio::ip::tcp::endpoint>& cendpoints = endpoints;
  std::vector<xio::ip::tcp::endpoint>::const_iterator result;
  xio::error_code ec;

  result = xio::connect(socket,
      cendpoints.begin(), cendpoints.end(), ec);
  XIO_CHECK(result == cendpoints.end());
  XIO_CHECK(ec == xio::error::not_found);

  endpoints.push_back(sink.target_endpoint());

  result = xio::connect(socket,
      cendpoints.begin(), cendpoints.end(), ec);
  XIO_CHECK(result == cendpoints.begin());
  XIO_CHECK(!ec);

  endpoints.push_back(sink.target_endpoint());

  result = xio::connect(socket,
      cendpoints.begin(), cendpoints.end(), ec);
  XIO_CHECK(result == cendpoints.begin());
  XIO_CHECK(!ec);

  endpoints.insert(endpoints.begin(), xio::ip::tcp::endpoint());

  result = xio::connect(socket,
      cendpoints.begin(), cendpoints.end(), ec);
  XIO_CHECK(result == cendpoints.begin() + 1);
  XIO_CHECK(!ec);
}

void test_connect_iter_cond()
{
  connection_sink sink;
  xio::io_context io_context;
  xio::ip::tcp::socket socket(io_context);
  std::vector<xio::ip::tcp::endpoint> endpoints;
  const std::vector<xio::ip::tcp::endpoint>& cendpoints = endpoints;
  std::vector<xio::ip::tcp::endpoint>::const_iterator result;

  try
  {
    result = xio::connect(socket, cendpoints.begin(),
        cendpoints.end(), true_cond_1);
    XIO_CHECK(false);
  }
  catch (std::system_error& e)
  {
    XIO_CHECK(e.code() == xio::error::not_found);
  }

  try
  {
    result = xio::connect(socket, cendpoints.begin(),
        cendpoints.end(), true_cond_2());
    XIO_CHECK(false);
  }
  catch (std::system_error& e)
  {
    XIO_CHECK(e.code() == xio::error::not_found);
  }

  try
  {
    result = xio::connect(socket, cendpoints.begin(),
        cendpoints.end(), legacy_true_cond_1);
    XIO_CHECK(false);
  }
  catch (std::system_error& e)
  {
    XIO_CHECK(e.code() == xio::error::not_found);
  }

  try
  {
    result = xio::connect(socket, cendpoints.begin(),
        cendpoints.end(), legacy_true_cond_2());
    XIO_CHECK(false);
  }
  catch (std::system_error& e)
  {
    XIO_CHECK(e.code() == xio::error::not_found);
  }

  try
  {
    result = xio::connect(socket, cendpoints.begin(),
        cendpoints.end(), false_cond);
    XIO_CHECK(false);
  }
  catch (std::system_error& e)
  {
    XIO_CHECK(e.code() == xio::error::not_found);
  }

  endpoints.push_back(sink.target_endpoint());

  result = xio::connect(socket, cendpoints.begin(),
      cendpoints.end(), true_cond_1);
  XIO_CHECK(result == cendpoints.begin());

  result = xio::connect(socket, cendpoints.begin(),
      cendpoints.end(), true_cond_2());
  XIO_CHECK(result == cendpoints.begin());

  result = xio::connect(socket, cendpoints.begin(),
      cendpoints.end(), legacy_true_cond_1);
  XIO_CHECK(result == cendpoints.begin());

  result = xio::connect(socket, cendpoints.begin(),
      cendpoints.end(), legacy_true_cond_2());
  XIO_CHECK(result == cendpoints.begin());

  try
  {
    result = xio::connect(socket, cendpoints.begin(),
        cendpoints.end(), false_cond);
    XIO_CHECK(false);
  }
  catch (std::system_error& e)
  {
    XIO_CHECK(e.code() == xio::error::not_found);
  }

  endpoints.push_back(sink.target_endpoint());

  result = xio::connect(socket, cendpoints.begin(),
      cendpoints.end(), true_cond_1);
  XIO_CHECK(result == cendpoints.begin());

  result = xio::connect(socket, cendpoints.begin(),
      cendpoints.end(), true_cond_2());
  XIO_CHECK(result == cendpoints.begin());

  result = xio::connect(socket, cendpoints.begin(),
      cendpoints.end(), legacy_true_cond_1);
  XIO_CHECK(result == cendpoints.begin());

  result = xio::connect(socket, cendpoints.begin(),
      cendpoints.end(), legacy_true_cond_2());
  XIO_CHECK(result == cendpoints.begin());

  try
  {
    result = xio::connect(socket, cendpoints.begin(),
        cendpoints.end(), false_cond);
    XIO_CHECK(false);
  }
  catch (std::system_error& e)
  {
    XIO_CHECK(e.code() == xio::error::not_found);
  }

  endpoints.insert(endpoints.begin(), xio::ip::tcp::endpoint());

  result = xio::connect(socket, cendpoints.begin(),
      cendpoints.end(), true_cond_1);
  XIO_CHECK(result == cendpoints.begin() + 1);

  result = xio::connect(socket, cendpoints.begin(),
      cendpoints.end(), true_cond_2());
  XIO_CHECK(result == cendpoints.begin() + 1);

  result = xio::connect(socket, cendpoints.begin(),
      cendpoints.end(), legacy_true_cond_1);
  XIO_CHECK(result == cendpoints.begin() + 1);

  result = xio::connect(socket, cendpoints.begin(),
      cendpoints.end(), legacy_true_cond_2());
  XIO_CHECK(result == cendpoints.begin() + 1);

  try
  {
    result = xio::connect(socket, cendpoints.begin(),
        cendpoints.end(), false_cond);
    XIO_CHECK(false);
  }
  catch (std::system_error& e)
  {
    XIO_CHECK(e.code() == xio::error::not_found);
  }
}

void test_connect_iter_cond_ec()
{
  connection_sink sink;
  xio::io_context io_context;
  xio::ip::tcp::socket socket(io_context);
  std::vector<xio::ip::tcp::endpoint> endpoints;
  const std::vector<xio::ip::tcp::endpoint>& cendpoints = endpoints;
  std::vector<xio::ip::tcp::endpoint>::const_iterator result;
  xio::error_code ec;

  result = xio::connect(socket, cendpoints.begin(),
      cendpoints.end(), true_cond_1, ec);
  XIO_CHECK(result == cendpoints.end());
  XIO_CHECK(ec == xio::error::not_found);

  result = xio::connect(socket, cendpoints.begin(),
      cendpoints.end(), true_cond_2(), ec);
  XIO_CHECK(result == cendpoints.end());
  XIO_CHECK(ec == xio::error::not_found);

  result = xio::connect(socket, cendpoints.begin(),
      cendpoints.end(), legacy_true_cond_1, ec);
  XIO_CHECK(result == cendpoints.end());
  XIO_CHECK(ec == xio::error::not_found);

  result = xio::connect(socket, cendpoints.begin(),
      cendpoints.end(), legacy_true_cond_2(), ec);
  XIO_CHECK(result == cendpoints.end());
  XIO_CHECK(ec == xio::error::not_found);

  result = xio::connect(socket, cendpoints.begin(),
      cendpoints.end(), false_cond, ec);
  XIO_CHECK(result == cendpoints.end());
  XIO_CHECK(ec == xio::error::not_found);

  endpoints.push_back(sink.target_endpoint());

  result = xio::connect(socket, cendpoints.begin(),
      cendpoints.end(), true_cond_1, ec);
  XIO_CHECK(result == cendpoints.begin());
  XIO_CHECK(!ec);

  result = xio::connect(socket, cendpoints.begin(),
      cendpoints.end(), true_cond_2(), ec);
  XIO_CHECK(result == cendpoints.begin());
  XIO_CHECK(!ec);

  result = xio::connect(socket, cendpoints.begin(),
      cendpoints.end(), legacy_true_cond_1, ec);
  XIO_CHECK(result == cendpoints.begin());
  XIO_CHECK(!ec);

  result = xio::connect(socket, cendpoints.begin(),
      cendpoints.end(), legacy_true_cond_2(), ec);
  XIO_CHECK(result == cendpoints.begin());
  XIO_CHECK(!ec);

  result = xio::connect(socket, cendpoints.begin(),
      cendpoints.end(), false_cond, ec);
  XIO_CHECK(result == cendpoints.end());
  XIO_CHECK(ec == xio::error::not_found);

  endpoints.push_back(sink.target_endpoint());

  result = xio::connect(socket, cendpoints.begin(),
      cendpoints.end(), true_cond_1, ec);
  XIO_CHECK(result == cendpoints.begin());
  XIO_CHECK(!ec);

  result = xio::connect(socket, cendpoints.begin(),
      cendpoints.end(), true_cond_2(), ec);
  XIO_CHECK(result == cendpoints.begin());
  XIO_CHECK(!ec);

  result = xio::connect(socket, cendpoints.begin(),
      cendpoints.end(), legacy_true_cond_1, ec);
  XIO_CHECK(result == cendpoints.begin());
  XIO_CHECK(!ec);

  result = xio::connect(socket, cendpoints.begin(),
      cendpoints.end(), legacy_true_cond_2(), ec);
  XIO_CHECK(result == cendpoints.begin());
  XIO_CHECK(!ec);

  result = xio::connect(socket, cendpoints.begin(),
      cendpoints.end(), false_cond, ec);
  XIO_CHECK(result == cendpoints.end());
  XIO_CHECK(ec == xio::error::not_found);

  endpoints.insert(endpoints.begin(), xio::ip::tcp::endpoint());

  result = xio::connect(socket, cendpoints.begin(),
      cendpoints.end(), true_cond_1, ec);
  XIO_CHECK(result == cendpoints.begin() + 1);
  XIO_CHECK(!ec);

  result = xio::connect(socket, cendpoints.begin(),
      cendpoints.end(), true_cond_2(), ec);
  XIO_CHECK(result == cendpoints.begin() + 1);
  XIO_CHECK(!ec);

  result = xio::connect(socket, cendpoints.begin(),
      cendpoints.end(), legacy_true_cond_1, ec);
  XIO_CHECK(result == cendpoints.begin() + 1);
  XIO_CHECK(!ec);

  result = xio::connect(socket, cendpoints.begin(),
      cendpoints.end(), legacy_true_cond_2(), ec);
  XIO_CHECK(result == cendpoints.begin() + 1);
  XIO_CHECK(!ec);

  result = xio::connect(socket, cendpoints.begin(),
      cendpoints.end(), false_cond, ec);
  XIO_CHECK(result == cendpoints.end());
  XIO_CHECK(ec == xio::error::not_found);
}

void test_async_connect_range()
{
  connection_sink sink;
  xio::io_context io_context;
  xio::ip::tcp::socket socket(io_context);
  std::vector<xio::ip::tcp::endpoint> endpoints;
  xio::ip::tcp::endpoint result;
  xio::error_code ec;

  xio::async_connect(socket, endpoints,
      bindns::bind(range_handler, _1, _2, &ec, &result));
  io_context.restart();
  io_context.run();
  XIO_CHECK(result == xio::ip::tcp::endpoint());
  XIO_CHECK(ec == xio::error::not_found);

  endpoints.push_back(sink.target_endpoint());

  xio::async_connect(socket, endpoints,
      bindns::bind(range_handler, _1, _2, &ec, &result));
  io_context.restart();
  io_context.run();
  XIO_CHECK(result == endpoints[0]);
  XIO_CHECK(!ec);

  endpoints.push_back(sink.target_endpoint());

  xio::async_connect(socket, endpoints,
      bindns::bind(range_handler, _1, _2, &ec, &result));
  io_context.restart();
  io_context.run();
  XIO_CHECK(result == endpoints[0]);
  XIO_CHECK(!ec);

  endpoints.insert(endpoints.begin(), xio::ip::tcp::endpoint());

  xio::async_connect(socket, endpoints,
      bindns::bind(range_handler, _1, _2, &ec, &result));
  io_context.restart();
  io_context.run();
  XIO_CHECK(result == endpoints[1]);
  XIO_CHECK(!ec);

  xio::async_connect(socket, endpoints)(
      bindns::bind(range_handler, _1, _2, &ec, &result));
  io_context.restart();
  io_context.run();
  XIO_CHECK(result == endpoints[1]);
  XIO_CHECK(!ec);
}

void test_async_connect_range_cond()
{
  connection_sink sink;
  xio::io_context io_context;
  xio::ip::tcp::socket socket(io_context);
  std::vector<xio::ip::tcp::endpoint> endpoints;
  xio::ip::tcp::endpoint result;
  xio::error_code ec;

  xio::async_connect(socket, endpoints, true_cond_1,
      bindns::bind(range_handler, _1, _2, &ec, &result));
  io_context.restart();
  io_context.run();
  XIO_CHECK(result == xio::ip::tcp::endpoint());
  XIO_CHECK(ec == xio::error::not_found);

  xio::async_connect(socket, endpoints, true_cond_2(),
      bindns::bind(range_handler, _1, _2, &ec, &result));
  io_context.restart();
  io_context.run();
  XIO_CHECK(result == xio::ip::tcp::endpoint());
  XIO_CHECK(ec == xio::error::not_found);

  xio::async_connect(socket, endpoints, legacy_true_cond_1,
      bindns::bind(range_handler, _1, _2, &ec, &result));
  io_context.restart();
  io_context.run();
  XIO_CHECK(result == xio::ip::tcp::endpoint());
  XIO_CHECK(ec == xio::error::not_found);

  xio::async_connect(socket, endpoints, legacy_true_cond_2(),
      bindns::bind(range_handler, _1, _2, &ec, &result));
  io_context.restart();
  io_context.run();
  XIO_CHECK(result == xio::ip::tcp::endpoint());
  XIO_CHECK(ec == xio::error::not_found);

  xio::async_connect(socket, endpoints, false_cond,
      bindns::bind(range_handler, _1, _2, &ec, &result));
  io_context.restart();
  io_context.run();
  XIO_CHECK(result == xio::ip::tcp::endpoint());
  XIO_CHECK(ec == xio::error::not_found);

  endpoints.push_back(sink.target_endpoint());

  xio::async_connect(socket, endpoints, true_cond_1,
      bindns::bind(range_handler, _1, _2, &ec, &result));
  io_context.restart();
  io_context.run();
  XIO_CHECK(result == endpoints[0]);
  XIO_CHECK(!ec);

  xio::async_connect(socket, endpoints, true_cond_2(),
      bindns::bind(range_handler, _1, _2, &ec, &result));
  io_context.restart();
  io_context.run();
  XIO_CHECK(result == endpoints[0]);
  XIO_CHECK(!ec);

  xio::async_connect(socket, endpoints, legacy_true_cond_1,
      bindns::bind(range_handler, _1, _2, &ec, &result));
  io_context.restart();
  io_context.run();
  XIO_CHECK(result == endpoints[0]);
  XIO_CHECK(!ec);

  xio::async_connect(socket, endpoints, legacy_true_cond_2(),
      bindns::bind(range_handler, _1, _2, &ec, &result));
  io_context.restart();
  io_context.run();
  XIO_CHECK(result == endpoints[0]);
  XIO_CHECK(!ec);

  xio::async_connect(socket, endpoints, false_cond,
      bindns::bind(range_handler, _1, _2, &ec, &result));
  io_context.restart();
  io_context.run();
  XIO_CHECK(result == xio::ip::tcp::endpoint());
  XIO_CHECK(ec == xio::error::not_found);

  endpoints.push_back(sink.target_endpoint());

  xio::async_connect(socket, endpoints, true_cond_1,
      bindns::bind(range_handler, _1, _2, &ec, &result));
  io_context.restart();
  io_context.run();
  XIO_CHECK(result == endpoints[0]);
  XIO_CHECK(!ec);

  xio::async_connect(socket, endpoints, true_cond_2(),
      bindns::bind(range_handler, _1, _2, &ec, &result));
  io_context.restart();
  io_context.run();
  XIO_CHECK(result == endpoints[0]);
  XIO_CHECK(!ec);

  xio::async_connect(socket, endpoints, legacy_true_cond_1,
      bindns::bind(range_handler, _1, _2, &ec, &result));
  io_context.restart();
  io_context.run();
  XIO_CHECK(result == endpoints[0]);
  XIO_CHECK(!ec);

  xio::async_connect(socket, endpoints, legacy_true_cond_2(),
      bindns::bind(range_handler, _1, _2, &ec, &result));
  io_context.restart();
  io_context.run();
  XIO_CHECK(result == endpoints[0]);
  XIO_CHECK(!ec);

  xio::async_connect(socket, endpoints, false_cond,
      bindns::bind(range_handler, _1, _2, &ec, &result));
  io_context.restart();
  io_context.run();
  XIO_CHECK(result == xio::ip::tcp::endpoint());
  XIO_CHECK(ec == xio::error::not_found);

  endpoints.insert(endpoints.begin(), xio::ip::tcp::endpoint());

  xio::async_connect(socket, endpoints, true_cond_1,
      bindns::bind(range_handler, _1, _2, &ec, &result));
  io_context.restart();
  io_context.run();
  XIO_CHECK(result == endpoints[1]);
  XIO_CHECK(!ec);

  xio::async_connect(socket, endpoints, true_cond_2(),
      bindns::bind(range_handler, _1, _2, &ec, &result));
  io_context.restart();
  io_context.run();
  XIO_CHECK(result == endpoints[1]);
  XIO_CHECK(!ec);

  xio::async_connect(socket, endpoints, legacy_true_cond_1,
      bindns::bind(range_handler, _1, _2, &ec, &result));
  io_context.restart();
  io_context.run();
  XIO_CHECK(result == endpoints[1]);
  XIO_CHECK(!ec);

  xio::async_connect(socket, endpoints, legacy_true_cond_2(),
      bindns::bind(range_handler, _1, _2, &ec, &result));
  io_context.restart();
  io_context.run();
  XIO_CHECK(result == endpoints[1]);
  XIO_CHECK(!ec);

  xio::async_connect(socket, endpoints, false_cond,
      bindns::bind(range_handler, _1, _2, &ec, &result));
  io_context.restart();
  io_context.run();
  XIO_CHECK(result == xio::ip::tcp::endpoint());
  XIO_CHECK(ec == xio::error::not_found);

  xio::async_connect(socket, endpoints, false_cond)(
      bindns::bind(range_handler, _1, _2, &ec, &result));
  io_context.restart();
  io_context.run();
  XIO_CHECK(result == xio::ip::tcp::endpoint());
  XIO_CHECK(ec == xio::error::not_found);
}

void test_async_connect_iter()
{
  connection_sink sink;
  xio::io_context io_context;
  xio::ip::tcp::socket socket(io_context);
  std::vector<xio::ip::tcp::endpoint> endpoints;
  const std::vector<xio::ip::tcp::endpoint>& cendpoints = endpoints;
  std::vector<xio::ip::tcp::endpoint>::const_iterator result;
  xio::error_code ec;

  xio::async_connect(socket, cendpoints.begin(), cendpoints.end(),
      bindns::bind(iter_handler, _1, _2, &ec, &result));
  io_context.restart();
  io_context.run();
  XIO_CHECK(result == cendpoints.end());
  XIO_CHECK(ec == xio::error::not_found);

  endpoints.push_back(sink.target_endpoint());

  xio::async_connect(socket, cendpoints.begin(), cendpoints.end(),
      bindns::bind(iter_handler, _1, _2, &ec, &result));
  io_context.restart();
  io_context.run();
  XIO_CHECK(result == cendpoints.begin());
  XIO_CHECK(!ec);

  endpoints.push_back(sink.target_endpoint());

  xio::async_connect(socket, cendpoints.begin(), cendpoints.end(),
      bindns::bind(iter_handler, _1, _2, &ec, &result));
  io_context.restart();
  io_context.run();
  XIO_CHECK(result == cendpoints.begin());
  XIO_CHECK(!ec);

  endpoints.insert(endpoints.begin(), xio::ip::tcp::endpoint());

  xio::async_connect(socket, cendpoints.begin(), cendpoints.end(),
      bindns::bind(iter_handler, _1, _2, &ec, &result));
  io_context.restart();
  io_context.run();
  XIO_CHECK(result == cendpoints.begin() + 1);
  XIO_CHECK(!ec);

  xio::async_connect(socket, cendpoints.begin(), cendpoints.end())(
      bindns::bind(iter_handler, _1, _2, &ec, &result));
  io_context.restart();
  io_context.run();
  XIO_CHECK(result == cendpoints.begin() + 1);
  XIO_CHECK(!ec);
}

void test_async_connect_iter_cond()
{
  connection_sink sink;
  xio::io_context io_context;
  xio::ip::tcp::socket socket(io_context);
  std::vector<xio::ip::tcp::endpoint> endpoints;
  const std::vector<xio::ip::tcp::endpoint>& cendpoints = endpoints;
  std::vector<xio::ip::tcp::endpoint>::const_iterator result;
  xio::error_code ec;

  xio::async_connect(socket, cendpoints.begin(), cendpoints.end(),
      true_cond_1, bindns::bind(iter_handler, _1, _2, &ec, &result));
  io_context.restart();
  io_context.run();
  XIO_CHECK(result == cendpoints.end());
  XIO_CHECK(ec == xio::error::not_found);

  xio::async_connect(socket, cendpoints.begin(), cendpoints.end(),
      true_cond_2(), bindns::bind(iter_handler, _1, _2, &ec, &result));
  io_context.restart();
  io_context.run();
  XIO_CHECK(result == cendpoints.end());
  XIO_CHECK(ec == xio::error::not_found);

  xio::async_connect(socket, cendpoints.begin(), cendpoints.end(),
      legacy_true_cond_1, bindns::bind(iter_handler, _1, _2, &ec, &result));
  io_context.restart();
  io_context.run();
  XIO_CHECK(result == cendpoints.end());
  XIO_CHECK(ec == xio::error::not_found);

  xio::async_connect(socket, cendpoints.begin(), cendpoints.end(),
      legacy_true_cond_2(), bindns::bind(iter_handler, _1, _2, &ec, &result));
  io_context.restart();
  io_context.run();
  XIO_CHECK(result == cendpoints.end());
  XIO_CHECK(ec == xio::error::not_found);

  xio::async_connect(socket, cendpoints.begin(), cendpoints.end(),
      false_cond, bindns::bind(iter_handler, _1, _2, &ec, &result));
  io_context.restart();
  io_context.run();
  XIO_CHECK(result == cendpoints.end());
  XIO_CHECK(ec == xio::error::not_found);

  endpoints.push_back(sink.target_endpoint());

  xio::async_connect(socket, cendpoints.begin(), cendpoints.end(),
      true_cond_1, bindns::bind(iter_handler, _1, _2, &ec, &result));
  io_context.restart();
  io_context.run();
  XIO_CHECK(result == cendpoints.begin());
  XIO_CHECK(!ec);

  xio::async_connect(socket, cendpoints.begin(), cendpoints.end(),
      true_cond_2(), bindns::bind(iter_handler, _1, _2, &ec, &result));
  io_context.restart();
  io_context.run();
  XIO_CHECK(result == cendpoints.begin());
  XIO_CHECK(!ec);

  xio::async_connect(socket, cendpoints.begin(), cendpoints.end(),
      legacy_true_cond_1, bindns::bind(iter_handler, _1, _2, &ec, &result));
  io_context.restart();
  io_context.run();
  XIO_CHECK(result == cendpoints.begin());
  XIO_CHECK(!ec);

  xio::async_connect(socket, cendpoints.begin(), cendpoints.end(),
      legacy_true_cond_2(), bindns::bind(iter_handler, _1, _2, &ec, &result));
  io_context.restart();
  io_context.run();
  XIO_CHECK(result == cendpoints.begin());
  XIO_CHECK(!ec);

  xio::async_connect(socket, cendpoints.begin(), cendpoints.end(),
      false_cond, bindns::bind(iter_handler, _1, _2, &ec, &result));
  io_context.restart();
  io_context.run();
  XIO_CHECK(result == cendpoints.end());
  XIO_CHECK(ec == xio::error::not_found);

  endpoints.push_back(sink.target_endpoint());

  xio::async_connect(socket, cendpoints.begin(), cendpoints.end(),
      true_cond_1, bindns::bind(iter_handler, _1, _2, &ec, &result));
  io_context.restart();
  io_context.run();
  XIO_CHECK(result == cendpoints.begin());
  XIO_CHECK(!ec);

  xio::async_connect(socket, cendpoints.begin(), cendpoints.end(),
      true_cond_2(), bindns::bind(iter_handler, _1, _2, &ec, &result));
  io_context.restart();
  io_context.run();
  XIO_CHECK(result == cendpoints.begin());
  XIO_CHECK(!ec);

  xio::async_connect(socket, cendpoints.begin(), cendpoints.end(),
      legacy_true_cond_1, bindns::bind(iter_handler, _1, _2, &ec, &result));
  io_context.restart();
  io_context.run();
  XIO_CHECK(result == cendpoints.begin());
  XIO_CHECK(!ec);

  xio::async_connect(socket, cendpoints.begin(), cendpoints.end(),
      legacy_true_cond_2(), bindns::bind(iter_handler, _1, _2, &ec, &result));
  io_context.restart();
  io_context.run();
  XIO_CHECK(result == cendpoints.begin());
  XIO_CHECK(!ec);

  xio::async_connect(socket, cendpoints.begin(), cendpoints.end(),
      false_cond, bindns::bind(iter_handler, _1, _2, &ec, &result));
  io_context.restart();
  io_context.run();
  XIO_CHECK(result == cendpoints.end());
  XIO_CHECK(ec == xio::error::not_found);

  endpoints.insert(endpoints.begin(), xio::ip::tcp::endpoint());

  xio::async_connect(socket, cendpoints.begin(), cendpoints.end(),
      true_cond_1, bindns::bind(iter_handler, _1, _2, &ec, &result));
  io_context.restart();
  io_context.run();
  XIO_CHECK(result == cendpoints.begin() + 1);
  XIO_CHECK(!ec);

  xio::async_connect(socket, cendpoints.begin(), cendpoints.end(),
      true_cond_2(), bindns::bind(iter_handler, _1, _2, &ec, &result));
  io_context.restart();
  io_context.run();
  XIO_CHECK(result == cendpoints.begin() + 1);
  XIO_CHECK(!ec);

  xio::async_connect(socket, cendpoints.begin(), cendpoints.end(),
      legacy_true_cond_1, bindns::bind(iter_handler, _1, _2, &ec, &result));
  io_context.restart();
  io_context.run();
  XIO_CHECK(result == cendpoints.begin() + 1);
  XIO_CHECK(!ec);

  xio::async_connect(socket, cendpoints.begin(), cendpoints.end(),
      legacy_true_cond_2(), bindns::bind(iter_handler, _1, _2, &ec, &result));
  io_context.restart();
  io_context.run();
  XIO_CHECK(result == cendpoints.begin() + 1);
  XIO_CHECK(!ec);

  xio::async_connect(socket, cendpoints.begin(), cendpoints.end(),
      false_cond, bindns::bind(iter_handler, _1, _2, &ec, &result));
  io_context.restart();
  io_context.run();
  XIO_CHECK(result == cendpoints.end());
  XIO_CHECK(ec == xio::error::not_found);

  xio::async_connect(socket, cendpoints.begin(),
      cendpoints.end(), false_cond)(
        bindns::bind(iter_handler, _1, _2, &ec, &result));
  io_context.restart();
  io_context.run();
  XIO_CHECK(result == cendpoints.end());
  XIO_CHECK(ec == xio::error::not_found);
}

XIO_TEST_SUITE
(
  "connect",
  XIO_TEST_CASE(test_connect_range)
  XIO_TEST_CASE(test_connect_range_ec)
  XIO_TEST_CASE(test_connect_range_cond)
  XIO_TEST_CASE(test_connect_range_cond_ec)
  XIO_TEST_CASE(test_connect_iter)
  XIO_TEST_CASE(test_connect_iter_ec)
  XIO_TEST_CASE(test_connect_iter_cond)
  XIO_TEST_CASE(test_connect_iter_cond_ec)
  XIO_TEST_CASE(test_async_connect_range)
  XIO_TEST_CASE(test_async_connect_range_cond)
  XIO_TEST_CASE(test_async_connect_iter)
  XIO_TEST_CASE(test_async_connect_iter_cond)
)
