//
// error.cpp
// ~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//



// Test that header file is self-contained.
#include <xio/error.h>

#include <sstream>
#include "unit_test.hpp"

void test_error_code(const xio::error_code& code)
{
  xio::error_code error(code);
  ASIO_CHECK(code == error);

  ASIO_CHECK(!code || error);
  ASIO_CHECK(!code || !!error);

  xio::error_code error2(error);
  ASIO_CHECK(error == error2);
  ASIO_CHECK(!(error != error2));

  xio::error_code error3;
  error3 = error;
  ASIO_CHECK(error == error3);
  ASIO_CHECK(!(error != error3));

  std::ostringstream os;
  os << error;
  ASIO_CHECK(!os.str().empty());
}

void error_test()
{
  test_error_code(xio::error::access_denied);
  test_error_code(xio::error::address_family_not_supported);
  test_error_code(xio::error::address_in_use);
  test_error_code(xio::error::already_connected);
  test_error_code(xio::error::already_started);
  test_error_code(xio::error::connection_aborted);
  test_error_code(xio::error::connection_refused);
  test_error_code(xio::error::connection_reset);
  test_error_code(xio::error::bad_descriptor);
  test_error_code(xio::error::eof);
  test_error_code(xio::error::fault);
  test_error_code(xio::error::host_not_found);
  test_error_code(xio::error::host_not_found_try_again);
  test_error_code(xio::error::host_unreachable);
  test_error_code(xio::error::in_progress);
  test_error_code(xio::error::interrupted);
  test_error_code(xio::error::invalid_argument);
  test_error_code(xio::error::message_size);
  test_error_code(xio::error::network_down);
  test_error_code(xio::error::network_reset);
  test_error_code(xio::error::network_unreachable);
  test_error_code(xio::error::no_descriptors);
  test_error_code(xio::error::no_buffer_space);
  test_error_code(xio::error::no_data);
  test_error_code(xio::error::no_memory);
  test_error_code(xio::error::no_permission);
  test_error_code(xio::error::no_protocol_option);
  test_error_code(xio::error::no_recovery);
  test_error_code(xio::error::not_connected);
  test_error_code(xio::error::not_socket);
  test_error_code(xio::error::operation_aborted);
  test_error_code(xio::error::operation_not_supported);
  test_error_code(xio::error::service_not_found);
  test_error_code(xio::error::shut_down);
  test_error_code(xio::error::timed_out);
  test_error_code(xio::error::try_again);
  test_error_code(xio::error::would_block);
}

ASIO_TEST_SUITE
(
  "error",
  ASIO_TEST_CASE(error_test)
)
