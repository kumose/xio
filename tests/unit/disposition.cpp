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

  XIO_CHECK(no_error == no_error);
  XIO_CHECK(!(no_error != no_error));
}

void error_code_disposition_test()
{
  using xio::no_error;

  xio::error_code ec1;

  XIO_CHECK(ec1 == no_error);
  XIO_CHECK(no_error == ec1);
  XIO_CHECK(!(ec1 != no_error));
  XIO_CHECK(!(no_error != ec1));

  std::exception_ptr ep1 = xio::to_exception_ptr(ec1);
  XIO_CHECK(ep1 == nullptr);

  xio::error_code ec2 = xio::error::eof;

  XIO_CHECK(!(ec2 == no_error));
  XIO_CHECK(!(no_error == ec2));
  XIO_CHECK(ec2 != no_error);
  XIO_CHECK(no_error != ec2);

#if !defined(XIO_NO_EXCEPTIONS)
  bool caught;
  try
  {
    xio::throw_exception(ec2);
    caught = false;
  }
  catch (const std::system_error& ex)
  {
    caught = true;
    XIO_CHECK(ex.code() == xio::error::eof);
  }
  XIO_CHECK(caught);
#endif // !defined(XIO_NO_EXCEPTIONS)

  std::exception_ptr ep2 = xio::to_exception_ptr(ec2);
  XIO_CHECK(ep2 != nullptr);
}

void exception_ptr_disposition_test()
{
  using xio::no_error;

  std::exception_ptr ep1;

  XIO_CHECK(ep1 == no_error);
  XIO_CHECK(no_error == ep1);
  XIO_CHECK(!(ep1 != no_error));
  XIO_CHECK(!(no_error != ep1));

  std::exception_ptr ep2 = xio::to_exception_ptr(ep1);
  XIO_CHECK(ep1 == nullptr);

  std::exception_ptr ep3 = std::make_exception_ptr(
      std::system_error(xio::error::eof));

  XIO_CHECK(!(ep3 == no_error));
  XIO_CHECK(!(no_error == ep3));
  XIO_CHECK(ep3 != no_error);
  XIO_CHECK(no_error != ep3);

#if !defined(XIO_NO_EXCEPTIONS)
  bool caught;
  try
  {
    xio::throw_exception(ep3);
    caught = false;
  }
  catch (const std::system_error& ex)
  {
    caught = true;
    XIO_CHECK(ex.code() == xio::error::eof);
  }
  XIO_CHECK(caught);
#endif // !defined(XIO_NO_EXCEPTIONS)

  std::exception_ptr ep4 = xio::to_exception_ptr(ep3);
  XIO_CHECK(ep4 != nullptr);
}

XIO_TEST_SUITE
(
  "disposition",
  XIO_TEST_CASE(no_error_test)
  XIO_TEST_CASE(error_code_disposition_test)
  XIO_TEST_CASE(exception_ptr_disposition_test)
)
