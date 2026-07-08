//
// experimental/concurrent_channel.cpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//



// Test that header file is self-contained.
#include <xio/experimental/concurrent_channel.h>

#include <utility>
#include <xio/error.h>
#include <xio/io_context.h>
#include "../unit_test.hpp"

using namespace xio;
using namespace xio::experimental;

void unbuffered_concurrent_channel_test()
{
  io_context ctx;

  concurrent_channel<void(xio::error_code, std::string)> ch1(ctx);

  XIO_CHECK(ch1.is_open());
  XIO_CHECK(!ch1.ready());

  bool b1 = ch1.try_send(xio::error::eof, "hello");

  XIO_CHECK(!b1);

  std::string s1 = "abcdefghijklmnopqrstuvwxyz";
  bool b2 = ch1.try_send(xio::error::eof, std::move(s1));

  XIO_CHECK(!b2);
  XIO_CHECK(!s1.empty());

  xio::error_code ec1;
  std::string s2;
  ch1.async_receive(
      [&](xio::error_code ec, std::string s)
      {
        ec1 = ec;
        s2 = std::move(s);
      });

  bool b3 = ch1.try_send(xio::error::eof, std::move(s1));

  XIO_CHECK(b3);
  XIO_CHECK(s1.empty());

  ctx.run();

  XIO_CHECK(ec1 == xio::error::eof);
  XIO_CHECK(s2 == "abcdefghijklmnopqrstuvwxyz");

  bool b4 = ch1.try_receive([](xio::error_code, std::string){});

  XIO_CHECK(!b4);

  xio::error_code ec2 = xio::error::would_block;
  std::string s3 = "zyxwvutsrqponmlkjihgfedcba";
  ch1.async_send(xio::error::eof, std::move(s3),
      [&](xio::error_code ec)
      {
        ec2 = ec;
      });

  xio::error_code ec3;
  std::string s4;
  bool b5 = ch1.try_receive(
      [&](xio::error_code ec, std::string s)
      {
        ec3 = ec;
        s4 = s;
      });

  XIO_CHECK(b5);
  XIO_CHECK(ec3 == xio::error::eof);
  XIO_CHECK(s4 == "zyxwvutsrqponmlkjihgfedcba");

  ctx.restart();
  ctx.run();

  XIO_CHECK(!ec2);
};

void buffered_concurrent_channel_test()
{
  io_context ctx;

  concurrent_channel<void(xio::error_code, std::string)> ch1(ctx, 1);

  XIO_CHECK(ch1.is_open());
  XIO_CHECK(!ch1.ready());

  bool b1 = ch1.try_send(xio::error::eof, "hello");

  XIO_CHECK(b1);

  std::string s1 = "abcdefghijklmnopqrstuvwxyz";
  bool b2 = ch1.try_send(xio::error::eof, std::move(s1));

  XIO_CHECK(!b2);
  XIO_CHECK(!s1.empty());

  xio::error_code ec1;
  std::string s2;
  ch1.async_receive(
      [&](xio::error_code ec, std::string s)
      {
        ec1 = ec;
        s2 = std::move(s);
      });

  ctx.run();

  XIO_CHECK(ec1 == xio::error::eof);
  XIO_CHECK(s2 == "hello");

  bool b4 = ch1.try_receive([](xio::error_code, std::string){});

  XIO_CHECK(!b4);

  xio::error_code ec2 = xio::error::would_block;
  std::string s3 = "zyxwvutsrqponmlkjihgfedcba";
  ch1.async_send(xio::error::eof, std::move(s3),
      [&](xio::error_code ec)
      {
        ec2 = ec;
      });

  xio::error_code ec3;
  std::string s4;
  bool b5 = ch1.try_receive(
      [&](xio::error_code ec, std::string s)
      {
        ec3 = ec;
        s4 = s;
      });

  XIO_CHECK(b5);
  XIO_CHECK(ec3 == xio::error::eof);
  XIO_CHECK(s4 == "zyxwvutsrqponmlkjihgfedcba");

  ctx.restart();
  ctx.run();

  XIO_CHECK(!ec2);
};

void concurrent_channel_move_test()
{
  io_context ctx;

  concurrent_channel<void(xio::error_code)> ch1(ctx);
  concurrent_channel<void(xio::error_code)> ch2 = std::move(ch1);
  (void)ch2;

  concurrent_channel<void(xio::error_code, std::string)> ch3(ctx);
  concurrent_channel<void(xio::error_code, std::string)> ch4 =
    std::move(ch3);
  (void)ch4;
}

XIO_TEST_SUITE
(
  "experimental/concurrent_channel",
  XIO_TEST_CASE(unbuffered_concurrent_channel_test)
  XIO_TEST_CASE(buffered_concurrent_channel_test)
  XIO_COMPILE_TEST_CASE(concurrent_channel_move_test)
)
