//
// v6_only.cpp
// ~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//



// Test that header file is self-contained.
#include <xio/ip/v6_only.h>

#include <xio/io_context.h>
#include <xio/ip/tcp.h>
#include <xio/ip/udp.h>
#include "../unit_test.hpp"

//------------------------------------------------------------------------------

// ip_v6_only_compile test
// ~~~~~~~~~~~~~~~~~~~~~~~
// The following test checks that the ip::v6_only socket option compiles and
// link correctly. Runtime failures are ignored.

namespace ip_v6_only_compile {

void test()
{
  using namespace xio;
  namespace ip = xio::ip;

  try
  {
    io_context ioc;
    ip::udp::socket sock(ioc);

    // v6_only class.

    ip::v6_only v6_only1(true);
    sock.set_option(v6_only1);
    ip::v6_only v6_only2;
    sock.get_option(v6_only2);
    v6_only1 = true;
    (void)static_cast<bool>(v6_only1);
    (void)static_cast<bool>(!v6_only1);
    (void)static_cast<bool>(v6_only1.value());
  }
  catch (std::exception&)
  {
  }
}

} // namespace ip_v6_only_compile

//------------------------------------------------------------------------------

// ip_v6_only_runtime test
// ~~~~~~~~~~~~~~~~~~~~~~~
// The following test checks the runtime operation of the ip::v6_only socket
// option.

namespace ip_v6_only_runtime {

void test()
{
  using namespace xio;
  namespace ip = xio::ip;

  io_context ioc;
  xio::error_code ec;

  ip::tcp::endpoint ep_v6(ip::address_v6::loopback(), 0);
  ip::tcp::acceptor acceptor_v6(ioc);
  acceptor_v6.open(ep_v6.protocol(), ec);
  acceptor_v6.bind(ep_v6, ec);
  bool have_v6 = !ec;
  acceptor_v6.close(ec);
  acceptor_v6.open(ep_v6.protocol(), ec);

  if (have_v6)
  {
    ip::v6_only v6_only1;
    acceptor_v6.get_option(v6_only1, ec);
    XIO_CHECK(!ec);
    bool have_dual_stack = !v6_only1.value();

    if (have_dual_stack)
    {
      ip::v6_only v6_only2(false);
      XIO_CHECK(!v6_only2.value());
      XIO_CHECK(!static_cast<bool>(v6_only2));
      XIO_CHECK(!v6_only2);
      acceptor_v6.set_option(v6_only2, ec);
      XIO_CHECK(!ec);

      ip::v6_only v6_only3;
      acceptor_v6.get_option(v6_only3, ec);
      XIO_CHECK(!ec);
      XIO_CHECK(!v6_only3.value());
      XIO_CHECK(!static_cast<bool>(v6_only3));
      XIO_CHECK(!v6_only3);

      ip::v6_only v6_only4(true);
      XIO_CHECK(v6_only4.value());
      XIO_CHECK(static_cast<bool>(v6_only4));
      XIO_CHECK(!!v6_only4);
      acceptor_v6.set_option(v6_only4, ec);
      XIO_CHECK(!ec);

      ip::v6_only v6_only5;
      acceptor_v6.get_option(v6_only5, ec);
      XIO_CHECK(!ec);
      XIO_CHECK(v6_only5.value());
      XIO_CHECK(static_cast<bool>(v6_only5));
      XIO_CHECK(!!v6_only5);
    }
  }
}

} // namespace ip_v6_only_runtime

//------------------------------------------------------------------------------

XIO_TEST_SUITE
(
  "ip/v6_only",
  XIO_COMPILE_TEST_CASE(ip_v6_only_compile::test)
  XIO_TEST_CASE(ip_v6_only_runtime::test)
)
