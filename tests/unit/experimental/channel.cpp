//
// experimental/channel.cpp
// ~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//



// Test that header file is self-contained.
#include <xio/experimental/channel.h>

#include <utility>
#include <xio/any_completion_handler.h>
#include <xio/bind_executor.h>
#include <xio/bind_immediate_executor.h>
#include <xio/error.h>
#include <xio/inline_executor.h>
#include <xio/io_context.h>
#include "../unit_test.hpp"

using namespace xio;
using namespace xio::experimental;

void unbuffered_channel_test()
{
  io_context ctx;

  channel<void(xio::error_code, std::string)> ch1(ctx);

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
}

void buffered_channel_test()
{
  io_context ctx;

  channel<void(xio::error_code, std::string)> ch1(ctx, 1);

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

  bool b6 = ch1.try_send(xio::error_code(), "goodbye");

  XIO_CHECK(b6);

  ch1.close();

  xio::error_code ec4;
  std::string s5;
  ch1.async_receive(
      [&](xio::error_code ec, std::string s)
      {
        ec4 = ec;
        s5 = std::move(s);
      });

  ctx.restart();
  ctx.run();

  XIO_CHECK(!ec4);
  XIO_CHECK(s5 == "goodbye");

  xio::error_code ec5;
  std::string s6;
  ch1.async_receive(
      [&](xio::error_code ec, std::string s)
      {
        ec5 = ec;
        s6 = std::move(s);
      });

  ctx.restart();
  ctx.run();

  XIO_CHECK(ec5 == xio::experimental::channel_errc::channel_closed);
  XIO_CHECK(s6.empty());
}

void buffered_error_channel_test()
{
  io_context ctx;

  channel<void(xio::error_code)> ch1(ctx, 1);

  XIO_CHECK(ch1.is_open());
  XIO_CHECK(!ch1.ready());

  bool b1 = ch1.try_send(xio::error::eof);

  XIO_CHECK(b1);

  bool b2 = ch1.try_send(xio::error::eof);

  XIO_CHECK(!b2);

  xio::error_code ec1;
  ch1.async_receive(
      [&](xio::error_code ec)
      {
        ec1 = ec;
      });

  ctx.run();

  XIO_CHECK(ec1 == xio::error::eof);

  bool b4 = ch1.try_receive([](xio::error_code){});

  XIO_CHECK(!b4);

  xio::error_code ec2 = xio::error::would_block;
  ch1.async_send(xio::error::eof,
      [&](xio::error_code ec)
      {
        ec2 = ec;
      });

  xio::error_code ec3;
  bool b5 = ch1.try_receive(
      [&](xio::error_code ec)
      {
        ec3 = ec;
      });

  XIO_CHECK(b5);
  XIO_CHECK(ec3 == xio::error::eof);

  ctx.restart();
  ctx.run();

  XIO_CHECK(!ec2);
}

void unbuffered_non_immediate_receive()
{
  io_context ctx;

  channel<void(xio::error_code, std::string)> ch1(ctx);

  xio::error_code ec1 = xio::error::would_block;
  std::string s1 = "0123456789";
  ch1.async_send(xio::error::eof, std::move(s1),
      [&](xio::error_code ec)
      {
        ec1 = ec;
      });

  XIO_CHECK(ec1 == xio::error::would_block);

  xio::error_code ec2 = xio::error::would_block;
  std::string s2;
  ch1.async_receive(
      [&](xio::error_code ec, std::string s)
      {
        ec2 = ec;
        s2 = std::move(s);
      });

  XIO_CHECK(ec2 == xio::error::would_block);

  ctx.run();

  XIO_CHECK(!ec1);
  XIO_CHECK(ec2 == xio::error::eof);
  XIO_CHECK(s2 == "0123456789");
}

void unbuffered_immediate_receive()
{
  io_context ctx;

  channel<void(xio::error_code, std::string)> ch1(ctx);

  xio::error_code ec1 = xio::error::would_block;
  std::string s1 = "0123456789";
  ch1.async_send(xio::error::eof, std::move(s1),
      [&](xio::error_code ec)
      {
        ec1 = ec;
      });

  XIO_CHECK(ec1 == xio::error::would_block);

  xio::error_code ec2 = xio::error::would_block;
  std::string s2;
  ch1.async_receive(
      bind_immediate_executor(inline_executor(),
        [&](xio::error_code ec, std::string s)
        {
          ec2 = ec;
          s2 = std::move(s);
        }));

  XIO_CHECK(ec1 == xio::error::would_block);
  XIO_CHECK(ec2 == xio::error::eof);
  XIO_CHECK(s2 == "0123456789");

  ctx.run();

  XIO_CHECK(!ec1);
}

void unbuffered_executor_receive()
{
  io_context ctx;
  io_context ctx2;

  channel<void(xio::error_code, std::string)> ch1(ctx);

  xio::error_code ec1 = xio::error::would_block;
  std::string s1 = "0123456789";
  ch1.async_send(xio::error::eof, std::move(s1),
      [&](xio::error_code ec)
      {
        ec1 = ec;
      });

  XIO_CHECK(ec1 == xio::error::would_block);

  xio::error_code ec2 = xio::error::would_block;
  std::string s2;
  ch1.async_receive(
      bind_executor(ctx2,
        [&](xio::error_code ec, std::string s)
        {
          ec2 = ec;
          s2 = std::move(s);
        }));

  XIO_CHECK(ec1 == xio::error::would_block);
  XIO_CHECK(ec2 == xio::error::would_block);

  ctx.run();

  XIO_CHECK(!ec1);
  XIO_CHECK(ec2 == xio::error::would_block);

  ctx2.run();

  XIO_CHECK(ec2 == xio::error::eof);
  XIO_CHECK(s2 == "0123456789");
}

void unbuffered_non_immediate_send()
{
  io_context ctx;

  channel<void(xio::error_code, std::string)> ch1(ctx);

  xio::error_code ec1 = xio::error::would_block;
  std::string s1;
  ch1.async_receive(
      [&](xio::error_code ec, std::string s)
      {
        ec1 = ec;
        s1 = std::move(s);
      });

  XIO_CHECK(ec1 == xio::error::would_block);

  xio::error_code ec2 = xio::error::would_block;
  std::string s2 = "0123456789";
  ch1.async_send(xio::error::eof, std::move(s2),
      [&](xio::error_code ec)
      {
        ec2 = ec;
      });

  XIO_CHECK(ec2 == xio::error::would_block);

  ctx.run();

  XIO_CHECK(ec1 == xio::error::eof);
  XIO_CHECK(s1 == "0123456789");
  XIO_CHECK(!ec2);
}

void unbuffered_immediate_send()
{
  io_context ctx;

  channel<void(xio::error_code, std::string)> ch1(ctx);

  xio::error_code ec1 = xio::error::would_block;
  std::string s1;
  ch1.async_receive(
      [&](xio::error_code ec, std::string s)
      {
        ec1 = ec;
        s1 = std::move(s);
      });

  XIO_CHECK(ec1 == xio::error::would_block);

  xio::error_code ec2 = xio::error::would_block;
  std::string s2 = "0123456789";
  ch1.async_send(xio::error::eof, std::move(s2),
      bind_immediate_executor(inline_executor(),
        [&](xio::error_code ec)
        {
          ec2 = ec;
        }));

  XIO_CHECK(ec1 == xio::error::would_block);
  XIO_CHECK(!ec2);

  ctx.run();

  XIO_CHECK(ec1 == xio::error::eof);
  XIO_CHECK(s1 == "0123456789");
}

void unbuffered_executor_send()
{
  io_context ctx;
  io_context ctx2;

  channel<void(xio::error_code, std::string)> ch1(ctx);

  xio::error_code ec1 = xio::error::would_block;
  std::string s1;
  ch1.async_receive(
      [&](xio::error_code ec, std::string s)
      {
        ec1 = ec;
        s1 = std::move(s);
      });

  XIO_CHECK(ec1 == xio::error::would_block);

  xio::error_code ec2 = xio::error::would_block;
  std::string s2 = "0123456789";
  ch1.async_send(xio::error::eof, std::move(s2),
      bind_executor(ctx2,
        [&](xio::error_code ec)
        {
          ec2 = ec;
        }));

  XIO_CHECK(ec1 == xio::error::would_block);
  XIO_CHECK(ec2 == xio::error::would_block);

  ctx.run();

  XIO_CHECK(ec1 == xio::error::eof);
  XIO_CHECK(s1 == "0123456789");
  XIO_CHECK(ec2 == xio::error::would_block);

  ctx2.run();

  XIO_CHECK(!ec2);
}

void buffered_non_immediate_receive()
{
  io_context ctx;

  channel<void(xio::error_code, std::string)> ch1(ctx, 1);

  xio::error_code ec1 = xio::error::would_block;
  std::string s1 = "0123456789";
  ch1.async_send(xio::error::eof, std::move(s1),
      [&](xio::error_code ec)
      {
        ec1 = ec;
      });

  XIO_CHECK(ec1 == xio::error::would_block);

  ctx.run();

  XIO_CHECK(!ec1);

  xio::error_code ec2 = xio::error::would_block;
  std::string s2;
  ch1.async_receive(
      [&](xio::error_code ec, std::string s)
      {
        ec2 = ec;
        s2 = std::move(s);
      });

  XIO_CHECK(ec2 == xio::error::would_block);

  ctx.restart();
  ctx.run();

  XIO_CHECK(ec2 == xio::error::eof);
  XIO_CHECK(s2 == "0123456789");
}

void buffered_immediate_receive()
{
  io_context ctx;

  channel<void(xio::error_code, std::string)> ch1(ctx, 1);

  xio::error_code ec1 = xio::error::would_block;
  std::string s1 = "0123456789";
  ch1.async_send(xio::error::eof, std::move(s1),
      [&](xio::error_code ec)
      {
        ec1 = ec;
      });

  XIO_CHECK(ec1 == xio::error::would_block);

  ctx.run();

  XIO_CHECK(!ec1);

  xio::error_code ec2 = xio::error::would_block;
  std::string s2;
  ch1.async_receive(
      bind_immediate_executor(inline_executor(),
        [&](xio::error_code ec, std::string s)
        {
          ec2 = ec;
          s2 = std::move(s);
        }));

  XIO_CHECK(ec2 == xio::error::eof);
  XIO_CHECK(s2 == "0123456789");

  ctx.restart();
  ctx.run();
}

void buffered_executor_receive()
{
  io_context ctx;
  io_context ctx2;

  channel<void(xio::error_code, std::string)> ch1(ctx, 1);

  xio::error_code ec1 = xio::error::would_block;
  std::string s1 = "0123456789";
  ch1.async_send(xio::error::eof, std::move(s1),
      [&](xio::error_code ec)
      {
        ec1 = ec;
      });

  XIO_CHECK(ec1 == xio::error::would_block);

  ctx.run();

  XIO_CHECK(!ec1);

  xio::error_code ec2 = xio::error::would_block;
  std::string s2;
  ch1.async_receive(
      bind_executor(ctx2,
        [&](xio::error_code ec, std::string s)
        {
          ec2 = ec;
          s2 = std::move(s);
        }));

  XIO_CHECK(ec2 == xio::error::would_block);

  ctx.restart();
  ctx.run();

  XIO_CHECK(ec2 == xio::error::would_block);

  ctx2.run();

  XIO_CHECK(ec2 == xio::error::eof);
  XIO_CHECK(s2 == "0123456789");
}

void buffered_non_immediate_send()
{
  io_context ctx;

  channel<void(xio::error_code, std::string)> ch1(ctx, 1);

  xio::error_code ec1 = xio::error::would_block;
  std::string s1 = "0123456789";
  ch1.async_send(xio::error::eof, std::move(s1),
      [&](xio::error_code ec)
      {
        ec1 = ec;
      });

  XIO_CHECK(ec1 == xio::error::would_block);

  ctx.run();

  XIO_CHECK(!ec1);
}

void buffered_immediate_send()
{
  io_context ctx;

  channel<void(xio::error_code, std::string)> ch1(ctx, 1);

  xio::error_code ec1 = xio::error::would_block;
  std::string s1 = "0123456789";
  ch1.async_send(xio::error::eof, std::move(s1),
      bind_immediate_executor(inline_executor(),
        [&](xio::error_code ec)
        {
          ec1 = ec;
        }));

  XIO_CHECK(!ec1);

  ctx.run();
}

void buffered_executor_send()
{
  io_context ctx;
  io_context ctx2;

  channel<void(xio::error_code, std::string)> ch1(ctx, 1);

  xio::error_code ec1 = xio::error::would_block;
  std::string s1 = "0123456789";
  ch1.async_send(xio::error::eof, std::move(s1),
      bind_executor(ctx2,
        [&](xio::error_code ec)
        {
          ec1 = ec;
        }));

  XIO_CHECK(ec1 == xio::error::would_block);

  ctx.run();

  XIO_CHECK(ec1 == xio::error::would_block);

  ctx2.run();

  XIO_CHECK(!ec1);
}

void try_send_via_dispatch()
{
  io_context ctx;

  channel<void(xio::error_code, std::string)> ch1(ctx);

  xio::error_code ec1 = xio::error::would_block;
  std::string s1;
  ch1.async_receive(
      bind_executor(xio::inline_executor(),
        [&](xio::error_code ec, std::string s)
        {
          ec1 = ec;
          s1 = std::move(s);
        }));

  XIO_CHECK(ec1 == xio::error::would_block);

  ctx.poll();

  XIO_CHECK(ec1 == xio::error::would_block);

  std::string s2 = "0123456789";
  ch1.try_send_via_dispatch(xio::error::eof, std::move(s2));

  XIO_CHECK(ec1 == xio::error::eof);
  XIO_CHECK(s1 == "0123456789");
  XIO_CHECK(s2.empty());
}

void try_send_n_via_dispatch()
{
  io_context ctx;

  channel<void(xio::error_code, std::string)> ch1(ctx);

  xio::error_code ec1 = xio::error::would_block;
  std::string s1;
  ch1.async_receive(
      bind_executor(xio::inline_executor(),
        [&](xio::error_code ec, std::string s)
        {
          ec1 = ec;
          s1 = std::move(s);
        }));

  XIO_CHECK(ec1 == xio::error::would_block);

  xio::error_code ec2 = xio::error::would_block;
  std::string s2;
  ch1.async_receive(
      bind_executor(xio::inline_executor(),
        [&](xio::error_code ec, std::string s)
        {
          ec2 = ec;
          s2 = std::move(s);
        }));

  XIO_CHECK(ec1 == xio::error::would_block);

  ctx.poll();

  XIO_CHECK(ec1 == xio::error::would_block);
  XIO_CHECK(ec2 == xio::error::would_block);

  std::string s3 = "0123456789";
  ch1.try_send_n_via_dispatch(2, xio::error::eof, std::move(s3));

  XIO_CHECK(ec1 == xio::error::eof);
  XIO_CHECK(s1 == "0123456789");
  XIO_CHECK(ec2 == xio::error::eof);
  XIO_CHECK(s2 == "0123456789");
  XIO_CHECK(s3.empty());
}

struct multi_signature_handler
{
  std::string* s_;
  xio::error_code* ec_;

  void operator()(std::string s)
  {
    *s_ = s;
  }

  void operator()(xio::error_code ec)
  {
    *ec_ = ec;
  }
};

void implicit_error_signature_channel_test()
{
  io_context ctx;

  channel<void(std::string)> ch1(ctx);

  XIO_CHECK(ch1.is_open());
  XIO_CHECK(!ch1.ready());

  bool b1 = ch1.try_send("hello");

  XIO_CHECK(!b1);

  std::string s1 = "abcdefghijklmnopqrstuvwxyz";
  bool b2 = ch1.try_send(std::move(s1));

  XIO_CHECK(!b2);
  XIO_CHECK(!s1.empty());

  std::string s2;
  xio::error_code ec1 = xio::error::would_block;
  multi_signature_handler h1 = {&s2, &ec1};
  ch1.async_receive(h1);

  bool b3 = ch1.try_send(std::move(s1));

  XIO_CHECK(b3);
  XIO_CHECK(s1.empty());

  ctx.run();

  XIO_CHECK(s2 == "abcdefghijklmnopqrstuvwxyz");
  XIO_CHECK(ec1 == xio::error::would_block);

  std::string s3;
  xio::error_code ec2;
  multi_signature_handler h2 = {&s3, &ec2};
  bool b4 = ch1.try_receive(h2);

  XIO_CHECK(!b4);

  std::string s4 = "zyxwvutsrqponmlkjihgfedcba";
  xio::error_code ec3;
  ch1.async_send(std::move(s4),
      [&](xio::error_code ec)
      {
        ec3 = ec;
      });

  std::string s5;
  xio::error_code ec4 = xio::error::would_block;
  multi_signature_handler h3 = {&s5, &ec4};
  bool b5 = ch1.try_receive(h3);

  XIO_CHECK(b5);
  XIO_CHECK(ec4 == xio::error::would_block);
  XIO_CHECK(s5 == "zyxwvutsrqponmlkjihgfedcba");

  ctx.restart();
  ctx.run();

  XIO_CHECK(!ec3);

  std::string s6;
  xio::error_code ec5 = xio::error::would_block;
  multi_signature_handler h4 = {&s6, &ec5};
  ch1.async_receive(h4);

  ch1.close();

  ctx.restart();
  ctx.run();

  XIO_CHECK(s6.empty());
  XIO_CHECK(ec5 == xio::experimental::channel_errc::channel_closed);
}

void channel_with_any_completion_handler_test()
{
  io_context ctx;

  channel<void(xio::error_code, std::string)> ch1(ctx);

  xio::error_code ec1 = xio::error::would_block;
  std::string s1;
  ch1.async_receive(
      xio::any_completion_handler<
        void(xio::error_code, std::string)>(
          [&](xio::error_code ec, std::string s)
          {
            ec1 = ec;
            s1 = std::move(s);
          }));

  xio::error_code ec2 = xio::error::would_block;
  std::string s2 = "zyxwvutsrqponmlkjihgfedcba";
  ch1.async_send(xio::error::eof, std::move(s2),
      xio::any_completion_handler<void(xio::error_code)>(
        [&](xio::error_code ec)
        {
          ec2 = ec;
        }));

  XIO_CHECK(ec1 == xio::error::would_block);
  XIO_CHECK(ec2 == xio::error::would_block);

  ctx.run();

  XIO_CHECK(ec1 == xio::error::eof);
  XIO_CHECK(s1 == "zyxwvutsrqponmlkjihgfedcba");
  XIO_CHECK(!ec2);
}

void channel_move_test()
{
  io_context ctx;

  channel<void(xio::error_code)> ch1(ctx);
  channel<void(xio::error_code)> ch2 = std::move(ch1);
  (void)ch2;

  channel<void(xio::error_code, std::string)> ch3(ctx);
  channel<void(xio::error_code, std::string)> ch4 = std::move(ch3);
  (void)ch4;
}

XIO_TEST_SUITE
(
  "experimental/channel",
  XIO_TEST_CASE(unbuffered_channel_test)
  XIO_TEST_CASE(buffered_channel_test)
  XIO_TEST_CASE(buffered_error_channel_test)
  XIO_TEST_CASE(unbuffered_non_immediate_receive)
  XIO_TEST_CASE(unbuffered_immediate_receive)
  XIO_TEST_CASE(unbuffered_executor_receive)
  XIO_TEST_CASE(unbuffered_non_immediate_send)
  XIO_TEST_CASE(unbuffered_immediate_send)
  XIO_TEST_CASE(unbuffered_executor_send)
  XIO_TEST_CASE(buffered_non_immediate_receive)
  XIO_TEST_CASE(buffered_immediate_receive)
  XIO_TEST_CASE(buffered_executor_receive)
  XIO_TEST_CASE(buffered_non_immediate_send)
  XIO_TEST_CASE(buffered_immediate_send)
  XIO_TEST_CASE(buffered_executor_send)
  XIO_TEST_CASE(try_send_via_dispatch)
  XIO_TEST_CASE(try_send_n_via_dispatch)
  XIO_TEST_CASE(implicit_error_signature_channel_test)
  XIO_TEST_CASE(channel_with_any_completion_handler_test)
  XIO_COMPILE_TEST_CASE(channel_move_test)
)
