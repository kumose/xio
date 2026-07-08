//
// network_v6.cpp
// ~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
// Copyright (c) 2014 Oliver Kowalke (oliver dot kowalke at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//



// Test that header file is self-contained.
#include <xio/ip/network_v6.h>

#include "../unit_test.hpp"
#include <sstream>

//------------------------------------------------------------------------------

// ip_network_v6_compile test
// ~~~~~~~~~~~~~~~~~~~~~~~~~~
// The following test checks that all public member functions on the class
// ip::network_v6 compile and link correctly. Runtime failures are ignored.

namespace ip_network_v6_compile {

void test()
{
  using namespace xio;
  namespace ip = xio::ip;

  try
  {
    xio::error_code ec;

    // network_v6 constructors.

    ip::network_v6 net1(ip::make_address_v6("2001:370::10:7344"), 64);
    ip::network_v6 net2(ip::make_address_v6("2001:370::10:7345"), 96);

    // network_v6 functions.

    ip::address_v6 addr1 = net1.address();
    (void)addr1;

    unsigned short prefix_len = net1.prefix_length();
    (void)prefix_len;

    ip::address_v6 addr3 = net1.network();
    (void)addr3;

    ip::address_v6_range hosts = net1.hosts();
    (void)hosts;

    ip::network_v6 net3 = net1.canonical();
    (void)net3;

    bool b1 = net1.is_host();
    (void)b1;

    bool b2 = net1.is_subnet_of(net2);
    (void)b2;

    std::string s1 = net1.to_string();
    (void)s1;

    std::string s2 = net1.to_string(ec);
    (void)s2;

    // network_v6 comparisons.

    bool b3 = (net1 == net2);
    (void)b3;

    bool b4 = (net1 != net2);
    (void)b4;

    // network_v6 creation functions.

    net1 = ip::make_network_v6(ip::address_v6(), 24);
    net1 = ip::make_network_v6("10.0.0.0/8");
    net1 = ip::make_network_v6("10.0.0.0/8", ec);
    net1 = ip::make_network_v6(s1);
    net1 = ip::make_network_v6(s1, ec);
#if defined(XIO_HAS_STRING_VIEW)
# if defined(XIO_HAS_STD_STRING_VIEW)
    std::string_view string_view_value("0::0/8");
# elif defined(XIO_HAS_STD_EXPERIMENTAL_STRING_VIEW)
    std::experimental::std::string_view string_view_value("0::0/8");
# endif // defined(XIO_HAS_STD_EXPERIMENTAL_STRING_VIEW)
    net1 = ip::make_network_v6(string_view_value);
    net1 = ip::make_network_v6(string_view_value, ec);
#endif // defined(XIO_STD_STRING_VIEW)

    // network_v6 I/O.

    std::ostringstream os;
    os << net1;

#if !defined(BOOST_NO_STD_WSTREAMBUF)
    std::wostringstream wos;
    wos << net1;
#endif // !defined(BOOST_NO_STD_WSTREAMBUF)
  }
  catch (std::exception&)
  {
  }
}

} // namespace ip_network_v6_compile

//------------------------------------------------------------------------------

// ip_network_v6_runtime test
// ~~~~~~~~~~~~~~~~~~~~~~~~~~
// The following test checks that the various public member functions meet the
// necessary postconditions.

namespace ip_network_v6_runtime {

void test()
{
  using xio::ip::address_v6;
  using xio::ip::make_address_v6;
  using xio::ip::network_v6;
  using xio::ip::make_network_v6;

  address_v6 addr = make_address_v6("2001:370::10:7344");

  std::string msg;
  try
  {
    make_network_v6(addr, 129);
  }
  catch(std::out_of_range& ex)
  {
    msg = ex.what();
  }
  XIO_CHECK(msg == std::string("prefix length too large"));

  // construct address range from address and prefix length
  XIO_CHECK(network_v6(make_address_v6("2001:370::10:7344"), 128).network() == make_address_v6("2001:370::10:7344"));
  XIO_CHECK(network_v6(make_address_v6("2001:370::10:7344"), 64).network() == make_address_v6("2001:370::"));
  XIO_CHECK(network_v6(make_address_v6("2001:370::10:7344"), 27).network() == make_address_v6("2001:360::"));

  // construct address range from string in CIDR notation
  XIO_CHECK(make_network_v6("2001:370::10:7344/128").network() == make_address_v6("2001:370::10:7344"));
  XIO_CHECK(make_network_v6("2001:370::10:7344/64").network() == make_address_v6("2001:370::"));
  XIO_CHECK(make_network_v6("2001:370::10:7344/27").network() == make_address_v6("2001:360::"));

  // construct network from invalid string
  xio::error_code ec;
  make_network_v6("a:b/24", ec);
  XIO_CHECK(!!ec);
  make_network_v6("2001:370::10:7344/129", ec);
  XIO_CHECK(!!ec);
  make_network_v6("2001:370::10:7344/-1", ec);
  XIO_CHECK(!!ec);
  make_network_v6("2001:370::10:7344/", ec);
  XIO_CHECK(!!ec);
  make_network_v6("2001:370::10:7344", ec);
  XIO_CHECK(!!ec);

  // prefix length
  XIO_CHECK(make_network_v6("2001:370::10:7344/128").prefix_length() == 128);
  XIO_CHECK(network_v6(make_address_v6("2001:370::10:7344"), 27).prefix_length() == 27);

  // to string
  std::string a("2001:370::10:7344/64");
  XIO_CHECK(make_network_v6(a.c_str()).to_string() == a);
  XIO_CHECK(network_v6(make_address_v6("2001:370::10:7344"), 27).to_string() == std::string("2001:370::10:7344/27"));

  // return host part
  XIO_CHECK(make_network_v6("2001:370::10:7344/64").address() == make_address_v6("2001:370::10:7344"));
  XIO_CHECK(make_network_v6("2001:370::10:7344/27").address().to_string() == "2001:370::10:7344");

  // return network in CIDR notation
  XIO_CHECK(make_network_v6("2001:370::10:7344/27").canonical().to_string() == "2001:360::/27");

  // is host
  XIO_CHECK(make_network_v6("2001:370::10:7344/128").is_host());
  XIO_CHECK(!make_network_v6("2001:370::10:7344/127").is_host());

  // is real subnet of
  XIO_CHECK(make_network_v6("2001:370::10:3744/64").is_subnet_of(make_network_v6("2001:370::/16")));
  XIO_CHECK(make_network_v6("2001:370::/64").is_subnet_of(make_network_v6("2001:370::/16")));
  XIO_CHECK(make_network_v6("2001:0db8:85a3::/64").is_subnet_of(make_network_v6("2001:0d00::/24")));

  XIO_CHECK(!make_network_v6("2001:370::10:3744/128").is_subnet_of(make_network_v6("2001:370::10:3744/128")));
  XIO_CHECK(make_network_v6("2001:0db8:85a3::/64").is_subnet_of(make_network_v6("2001:0dc0::/24")));

  network_v6 r(make_network_v6("2001:370::/64"));
  XIO_CHECK(!r.is_subnet_of(r));

  network_v6 net12(make_network_v6("2001:370::10:7344/64"));
  network_v6 net13(make_network_v6("2001:0db8::/127"));
  network_v6 net14(make_network_v6("2001:0db8::/125"));
  network_v6 net15(make_network_v6("2001:0db8::/119"));

  // network
  XIO_CHECK(net12.network() == make_address_v6("2001:370::"));
  XIO_CHECK(net13.network() == make_address_v6("2001:0db8::"));
  XIO_CHECK(net14.network() == make_address_v6("2001:0db8::"));
  XIO_CHECK(net15.network() == make_address_v6("2001:0db8::"));

  // iterator
  //XIO_CHECK(std::distance(net12.hosts().begin(),net12.hosts().end()) == 18446744073709552000);
  XIO_CHECK(std::distance(net13.hosts().begin(),net13.hosts().end()) == 2);
  XIO_CHECK(std::distance(net14.hosts().begin(),net14.hosts().end()) == 8);
  XIO_CHECK(std::distance(net15.hosts().begin(),net15.hosts().end()) == 512);
  XIO_CHECK(*net12.hosts().begin() == make_address_v6("2001:0370::"));
  XIO_CHECK(net12.hosts().end() != net12.hosts().find(make_address_v6("2001:0370::ffff:ffff:ffff:ffff")));
  XIO_CHECK(*net13.hosts().begin() == make_address_v6("2001:0db8::"));
  XIO_CHECK(net13.hosts().end() != net13.hosts().find(make_address_v6("2001:0db8::1")));
  XIO_CHECK(net13.hosts().end() == net13.hosts().find(make_address_v6("2001:0db8::2")));
  XIO_CHECK(*net14.hosts().begin() == make_address_v6("2001:0db8::"));
  XIO_CHECK(net14.hosts().end() != net14.hosts().find(make_address_v6("2001:0db8::7")));
  XIO_CHECK(net14.hosts().end() == net14.hosts().find(make_address_v6("2001:0db8::8")));
  XIO_CHECK(*net15.hosts().begin() == make_address_v6("2001:0db8::"));
  XIO_CHECK(net15.hosts().end() != net15.hosts().find(make_address_v6("2001:0db8::01ff")));
  XIO_CHECK(net15.hosts().end() == net15.hosts().find(make_address_v6("2001:0db8::0200")));
}

} // namespace ip_network_v6_runtime

//------------------------------------------------------------------------------

XIO_TEST_SUITE
(
  "ip/network_v6",
  XIO_COMPILE_TEST_CASE(ip_network_v6_compile::test)
  XIO_TEST_CASE(ip_network_v6_runtime::test)
)
