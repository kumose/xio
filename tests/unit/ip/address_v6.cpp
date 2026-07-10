//
// address_v6.cpp
// ~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//



// Test that header file is self-contained.
#include <xio/ip/address_v6.h>

#include "../unit_test.hpp"
#include <sstream>

//------------------------------------------------------------------------------

// ip_address_v6_compile test
// ~~~~~~~~~~~~~~~~~~~~~~~~~~
// The following test checks that all public member functions on the class
// ip::address_v6 compile and link correctly. Runtime failures are ignored.

namespace ip_address_v6_compile {

void test()
{
  using namespace xio;
  namespace ip = xio::ip;

  try
  {
    xio::error_code ec;

    // address_v6 constructors.

    ip::address_v6 addr1;
    const ip::address_v6::bytes_type const_bytes_value = { { 0 } };
    ip::address_v6 addr2(const_bytes_value);

    // address_v6 functions.

    unsigned long scope_id = addr1.scope_id();
    addr1.scope_id(scope_id);

    bool b = addr1.is_unspecified();
    (void)b;

    b = addr1.is_loopback();
    (void)b;

    b = addr1.is_multicast();
    (void)b;

    b = addr1.is_link_local();
    (void)b;

    b = addr1.is_site_local();
    (void)b;

    b = addr1.is_v4_mapped();
    (void)b;

    b = addr1.is_multicast_node_local();
    (void)b;

    b = addr1.is_multicast_link_local();
    (void)b;

    b = addr1.is_multicast_site_local();
    (void)b;

    b = addr1.is_multicast_org_local();
    (void)b;

    b = addr1.is_multicast_global();
    (void)b;

    ip::address_v6::bytes_type bytes_value = addr1.to_bytes();
    (void)bytes_value;

    std::string string_value = addr1.to_string();

    // address_v6 static functions.

    addr1 = ip::address_v6::any();

    addr1 = ip::address_v6::loopback();

    // address_v6 comparisons.

    b = (addr1 == addr2);
    (void)b;

    b = (addr1 != addr2);
    (void)b;

    b = (addr1 < addr2);
    (void)b;

    b = (addr1 > addr2);
    (void)b;

    b = (addr1 <= addr2);
    (void)b;

    b = (addr1 >= addr2);
    (void)b;

    // address_v6 creation functions.

    addr1 = ip::make_address_v6(const_bytes_value, scope_id);
    addr1 = ip::make_address_v6("0::0");
    addr1 = ip::make_address_v6("0::0", ec);
    addr1 = ip::make_address_v6(string_value);
    addr1 = ip::make_address_v6(string_value, ec);
#if defined(XIO_HAS_STRING_VIEW)
# if defined(XIO_HAS_STD_STRING_VIEW)
    std::string_view string_view_value("0::0");
# else // defined(XIO_HAS_STD_EXPERIMENTAL_STRING_VIEW)
    std::experimental::std::string_view string_view_value("0::0");
# endif // defined(XIO_HAS_STD_EXPERIMENTAL_STRING_VIEW)
    addr1 = ip::make_address_v6(string_view_value);
    addr1 = ip::make_address_v6(string_view_value, ec);
#endif // defined(XIO_HAS_STRING_VIEW)

    // address_v6 IPv4-mapped conversion.
    ip::address_v4 addr3;
    addr1 = ip::make_address_v6(ip::v4_mapped, addr3);
    addr3 = ip::make_address_v4(ip::v4_mapped, addr1);

    // address_v6 I/O.

    std::ostringstream os;
    os << addr1;

#if !defined(BOOST_NO_STD_WSTREAMBUF)
    std::wostringstream wos;
    wos << addr1;
#endif // !defined(BOOST_NO_STD_WSTREAMBUF)

#if defined(XIO_HAS_STD_HASH)
    std::size_t hash1 = std::hash<ip::address_v6>()(addr1);
    (void)hash1;
#endif // defined(XIO_HAS_STD_HASH)
  }
  catch (std::exception&)
  {
  }
}

} // namespace ip_address_v6_compile

//------------------------------------------------------------------------------

// ip_address_v6_runtime test
// ~~~~~~~~~~~~~~~~~~~~~~~~~~
// The following test checks that the various public member functions meet the
// necessary postconditions.

namespace ip_address_v6_runtime {

void test()
{
  using xio::ip::address_v6;

  address_v6 a1;
  XIO_CHECK(a1.is_unspecified());
  XIO_CHECK(a1.scope_id() == 0);

  address_v6::bytes_type b1 = {{ 1, 2, 3, 4, 5,
    6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 }};
  address_v6 a2(b1, 12345);
  XIO_CHECK(a2.to_bytes()[0] == 1);
  XIO_CHECK(a2.to_bytes()[1] == 2);
  XIO_CHECK(a2.to_bytes()[2] == 3);
  XIO_CHECK(a2.to_bytes()[3] == 4);
  XIO_CHECK(a2.to_bytes()[4] == 5);
  XIO_CHECK(a2.to_bytes()[5] == 6);
  XIO_CHECK(a2.to_bytes()[6] == 7);
  XIO_CHECK(a2.to_bytes()[7] == 8);
  XIO_CHECK(a2.to_bytes()[8] == 9);
  XIO_CHECK(a2.to_bytes()[9] == 10);
  XIO_CHECK(a2.to_bytes()[10] == 11);
  XIO_CHECK(a2.to_bytes()[11] == 12);
  XIO_CHECK(a2.to_bytes()[12] == 13);
  XIO_CHECK(a2.to_bytes()[13] == 14);
  XIO_CHECK(a2.to_bytes()[14] == 15);
  XIO_CHECK(a2.to_bytes()[15] == 16);
  XIO_CHECK(a2.scope_id() == 12345);

  address_v6 a3;
  a3.scope_id(12345);
  XIO_CHECK(a3.scope_id() == 12345);

  address_v6 unspecified_address;
  address_v6::bytes_type loopback_bytes = {{ 0 }};
  loopback_bytes[15] = 1;
  address_v6 loopback_address(loopback_bytes);
  address_v6::bytes_type link_local_bytes = {{ 0xFE, 0x80, 1 }};
  address_v6 link_local_address(link_local_bytes);
  address_v6::bytes_type site_local_bytes = {{ 0xFE, 0xC0, 1 }};
  address_v6 site_local_address(site_local_bytes);
  address_v6::bytes_type v4_mapped_bytes = {{ 0 }};
  v4_mapped_bytes[10] = 0xFF, v4_mapped_bytes[11] = 0xFF;
  v4_mapped_bytes[12] = 1, v4_mapped_bytes[13] = 2;
  v4_mapped_bytes[14] = 3, v4_mapped_bytes[15] = 4;
  address_v6 v4_mapped_address(v4_mapped_bytes);
  address_v6::bytes_type v4_compat_bytes = {{ 0 }};
  v4_compat_bytes[12] = 1, v4_compat_bytes[13] = 2;
  v4_compat_bytes[14] = 3, v4_compat_bytes[15] = 4;
  address_v6 v4_compat_address(v4_compat_bytes);
  address_v6::bytes_type mcast_global_bytes = {{ 0xFF, 0x0E, 1 }};
  address_v6 mcast_global_address(mcast_global_bytes);
  address_v6::bytes_type mcast_link_local_bytes = {{ 0xFF, 0x02, 1 }};
  address_v6 mcast_link_local_address(mcast_link_local_bytes);
  address_v6::bytes_type mcast_node_local_bytes = {{ 0xFF, 0x01, 1 }};
  address_v6 mcast_node_local_address(mcast_node_local_bytes);
  address_v6::bytes_type mcast_org_local_bytes = {{ 0xFF, 0x08, 1 }};
  address_v6 mcast_org_local_address(mcast_org_local_bytes);
  address_v6::bytes_type mcast_site_local_bytes = {{ 0xFF, 0x05, 1 }};
  address_v6 mcast_site_local_address(mcast_site_local_bytes);

  XIO_CHECK(!unspecified_address.is_loopback());
  XIO_CHECK(loopback_address.is_loopback());
  XIO_CHECK(!link_local_address.is_loopback());
  XIO_CHECK(!site_local_address.is_loopback());
  XIO_CHECK(!v4_mapped_address.is_loopback());
  XIO_CHECK(!v4_compat_address.is_loopback());
  XIO_CHECK(!mcast_global_address.is_loopback());
  XIO_CHECK(!mcast_link_local_address.is_loopback());
  XIO_CHECK(!mcast_node_local_address.is_loopback());
  XIO_CHECK(!mcast_org_local_address.is_loopback());
  XIO_CHECK(!mcast_site_local_address.is_loopback());

  XIO_CHECK(unspecified_address.is_unspecified());
  XIO_CHECK(!loopback_address.is_unspecified());
  XIO_CHECK(!link_local_address.is_unspecified());
  XIO_CHECK(!site_local_address.is_unspecified());
  XIO_CHECK(!v4_mapped_address.is_unspecified());
  XIO_CHECK(!v4_compat_address.is_unspecified());
  XIO_CHECK(!mcast_global_address.is_unspecified());
  XIO_CHECK(!mcast_link_local_address.is_unspecified());
  XIO_CHECK(!mcast_node_local_address.is_unspecified());
  XIO_CHECK(!mcast_org_local_address.is_unspecified());
  XIO_CHECK(!mcast_site_local_address.is_unspecified());

  XIO_CHECK(!unspecified_address.is_link_local());
  XIO_CHECK(!loopback_address.is_link_local());
  XIO_CHECK(link_local_address.is_link_local());
  XIO_CHECK(!site_local_address.is_link_local());
  XIO_CHECK(!v4_mapped_address.is_link_local());
  XIO_CHECK(!v4_compat_address.is_link_local());
  XIO_CHECK(!mcast_global_address.is_link_local());
  XIO_CHECK(!mcast_link_local_address.is_link_local());
  XIO_CHECK(!mcast_node_local_address.is_link_local());
  XIO_CHECK(!mcast_org_local_address.is_link_local());
  XIO_CHECK(!mcast_site_local_address.is_link_local());

  XIO_CHECK(!unspecified_address.is_site_local());
  XIO_CHECK(!loopback_address.is_site_local());
  XIO_CHECK(!link_local_address.is_site_local());
  XIO_CHECK(site_local_address.is_site_local());
  XIO_CHECK(!v4_mapped_address.is_site_local());
  XIO_CHECK(!v4_compat_address.is_site_local());
  XIO_CHECK(!mcast_global_address.is_site_local());
  XIO_CHECK(!mcast_link_local_address.is_site_local());
  XIO_CHECK(!mcast_node_local_address.is_site_local());
  XIO_CHECK(!mcast_org_local_address.is_site_local());
  XIO_CHECK(!mcast_site_local_address.is_site_local());

  XIO_CHECK(!unspecified_address.is_v4_mapped());
  XIO_CHECK(!loopback_address.is_v4_mapped());
  XIO_CHECK(!link_local_address.is_v4_mapped());
  XIO_CHECK(!site_local_address.is_v4_mapped());
  XIO_CHECK(v4_mapped_address.is_v4_mapped());
  XIO_CHECK(!v4_compat_address.is_v4_mapped());
  XIO_CHECK(!mcast_global_address.is_v4_mapped());
  XIO_CHECK(!mcast_link_local_address.is_v4_mapped());
  XIO_CHECK(!mcast_node_local_address.is_v4_mapped());
  XIO_CHECK(!mcast_org_local_address.is_v4_mapped());
  XIO_CHECK(!mcast_site_local_address.is_v4_mapped());

  XIO_CHECK(!unspecified_address.is_multicast());
  XIO_CHECK(!loopback_address.is_multicast());
  XIO_CHECK(!link_local_address.is_multicast());
  XIO_CHECK(!site_local_address.is_multicast());
  XIO_CHECK(!v4_mapped_address.is_multicast());
  XIO_CHECK(!v4_compat_address.is_multicast());
  XIO_CHECK(mcast_global_address.is_multicast());
  XIO_CHECK(mcast_link_local_address.is_multicast());
  XIO_CHECK(mcast_node_local_address.is_multicast());
  XIO_CHECK(mcast_org_local_address.is_multicast());
  XIO_CHECK(mcast_site_local_address.is_multicast());

  XIO_CHECK(!unspecified_address.is_multicast_global());
  XIO_CHECK(!loopback_address.is_multicast_global());
  XIO_CHECK(!link_local_address.is_multicast_global());
  XIO_CHECK(!site_local_address.is_multicast_global());
  XIO_CHECK(!v4_mapped_address.is_multicast_global());
  XIO_CHECK(!v4_compat_address.is_multicast_global());
  XIO_CHECK(mcast_global_address.is_multicast_global());
  XIO_CHECK(!mcast_link_local_address.is_multicast_global());
  XIO_CHECK(!mcast_node_local_address.is_multicast_global());
  XIO_CHECK(!mcast_org_local_address.is_multicast_global());
  XIO_CHECK(!mcast_site_local_address.is_multicast_global());

  XIO_CHECK(!unspecified_address.is_multicast_link_local());
  XIO_CHECK(!loopback_address.is_multicast_link_local());
  XIO_CHECK(!link_local_address.is_multicast_link_local());
  XIO_CHECK(!site_local_address.is_multicast_link_local());
  XIO_CHECK(!v4_mapped_address.is_multicast_link_local());
  XIO_CHECK(!v4_compat_address.is_multicast_link_local());
  XIO_CHECK(!mcast_global_address.is_multicast_link_local());
  XIO_CHECK(mcast_link_local_address.is_multicast_link_local());
  XIO_CHECK(!mcast_node_local_address.is_multicast_link_local());
  XIO_CHECK(!mcast_org_local_address.is_multicast_link_local());
  XIO_CHECK(!mcast_site_local_address.is_multicast_link_local());

  XIO_CHECK(!unspecified_address.is_multicast_node_local());
  XIO_CHECK(!loopback_address.is_multicast_node_local());
  XIO_CHECK(!link_local_address.is_multicast_node_local());
  XIO_CHECK(!site_local_address.is_multicast_node_local());
  XIO_CHECK(!v4_mapped_address.is_multicast_node_local());
  XIO_CHECK(!v4_compat_address.is_multicast_node_local());
  XIO_CHECK(!mcast_global_address.is_multicast_node_local());
  XIO_CHECK(!mcast_link_local_address.is_multicast_node_local());
  XIO_CHECK(mcast_node_local_address.is_multicast_node_local());
  XIO_CHECK(!mcast_org_local_address.is_multicast_node_local());
  XIO_CHECK(!mcast_site_local_address.is_multicast_node_local());

  XIO_CHECK(!unspecified_address.is_multicast_org_local());
  XIO_CHECK(!loopback_address.is_multicast_org_local());
  XIO_CHECK(!link_local_address.is_multicast_org_local());
  XIO_CHECK(!site_local_address.is_multicast_org_local());
  XIO_CHECK(!v4_mapped_address.is_multicast_org_local());
  XIO_CHECK(!v4_compat_address.is_multicast_org_local());
  XIO_CHECK(!mcast_global_address.is_multicast_org_local());
  XIO_CHECK(!mcast_link_local_address.is_multicast_org_local());
  XIO_CHECK(!mcast_node_local_address.is_multicast_org_local());
  XIO_CHECK(mcast_org_local_address.is_multicast_org_local());
  XIO_CHECK(!mcast_site_local_address.is_multicast_org_local());

  XIO_CHECK(!unspecified_address.is_multicast_site_local());
  XIO_CHECK(!loopback_address.is_multicast_site_local());
  XIO_CHECK(!link_local_address.is_multicast_site_local());
  XIO_CHECK(!site_local_address.is_multicast_site_local());
  XIO_CHECK(!v4_mapped_address.is_multicast_site_local());
  XIO_CHECK(!v4_compat_address.is_multicast_site_local());
  XIO_CHECK(!mcast_global_address.is_multicast_site_local());
  XIO_CHECK(!mcast_link_local_address.is_multicast_site_local());
  XIO_CHECK(!mcast_node_local_address.is_multicast_site_local());
  XIO_CHECK(!mcast_org_local_address.is_multicast_site_local());
  XIO_CHECK(mcast_site_local_address.is_multicast_site_local());

  XIO_CHECK(address_v6::loopback().is_loopback());
}

} // namespace ip_address_v6_runtime

//------------------------------------------------------------------------------

XIO_TEST_SUITE
(
  "ip/address_v6",
  XIO_COMPILE_TEST_CASE(ip_address_v6_compile::test)
  XIO_TEST_CASE(ip_address_v6_runtime::test)
)
