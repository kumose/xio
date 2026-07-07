//
// disposition.cpp
// ~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//



// Test that header file is self-contained.
#include <xio/disposition.h>

#include <xio/error.h>

#include "unit_test.hpp"

void no_error_test()
{
  using xio::no_error;

  ASIO_CHECK(no_error == no_error);
  ASIO_CHECK(!(no_error != no_error));
}

void error_code_disposition_test()
{
  using xio::no_error;

  xio::error_code ec1;

  ASIO_CHECK(ec1 == no_error);
  ASIO_CHECK(no_error == ec1);
  ASIO_CHECK(!(ec1 != no_error));
  ASIO_CHECK(!(no_error != ec1));

  std::exception_ptr ep1 = xio::to_exception_ptr(ec1);
  ASIO_CHECK(ep1 == nullptr);

  xio::error_code ec2 = xio::error::eof;

  ASIO_CHECK(!(ec2 == no_error));
  ASIO_CHECK(!(no_error == ec2));
  ASIO_CHECK(ec2 != no_error);
  ASIO_CHECK(no_error != ec2);

#if !defined(ASIO_NO_EXCEPTIONS)
  bool caught;
  try
  {
    xio::throw_exception(ec2);
    caught = false;
  }
  catch (const xio::system_error& ex)
  {
    caught = true;
    ASIO_CHECK(ex.code() == xio::error::eof);
  }
  ASIO_CHECK(caught);
#endif // !defined(ASIO_NO_EXCEPTIONS)

  std::exception_ptr ep2 = xio::to_exception_ptr(ec2);
  ASIO_CHECK(ep2 != nullptr);
}

void exception_ptr_disposition_test()
{
  using xio::no_error;

  std::exception_ptr ep1;

  ASIO_CHECK(ep1 == no_error);
  ASIO_CHECK(no_error == ep1);
  ASIO_CHECK(!(ep1 != no_error));
  ASIO_CHECK(!(no_error != ep1));

  std::exception_ptr ep2 = xio::to_exception_ptr(ep1);
  ASIO_CHECK(ep1 == nullptr);

  std::exception_ptr ep3 = std::make_exception_ptr(
      xio::system_error(xio::error::eof));

  ASIO_CHECK(!(ep3 == no_error));
  ASIO_CHECK(!(no_error == ep3));
  ASIO_CHECK(ep3 != no_error);
  ASIO_CHECK(no_error != ep3);

#if !defined(ASIO_NO_EXCEPTIONS)
  bool caught;
  try
  {
    xio::throw_exception(ep3);
    caught = false;
  }
  catch (const xio::system_error& ex)
  {
    caught = true;
    ASIO_CHECK(ex.code() == xio::error::eof);
  }
  ASIO_CHECK(caught);
#endif // !defined(ASIO_NO_EXCEPTIONS)

  std::exception_ptr ep4 = xio::to_exception_ptr(ep3);
  ASIO_CHECK(ep4 != nullptr);
}

ASIO_TEST_SUITE
(
  "disposition",
  ASIO_TEST_CASE(no_error_test)
  ASIO_TEST_CASE(error_code_disposition_test)
  ASIO_TEST_CASE(exception_ptr_disposition_test)
)
