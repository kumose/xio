//
// config.cpp
// ~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//



// Test that header file is self-contained.
#include <xio/config.h>

#include <xio/io_context.h>
#include <cstdlib>
#include "unit_test.hpp"

void config_from_string_test()
{
  xio::io_context ctx1(
      xio::config_from_string(
        "scheduler.concurrency_hint=123\n"
        " scheduler.locking = 1 \n"
        "# comment\n"
        "garbage\n"
        "reactor.registration_locking= 0 # comment\n"
        "reactor.io_locking=1"));

  xio::config cfg1(ctx1);
  XIO_CHECK(cfg1.get("scheduler", "concurrency_hint", 0) == 123);
  XIO_CHECK(cfg1.get("scheduler", "locking", false) == true);
  XIO_CHECK(cfg1.get("reactor", "registration_locking", true) == false);
  XIO_CHECK(cfg1.get("reactor", "io_locking", false) == true);

  xio::io_context ctx2(
      xio::config_from_string(
        "prefix.scheduler.concurrency_hint=456\n"
        " prefix.scheduler.locking = 1 \n"
        "# comment\n"
        "garbage\n"
        "prefix.reactor.registration_locking= 0 # comment\n"
        "prefix.reactor.io_locking=1",
        "prefix"));

  xio::config cfg2(ctx2);
  XIO_CHECK(cfg2.get("scheduler", "concurrency_hint", 0) == 456);
  XIO_CHECK(cfg2.get("scheduler", "locking", false) == true);
  XIO_CHECK(cfg2.get("reactor", "registration_locking", true) == false);
  XIO_CHECK(cfg2.get("reactor", "io_locking", false) == true);
}

void config_from_concurrency_hint_test()
{
  xio::io_context ctx0;

  xio::config cfg0(ctx0);
  XIO_CHECK(cfg0.get("scheduler", "concurrency_hint", 0) == -1);
  XIO_CHECK(cfg0.get("scheduler", "locking", false) == true);
  XIO_CHECK(cfg0.get("reactor", "registration_locking", true) == true);
  XIO_CHECK(cfg0.get("reactor", "io_locking", false) == true);

  xio::io_context ctx1(0);

  xio::config cfg1(ctx1);
  XIO_CHECK(cfg1.get("scheduler", "concurrency_hint", 0) == 0);
  XIO_CHECK(cfg1.get("scheduler", "locking", false) == true);
  XIO_CHECK(cfg1.get("reactor", "registration_locking", true) == true);
  XIO_CHECK(cfg1.get("reactor", "io_locking", false) == true);

  xio::io_context ctx2(1);

  xio::config cfg2(ctx2);
  XIO_CHECK(cfg2.get("scheduler", "concurrency_hint", 0) == 1);
  XIO_CHECK(cfg2.get("scheduler", "locking", false) == true);
  XIO_CHECK(cfg2.get("reactor", "registration_locking", true) == true);
  XIO_CHECK(cfg2.get("reactor", "io_locking", false) == true);

  xio::io_context ctx3(42);

  xio::config cfg3(ctx3);
  XIO_CHECK(cfg3.get("scheduler", "concurrency_hint", 0) == 42);
  XIO_CHECK(cfg3.get("scheduler", "locking", false) == true);
  XIO_CHECK(cfg3.get("reactor", "registration_locking", true) == true);
  XIO_CHECK(cfg3.get("reactor", "io_locking", false) == true);

  xio::io_context ctx4(XIO_CONCURRENCY_HINT_UNSAFE);

  xio::config cfg4(ctx4);
  XIO_CHECK(cfg4.get("scheduler", "concurrency_hint", 0) == 1);
  XIO_CHECK(cfg4.get("scheduler", "locking", false) == false);
  XIO_CHECK(cfg4.get("reactor", "registration_locking", true) == false);
  XIO_CHECK(cfg4.get("reactor", "io_locking", false) == false);

  xio::io_context ctx5(XIO_CONCURRENCY_HINT_UNSAFE_IO);

  xio::config cfg5(ctx5);
  XIO_CHECK(cfg5.get("scheduler", "concurrency_hint", 0) == 1);
  XIO_CHECK(cfg5.get("scheduler", "locking", false) == true);
  XIO_CHECK(cfg5.get("reactor", "registration_locking", true) == true);
  XIO_CHECK(cfg5.get("reactor", "io_locking", false) == false);

  xio::io_context ctx6(XIO_CONCURRENCY_HINT_SAFE);

  xio::config cfg6(ctx6);
  XIO_CHECK(cfg6.get("scheduler", "concurrency_hint", 0) == -1);
  XIO_CHECK(cfg6.get("scheduler", "locking", false) == true);
  XIO_CHECK(cfg6.get("reactor", "registration_locking", true) == true);
  XIO_CHECK(cfg6.get("reactor", "io_locking", false) == true);
}

XIO_TEST_SUITE
(
  "config",
  XIO_TEST_CASE(config_from_string_test)
  XIO_TEST_CASE(config_from_concurrency_hint_test)
)
