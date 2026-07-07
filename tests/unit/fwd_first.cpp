//
// fwd_first.cpp
// ~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//



// Test that header file is self-contained.
#include <xio/fwd.h>

// Test that forward declarations don't conflict with full declarations.
#include "asio.hpp"

#include "unit_test.hpp"

ASIO_TEST_SUITE
(
  "fwd_first",
  ASIO_TEST_CASE(null_test)
)
