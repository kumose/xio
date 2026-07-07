//
// redirect_disposition.cpp
// ~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//



// Test that header file is self-contained.
#include <xio/redirect_disposition.h>

#include <xio/bind_executor.h>
#include <xio/deferred.h>
#include <xio/io_context.h>
#include <xio/post.h>
#include <xio/system_timer.h>
#include <xio/use_future.h>
#include "unit_test.hpp"

struct redirect_disposition_handler
{
  int* count_;

  explicit redirect_disposition_handler(int* c)
    : count_(c)
  {
  }

  void operator()()
  {
    ++(*count_);
  }
};

void redirect_disposition_test()
{
  xio::io_context io1;
  xio::io_context io2;
  xio::system_timer timer1(io1);
  xio::error_code ec = xio::error::would_block;
  int count = 0;

  timer1.expires_after(std::chrono::seconds(0));
  timer1.async_wait(
      xio::redirect_disposition(
        xio::bind_executor(io2.get_executor(),
          redirect_disposition_handler(&count)), ec));

  ASIO_CHECK(ec == xio::error::would_block);
  ASIO_CHECK(count == 0);

  io1.run();

  ASIO_CHECK(ec == xio::error::would_block);
  ASIO_CHECK(count == 0);

  io2.run();

  ASIO_CHECK(!ec);
  ASIO_CHECK(count == 1);

  ec = xio::error::would_block;
  timer1.async_wait(
      xio::redirect_disposition(
        xio::bind_executor(io2.get_executor(),
          xio::deferred), ec))(redirect_disposition_handler(&count));

  ASIO_CHECK(ec == xio::error::would_block);
  ASIO_CHECK(count == 1);

  io1.restart();
  io1.run();

  ASIO_CHECK(ec == xio::error::would_block);
  ASIO_CHECK(count == 1);

  io2.restart();
  io2.run();

  ASIO_CHECK(!ec);
  ASIO_CHECK(count == 2);


  ec = xio::error::would_block;
  std::future<void> f = timer1.async_wait(
      xio::redirect_disposition(
        xio::bind_executor(io2.get_executor(),
          xio::use_future), ec));

  ASIO_CHECK(ec == xio::error::would_block);
  ASIO_CHECK(f.wait_for(std::chrono::seconds(0))
      == std::future_status::timeout);

  io1.restart();
  io1.run();

  ASIO_CHECK(ec == xio::error::would_block);
  ASIO_CHECK(f.wait_for(std::chrono::seconds(0))
      == std::future_status::timeout);

  io2.restart();
  io2.run();

  ASIO_CHECK(!ec);
  ASIO_CHECK(f.wait_for(std::chrono::seconds(0))
      == std::future_status::ready);

}

void partial_redirect_disposition_test()
{
  xio::io_context io1;
  xio::io_context io2;
  xio::system_timer timer1(io1);
  xio::error_code ec = xio::error::would_block;
  int count = 0;

  timer1.expires_after(std::chrono::seconds(0));
  timer1.async_wait(xio::redirect_disposition(ec))(
      xio::bind_executor(io2.get_executor(),
        redirect_disposition_handler(&count)));

  ASIO_CHECK(ec == xio::error::would_block);
  ASIO_CHECK(count == 0);

  io1.run();

  ASIO_CHECK(ec == xio::error::would_block);
  ASIO_CHECK(count == 0);

  io2.run();

  ASIO_CHECK(!ec);
  ASIO_CHECK(count == 1);

  ec = xio::error::would_block;
  timer1.async_wait(xio::redirect_disposition(ec))(
      xio::bind_executor(io2.get_executor(),
        xio::deferred))(redirect_disposition_handler(&count));

  ASIO_CHECK(ec == xio::error::would_block);
  ASIO_CHECK(count == 1);

  io1.restart();
  io1.run();

  ASIO_CHECK(ec == xio::error::would_block);
  ASIO_CHECK(count == 1);

  io2.restart();
  io2.run();

  ASIO_CHECK(!ec);
  ASIO_CHECK(count == 2);

  ec = xio::error::would_block;
  timer1.async_wait()(xio::redirect_disposition(ec))(
      xio::bind_executor(io2.get_executor(),
        xio::deferred))(redirect_disposition_handler(&count));

  ASIO_CHECK(ec == xio::error::would_block);
  ASIO_CHECK(count == 2);

  io1.restart();
  io1.run();

  ASIO_CHECK(ec == xio::error::would_block);
  ASIO_CHECK(count == 2);

  io2.restart();
  io2.run();

  ASIO_CHECK(!ec);
  ASIO_CHECK(count == 3);

  ec = xio::error::would_block;
  std::future<void> f = timer1.async_wait(xio::redirect_disposition(ec))(
      xio::bind_executor(io2.get_executor(), xio::use_future));

  ASIO_CHECK(ec == xio::error::would_block);
  ASIO_CHECK(f.wait_for(std::chrono::seconds(0))
      == std::future_status::timeout);

  io1.restart();
  io1.run();

  ASIO_CHECK(ec == xio::error::would_block);
  ASIO_CHECK(f.wait_for(std::chrono::seconds(0))
      == std::future_status::timeout);

  io2.restart();
  io2.run();

  ASIO_CHECK(!ec);
  ASIO_CHECK(f.wait_for(std::chrono::seconds(0))
      == std::future_status::ready);
}

void redirect_disposition_to_exception_ptr_test()
{
  xio::io_context io1;
  xio::io_context io2;
  xio::system_timer timer1(io1);
  std::exception_ptr ex = nullptr;
  int count = 0;

  timer1.expires_after(std::chrono::seconds(100));
  timer1.async_wait(
      xio::redirect_disposition(
        xio::bind_executor(io2.get_executor(),
          redirect_disposition_handler(&count)), ex));
  timer1.cancel();

  ASIO_CHECK(ex == nullptr);
  ASIO_CHECK(count == 0);

  io1.run();

  ASIO_CHECK(ex == nullptr);
  ASIO_CHECK(count == 0);

  io2.run();

  ASIO_CHECK(ex != nullptr);
  ASIO_CHECK(count == 1);

#if !defined(ASIO_NO_EXCEPTIONS)
  try
  {
    std::rethrow_exception(ex);
  }
  catch (const std::system_error& e)
  {
    ASIO_CHECK(e.code() == xio::error::operation_aborted);
  }
#endif // !defined(ASIO_NO_EXCEPTIONS)

  ex = nullptr;
  timer1.async_wait(
      xio::redirect_disposition(
        xio::bind_executor(io2.get_executor(),
          xio::deferred), ex))(redirect_disposition_handler(&count));
  timer1.cancel();

  ASIO_CHECK(ex == nullptr);
  ASIO_CHECK(count == 1);

  io1.restart();
  io1.run();

  ASIO_CHECK(ex == nullptr);
  ASIO_CHECK(count == 1);

  io2.restart();
  io2.run();

  ASIO_CHECK(ex != nullptr);
  ASIO_CHECK(count == 2);

#if !defined(ASIO_NO_EXCEPTIONS)
  try
  {
    std::rethrow_exception(ex);
  }
  catch (const std::system_error& e)
  {
    ASIO_CHECK(e.code() == xio::error::operation_aborted);
  }
#endif // !defined(ASIO_NO_EXCEPTIONS)

  ex = nullptr;
  std::future<void> f = timer1.async_wait(
      xio::redirect_disposition(
        xio::bind_executor(io2.get_executor(),
          xio::use_future), ex));
  timer1.cancel();

  ASIO_CHECK(ex == nullptr);
  ASIO_CHECK(f.wait_for(std::chrono::seconds(0))
      == std::future_status::timeout);

  io1.restart();
  io1.run();

  ASIO_CHECK(ex == nullptr);
  ASIO_CHECK(f.wait_for(std::chrono::seconds(0))
      == std::future_status::timeout);

  io2.restart();
  io2.run();

  ASIO_CHECK(ex != nullptr);
  ASIO_CHECK(f.wait_for(std::chrono::seconds(0))
      == std::future_status::ready);
}

void partial_redirect_disposition_to_exception_ptr_test()
{
  xio::io_context io1;
  xio::io_context io2;
  xio::system_timer timer1(io1);
  std::exception_ptr ex = nullptr;
  int count = 0;

  timer1.expires_after(std::chrono::seconds(100));
  timer1.async_wait(xio::redirect_disposition(ex))(
      xio::bind_executor(io2.get_executor(),
        redirect_disposition_handler(&count)));
  timer1.cancel();

  ASIO_CHECK(ex == nullptr);
  ASIO_CHECK(count == 0);

  io1.run();

  ASIO_CHECK(ex == nullptr);
  ASIO_CHECK(count == 0);

  io2.run();

  ASIO_CHECK(ex != nullptr);
  ASIO_CHECK(count == 1);

#if !defined(ASIO_NO_EXCEPTIONS)
  try
  {
    std::rethrow_exception(ex);
  }
  catch (const std::system_error& e)
  {
    ASIO_CHECK(e.code() == xio::error::operation_aborted);
  }
#endif // !defined(ASIO_NO_EXCEPTIONS)

  ex = nullptr;
  timer1.async_wait(xio::redirect_disposition(ex))(
      xio::bind_executor(io2.get_executor(),
        xio::deferred))(redirect_disposition_handler(&count));
  timer1.cancel();

  ASIO_CHECK(ex == nullptr);
  ASIO_CHECK(count == 1);

  io1.restart();
  io1.run();

  ASIO_CHECK(ex == nullptr);
  ASIO_CHECK(count == 1);

  io2.restart();
  io2.run();

  ASIO_CHECK(ex != nullptr);
  ASIO_CHECK(count == 2);

#if !defined(ASIO_NO_EXCEPTIONS)
  try
  {
    std::rethrow_exception(ex);
  }
  catch (const std::system_error& e)
  {
    ASIO_CHECK(e.code() == xio::error::operation_aborted);
  }
#endif // !defined(ASIO_NO_EXCEPTIONS)

  ex = nullptr;
  timer1.async_wait()(xio::redirect_disposition(ex))(
      xio::bind_executor(io2.get_executor(),
        xio::deferred))(redirect_disposition_handler(&count));
  timer1.cancel();

  ASIO_CHECK(ex == nullptr);
  ASIO_CHECK(count == 2);

  io1.restart();
  io1.run();

  ASIO_CHECK(ex == nullptr);
  ASIO_CHECK(count == 2);

  io2.restart();
  io2.run();

  ASIO_CHECK(ex != nullptr);
  ASIO_CHECK(count == 3);

#if !defined(ASIO_NO_EXCEPTIONS)
  try
  {
    std::rethrow_exception(ex);
  }
  catch (const std::system_error& e)
  {
    ASIO_CHECK(e.code() == xio::error::operation_aborted);
  }
#endif // !defined(ASIO_NO_EXCEPTIONS)

  ex = nullptr;
  std::future<void> f = timer1.async_wait(xio::redirect_disposition(ex))(
      xio::bind_executor(io2.get_executor(), xio::use_future));
  timer1.cancel();

  ASIO_CHECK(ex == nullptr);
  ASIO_CHECK(f.wait_for(std::chrono::seconds(0))
      == std::future_status::timeout);

  io1.restart();
  io1.run();

  ASIO_CHECK(ex == nullptr);
  ASIO_CHECK(f.wait_for(std::chrono::seconds(0))
      == std::future_status::timeout);

  io2.restart();
  io2.run();

  ASIO_CHECK(ex != nullptr);
  ASIO_CHECK(f.wait_for(std::chrono::seconds(0))
      == std::future_status::ready);

}

ASIO_TEST_SUITE
(
  "redirect_disposition",
  ASIO_TEST_CASE(redirect_disposition_test)
  ASIO_TEST_CASE(partial_redirect_disposition_test)
  ASIO_TEST_CASE(redirect_disposition_to_exception_ptr_test)
  ASIO_TEST_CASE(partial_redirect_disposition_to_exception_ptr_test)
)
