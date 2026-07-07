//
// is_read_buffered.cpp
// ~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//



// Test that header file is self-contained.
#include <xio/is_read_buffered.h>

#include <xio/buffered_read_stream.h>
#include <xio/buffered_write_stream.h>
#include <xio/io_context.h>
#include <xio/ip/tcp.h>
#include "unit_test.hpp"

using namespace std; // For memcmp, memcpy and memset.

class test_stream
{
public:
  typedef xio::io_context io_context_type;

  typedef test_stream lowest_layer_type;

  typedef io_context_type::executor_type executor_type;

  test_stream(xio::io_context& io_context)
    : io_context_(io_context)
  {
  }

  io_context_type& io_context()
  {
    return io_context_;
  }

  lowest_layer_type& lowest_layer()
  {
    return *this;
  }

  template <typename Const_Buffers>
  size_t write(const Const_Buffers&)
  {
    return 0;
  }

  template <typename Const_Buffers>
  size_t write(const Const_Buffers&, xio::error_code& ec)
  {
    ec = xio::error_code();
    return 0;
  }

  template <typename Const_Buffers, typename Handler>
  void async_write(const Const_Buffers&, Handler handler)
  {
    xio::error_code error;
    xio::post(io_context_,
        xio::detail::bind_handler(handler, error, 0));
  }

  template <typename Mutable_Buffers>
  size_t read(const Mutable_Buffers&)
  {
    return 0;
  }

  template <typename Mutable_Buffers>
  size_t read(const Mutable_Buffers&, xio::error_code& ec)
  {
    ec = xio::error_code();
    return 0;
  }

  template <typename Mutable_Buffers, typename Handler>
  void async_read(const Mutable_Buffers&, Handler handler)
  {
    xio::error_code error;
    xio::post(io_context_,
        xio::detail::bind_handler(handler, error, 0));
  }

private:
  io_context_type& io_context_;
};

void is_read_buffered_test()
{
  ASIO_CHECK(!xio::is_read_buffered<
      xio::ip::tcp::socket>::value);

  ASIO_CHECK(!!xio::is_read_buffered<
      xio::buffered_read_stream<
        xio::ip::tcp::socket> >::value);

  ASIO_CHECK(!xio::is_read_buffered<
      xio::buffered_write_stream<
        xio::ip::tcp::socket> >::value);

  ASIO_CHECK(!!xio::is_read_buffered<
      xio::buffered_stream<xio::ip::tcp::socket> >::value);

  ASIO_CHECK(!xio::is_read_buffered<test_stream>::value);

  ASIO_CHECK(!!xio::is_read_buffered<
      xio::buffered_read_stream<test_stream> >::value);

  ASIO_CHECK(!xio::is_read_buffered<
      xio::buffered_write_stream<test_stream> >::value);

  ASIO_CHECK(!!xio::is_read_buffered<
      xio::buffered_stream<test_stream> >::value);
}

ASIO_TEST_SUITE
(
  "is_read_buffered",
  ASIO_TEST_CASE(is_read_buffered_test)
)
