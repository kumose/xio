//
// connect_pipe.cpp
// ~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//



// Test that header file is self-contained.
#include <xio/connect_pipe.h>

#include <functional>
#include <string>
#include <xio/io_context.h>
#include <xio/read.h>
#include <xio/readable_pipe.h>
#include <xio/writable_pipe.h>
#include <xio/write.h>
#include "unit_test.hpp"

//------------------------------------------------------------------------------

// connect_pipe_compile test
// ~~~~~~~~~~~~~~~~~~~~~~~~~
// The following test checks that all connect_pipe functions compile and link
// correctly. Runtime failures are ignored.

namespace connect_pipe_compile {

void test()
{
#if defined(XIO_HAS_PIPE)
  using namespace xio;

  try
  {
    xio::io_context io_context;
    xio::error_code ec1;

    readable_pipe p1(io_context);
    writable_pipe p2(io_context);
    connect_pipe(p1, p2);

    readable_pipe p3(io_context);
    writable_pipe p4(io_context);
    connect_pipe(p3, p4, ec1);
  }
  catch (std::exception&)
  {
  }
#endif // defined(XIO_HAS_PIPE)
}

} // namespace connect_pipe_compile

//------------------------------------------------------------------------------

// connect_pipe_runtime test
// ~~~~~~~~~~~~~~~~~~~~~~~~~
// The following test checks that connect_pipe operates correctly at runtime.

namespace connect_pipe_runtime {

static const char write_data[]
  = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

void handle_read(const xio::error_code& err,
    size_t bytes_transferred, bool* called)
{
  *called = true;
  XIO_CHECK(!err);
  XIO_CHECK(bytes_transferred == sizeof(write_data));
}

void handle_write(const xio::error_code& err,
    size_t bytes_transferred, bool* called)
{
  *called = true;
  XIO_CHECK(!err);
  XIO_CHECK(bytes_transferred == sizeof(write_data));
}

void test()
{
#if defined(XIO_HAS_PIPE)
  using namespace std; // For memcmp.
  using namespace xio;

  namespace bindns = std;
  using bindns::placeholders::_1;
  using bindns::placeholders::_2;

  try
  {
    xio::io_context io_context;

    readable_pipe p1(io_context);
    writable_pipe p2(io_context);
    connect_pipe(p1, p2);

    std::string data1 = write_data;
    xio::write(p2, xio::buffer(data1));

    std::string data2;
    data2.resize(data1.size());
    xio::read(p1, xio::buffer(data2));

    XIO_CHECK(data1 == data2);

    char read_buffer[sizeof(write_data)];
    bool read_completed = false;
    xio::async_read(p1,
        xio::buffer(read_buffer),
        bindns::bind(handle_read,
          _1, _2, &read_completed));

    bool write_completed = false;
    xio::async_write(p2,
        xio::buffer(write_data),
        bindns::bind(handle_write,
          _1, _2, &write_completed));

    io_context.run();

    XIO_CHECK(read_completed);
    XIO_CHECK(write_completed);
    XIO_CHECK(memcmp(read_buffer, write_data, sizeof(write_data)) == 0);
  }
  catch (std::exception&)
  {
    XIO_CHECK(false);
  }
#endif // defined(XIO_HAS_PIPE)
}

} // namespace connect_pipe_compile

//------------------------------------------------------------------------------

XIO_TEST_SUITE
(
  "connect_pipe",
  XIO_COMPILE_TEST_CASE(connect_pipe_compile::test)
  XIO_TEST_CASE(connect_pipe_runtime::test)
)
