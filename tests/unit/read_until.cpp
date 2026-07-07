//
// read_until.cpp
// ~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//



// Test that header file is self-contained.
#include <xio/read_until.h>

#include <cstring>
#include <functional>
#include "archetypes/async_result.hpp"
#include <xio/io_context.h>
#include <xio/post.h>
#include <xio/streambuf.h>
#include "unit_test.hpp"

class test_stream
{
public:
  typedef xio::io_context::executor_type executor_type;

  test_stream(xio::io_context& io_context)
    : io_context_(io_context),
      length_(0),
      position_(0),
      next_read_length_(0)
  {
  }

  executor_type get_executor() noexcept
  {
    return io_context_.get_executor();
  }

  void reset(const void* data, size_t length)
  {
    using namespace std; // For memcpy.

    ASIO_CHECK(length <= max_length);

    memcpy(data_, data, length);
    length_ = length;
    position_ = 0;
    next_read_length_ = length;
  }

  void next_read_length(size_t length)
  {
    next_read_length_ = length;
  }

  template <typename Mutable_Buffers>
  size_t read_some(const Mutable_Buffers& buffers)
  {
    size_t n = xio::buffer_copy(buffers,
        xio::buffer(data_, length_) + position_,
        next_read_length_);
    position_ += n;
    return n;
  }

  template <typename Mutable_Buffers>
  size_t read_some(const Mutable_Buffers& buffers,
      xio::error_code& ec)
  {
    ec = xio::error_code();
    return read_some(buffers);
  }

  template <typename Mutable_Buffers, typename Handler>
  void async_read_some(const Mutable_Buffers& buffers, Handler handler)
  {
    size_t bytes_transferred = read_some(buffers);
    xio::post(get_executor(),
        xio::detail::bind_handler(
          static_cast<Handler&&>(handler),
          xio::error_code(), bytes_transferred));
  }

private:
  xio::io_context& io_context_;
  enum { max_length = 8192 };
  char data_[max_length];
  size_t length_;
  size_t position_;
  size_t next_read_length_;
};

static const char read_data[]
  = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

void test_dynamic_string_read_until_char()
{
  xio::io_context ioc;
  test_stream s(ioc);
  std::string data1, data2;
  xio::dynamic_string_buffer<char, std::string::traits_type,
    std::string::allocator_type> sb1 = xio::dynamic_buffer(data1);
  xio::dynamic_string_buffer<char, std::string::traits_type,
    std::string::allocator_type> sb2 = xio::dynamic_buffer(data2, 25);
  xio::error_code ec;

  s.reset(read_data, sizeof(read_data));
  sb1.consume(sb1.size());
  std::size_t length = xio::read_until(s, sb1, 'Z');
  ASIO_CHECK(length == 26);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb1.consume(sb1.size());
  length = xio::read_until(s, sb1, 'Z');
  ASIO_CHECK(length == 26);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb1.consume(sb1.size());
  length = xio::read_until(s, sb1, 'Z');
  ASIO_CHECK(length == 26);

  s.reset(read_data, sizeof(read_data));
  sb1.consume(sb1.size());
  length = xio::read_until(s, sb1, 'Z', ec);
  ASIO_CHECK(!ec);
  ASIO_CHECK(length == 26);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb1.consume(sb1.size());
  length = xio::read_until(s, sb1, 'Z', ec);
  ASIO_CHECK(!ec);
  ASIO_CHECK(length == 26);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb1.consume(sb1.size());
  length = xio::read_until(s, sb1, 'Z', ec);
  ASIO_CHECK(!ec);
  ASIO_CHECK(length == 26);

  s.reset(read_data, sizeof(read_data));
  sb2.consume(sb2.size());
  length = xio::read_until(s, sb2, 'Z', ec);
  ASIO_CHECK(ec == xio::error::not_found);
  ASIO_CHECK(length == 0);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb2.consume(sb2.size());
  length = xio::read_until(s, sb2, 'Z', ec);
  ASIO_CHECK(ec == xio::error::not_found);
  ASIO_CHECK(length == 0);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb2.consume(sb2.size());
  length = xio::read_until(s, sb2, 'Z', ec);
  ASIO_CHECK(ec == xio::error::not_found);
  ASIO_CHECK(length == 0);

  s.reset(read_data, sizeof(read_data));
  sb2.consume(sb2.size());
  length = xio::read_until(s, sb2, 'Y', ec);
  ASIO_CHECK(!ec);
  ASIO_CHECK(length == 25);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb2.consume(sb2.size());
  length = xio::read_until(s, sb2, 'Y', ec);
  ASIO_CHECK(!ec);
  ASIO_CHECK(length == 25);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb2.consume(sb2.size());
  length = xio::read_until(s, sb2, 'Y', ec);
  ASIO_CHECK(!ec);
  ASIO_CHECK(length == 25);
}

void test_streambuf_read_until_char()
{
#if !defined(ASIO_NO_DYNAMIC_BUFFER_V1)
  xio::io_context ioc;
  test_stream s(ioc);
  xio::streambuf sb1;
  xio::streambuf sb2(25);
  xio::error_code ec;

  s.reset(read_data, sizeof(read_data));
  sb1.consume(sb1.size());
  std::size_t length = xio::read_until(s, sb1, 'Z');
  ASIO_CHECK(length == 26);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb1.consume(sb1.size());
  length = xio::read_until(s, sb1, 'Z');
  ASIO_CHECK(length == 26);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb1.consume(sb1.size());
  length = xio::read_until(s, sb1, 'Z');
  ASIO_CHECK(length == 26);

  s.reset(read_data, sizeof(read_data));
  sb1.consume(sb1.size());
  length = xio::read_until(s, sb1, 'Z', ec);
  ASIO_CHECK(!ec);
  ASIO_CHECK(length == 26);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb1.consume(sb1.size());
  length = xio::read_until(s, sb1, 'Z', ec);
  ASIO_CHECK(!ec);
  ASIO_CHECK(length == 26);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb1.consume(sb1.size());
  length = xio::read_until(s, sb1, 'Z', ec);
  ASIO_CHECK(!ec);
  ASIO_CHECK(length == 26);

  s.reset(read_data, sizeof(read_data));
  sb2.consume(sb2.size());
  length = xio::read_until(s, sb2, 'Z', ec);
  ASIO_CHECK(ec == xio::error::not_found);
  ASIO_CHECK(length == 0);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb2.consume(sb2.size());
  length = xio::read_until(s, sb2, 'Z', ec);
  ASIO_CHECK(ec == xio::error::not_found);
  ASIO_CHECK(length == 0);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb2.consume(sb2.size());
  length = xio::read_until(s, sb2, 'Z', ec);
  ASIO_CHECK(ec == xio::error::not_found);
  ASIO_CHECK(length == 0);

  s.reset(read_data, sizeof(read_data));
  sb2.consume(sb2.size());
  length = xio::read_until(s, sb2, 'Y', ec);
  ASIO_CHECK(!ec);
  ASIO_CHECK(length == 25);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb2.consume(sb2.size());
  length = xio::read_until(s, sb2, 'Y', ec);
  ASIO_CHECK(!ec);
  ASIO_CHECK(length == 25);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb2.consume(sb2.size());
  length = xio::read_until(s, sb2, 'Y', ec);
  ASIO_CHECK(!ec);
  ASIO_CHECK(length == 25);
#endif // !defined(ASIO_NO_DYNAMIC_BUFFER_V1)
}

void test_dynamic_string_read_until_string()
{
  xio::io_context ioc;
  test_stream s(ioc);
  std::string data1, data2;
  xio::dynamic_string_buffer<char, std::string::traits_type,
    std::string::allocator_type> sb1 = xio::dynamic_buffer(data1);
  xio::dynamic_string_buffer<char, std::string::traits_type,
    std::string::allocator_type> sb2 = xio::dynamic_buffer(data2, 25);
  xio::error_code ec;

  s.reset(read_data, sizeof(read_data));
  sb1.consume(sb1.size());
  std::size_t length = xio::read_until(s, sb1, "XYZ");
  ASIO_CHECK(length == 26);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb1.consume(sb1.size());
  length = xio::read_until(s, sb1, "XYZ");
  ASIO_CHECK(length == 26);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb1.consume(sb1.size());
  length = xio::read_until(s, sb1, "XYZ");
  ASIO_CHECK(length == 26);

  s.reset(read_data, sizeof(read_data));
  sb1.consume(sb1.size());
  length = xio::read_until(s, sb1, "XYZ", ec);
  ASIO_CHECK(!ec);
  ASIO_CHECK(length == 26);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb1.consume(sb1.size());
  length = xio::read_until(s, sb1, "XYZ", ec);
  ASIO_CHECK(!ec);
  ASIO_CHECK(length == 26);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb1.consume(sb1.size());
  length = xio::read_until(s, sb1, "XYZ", ec);
  ASIO_CHECK(!ec);
  ASIO_CHECK(length == 26);

  s.reset(read_data, sizeof(read_data));
  sb2.consume(sb2.size());
  length = xio::read_until(s, sb2, "XYZ", ec);
  ASIO_CHECK(ec == xio::error::not_found);
  ASIO_CHECK(length == 0);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb2.consume(sb2.size());
  length = xio::read_until(s, sb2, "XYZ", ec);
  ASIO_CHECK(ec == xio::error::not_found);
  ASIO_CHECK(length == 0);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb2.consume(sb2.size());
  length = xio::read_until(s, sb2, "XYZ", ec);
  ASIO_CHECK(ec == xio::error::not_found);
  ASIO_CHECK(length == 0);

  s.reset(read_data, sizeof(read_data));
  sb2.consume(sb2.size());
  length = xio::read_until(s, sb2, "WXY", ec);
  ASIO_CHECK(!ec);
  ASIO_CHECK(length == 25);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb2.consume(sb2.size());
  length = xio::read_until(s, sb2, "WXY", ec);
  ASIO_CHECK(!ec);
  ASIO_CHECK(length == 25);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb2.consume(sb2.size());
  length = xio::read_until(s, sb2, "WXY", ec);
  ASIO_CHECK(!ec);
  ASIO_CHECK(length == 25);
}

void test_streambuf_read_until_string()
{
#if !defined(ASIO_NO_DYNAMIC_BUFFER_V1)
  xio::io_context ioc;
  test_stream s(ioc);
  xio::streambuf sb1;
  xio::streambuf sb2(25);
  xio::error_code ec;

  s.reset(read_data, sizeof(read_data));
  sb1.consume(sb1.size());
  std::size_t length = xio::read_until(s, sb1, "XYZ");
  ASIO_CHECK(length == 26);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb1.consume(sb1.size());
  length = xio::read_until(s, sb1, "XYZ");
  ASIO_CHECK(length == 26);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb1.consume(sb1.size());
  length = xio::read_until(s, sb1, "XYZ");
  ASIO_CHECK(length == 26);

  s.reset(read_data, sizeof(read_data));
  sb1.consume(sb1.size());
  length = xio::read_until(s, sb1, "XYZ", ec);
  ASIO_CHECK(!ec);
  ASIO_CHECK(length == 26);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb1.consume(sb1.size());
  length = xio::read_until(s, sb1, "XYZ", ec);
  ASIO_CHECK(!ec);
  ASIO_CHECK(length == 26);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb1.consume(sb1.size());
  length = xio::read_until(s, sb1, "XYZ", ec);
  ASIO_CHECK(!ec);
  ASIO_CHECK(length == 26);

  s.reset(read_data, sizeof(read_data));
  sb2.consume(sb2.size());
  length = xio::read_until(s, sb2, "XYZ", ec);
  ASIO_CHECK(ec == xio::error::not_found);
  ASIO_CHECK(length == 0);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb2.consume(sb2.size());
  length = xio::read_until(s, sb2, "XYZ", ec);
  ASIO_CHECK(ec == xio::error::not_found);
  ASIO_CHECK(length == 0);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb2.consume(sb2.size());
  length = xio::read_until(s, sb2, "XYZ", ec);
  ASIO_CHECK(ec == xio::error::not_found);
  ASIO_CHECK(length == 0);

  s.reset(read_data, sizeof(read_data));
  sb2.consume(sb2.size());
  length = xio::read_until(s, sb2, "WXY", ec);
  ASIO_CHECK(!ec);
  ASIO_CHECK(length == 25);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb2.consume(sb2.size());
  length = xio::read_until(s, sb2, "WXY", ec);
  ASIO_CHECK(!ec);
  ASIO_CHECK(length == 25);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb2.consume(sb2.size());
  length = xio::read_until(s, sb2, "WXY", ec);
  ASIO_CHECK(!ec);
  ASIO_CHECK(length == 25);
#endif // !defined(ASIO_NO_DYNAMIC_BUFFER_V1)
}

class match_char
{
public:
  explicit match_char(char c) : c_(c) {}

  template <typename Iterator>
  std::pair<Iterator, bool> operator()(
      Iterator begin, Iterator end) const
  {
    Iterator i = begin;
    while (i != end)
      if (c_ == *i++)
        return std::make_pair(i, true);
    return std::make_pair(i, false);
  }

private:
  char c_;
};

namespace xio {
  template <> struct is_match_condition<match_char>
  {
    enum { value = true };
  };
} // namespace xio

void test_dynamic_string_read_until_match_condition()
{
  xio::io_context ioc;
  test_stream s(ioc);
  std::string data1, data2;
  xio::dynamic_string_buffer<char, std::string::traits_type,
    std::string::allocator_type> sb1 = xio::dynamic_buffer(data1);
  xio::dynamic_string_buffer<char, std::string::traits_type,
    std::string::allocator_type> sb2 = xio::dynamic_buffer(data2, 25);
  xio::error_code ec;

  s.reset(read_data, sizeof(read_data));
  sb1.consume(sb1.size());
  std::size_t length = xio::read_until(s, sb1, match_char('Z'));
  ASIO_CHECK(length == 26);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb1.consume(sb1.size());
  length = xio::read_until(s, sb1, match_char('Z'));
  ASIO_CHECK(length == 26);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb1.consume(sb1.size());
  length = xio::read_until(s, sb1, match_char('Z'));
  ASIO_CHECK(length == 26);

  s.reset(read_data, sizeof(read_data));
  sb1.consume(sb1.size());
  length = xio::read_until(s, sb1, match_char('Z'), ec);
  ASIO_CHECK(!ec);
  ASIO_CHECK(length == 26);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb1.consume(sb1.size());
  length = xio::read_until(s, sb1, match_char('Z'), ec);
  ASIO_CHECK(!ec);
  ASIO_CHECK(length == 26);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb1.consume(sb1.size());
  length = xio::read_until(s, sb1, match_char('Z'), ec);
  ASIO_CHECK(!ec);
  ASIO_CHECK(length == 26);

  s.reset(read_data, sizeof(read_data));
  sb2.consume(sb2.size());
  length = xio::read_until(s, sb2, match_char('Z'), ec);
  ASIO_CHECK(ec == xio::error::not_found);
  ASIO_CHECK(length == 0);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb2.consume(sb2.size());
  length = xio::read_until(s, sb2, match_char('Z'), ec);
  ASIO_CHECK(ec == xio::error::not_found);
  ASIO_CHECK(length == 0);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb2.consume(sb2.size());
  length = xio::read_until(s, sb2, match_char('Z'), ec);
  ASIO_CHECK(ec == xio::error::not_found);
  ASIO_CHECK(length == 0);

  s.reset(read_data, sizeof(read_data));
  sb2.consume(sb2.size());
  length = xio::read_until(s, sb2, match_char('Y'), ec);
  ASIO_CHECK(!ec);
  ASIO_CHECK(length == 25);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb2.consume(sb2.size());
  length = xio::read_until(s, sb2, match_char('Y'), ec);
  ASIO_CHECK(!ec);
  ASIO_CHECK(length == 25);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb2.consume(sb2.size());
  length = xio::read_until(s, sb2, match_char('Y'), ec);
  ASIO_CHECK(!ec);
  ASIO_CHECK(length == 25);
}

void test_streambuf_read_until_match_condition()
{
#if !defined(ASIO_NO_DYNAMIC_BUFFER_V1)
  xio::io_context ioc;
  test_stream s(ioc);
  xio::streambuf sb1;
  xio::streambuf sb2(25);
  xio::error_code ec;

  s.reset(read_data, sizeof(read_data));
  sb1.consume(sb1.size());
  std::size_t length = xio::read_until(s, sb1, match_char('Z'));
  ASIO_CHECK(length == 26);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb1.consume(sb1.size());
  length = xio::read_until(s, sb1, match_char('Z'));
  ASIO_CHECK(length == 26);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb1.consume(sb1.size());
  length = xio::read_until(s, sb1, match_char('Z'));
  ASIO_CHECK(length == 26);

  s.reset(read_data, sizeof(read_data));
  sb1.consume(sb1.size());
  length = xio::read_until(s, sb1, match_char('Z'), ec);
  ASIO_CHECK(!ec);
  ASIO_CHECK(length == 26);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb1.consume(sb1.size());
  length = xio::read_until(s, sb1, match_char('Z'), ec);
  ASIO_CHECK(!ec);
  ASIO_CHECK(length == 26);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb1.consume(sb1.size());
  length = xio::read_until(s, sb1, match_char('Z'), ec);
  ASIO_CHECK(!ec);
  ASIO_CHECK(length == 26);

  s.reset(read_data, sizeof(read_data));
  sb2.consume(sb2.size());
  length = xio::read_until(s, sb2, match_char('Z'), ec);
  ASIO_CHECK(ec == xio::error::not_found);
  ASIO_CHECK(length == 0);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb2.consume(sb2.size());
  length = xio::read_until(s, sb2, match_char('Z'), ec);
  ASIO_CHECK(ec == xio::error::not_found);
  ASIO_CHECK(length == 0);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb2.consume(sb2.size());
  length = xio::read_until(s, sb2, match_char('Z'), ec);
  ASIO_CHECK(ec == xio::error::not_found);
  ASIO_CHECK(length == 0);

  s.reset(read_data, sizeof(read_data));
  sb2.consume(sb2.size());
  length = xio::read_until(s, sb2, match_char('Y'), ec);
  ASIO_CHECK(!ec);
  ASIO_CHECK(length == 25);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb2.consume(sb2.size());
  length = xio::read_until(s, sb2, match_char('Y'), ec);
  ASIO_CHECK(!ec);
  ASIO_CHECK(length == 25);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb2.consume(sb2.size());
  length = xio::read_until(s, sb2, match_char('Y'), ec);
  ASIO_CHECK(!ec);
  ASIO_CHECK(length == 25);
#endif // !defined(ASIO_NO_DYNAMIC_BUFFER_V1)
}

void async_read_handler(
    const xio::error_code& err, xio::error_code* err_out,
    std::size_t bytes_transferred, std::size_t* bytes_out, bool* called)
{
  *err_out = err;
  *bytes_out = bytes_transferred;
  *called = true;
}

void test_dynamic_string_async_read_until_char()
{
  namespace bindns = std;
  using bindns::placeholders::_1;
  using bindns::placeholders::_2;

  xio::io_context ioc;
  test_stream s(ioc);
  std::string data1, data2;
  xio::dynamic_string_buffer<char, std::string::traits_type,
    std::string::allocator_type> sb1 = xio::dynamic_buffer(data1);
  xio::dynamic_string_buffer<char, std::string::traits_type,
    std::string::allocator_type> sb2 = xio::dynamic_buffer(data2, 25);
  xio::error_code ec;
  std::size_t length;
  bool called;

  s.reset(read_data, sizeof(read_data));
  ec = xio::error_code();
  length = 0;
  called = false;
  sb1.consume(sb1.size());
  xio::async_read_until(s, sb1, 'Z',
      bindns::bind(async_read_handler, _1, &ec,
        _2, &length, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(!ec);
  ASIO_CHECK(length == 26);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  ec = xio::error_code();
  length = 0;
  called = false;
  sb1.consume(sb1.size());
  xio::async_read_until(s, sb1, 'Z',
      bindns::bind(async_read_handler, _1, &ec,
        _2, &length, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(!ec);
  ASIO_CHECK(length == 26);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  ec = xio::error_code();
  length = 0;
  called = false;
  sb1.consume(sb1.size());
  xio::async_read_until(s, sb1, 'Z',
      bindns::bind(async_read_handler, _1, &ec,
        _2, &length, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(!ec);
  ASIO_CHECK(length == 26);

  s.reset(read_data, sizeof(read_data));
  ec = xio::error_code();
  length = 0;
  called = false;
  sb2.consume(sb2.size());
  xio::async_read_until(s, sb2, 'Z',
      bindns::bind(async_read_handler, _1, &ec,
        _2, &length, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(ec == xio::error::not_found);
  ASIO_CHECK(length == 0);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  ec = xio::error_code();
  length = 0;
  called = false;
  sb2.consume(sb2.size());
  xio::async_read_until(s, sb2, 'Z',
      bindns::bind(async_read_handler, _1, &ec,
        _2, &length, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(ec == xio::error::not_found);
  ASIO_CHECK(length == 0);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  ec = xio::error_code();
  length = 0;
  called = false;
  sb2.consume(sb2.size());
  xio::async_read_until(s, sb2, 'Z',
      bindns::bind(async_read_handler, _1, &ec,
        _2, &length, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(ec == xio::error::not_found);
  ASIO_CHECK(length == 0);

  s.reset(read_data, sizeof(read_data));
  ec = xio::error_code();
  length = 0;
  called = false;
  sb2.consume(sb2.size());
  xio::async_read_until(s, sb2, 'Y',
      bindns::bind(async_read_handler, _1, &ec,
        _2, &length, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(!ec);
  ASIO_CHECK(length == 25);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  ec = xio::error_code();
  length = 0;
  called = false;
  sb2.consume(sb2.size());
  xio::async_read_until(s, sb2, 'Y',
      bindns::bind(async_read_handler, _1, &ec,
        _2, &length, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(!ec);
  ASIO_CHECK(length == 25);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  ec = xio::error_code();
  length = 0;
  called = false;
  sb2.consume(sb2.size());
  xio::async_read_until(s, sb2, 'Y',
      bindns::bind(async_read_handler, _1, &ec,
        _2, &length, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(!ec);
  ASIO_CHECK(length == 25);

  s.reset(read_data, sizeof(read_data));
  sb2.consume(sb2.size());
  int i = xio::async_read_until(s, sb2, 'Y',
      archetypes::lazy_handler());
  ASIO_CHECK(i == 42);
  ioc.restart();
  ioc.run();

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  ec = xio::error_code();
  length = 0;
  called = false;
  sb2.consume(sb2.size());
  xio::async_read_until(s, sb2, 'Y')(
      bindns::bind(async_read_handler, _1, &ec,
        _2, &length, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(!ec);
  ASIO_CHECK(length == 25);
}

void test_streambuf_async_read_until_char()
{
#if !defined(ASIO_NO_DYNAMIC_BUFFER_V1)
  namespace bindns = std;
  using bindns::placeholders::_1;
  using bindns::placeholders::_2;

  xio::io_context ioc;
  test_stream s(ioc);
  xio::streambuf sb1;
  xio::streambuf sb2(25);
  xio::error_code ec;
  std::size_t length;
  bool called;

  s.reset(read_data, sizeof(read_data));
  ec = xio::error_code();
  length = 0;
  called = false;
  sb1.consume(sb1.size());
  xio::async_read_until(s, sb1, 'Z',
      bindns::bind(async_read_handler, _1, &ec,
        _2, &length, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(!ec);
  ASIO_CHECK(length == 26);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  ec = xio::error_code();
  length = 0;
  called = false;
  sb1.consume(sb1.size());
  xio::async_read_until(s, sb1, 'Z',
      bindns::bind(async_read_handler, _1, &ec,
        _2, &length, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(!ec);
  ASIO_CHECK(length == 26);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  ec = xio::error_code();
  length = 0;
  called = false;
  sb1.consume(sb1.size());
  xio::async_read_until(s, sb1, 'Z',
      bindns::bind(async_read_handler, _1, &ec,
        _2, &length, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(!ec);
  ASIO_CHECK(length == 26);

  s.reset(read_data, sizeof(read_data));
  ec = xio::error_code();
  length = 0;
  called = false;
  sb2.consume(sb2.size());
  xio::async_read_until(s, sb2, 'Z',
      bindns::bind(async_read_handler, _1, &ec,
        _2, &length, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(ec == xio::error::not_found);
  ASIO_CHECK(length == 0);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  ec = xio::error_code();
  length = 0;
  called = false;
  sb2.consume(sb2.size());
  xio::async_read_until(s, sb2, 'Z',
      bindns::bind(async_read_handler, _1, &ec,
        _2, &length, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(ec == xio::error::not_found);
  ASIO_CHECK(length == 0);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  ec = xio::error_code();
  length = 0;
  called = false;
  sb2.consume(sb2.size());
  xio::async_read_until(s, sb2, 'Z',
      bindns::bind(async_read_handler, _1, &ec,
        _2, &length, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(ec == xio::error::not_found);
  ASIO_CHECK(length == 0);

  s.reset(read_data, sizeof(read_data));
  ec = xio::error_code();
  length = 0;
  called = false;
  sb2.consume(sb2.size());
  xio::async_read_until(s, sb2, 'Y',
      bindns::bind(async_read_handler, _1, &ec,
        _2, &length, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(!ec);
  ASIO_CHECK(length == 25);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  ec = xio::error_code();
  length = 0;
  called = false;
  sb2.consume(sb2.size());
  xio::async_read_until(s, sb2, 'Y',
      bindns::bind(async_read_handler, _1, &ec,
        _2, &length, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(!ec);
  ASIO_CHECK(length == 25);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  ec = xio::error_code();
  length = 0;
  called = false;
  sb2.consume(sb2.size());
  xio::async_read_until(s, sb2, 'Y',
      bindns::bind(async_read_handler, _1, &ec,
        _2, &length, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(!ec);
  ASIO_CHECK(length == 25);

  s.reset(read_data, sizeof(read_data));
  sb2.consume(sb2.size());
  int i = xio::async_read_until(s, sb2, 'Y',
      archetypes::lazy_handler());
  ASIO_CHECK(i == 42);
  ioc.restart();
  ioc.run();

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  ec = xio::error_code();
  length = 0;
  called = false;
  sb2.consume(sb2.size());
  xio::async_read_until(s, sb2, 'Y')(
      bindns::bind(async_read_handler, _1, &ec,
        _2, &length, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(!ec);
  ASIO_CHECK(length == 25);
#endif // !defined(ASIO_NO_DYNAMIC_BUFFER_V1)
}

void test_dynamic_string_async_read_until_string()
{
  namespace bindns = std;
  using bindns::placeholders::_1;
  using bindns::placeholders::_2;

  xio::io_context ioc;
  test_stream s(ioc);
  std::string data1, data2;
  xio::dynamic_string_buffer<char, std::string::traits_type,
    std::string::allocator_type> sb1 = xio::dynamic_buffer(data1);
  xio::dynamic_string_buffer<char, std::string::traits_type,
    std::string::allocator_type> sb2 = xio::dynamic_buffer(data2, 25);
  xio::error_code ec;
  std::size_t length;
  bool called;

  s.reset(read_data, sizeof(read_data));
  ec = xio::error_code();
  length = 0;
  called = false;
  sb1.consume(sb1.size());
  xio::async_read_until(s, sb1, "XYZ",
      bindns::bind(async_read_handler, _1, &ec,
        _2, &length, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(!ec);
  ASIO_CHECK(length == 26);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  ec = xio::error_code();
  length = 0;
  called = false;
  sb1.consume(sb1.size());
  xio::async_read_until(s, sb1, "XYZ",
      bindns::bind(async_read_handler, _1, &ec,
        _2, &length, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(!ec);
  ASIO_CHECK(length == 26);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  ec = xio::error_code();
  length = 0;
  called = false;
  sb1.consume(sb1.size());
  xio::async_read_until(s, sb1, "XYZ",
      bindns::bind(async_read_handler, _1, &ec,
        _2, &length, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(!ec);
  ASIO_CHECK(length == 26);

  s.reset(read_data, sizeof(read_data));
  ec = xio::error_code();
  length = 0;
  called = false;
  sb2.consume(sb2.size());
  xio::async_read_until(s, sb2, "XYZ",
      bindns::bind(async_read_handler, _1, &ec,
        _2, &length, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(ec == xio::error::not_found);
  ASIO_CHECK(length == 0);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  ec = xio::error_code();
  length = 0;
  called = false;
  sb2.consume(sb2.size());
  xio::async_read_until(s, sb2, "XYZ",
      bindns::bind(async_read_handler, _1, &ec,
        _2, &length, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(ec == xio::error::not_found);
  ASIO_CHECK(length == 0);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  ec = xio::error_code();
  length = 0;
  called = false;
  sb2.consume(sb2.size());
  xio::async_read_until(s, sb2, "XYZ",
      bindns::bind(async_read_handler, _1, &ec,
        _2, &length, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(ec == xio::error::not_found);
  ASIO_CHECK(length == 0);

  s.reset(read_data, sizeof(read_data));
  ec = xio::error_code();
  length = 0;
  called = false;
  sb2.consume(sb2.size());
  xio::async_read_until(s, sb2, "WXY",
      bindns::bind(async_read_handler, _1, &ec,
        _2, &length, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(!ec);
  ASIO_CHECK(length == 25);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  ec = xio::error_code();
  length = 0;
  called = false;
  sb2.consume(sb2.size());
  xio::async_read_until(s, sb2, "WXY",
      bindns::bind(async_read_handler, _1, &ec,
        _2, &length, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(!ec);
  ASIO_CHECK(length == 25);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  ec = xio::error_code();
  length = 0;
  called = false;
  sb2.consume(sb2.size());
  xio::async_read_until(s, sb2, "WXY",
      bindns::bind(async_read_handler, _1, &ec,
        _2, &length, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(!ec);
  ASIO_CHECK(length == 25);

  s.reset(read_data, sizeof(read_data));
  sb2.consume(sb2.size());
  int i = xio::async_read_until(s, sb2, "WXY",
      archetypes::lazy_handler());
  ASIO_CHECK(i == 42);
  ioc.restart();
  ioc.run();

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  ec = xio::error_code();
  length = 0;
  called = false;
  sb2.consume(sb2.size());
  xio::async_read_until(s, sb2, "WXY")(
      bindns::bind(async_read_handler, _1, &ec,
        _2, &length, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(!ec);
  ASIO_CHECK(length == 25);
}

void test_streambuf_async_read_until_string()
{
#if !defined(ASIO_NO_DYNAMIC_BUFFER_V1)
  namespace bindns = std;
  using bindns::placeholders::_1;
  using bindns::placeholders::_2;

  xio::io_context ioc;
  test_stream s(ioc);
  xio::streambuf sb1;
  xio::streambuf sb2(25);
  xio::error_code ec;
  std::size_t length;
  bool called;

  s.reset(read_data, sizeof(read_data));
  ec = xio::error_code();
  length = 0;
  called = false;
  sb1.consume(sb1.size());
  xio::async_read_until(s, sb1, "XYZ",
      bindns::bind(async_read_handler, _1, &ec,
        _2, &length, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(!ec);
  ASIO_CHECK(length == 26);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  ec = xio::error_code();
  length = 0;
  called = false;
  sb1.consume(sb1.size());
  xio::async_read_until(s, sb1, "XYZ",
      bindns::bind(async_read_handler, _1, &ec,
        _2, &length, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(!ec);
  ASIO_CHECK(length == 26);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  ec = xio::error_code();
  length = 0;
  called = false;
  sb1.consume(sb1.size());
  xio::async_read_until(s, sb1, "XYZ",
      bindns::bind(async_read_handler, _1, &ec,
        _2, &length, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(!ec);
  ASIO_CHECK(length == 26);

  s.reset(read_data, sizeof(read_data));
  ec = xio::error_code();
  length = 0;
  called = false;
  sb2.consume(sb2.size());
  xio::async_read_until(s, sb2, "XYZ",
      bindns::bind(async_read_handler, _1, &ec,
        _2, &length, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(ec == xio::error::not_found);
  ASIO_CHECK(length == 0);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  ec = xio::error_code();
  length = 0;
  called = false;
  sb2.consume(sb2.size());
  xio::async_read_until(s, sb2, "XYZ",
      bindns::bind(async_read_handler, _1, &ec,
        _2, &length, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(ec == xio::error::not_found);
  ASIO_CHECK(length == 0);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  ec = xio::error_code();
  length = 0;
  called = false;
  sb2.consume(sb2.size());
  xio::async_read_until(s, sb2, "XYZ",
      bindns::bind(async_read_handler, _1, &ec,
        _2, &length, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(ec == xio::error::not_found);
  ASIO_CHECK(length == 0);

  s.reset(read_data, sizeof(read_data));
  ec = xio::error_code();
  length = 0;
  called = false;
  sb2.consume(sb2.size());
  xio::async_read_until(s, sb2, "WXY",
      bindns::bind(async_read_handler, _1, &ec,
        _2, &length, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(!ec);
  ASIO_CHECK(length == 25);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  ec = xio::error_code();
  length = 0;
  called = false;
  sb2.consume(sb2.size());
  xio::async_read_until(s, sb2, "WXY",
      bindns::bind(async_read_handler, _1, &ec,
        _2, &length, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(!ec);
  ASIO_CHECK(length == 25);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  ec = xio::error_code();
  length = 0;
  called = false;
  sb2.consume(sb2.size());
  xio::async_read_until(s, sb2, "WXY",
      bindns::bind(async_read_handler, _1, &ec,
        _2, &length, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(!ec);
  ASIO_CHECK(length == 25);

  s.reset(read_data, sizeof(read_data));
  sb2.consume(sb2.size());
  int i = xio::async_read_until(s, sb2, "WXY",
      archetypes::lazy_handler());
  ASIO_CHECK(i == 42);
  ioc.restart();
  ioc.run();

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  ec = xio::error_code();
  length = 0;
  called = false;
  sb2.consume(sb2.size());
  xio::async_read_until(s, sb2, "WXY")(
      bindns::bind(async_read_handler, _1, &ec,
        _2, &length, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(!ec);
  ASIO_CHECK(length == 25);
#endif // !defined(ASIO_NO_DYNAMIC_BUFFER_V1)
}

void test_dynamic_string_async_read_until_match_condition()
{
  namespace bindns = std;
  using bindns::placeholders::_1;
  using bindns::placeholders::_2;

  xio::io_context ioc;
  test_stream s(ioc);
  std::string data1, data2;
  xio::dynamic_string_buffer<char, std::string::traits_type,
    std::string::allocator_type> sb1 = xio::dynamic_buffer(data1);
  xio::dynamic_string_buffer<char, std::string::traits_type,
    std::string::allocator_type> sb2 = xio::dynamic_buffer(data2, 25);
  xio::error_code ec;
  std::size_t length;
  bool called;

  s.reset(read_data, sizeof(read_data));
  ec = xio::error_code();
  length = 0;
  called = false;
  sb1.consume(sb1.size());
  xio::async_read_until(s, sb1, match_char('Z'),
      bindns::bind(async_read_handler, _1, &ec,
        _2, &length, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(!ec);
  ASIO_CHECK(length == 26);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  ec = xio::error_code();
  length = 0;
  called = false;
  sb1.consume(sb1.size());
  xio::async_read_until(s, sb1, match_char('Z'),
      bindns::bind(async_read_handler, _1, &ec,
        _2, &length, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(!ec);
  ASIO_CHECK(length == 26);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  ec = xio::error_code();
  length = 0;
  called = false;
  sb1.consume(sb1.size());
  xio::async_read_until(s, sb1, match_char('Z'),
      bindns::bind(async_read_handler, _1, &ec,
        _2, &length, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(!ec);
  ASIO_CHECK(length == 26);

  s.reset(read_data, sizeof(read_data));
  ec = xio::error_code();
  length = 0;
  called = false;
  sb2.consume(sb2.size());
  xio::async_read_until(s, sb2, match_char('Z'),
      bindns::bind(async_read_handler, _1, &ec,
        _2, &length, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(ec == xio::error::not_found);
  ASIO_CHECK(length == 0);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  ec = xio::error_code();
  length = 0;
  called = false;
  sb2.consume(sb2.size());
  xio::async_read_until(s, sb2, match_char('Z'),
      bindns::bind(async_read_handler, _1, &ec,
        _2, &length, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(ec == xio::error::not_found);
  ASIO_CHECK(length == 0);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  ec = xio::error_code();
  length = 0;
  called = false;
  sb2.consume(sb2.size());
  xio::async_read_until(s, sb2, match_char('Z'),
      bindns::bind(async_read_handler, _1, &ec,
        _2, &length, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(ec == xio::error::not_found);
  ASIO_CHECK(length == 0);

  s.reset(read_data, sizeof(read_data));
  ec = xio::error_code();
  length = 0;
  called = false;
  sb2.consume(sb2.size());
  xio::async_read_until(s, sb2, match_char('Y'),
      bindns::bind(async_read_handler, _1, &ec,
        _2, &length, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(!ec);
  ASIO_CHECK(length == 25);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  ec = xio::error_code();
  length = 0;
  called = false;
  sb2.consume(sb2.size());
  xio::async_read_until(s, sb2, match_char('Y'),
      bindns::bind(async_read_handler, _1, &ec,
        _2, &length, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(!ec);
  ASIO_CHECK(length == 25);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  ec = xio::error_code();
  length = 0;
  called = false;
  sb2.consume(sb2.size());
  xio::async_read_until(s, sb2, match_char('Y'),
      bindns::bind(async_read_handler, _1, &ec,
        _2, &length, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(!ec);
  ASIO_CHECK(length == 25);

  s.reset(read_data, sizeof(read_data));
  sb2.consume(sb2.size());
  int i = xio::async_read_until(s, sb2, match_char('Y'),
      archetypes::lazy_handler());
  ASIO_CHECK(i == 42);
  ioc.restart();
  ioc.run();

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  ec = xio::error_code();
  length = 0;
  called = false;
  sb2.consume(sb2.size());
  xio::async_read_until(s, sb2, match_char('Y'))(
      bindns::bind(async_read_handler, _1, &ec,
        _2, &length, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(!ec);
  ASIO_CHECK(length == 25);
}

void test_streambuf_async_read_until_match_condition()
{
#if !defined(ASIO_NO_DYNAMIC_BUFFER_V1)
  namespace bindns = std;
  using bindns::placeholders::_1;
  using bindns::placeholders::_2;

  xio::io_context ioc;
  test_stream s(ioc);
  xio::streambuf sb1;
  xio::streambuf sb2(25);
  xio::error_code ec;
  std::size_t length;
  bool called;

  s.reset(read_data, sizeof(read_data));
  ec = xio::error_code();
  length = 0;
  called = false;
  sb1.consume(sb1.size());
  xio::async_read_until(s, sb1, match_char('Z'),
      bindns::bind(async_read_handler, _1, &ec,
        _2, &length, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(!ec);
  ASIO_CHECK(length == 26);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  ec = xio::error_code();
  length = 0;
  called = false;
  sb1.consume(sb1.size());
  xio::async_read_until(s, sb1, match_char('Z'),
      bindns::bind(async_read_handler, _1, &ec,
        _2, &length, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(!ec);
  ASIO_CHECK(length == 26);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  ec = xio::error_code();
  length = 0;
  called = false;
  sb1.consume(sb1.size());
  xio::async_read_until(s, sb1, match_char('Z'),
      bindns::bind(async_read_handler, _1, &ec,
        _2, &length, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(!ec);
  ASIO_CHECK(length == 26);

  s.reset(read_data, sizeof(read_data));
  ec = xio::error_code();
  length = 0;
  called = false;
  sb2.consume(sb2.size());
  xio::async_read_until(s, sb2, match_char('Z'),
      bindns::bind(async_read_handler, _1, &ec,
        _2, &length, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(ec == xio::error::not_found);
  ASIO_CHECK(length == 0);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  ec = xio::error_code();
  length = 0;
  called = false;
  sb2.consume(sb2.size());
  xio::async_read_until(s, sb2, match_char('Z'),
      bindns::bind(async_read_handler, _1, &ec,
        _2, &length, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(ec == xio::error::not_found);
  ASIO_CHECK(length == 0);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  ec = xio::error_code();
  length = 0;
  called = false;
  sb2.consume(sb2.size());
  xio::async_read_until(s, sb2, match_char('Z'),
      bindns::bind(async_read_handler, _1, &ec,
        _2, &length, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(ec == xio::error::not_found);
  ASIO_CHECK(length == 0);

  s.reset(read_data, sizeof(read_data));
  ec = xio::error_code();
  length = 0;
  called = false;
  sb2.consume(sb2.size());
  xio::async_read_until(s, sb2, match_char('Y'),
      bindns::bind(async_read_handler, _1, &ec,
        _2, &length, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(!ec);
  ASIO_CHECK(length == 25);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  ec = xio::error_code();
  length = 0;
  called = false;
  sb2.consume(sb2.size());
  xio::async_read_until(s, sb2, match_char('Y'),
      bindns::bind(async_read_handler, _1, &ec,
        _2, &length, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(!ec);
  ASIO_CHECK(length == 25);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  ec = xio::error_code();
  length = 0;
  called = false;
  sb2.consume(sb2.size());
  xio::async_read_until(s, sb2, match_char('Y'),
      bindns::bind(async_read_handler, _1, &ec,
        _2, &length, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(!ec);
  ASIO_CHECK(length == 25);

  s.reset(read_data, sizeof(read_data));
  sb2.consume(sb2.size());
  int i = xio::async_read_until(s, sb2, match_char('Y'),
      archetypes::lazy_handler());
  ASIO_CHECK(i == 42);
  ioc.restart();
  ioc.run();

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  ec = xio::error_code();
  length = 0;
  called = false;
  sb2.consume(sb2.size());
  xio::async_read_until(s, sb2, match_char('Y'))(
      bindns::bind(async_read_handler, _1, &ec,
        _2, &length, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(!ec);
  ASIO_CHECK(length == 25);
#endif // !defined(ASIO_NO_DYNAMIC_BUFFER_V1)
}

ASIO_TEST_SUITE
(
  "read_until",
  ASIO_TEST_CASE(test_dynamic_string_read_until_char)
  ASIO_TEST_CASE(test_streambuf_read_until_char)
  ASIO_TEST_CASE(test_dynamic_string_read_until_string)
  ASIO_TEST_CASE(test_streambuf_read_until_string)
  ASIO_TEST_CASE(test_dynamic_string_read_until_match_condition)
  ASIO_TEST_CASE(test_streambuf_read_until_match_condition)
  ASIO_TEST_CASE(test_dynamic_string_async_read_until_char)
  ASIO_TEST_CASE(test_streambuf_async_read_until_char)
  ASIO_TEST_CASE(test_dynamic_string_async_read_until_string)
  ASIO_TEST_CASE(test_streambuf_async_read_until_string)
  ASIO_TEST_CASE(test_dynamic_string_async_read_until_match_condition)
  ASIO_TEST_CASE(test_streambuf_async_read_until_match_condition)
)
