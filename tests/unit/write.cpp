//
// write.cpp
// ~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

// Disable autolinking for unit tests.
#if !defined(BOOST_ALL_NO_LIB)
#define BOOST_ALL_NO_LIB 1
#endif // !defined(BOOST_ALL_NO_LIB)

// Test that header file is self-contained.
#include <xio/write.h>

#include <array>
#include <cstring>
#include <functional>
#include <vector>
#include "archetypes/async_result.hpp"
#include <xio/io_context.h>
#include <xio/post.h>
#include <xio/streambuf.h>
#include "unit_test.hpp"

#if defined(ASIO_HAS_BOOST_ARRAY)
#include <boost/array.hpp>
#endif // defined(ASIO_HAS_BOOST_ARRAY)

using namespace std; // For memcmp, memcpy and memset.

class test_stream
{
public:
  typedef xio::io_context::executor_type executor_type;

  test_stream(xio::io_context& io_context)
    : io_context_(io_context),
      length_(max_length),
      position_(0),
      next_write_length_(max_length)
  {
    memset(data_, 0, max_length);
  }

  executor_type get_executor() noexcept
  {
    return io_context_.get_executor();
  }

  void reset(size_t length = max_length)
  {
    ASIO_CHECK(length <= max_length);

    memset(data_, 0, max_length);
    length_ = length;
    position_ = 0;
    next_write_length_ = length;
  }

  void next_write_length(size_t length)
  {
    next_write_length_ = length;
  }

  template <typename Iterator>
  bool check_buffers(Iterator begin, Iterator end, size_t length)
  {
    if (length != position_)
      return false;

    Iterator iter = begin;
    size_t checked_length = 0;
    for (; iter != end && checked_length < length; ++iter)
    {
      size_t buffer_length = xio::buffer_size(*iter);
      if (buffer_length > length - checked_length)
        buffer_length = length - checked_length;
      if (memcmp(data_ + checked_length, iter->data(), buffer_length) != 0)
        return false;
      checked_length += buffer_length;
    }

    return true;
  }

  template <typename Const_Buffers>
  bool check_buffers(const Const_Buffers& buffers, size_t length)
  {
    return check_buffers(xio::buffer_sequence_begin(buffers),
        xio::buffer_sequence_end(buffers), length);
  }

  template <typename Const_Buffers>
  size_t write_some(const Const_Buffers& buffers)
  {
    size_t n = xio::buffer_copy(
        xio::buffer(data_, length_) + position_,
        buffers, next_write_length_);
    position_ += n;
    return n;
  }

  template <typename Const_Buffers>
  size_t write_some(const Const_Buffers& buffers, xio::error_code& ec)
  {
    ec = xio::error_code();
    return write_some(buffers);
  }

  template <typename Const_Buffers, typename Handler>
  void async_write_some(const Const_Buffers& buffers,
      Handler&& handler)
  {
    size_t bytes_transferred = write_some(buffers);
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
  size_t next_write_length_;
};

static const char write_data[]
  = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
static char mutable_write_data[]
  = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

void test_2_arg_zero_buffers_write()
{
  xio::io_context ioc;
  test_stream s(ioc);
  std::vector<xio::const_buffer> buffers;

  size_t bytes_transferred = xio::write(s, buffers);
  ASIO_CHECK(bytes_transferred == 0);
}

void test_2_arg_const_buffer_write()
{
  xio::io_context ioc;
  test_stream s(ioc);
  xio::const_buffer buffers
    = xio::buffer(write_data, sizeof(write_data));

  s.reset();
  size_t bytes_transferred = xio::write(s, buffers);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = xio::write(s, buffers);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = xio::write(s, buffers);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));
}

void test_2_arg_mutable_buffer_write()
{
  xio::io_context ioc;
  test_stream s(ioc);
  xio::mutable_buffer buffers
    = xio::buffer(mutable_write_data, sizeof(mutable_write_data));

  s.reset();
  size_t bytes_transferred = xio::write(s, buffers);
  ASIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  ASIO_CHECK(s.check_buffers(buffers, sizeof(mutable_write_data)));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = xio::write(s, buffers);
  ASIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  ASIO_CHECK(s.check_buffers(buffers, sizeof(mutable_write_data)));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = xio::write(s, buffers);
  ASIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  ASIO_CHECK(s.check_buffers(buffers, sizeof(mutable_write_data)));
}

void test_2_arg_vector_buffers_write()
{
  xio::io_context ioc;
  test_stream s(ioc);
  std::vector<xio::const_buffer> buffers;
  buffers.push_back(xio::buffer(write_data, 32));
  buffers.push_back(xio::buffer(write_data, 39) + 32);
  buffers.push_back(xio::buffer(write_data) + 39);

  s.reset();
  size_t bytes_transferred = xio::write(s, buffers);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = xio::write(s, buffers);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = xio::write(s, buffers);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));
}

void test_2_arg_dynamic_string_write()
{
  xio::io_context ioc;
  test_stream s(ioc);
  std::string data;
  xio::dynamic_string_buffer<char, std::string::traits_type,
    std::string::allocator_type> sb
      = xio::dynamic_buffer(data, sizeof(write_data));
  xio::const_buffer buffers
    = xio::buffer(write_data, sizeof(write_data));

  s.reset();
  data.assign(write_data, sizeof(write_data));
  size_t bytes_transferred = xio::write(s, sb);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  data.assign(write_data, sizeof(write_data));
  s.next_write_length(1);
  bytes_transferred = xio::write(s, sb);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  data.assign(write_data, sizeof(write_data));
  s.next_write_length(10);
  bytes_transferred = xio::write(s, sb);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));
}

void test_3_arg_nothrow_zero_buffers_write()
{
  xio::io_context ioc;
  test_stream s(ioc);
  std::vector<xio::const_buffer> buffers;

  xio::error_code error;
  size_t bytes_transferred = xio::write(s, buffers, error);
  ASIO_CHECK(bytes_transferred == 0);
  ASIO_CHECK(!error);
}

void test_3_arg_nothrow_const_buffer_write()
{
  xio::io_context ioc;
  test_stream s(ioc);
  xio::const_buffer buffers
    = xio::buffer(write_data, sizeof(write_data));

  s.reset();
  xio::error_code error;
  size_t bytes_transferred = xio::write(s, buffers, error);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  bytes_transferred = xio::write(s, buffers, error);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  bytes_transferred = xio::write(s, buffers, error);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));
  ASIO_CHECK(!error);
}

void test_3_arg_nothrow_mutable_buffer_write()
{
  xio::io_context ioc;
  test_stream s(ioc);
  xio::mutable_buffer buffers
    = xio::buffer(mutable_write_data, sizeof(mutable_write_data));

  s.reset();
  xio::error_code error;
  size_t bytes_transferred = xio::write(s, buffers, error);
  ASIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  ASIO_CHECK(s.check_buffers(buffers, sizeof(mutable_write_data)));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  bytes_transferred = xio::write(s, buffers, error);
  ASIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  ASIO_CHECK(s.check_buffers(buffers, sizeof(mutable_write_data)));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  bytes_transferred = xio::write(s, buffers, error);
  ASIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  ASIO_CHECK(s.check_buffers(buffers, sizeof(mutable_write_data)));
  ASIO_CHECK(!error);
}

void test_3_arg_nothrow_vector_buffers_write()
{
  xio::io_context ioc;
  test_stream s(ioc);
  std::vector<xio::const_buffer> buffers;
  buffers.push_back(xio::buffer(write_data, 32));
  buffers.push_back(xio::buffer(write_data, 39) + 32);
  buffers.push_back(xio::buffer(write_data) + 39);

  s.reset();
  xio::error_code error;
  size_t bytes_transferred = xio::write(s, buffers, error);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  bytes_transferred = xio::write(s, buffers, error);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  bytes_transferred = xio::write(s, buffers, error);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));
  ASIO_CHECK(!error);
}

void test_3_arg_nothrow_dynamic_string_write()
{
  xio::io_context ioc;
  test_stream s(ioc);
  std::string data;
  xio::dynamic_string_buffer<char, std::string::traits_type,
    std::string::allocator_type> sb
      = xio::dynamic_buffer(data, sizeof(write_data));
  xio::const_buffer buffers
    = xio::buffer(write_data, sizeof(write_data));

  s.reset();
  data.assign(write_data, sizeof(write_data));
  xio::error_code error;
  size_t bytes_transferred = xio::write(s, sb, error);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));
  ASIO_CHECK(!error);

  s.reset();
  data.assign(write_data, sizeof(write_data));
  s.next_write_length(1);
  bytes_transferred = xio::write(s, sb, error);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));
  ASIO_CHECK(!error);

  s.reset();
  data.assign(write_data, sizeof(write_data));
  s.next_write_length(10);
  bytes_transferred = xio::write(s, sb, error);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));
  ASIO_CHECK(!error);
}

bool old_style_transfer_all(const xio::error_code& ec,
    size_t /*bytes_transferred*/)
{
  return !!ec;
}

struct short_transfer
{
  short_transfer() {}
  short_transfer(short_transfer&&) {}
  size_t operator()(const xio::error_code& ec,
      size_t /*bytes_transferred*/)
  {
    return !!ec ? 0 : 3;
  }
};

void test_3_arg_const_buffer_write()
{
  xio::io_context ioc;
  test_stream s(ioc);
  xio::const_buffer buffers
    = xio::buffer(write_data, sizeof(write_data));

  s.reset();
  size_t bytes_transferred = xio::write(s, buffers,
      xio::transfer_all());
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = xio::write(s, buffers,
      xio::transfer_all());
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = xio::write(s, buffers,
      xio::transfer_all());
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  bytes_transferred = xio::write(s, buffers,
      xio::transfer_at_least(1));
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = xio::write(s, buffers,
      xio::transfer_at_least(1));
  ASIO_CHECK(bytes_transferred == 1);
  ASIO_CHECK(s.check_buffers(buffers, 1));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = xio::write(s, buffers,
      xio::transfer_at_least(1));
  ASIO_CHECK(bytes_transferred == 10);
  ASIO_CHECK(s.check_buffers(buffers, 10));

  s.reset();
  bytes_transferred = xio::write(s, buffers,
      xio::transfer_at_least(10));
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = xio::write(s, buffers,
      xio::transfer_at_least(10));
  ASIO_CHECK(bytes_transferred == 10);
  ASIO_CHECK(s.check_buffers(buffers, 10));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = xio::write(s, buffers,
      xio::transfer_at_least(10));
  ASIO_CHECK(bytes_transferred == 10);
  ASIO_CHECK(s.check_buffers(buffers, 10));

  s.reset();
  bytes_transferred = xio::write(s, buffers,
      xio::transfer_at_least(42));
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = xio::write(s, buffers,
      xio::transfer_at_least(42));
  ASIO_CHECK(bytes_transferred == 42);
  ASIO_CHECK(s.check_buffers(buffers, 42));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = xio::write(s, buffers,
      xio::transfer_at_least(42));
  ASIO_CHECK(bytes_transferred == 50);
  ASIO_CHECK(s.check_buffers(buffers, 50));

  s.reset();
  bytes_transferred = xio::write(s, buffers,
      xio::transfer_exactly(1));
  ASIO_CHECK(bytes_transferred == 1);
  ASIO_CHECK(s.check_buffers(buffers, 1));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = xio::write(s, buffers,
      xio::transfer_exactly(1));
  ASIO_CHECK(bytes_transferred == 1);
  ASIO_CHECK(s.check_buffers(buffers, 1));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = xio::write(s, buffers,
      xio::transfer_exactly(1));
  ASIO_CHECK(bytes_transferred == 1);
  ASIO_CHECK(s.check_buffers(buffers, 1));

  s.reset();
  bytes_transferred = xio::write(s, buffers,
      xio::transfer_exactly(10));
  ASIO_CHECK(bytes_transferred == 10);
  ASIO_CHECK(s.check_buffers(buffers, 10));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = xio::write(s, buffers,
      xio::transfer_exactly(10));
  ASIO_CHECK(bytes_transferred == 10);
  ASIO_CHECK(s.check_buffers(buffers, 10));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = xio::write(s, buffers,
      xio::transfer_exactly(10));
  ASIO_CHECK(bytes_transferred == 10);
  ASIO_CHECK(s.check_buffers(buffers, 10));

  s.reset();
  bytes_transferred = xio::write(s, buffers,
      xio::transfer_exactly(42));
  ASIO_CHECK(bytes_transferred == 42);
  ASIO_CHECK(s.check_buffers(buffers, 42));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = xio::write(s, buffers,
      xio::transfer_exactly(42));
  ASIO_CHECK(bytes_transferred == 42);
  ASIO_CHECK(s.check_buffers(buffers, 42));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = xio::write(s, buffers,
      xio::transfer_exactly(42));
  ASIO_CHECK(bytes_transferred == 42);
  ASIO_CHECK(s.check_buffers(buffers, 42));

  s.reset();
  bytes_transferred = xio::write(s, buffers, old_style_transfer_all);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = xio::write(s, buffers, old_style_transfer_all);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = xio::write(s, buffers, old_style_transfer_all);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  bytes_transferred = xio::write(s, buffers, short_transfer());
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = xio::write(s, buffers, short_transfer());
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = xio::write(s, buffers, short_transfer());
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));
}

void test_3_arg_mutable_buffer_write()
{
  xio::io_context ioc;
  test_stream s(ioc);
  xio::mutable_buffer buffers
    = xio::buffer(mutable_write_data, sizeof(mutable_write_data));

  s.reset();
  size_t bytes_transferred = xio::write(s, buffers,
      xio::transfer_all());
  ASIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  ASIO_CHECK(s.check_buffers(buffers, sizeof(mutable_write_data)));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = xio::write(s, buffers,
      xio::transfer_all());
  ASIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  ASIO_CHECK(s.check_buffers(buffers, sizeof(mutable_write_data)));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = xio::write(s, buffers,
      xio::transfer_all());
  ASIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  ASIO_CHECK(s.check_buffers(buffers, sizeof(mutable_write_data)));

  s.reset();
  bytes_transferred = xio::write(s, buffers,
      xio::transfer_at_least(1));
  ASIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  ASIO_CHECK(s.check_buffers(buffers, sizeof(mutable_write_data)));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = xio::write(s, buffers,
      xio::transfer_at_least(1));
  ASIO_CHECK(bytes_transferred == 1);
  ASIO_CHECK(s.check_buffers(buffers, 1));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = xio::write(s, buffers,
      xio::transfer_at_least(1));
  ASIO_CHECK(bytes_transferred == 10);
  ASIO_CHECK(s.check_buffers(buffers, 10));

  s.reset();
  bytes_transferred = xio::write(s, buffers,
      xio::transfer_at_least(10));
  ASIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  ASIO_CHECK(s.check_buffers(buffers, sizeof(mutable_write_data)));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = xio::write(s, buffers,
      xio::transfer_at_least(10));
  ASIO_CHECK(bytes_transferred == 10);
  ASIO_CHECK(s.check_buffers(buffers, 10));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = xio::write(s, buffers,
      xio::transfer_at_least(10));
  ASIO_CHECK(bytes_transferred == 10);
  ASIO_CHECK(s.check_buffers(buffers, 10));

  s.reset();
  bytes_transferred = xio::write(s, buffers,
      xio::transfer_at_least(42));
  ASIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  ASIO_CHECK(s.check_buffers(buffers, sizeof(mutable_write_data)));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = xio::write(s, buffers,
      xio::transfer_at_least(42));
  ASIO_CHECK(bytes_transferred == 42);
  ASIO_CHECK(s.check_buffers(buffers, 42));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = xio::write(s, buffers,
      xio::transfer_at_least(42));
  ASIO_CHECK(bytes_transferred == 50);
  ASIO_CHECK(s.check_buffers(buffers, 50));

  s.reset();
  bytes_transferred = xio::write(s, buffers,
      xio::transfer_exactly(1));
  ASIO_CHECK(bytes_transferred == 1);
  ASIO_CHECK(s.check_buffers(buffers, 1));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = xio::write(s, buffers,
      xio::transfer_exactly(1));
  ASIO_CHECK(bytes_transferred == 1);
  ASIO_CHECK(s.check_buffers(buffers, 1));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = xio::write(s, buffers,
      xio::transfer_exactly(1));
  ASIO_CHECK(bytes_transferred == 1);
  ASIO_CHECK(s.check_buffers(buffers, 1));

  s.reset();
  bytes_transferred = xio::write(s, buffers,
      xio::transfer_exactly(10));
  ASIO_CHECK(bytes_transferred == 10);
  ASIO_CHECK(s.check_buffers(buffers, 10));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = xio::write(s, buffers,
      xio::transfer_exactly(10));
  ASIO_CHECK(bytes_transferred == 10);
  ASIO_CHECK(s.check_buffers(buffers, 10));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = xio::write(s, buffers,
      xio::transfer_exactly(10));
  ASIO_CHECK(bytes_transferred == 10);
  ASIO_CHECK(s.check_buffers(buffers, 10));

  s.reset();
  bytes_transferred = xio::write(s, buffers,
      xio::transfer_exactly(42));
  ASIO_CHECK(bytes_transferred == 42);
  ASIO_CHECK(s.check_buffers(buffers, 42));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = xio::write(s, buffers,
      xio::transfer_exactly(42));
  ASIO_CHECK(bytes_transferred == 42);
  ASIO_CHECK(s.check_buffers(buffers, 42));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = xio::write(s, buffers,
      xio::transfer_exactly(42));
  ASIO_CHECK(bytes_transferred == 42);
  ASIO_CHECK(s.check_buffers(buffers, 42));

  s.reset();
  bytes_transferred = xio::write(s, buffers, old_style_transfer_all);
  ASIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  ASIO_CHECK(s.check_buffers(buffers, sizeof(mutable_write_data)));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = xio::write(s, buffers, old_style_transfer_all);
  ASIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  ASIO_CHECK(s.check_buffers(buffers, sizeof(mutable_write_data)));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = xio::write(s, buffers, old_style_transfer_all);
  ASIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  ASIO_CHECK(s.check_buffers(buffers, sizeof(mutable_write_data)));

  s.reset();
  bytes_transferred = xio::write(s, buffers, short_transfer());
  ASIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  ASIO_CHECK(s.check_buffers(buffers, sizeof(mutable_write_data)));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = xio::write(s, buffers, short_transfer());
  ASIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  ASIO_CHECK(s.check_buffers(buffers, sizeof(mutable_write_data)));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = xio::write(s, buffers, short_transfer());
  ASIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  ASIO_CHECK(s.check_buffers(buffers, sizeof(mutable_write_data)));
}

void test_3_arg_vector_buffers_write()
{
  xio::io_context ioc;
  test_stream s(ioc);
  std::vector<xio::const_buffer> buffers;
  buffers.push_back(xio::buffer(write_data, 32));
  buffers.push_back(xio::buffer(write_data, 39) + 32);
  buffers.push_back(xio::buffer(write_data) + 39);

  s.reset();
  size_t bytes_transferred = xio::write(s, buffers,
      xio::transfer_all());
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = xio::write(s, buffers,
      xio::transfer_all());
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = xio::write(s, buffers,
      xio::transfer_all());
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  bytes_transferred = xio::write(s, buffers,
      xio::transfer_at_least(1));
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = xio::write(s, buffers,
      xio::transfer_at_least(1));
  ASIO_CHECK(bytes_transferred == 1);
  ASIO_CHECK(s.check_buffers(buffers, 1));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = xio::write(s, buffers,
      xio::transfer_at_least(1));
  ASIO_CHECK(bytes_transferred == 10);
  ASIO_CHECK(s.check_buffers(buffers, 10));

  s.reset();
  bytes_transferred = xio::write(s, buffers,
      xio::transfer_at_least(10));
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = xio::write(s, buffers,
      xio::transfer_at_least(10));
  ASIO_CHECK(bytes_transferred == 10);
  ASIO_CHECK(s.check_buffers(buffers, 10));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = xio::write(s, buffers,
      xio::transfer_at_least(10));
  ASIO_CHECK(bytes_transferred == 10);
  ASIO_CHECK(s.check_buffers(buffers, 10));

  s.reset();
  bytes_transferred = xio::write(s, buffers,
      xio::transfer_at_least(42));
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = xio::write(s, buffers,
      xio::transfer_at_least(42));
  ASIO_CHECK(bytes_transferred == 42);
  ASIO_CHECK(s.check_buffers(buffers, 42));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = xio::write(s, buffers,
      xio::transfer_at_least(42));
  ASIO_CHECK(bytes_transferred == 50);
  ASIO_CHECK(s.check_buffers(buffers, 50));

  s.reset();
  bytes_transferred = xio::write(s, buffers,
      xio::transfer_exactly(1));
  ASIO_CHECK(bytes_transferred == 1);
  ASIO_CHECK(s.check_buffers(buffers, 1));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = xio::write(s, buffers,
      xio::transfer_exactly(1));
  ASIO_CHECK(bytes_transferred == 1);
  ASIO_CHECK(s.check_buffers(buffers, 1));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = xio::write(s, buffers,
      xio::transfer_exactly(1));
  ASIO_CHECK(bytes_transferred == 1);
  ASIO_CHECK(s.check_buffers(buffers, 1));

  s.reset();
  bytes_transferred = xio::write(s, buffers,
      xio::transfer_exactly(10));
  ASIO_CHECK(bytes_transferred == 10);
  ASIO_CHECK(s.check_buffers(buffers, 10));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = xio::write(s, buffers,
      xio::transfer_exactly(10));
  ASIO_CHECK(bytes_transferred == 10);
  ASIO_CHECK(s.check_buffers(buffers, 10));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = xio::write(s, buffers,
      xio::transfer_exactly(10));
  ASIO_CHECK(bytes_transferred == 10);
  ASIO_CHECK(s.check_buffers(buffers, 10));

  s.reset();
  bytes_transferred = xio::write(s, buffers,
      xio::transfer_exactly(42));
  ASIO_CHECK(bytes_transferred == 42);
  ASIO_CHECK(s.check_buffers(buffers, 42));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = xio::write(s, buffers,
      xio::transfer_exactly(42));
  ASIO_CHECK(bytes_transferred == 42);
  ASIO_CHECK(s.check_buffers(buffers, 42));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = xio::write(s, buffers,
      xio::transfer_exactly(42));
  ASIO_CHECK(bytes_transferred == 42);
  ASIO_CHECK(s.check_buffers(buffers, 42));

  s.reset();
  bytes_transferred = xio::write(s, buffers, old_style_transfer_all);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = xio::write(s, buffers, old_style_transfer_all);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = xio::write(s, buffers, old_style_transfer_all);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  bytes_transferred = xio::write(s, buffers, short_transfer());
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = xio::write(s, buffers, short_transfer());
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = xio::write(s, buffers, short_transfer());
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));
}

void test_3_arg_dynamic_string_write()
{
  xio::io_context ioc;
  test_stream s(ioc);
  std::string data;
  xio::dynamic_string_buffer<char, std::string::traits_type,
    std::string::allocator_type> sb
      = xio::dynamic_buffer(data, sizeof(write_data));
  xio::const_buffer buffers
    = xio::buffer(write_data, sizeof(write_data));

  s.reset();
  data.assign(write_data, sizeof(write_data));
  size_t bytes_transferred = xio::write(s, sb,
      xio::transfer_all());
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  data.assign(write_data, sizeof(write_data));
  s.next_write_length(1);
  bytes_transferred = xio::write(s, sb,
      xio::transfer_all());
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  data.assign(write_data, sizeof(write_data));
  s.next_write_length(10);
  bytes_transferred = xio::write(s, sb,
      xio::transfer_all());
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  data.assign(write_data, sizeof(write_data));
  bytes_transferred = xio::write(s, sb,
      xio::transfer_at_least(1));
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  data.assign(write_data, sizeof(write_data));
  s.next_write_length(1);
  bytes_transferred = xio::write(s, sb,
      xio::transfer_at_least(1));
  ASIO_CHECK(bytes_transferred == 1);
  ASIO_CHECK(s.check_buffers(buffers, 1));

  s.reset();
  data.assign(write_data, sizeof(write_data));
  s.next_write_length(10);
  bytes_transferred = xio::write(s, sb,
      xio::transfer_at_least(1));
  ASIO_CHECK(bytes_transferred == 10);
  ASIO_CHECK(s.check_buffers(buffers, 10));

  s.reset();
  data.assign(write_data, sizeof(write_data));
  bytes_transferred = xio::write(s, sb,
      xio::transfer_at_least(10));
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  data.assign(write_data, sizeof(write_data));
  s.next_write_length(1);
  bytes_transferred = xio::write(s, sb,
      xio::transfer_at_least(10));
  ASIO_CHECK(bytes_transferred == 10);
  ASIO_CHECK(s.check_buffers(buffers, 10));

  s.reset();
  data.assign(write_data, sizeof(write_data));
  s.next_write_length(10);
  bytes_transferred = xio::write(s, sb,
      xio::transfer_at_least(10));
  ASIO_CHECK(bytes_transferred == 10);
  ASIO_CHECK(s.check_buffers(buffers, 10));

  s.reset();
  data.assign(write_data, sizeof(write_data));
  bytes_transferred = xio::write(s, sb,
      xio::transfer_at_least(42));
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  data.assign(write_data, sizeof(write_data));
  s.next_write_length(1);
  bytes_transferred = xio::write(s, sb,
      xio::transfer_at_least(42));
  ASIO_CHECK(bytes_transferred == 42);
  ASIO_CHECK(s.check_buffers(buffers, 42));

  s.reset();
  data.assign(write_data, sizeof(write_data));
  s.next_write_length(10);
  bytes_transferred = xio::write(s, sb,
      xio::transfer_at_least(42));
  ASIO_CHECK(bytes_transferred == 50);
  ASIO_CHECK(s.check_buffers(buffers, 50));

  s.reset();
  data.assign(write_data, sizeof(write_data));
  bytes_transferred = xio::write(s, sb,
      xio::transfer_exactly(1));
  ASIO_CHECK(bytes_transferred == 1);
  ASIO_CHECK(s.check_buffers(buffers, 1));

  s.reset();
  data.assign(write_data, sizeof(write_data));
  s.next_write_length(1);
  bytes_transferred = xio::write(s, sb,
      xio::transfer_exactly(1));
  ASIO_CHECK(bytes_transferred == 1);
  ASIO_CHECK(s.check_buffers(buffers, 1));

  s.reset();
  data.assign(write_data, sizeof(write_data));
  s.next_write_length(10);
  bytes_transferred = xio::write(s, sb,
      xio::transfer_exactly(1));
  ASIO_CHECK(bytes_transferred == 1);
  ASIO_CHECK(s.check_buffers(buffers, 1));

  s.reset();
  data.assign(write_data, sizeof(write_data));
  bytes_transferred = xio::write(s, sb,
      xio::transfer_exactly(10));
  ASIO_CHECK(bytes_transferred == 10);
  ASIO_CHECK(s.check_buffers(buffers, 10));

  s.reset();
  data.assign(write_data, sizeof(write_data));
  s.next_write_length(1);
  bytes_transferred = xio::write(s, sb,
      xio::transfer_exactly(10));
  ASIO_CHECK(bytes_transferred == 10);
  ASIO_CHECK(s.check_buffers(buffers, 10));

  s.reset();
  data.assign(write_data, sizeof(write_data));
  s.next_write_length(10);
  bytes_transferred = xio::write(s, sb,
      xio::transfer_exactly(10));
  ASIO_CHECK(bytes_transferred == 10);
  ASIO_CHECK(s.check_buffers(buffers, 10));

  s.reset();
  data.assign(write_data, sizeof(write_data));
  bytes_transferred = xio::write(s, sb,
      xio::transfer_exactly(42));
  ASIO_CHECK(bytes_transferred == 42);
  ASIO_CHECK(s.check_buffers(buffers, 42));

  s.reset();
  data.assign(write_data, sizeof(write_data));
  s.next_write_length(1);
  bytes_transferred = xio::write(s, sb,
      xio::transfer_exactly(42));
  ASIO_CHECK(bytes_transferred == 42);
  ASIO_CHECK(s.check_buffers(buffers, 42));

  s.reset();
  data.assign(write_data, sizeof(write_data));
  s.next_write_length(10);
  bytes_transferred = xio::write(s, sb,
      xio::transfer_exactly(42));
  ASIO_CHECK(bytes_transferred == 42);
  ASIO_CHECK(s.check_buffers(buffers, 42));

  s.reset();
  data.assign(write_data, sizeof(write_data));
  bytes_transferred = xio::write(s, sb, old_style_transfer_all);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  data.assign(write_data, sizeof(write_data));
  s.next_write_length(1);
  bytes_transferred = xio::write(s, sb, old_style_transfer_all);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  data.assign(write_data, sizeof(write_data));
  s.next_write_length(10);
  bytes_transferred = xio::write(s, sb, old_style_transfer_all);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  data.assign(write_data, sizeof(write_data));
  bytes_transferred = xio::write(s, sb, short_transfer());
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  data.assign(write_data, sizeof(write_data));
  s.next_write_length(1);
  bytes_transferred = xio::write(s, sb, short_transfer());
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  data.assign(write_data, sizeof(write_data));
  s.next_write_length(10);
  bytes_transferred = xio::write(s, sb, short_transfer());
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));
}

void test_4_arg_const_buffer_write()
{
  xio::io_context ioc;
  test_stream s(ioc);
  xio::const_buffer buffers
    = xio::buffer(write_data, sizeof(write_data));

  s.reset();
  xio::error_code error;
  size_t bytes_transferred = xio::write(s, buffers,
      xio::transfer_all(), error);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = xio::error_code();
  bytes_transferred = xio::write(s, buffers,
      xio::transfer_all(), error);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = xio::error_code();
  bytes_transferred = xio::write(s, buffers,
      xio::transfer_all(), error);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));
  ASIO_CHECK(!error);

  s.reset();
  error = xio::error_code();
  bytes_transferred = xio::write(s, buffers,
      xio::transfer_at_least(1), error);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = xio::error_code();
  bytes_transferred = xio::write(s, buffers,
      xio::transfer_at_least(1), error);
  ASIO_CHECK(bytes_transferred == 1);
  ASIO_CHECK(s.check_buffers(buffers, 1));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = xio::error_code();
  bytes_transferred = xio::write(s, buffers,
      xio::transfer_at_least(1), error);
  ASIO_CHECK(bytes_transferred == 10);
  ASIO_CHECK(s.check_buffers(buffers, 10));
  ASIO_CHECK(!error);

  s.reset();
  error = xio::error_code();
  bytes_transferred = xio::write(s, buffers,
      xio::transfer_at_least(10), error);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = xio::error_code();
  bytes_transferred = xio::write(s, buffers,
      xio::transfer_at_least(10), error);
  ASIO_CHECK(bytes_transferred == 10);
  ASIO_CHECK(s.check_buffers(buffers, 10));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = xio::error_code();
  bytes_transferred = xio::write(s, buffers,
      xio::transfer_at_least(10), error);
  ASIO_CHECK(bytes_transferred == 10);
  ASIO_CHECK(s.check_buffers(buffers, 10));
  ASIO_CHECK(!error);

  s.reset();
  error = xio::error_code();
  bytes_transferred = xio::write(s, buffers,
      xio::transfer_at_least(42), error);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = xio::error_code();
  bytes_transferred = xio::write(s, buffers,
      xio::transfer_at_least(42), error);
  ASIO_CHECK(bytes_transferred == 42);
  ASIO_CHECK(s.check_buffers(buffers, 42));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = xio::error_code();
  bytes_transferred = xio::write(s, buffers,
      xio::transfer_at_least(42), error);
  ASIO_CHECK(bytes_transferred == 50);
  ASIO_CHECK(s.check_buffers(buffers, 50));
  ASIO_CHECK(!error);

  s.reset();
  error = xio::error_code();
  bytes_transferred = xio::write(s, buffers,
      xio::transfer_exactly(1), error);
  ASIO_CHECK(bytes_transferred == 1);
  ASIO_CHECK(s.check_buffers(buffers, 1));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = xio::error_code();
  bytes_transferred = xio::write(s, buffers,
      xio::transfer_exactly(1), error);
  ASIO_CHECK(bytes_transferred == 1);
  ASIO_CHECK(s.check_buffers(buffers, 1));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = xio::error_code();
  bytes_transferred = xio::write(s, buffers,
      xio::transfer_exactly(1), error);
  ASIO_CHECK(bytes_transferred == 1);
  ASIO_CHECK(s.check_buffers(buffers, 1));
  ASIO_CHECK(!error);

  s.reset();
  error = xio::error_code();
  bytes_transferred = xio::write(s, buffers,
      xio::transfer_exactly(10), error);
  ASIO_CHECK(bytes_transferred == 10);
  ASIO_CHECK(s.check_buffers(buffers, 10));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = xio::error_code();
  bytes_transferred = xio::write(s, buffers,
      xio::transfer_exactly(10), error);
  ASIO_CHECK(bytes_transferred == 10);
  ASIO_CHECK(s.check_buffers(buffers, 10));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = xio::error_code();
  bytes_transferred = xio::write(s, buffers,
      xio::transfer_exactly(10), error);
  ASIO_CHECK(bytes_transferred == 10);
  ASIO_CHECK(s.check_buffers(buffers, 10));
  ASIO_CHECK(!error);

  s.reset();
  error = xio::error_code();
  bytes_transferred = xio::write(s, buffers,
      xio::transfer_exactly(42), error);
  ASIO_CHECK(bytes_transferred == 42);
  ASIO_CHECK(s.check_buffers(buffers, 42));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = xio::error_code();
  bytes_transferred = xio::write(s, buffers,
      xio::transfer_exactly(42), error);
  ASIO_CHECK(bytes_transferred == 42);
  ASIO_CHECK(s.check_buffers(buffers, 42));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = xio::error_code();
  bytes_transferred = xio::write(s, buffers,
      xio::transfer_exactly(42), error);
  ASIO_CHECK(bytes_transferred == 42);
  ASIO_CHECK(s.check_buffers(buffers, 42));
  ASIO_CHECK(!error);

  s.reset();
  bytes_transferred = xio::write(s, buffers,
      old_style_transfer_all, error);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = xio::error_code();
  bytes_transferred = xio::write(s, buffers,
      old_style_transfer_all, error);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = xio::error_code();
  bytes_transferred = xio::write(s, buffers,
      old_style_transfer_all, error);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));
  ASIO_CHECK(!error);

  s.reset();
  bytes_transferred = xio::write(s, buffers, short_transfer(), error);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = xio::error_code();
  bytes_transferred = xio::write(s, buffers, short_transfer(), error);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = xio::error_code();
  bytes_transferred = xio::write(s, buffers, short_transfer(), error);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));
  ASIO_CHECK(!error);
}

void test_4_arg_mutable_buffer_write()
{
  xio::io_context ioc;
  test_stream s(ioc);
  xio::mutable_buffer buffers
    = xio::buffer(mutable_write_data, sizeof(mutable_write_data));

  s.reset();
  xio::error_code error;
  size_t bytes_transferred = xio::write(s, buffers,
      xio::transfer_all(), error);
  ASIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  ASIO_CHECK(s.check_buffers(buffers, sizeof(mutable_write_data)));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = xio::error_code();
  bytes_transferred = xio::write(s, buffers,
      xio::transfer_all(), error);
  ASIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  ASIO_CHECK(s.check_buffers(buffers, sizeof(mutable_write_data)));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = xio::error_code();
  bytes_transferred = xio::write(s, buffers,
      xio::transfer_all(), error);
  ASIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  ASIO_CHECK(s.check_buffers(buffers, sizeof(mutable_write_data)));
  ASIO_CHECK(!error);

  s.reset();
  error = xio::error_code();
  bytes_transferred = xio::write(s, buffers,
      xio::transfer_at_least(1), error);
  ASIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  ASIO_CHECK(s.check_buffers(buffers, sizeof(mutable_write_data)));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = xio::error_code();
  bytes_transferred = xio::write(s, buffers,
      xio::transfer_at_least(1), error);
  ASIO_CHECK(bytes_transferred == 1);
  ASIO_CHECK(s.check_buffers(buffers, 1));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = xio::error_code();
  bytes_transferred = xio::write(s, buffers,
      xio::transfer_at_least(1), error);
  ASIO_CHECK(bytes_transferred == 10);
  ASIO_CHECK(s.check_buffers(buffers, 10));
  ASIO_CHECK(!error);

  s.reset();
  error = xio::error_code();
  bytes_transferred = xio::write(s, buffers,
      xio::transfer_at_least(10), error);
  ASIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  ASIO_CHECK(s.check_buffers(buffers, sizeof(mutable_write_data)));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = xio::error_code();
  bytes_transferred = xio::write(s, buffers,
      xio::transfer_at_least(10), error);
  ASIO_CHECK(bytes_transferred == 10);
  ASIO_CHECK(s.check_buffers(buffers, 10));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = xio::error_code();
  bytes_transferred = xio::write(s, buffers,
      xio::transfer_at_least(10), error);
  ASIO_CHECK(bytes_transferred == 10);
  ASIO_CHECK(s.check_buffers(buffers, 10));
  ASIO_CHECK(!error);

  s.reset();
  error = xio::error_code();
  bytes_transferred = xio::write(s, buffers,
      xio::transfer_at_least(42), error);
  ASIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  ASIO_CHECK(s.check_buffers(buffers, sizeof(mutable_write_data)));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = xio::error_code();
  bytes_transferred = xio::write(s, buffers,
      xio::transfer_at_least(42), error);
  ASIO_CHECK(bytes_transferred == 42);
  ASIO_CHECK(s.check_buffers(buffers, 42));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = xio::error_code();
  bytes_transferred = xio::write(s, buffers,
      xio::transfer_at_least(42), error);
  ASIO_CHECK(bytes_transferred == 50);
  ASIO_CHECK(s.check_buffers(buffers, 50));
  ASIO_CHECK(!error);

  s.reset();
  error = xio::error_code();
  bytes_transferred = xio::write(s, buffers,
      xio::transfer_exactly(1), error);
  ASIO_CHECK(bytes_transferred == 1);
  ASIO_CHECK(s.check_buffers(buffers, 1));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = xio::error_code();
  bytes_transferred = xio::write(s, buffers,
      xio::transfer_exactly(1), error);
  ASIO_CHECK(bytes_transferred == 1);
  ASIO_CHECK(s.check_buffers(buffers, 1));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = xio::error_code();
  bytes_transferred = xio::write(s, buffers,
      xio::transfer_exactly(1), error);
  ASIO_CHECK(bytes_transferred == 1);
  ASIO_CHECK(s.check_buffers(buffers, 1));
  ASIO_CHECK(!error);

  s.reset();
  error = xio::error_code();
  bytes_transferred = xio::write(s, buffers,
      xio::transfer_exactly(10), error);
  ASIO_CHECK(bytes_transferred == 10);
  ASIO_CHECK(s.check_buffers(buffers, 10));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = xio::error_code();
  bytes_transferred = xio::write(s, buffers,
      xio::transfer_exactly(10), error);
  ASIO_CHECK(bytes_transferred == 10);
  ASIO_CHECK(s.check_buffers(buffers, 10));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = xio::error_code();
  bytes_transferred = xio::write(s, buffers,
      xio::transfer_exactly(10), error);
  ASIO_CHECK(bytes_transferred == 10);
  ASIO_CHECK(s.check_buffers(buffers, 10));
  ASIO_CHECK(!error);

  s.reset();
  error = xio::error_code();
  bytes_transferred = xio::write(s, buffers,
      xio::transfer_exactly(42), error);
  ASIO_CHECK(bytes_transferred == 42);
  ASIO_CHECK(s.check_buffers(buffers, 42));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = xio::error_code();
  bytes_transferred = xio::write(s, buffers,
      xio::transfer_exactly(42), error);
  ASIO_CHECK(bytes_transferred == 42);
  ASIO_CHECK(s.check_buffers(buffers, 42));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = xio::error_code();
  bytes_transferred = xio::write(s, buffers,
      xio::transfer_exactly(42), error);
  ASIO_CHECK(bytes_transferred == 42);
  ASIO_CHECK(s.check_buffers(buffers, 42));
  ASIO_CHECK(!error);

  s.reset();
  bytes_transferred = xio::write(s, buffers,
      old_style_transfer_all, error);
  ASIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  ASIO_CHECK(s.check_buffers(buffers, sizeof(mutable_write_data)));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = xio::error_code();
  bytes_transferred = xio::write(s, buffers,
      old_style_transfer_all, error);
  ASIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  ASIO_CHECK(s.check_buffers(buffers, sizeof(mutable_write_data)));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = xio::error_code();
  bytes_transferred = xio::write(s, buffers,
      old_style_transfer_all, error);
  ASIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  ASIO_CHECK(s.check_buffers(buffers, sizeof(mutable_write_data)));
  ASIO_CHECK(!error);

  s.reset();
  bytes_transferred = xio::write(s, buffers, short_transfer(), error);
  ASIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  ASIO_CHECK(s.check_buffers(buffers, sizeof(mutable_write_data)));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = xio::error_code();
  bytes_transferred = xio::write(s, buffers, short_transfer(), error);
  ASIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  ASIO_CHECK(s.check_buffers(buffers, sizeof(mutable_write_data)));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = xio::error_code();
  bytes_transferred = xio::write(s, buffers, short_transfer(), error);
  ASIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  ASIO_CHECK(s.check_buffers(buffers, sizeof(mutable_write_data)));
  ASIO_CHECK(!error);
}

void test_4_arg_vector_buffers_write()
{
  xio::io_context ioc;
  test_stream s(ioc);
  std::vector<xio::const_buffer> buffers;
  buffers.push_back(xio::buffer(write_data, 32));
  buffers.push_back(xio::buffer(write_data, 39) + 32);
  buffers.push_back(xio::buffer(write_data) + 39);

  s.reset();
  xio::error_code error;
  size_t bytes_transferred = xio::write(s, buffers,
      xio::transfer_all(), error);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = xio::error_code();
  bytes_transferred = xio::write(s, buffers,
      xio::transfer_all(), error);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = xio::error_code();
  bytes_transferred = xio::write(s, buffers,
      xio::transfer_all(), error);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));
  ASIO_CHECK(!error);

  s.reset();
  error = xio::error_code();
  bytes_transferred = xio::write(s, buffers,
      xio::transfer_at_least(1), error);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = xio::error_code();
  bytes_transferred = xio::write(s, buffers,
      xio::transfer_at_least(1), error);
  ASIO_CHECK(bytes_transferred == 1);
  ASIO_CHECK(s.check_buffers(buffers, 1));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = xio::error_code();
  bytes_transferred = xio::write(s, buffers,
      xio::transfer_at_least(1), error);
  ASIO_CHECK(bytes_transferred == 10);
  ASIO_CHECK(s.check_buffers(buffers, 10));
  ASIO_CHECK(!error);

  s.reset();
  error = xio::error_code();
  bytes_transferred = xio::write(s, buffers,
      xio::transfer_at_least(10), error);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = xio::error_code();
  bytes_transferred = xio::write(s, buffers,
      xio::transfer_at_least(10), error);
  ASIO_CHECK(bytes_transferred == 10);
  ASIO_CHECK(s.check_buffers(buffers, 10));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = xio::error_code();
  bytes_transferred = xio::write(s, buffers,
      xio::transfer_at_least(10), error);
  ASIO_CHECK(bytes_transferred == 10);
  ASIO_CHECK(s.check_buffers(buffers, 10));
  ASIO_CHECK(!error);

  s.reset();
  error = xio::error_code();
  bytes_transferred = xio::write(s, buffers,
      xio::transfer_at_least(42), error);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = xio::error_code();
  bytes_transferred = xio::write(s, buffers,
      xio::transfer_at_least(42), error);
  ASIO_CHECK(bytes_transferred == 42);
  ASIO_CHECK(s.check_buffers(buffers, 42));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = xio::error_code();
  bytes_transferred = xio::write(s, buffers,
      xio::transfer_at_least(42), error);
  ASIO_CHECK(bytes_transferred == 50);
  ASIO_CHECK(s.check_buffers(buffers, 50));
  ASIO_CHECK(!error);

  s.reset();
  error = xio::error_code();
  bytes_transferred = xio::write(s, buffers,
      xio::transfer_exactly(1), error);
  ASIO_CHECK(bytes_transferred == 1);
  ASIO_CHECK(s.check_buffers(buffers, 1));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = xio::error_code();
  bytes_transferred = xio::write(s, buffers,
      xio::transfer_exactly(1), error);
  ASIO_CHECK(bytes_transferred == 1);
  ASIO_CHECK(s.check_buffers(buffers, 1));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = xio::error_code();
  bytes_transferred = xio::write(s, buffers,
      xio::transfer_exactly(1), error);
  ASIO_CHECK(bytes_transferred == 1);
  ASIO_CHECK(s.check_buffers(buffers, 1));
  ASIO_CHECK(!error);

  s.reset();
  error = xio::error_code();
  bytes_transferred = xio::write(s, buffers,
      xio::transfer_exactly(10), error);
  ASIO_CHECK(bytes_transferred == 10);
  ASIO_CHECK(s.check_buffers(buffers, 10));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = xio::error_code();
  bytes_transferred = xio::write(s, buffers,
      xio::transfer_exactly(10), error);
  ASIO_CHECK(bytes_transferred == 10);
  ASIO_CHECK(s.check_buffers(buffers, 10));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = xio::error_code();
  bytes_transferred = xio::write(s, buffers,
      xio::transfer_exactly(10), error);
  ASIO_CHECK(bytes_transferred == 10);
  ASIO_CHECK(s.check_buffers(buffers, 10));
  ASIO_CHECK(!error);

  s.reset();
  error = xio::error_code();
  bytes_transferred = xio::write(s, buffers,
      xio::transfer_exactly(42), error);
  ASIO_CHECK(bytes_transferred == 42);
  ASIO_CHECK(s.check_buffers(buffers, 42));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = xio::error_code();
  bytes_transferred = xio::write(s, buffers,
      xio::transfer_exactly(42), error);
  ASIO_CHECK(bytes_transferred == 42);
  ASIO_CHECK(s.check_buffers(buffers, 42));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = xio::error_code();
  bytes_transferred = xio::write(s, buffers,
      xio::transfer_exactly(42), error);
  ASIO_CHECK(bytes_transferred == 42);
  ASIO_CHECK(s.check_buffers(buffers, 42));
  ASIO_CHECK(!error);

  s.reset();
  bytes_transferred = xio::write(s, buffers,
      old_style_transfer_all, error);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = xio::error_code();
  bytes_transferred = xio::write(s, buffers,
      old_style_transfer_all, error);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = xio::error_code();
  bytes_transferred = xio::write(s, buffers,
      old_style_transfer_all, error);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));
  ASIO_CHECK(!error);

  s.reset();
  bytes_transferred = xio::write(s, buffers, short_transfer(), error);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = xio::error_code();
  bytes_transferred = xio::write(s, buffers, short_transfer(), error);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));
  ASIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = xio::error_code();
  bytes_transferred = xio::write(s, buffers, short_transfer(), error);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));
  ASIO_CHECK(!error);
}

void test_4_arg_dynamic_string_write()
{
  xio::io_context ioc;
  test_stream s(ioc);
  std::string data;
  xio::dynamic_string_buffer<char, std::string::traits_type,
    std::string::allocator_type> sb
      = xio::dynamic_buffer(data, sizeof(write_data));
  xio::const_buffer buffers
    = xio::buffer(write_data, sizeof(write_data));

  s.reset();
  data.assign(write_data, sizeof(write_data));
  xio::error_code error;
  size_t bytes_transferred = xio::write(s, sb,
      xio::transfer_all(), error);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));
  ASIO_CHECK(!error);

  s.reset();
  data.assign(write_data, sizeof(write_data));
  s.next_write_length(1);
  error = xio::error_code();
  bytes_transferred = xio::write(s, sb,
      xio::transfer_all(), error);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));
  ASIO_CHECK(!error);

  s.reset();
  data.assign(write_data, sizeof(write_data));
  s.next_write_length(10);
  error = xio::error_code();
  bytes_transferred = xio::write(s, sb,
      xio::transfer_all(), error);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));
  ASIO_CHECK(!error);

  s.reset();
  data.assign(write_data, sizeof(write_data));
  error = xio::error_code();
  bytes_transferred = xio::write(s, sb,
      xio::transfer_at_least(1), error);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));
  ASIO_CHECK(!error);

  s.reset();
  data.assign(write_data, sizeof(write_data));
  s.next_write_length(1);
  error = xio::error_code();
  bytes_transferred = xio::write(s, sb,
      xio::transfer_at_least(1), error);
  ASIO_CHECK(bytes_transferred == 1);
  ASIO_CHECK(s.check_buffers(buffers, 1));
  ASIO_CHECK(!error);

  s.reset();
  data.assign(write_data, sizeof(write_data));
  s.next_write_length(10);
  error = xio::error_code();
  bytes_transferred = xio::write(s, sb,
      xio::transfer_at_least(1), error);
  ASIO_CHECK(bytes_transferred == 10);
  ASIO_CHECK(s.check_buffers(buffers, 10));
  ASIO_CHECK(!error);

  s.reset();
  data.assign(write_data, sizeof(write_data));
  error = xio::error_code();
  bytes_transferred = xio::write(s, sb,
      xio::transfer_at_least(10), error);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));
  ASIO_CHECK(!error);

  s.reset();
  data.assign(write_data, sizeof(write_data));
  s.next_write_length(1);
  error = xio::error_code();
  bytes_transferred = xio::write(s, sb,
      xio::transfer_at_least(10), error);
  ASIO_CHECK(bytes_transferred == 10);
  ASIO_CHECK(s.check_buffers(buffers, 10));
  ASIO_CHECK(!error);

  s.reset();
  data.assign(write_data, sizeof(write_data));
  s.next_write_length(10);
  error = xio::error_code();
  bytes_transferred = xio::write(s, sb,
      xio::transfer_at_least(10), error);
  ASIO_CHECK(bytes_transferred == 10);
  ASIO_CHECK(s.check_buffers(buffers, 10));
  ASIO_CHECK(!error);

  s.reset();
  data.assign(write_data, sizeof(write_data));
  error = xio::error_code();
  bytes_transferred = xio::write(s, sb,
      xio::transfer_at_least(42), error);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));
  ASIO_CHECK(!error);

  s.reset();
  data.assign(write_data, sizeof(write_data));
  s.next_write_length(1);
  error = xio::error_code();
  bytes_transferred = xio::write(s, sb,
      xio::transfer_at_least(42), error);
  ASIO_CHECK(bytes_transferred == 42);
  ASIO_CHECK(s.check_buffers(buffers, 42));
  ASIO_CHECK(!error);

  s.reset();
  data.assign(write_data, sizeof(write_data));
  s.next_write_length(10);
  error = xio::error_code();
  bytes_transferred = xio::write(s, sb,
      xio::transfer_at_least(42), error);
  ASIO_CHECK(bytes_transferred == 50);
  ASIO_CHECK(s.check_buffers(buffers, 50));
  ASIO_CHECK(!error);

  s.reset();
  data.assign(write_data, sizeof(write_data));
  error = xio::error_code();
  bytes_transferred = xio::write(s, sb,
      xio::transfer_exactly(1), error);
  ASIO_CHECK(bytes_transferred == 1);
  ASIO_CHECK(s.check_buffers(buffers, 1));
  ASIO_CHECK(!error);

  s.reset();
  data.assign(write_data, sizeof(write_data));
  s.next_write_length(1);
  error = xio::error_code();
  bytes_transferred = xio::write(s, sb,
      xio::transfer_exactly(1), error);
  ASIO_CHECK(bytes_transferred == 1);
  ASIO_CHECK(s.check_buffers(buffers, 1));
  ASIO_CHECK(!error);

  s.reset();
  data.assign(write_data, sizeof(write_data));
  s.next_write_length(10);
  error = xio::error_code();
  bytes_transferred = xio::write(s, sb,
      xio::transfer_exactly(1), error);
  ASIO_CHECK(bytes_transferred == 1);
  ASIO_CHECK(s.check_buffers(buffers, 1));
  ASIO_CHECK(!error);

  s.reset();
  data.assign(write_data, sizeof(write_data));
  error = xio::error_code();
  bytes_transferred = xio::write(s, sb,
      xio::transfer_exactly(10), error);
  ASIO_CHECK(bytes_transferred == 10);
  ASIO_CHECK(s.check_buffers(buffers, 10));
  ASIO_CHECK(!error);

  s.reset();
  data.assign(write_data, sizeof(write_data));
  s.next_write_length(1);
  error = xio::error_code();
  bytes_transferred = xio::write(s, sb,
      xio::transfer_exactly(10), error);
  ASIO_CHECK(bytes_transferred == 10);
  ASIO_CHECK(s.check_buffers(buffers, 10));
  ASIO_CHECK(!error);

  s.reset();
  data.assign(write_data, sizeof(write_data));
  s.next_write_length(10);
  error = xio::error_code();
  bytes_transferred = xio::write(s, sb,
      xio::transfer_exactly(10), error);
  ASIO_CHECK(bytes_transferred == 10);
  ASIO_CHECK(s.check_buffers(buffers, 10));
  ASIO_CHECK(!error);

  s.reset();
  data.assign(write_data, sizeof(write_data));
  error = xio::error_code();
  bytes_transferred = xio::write(s, sb,
      xio::transfer_exactly(42), error);
  ASIO_CHECK(bytes_transferred == 42);
  ASIO_CHECK(s.check_buffers(buffers, 42));
  ASIO_CHECK(!error);

  s.reset();
  data.assign(write_data, sizeof(write_data));
  s.next_write_length(1);
  error = xio::error_code();
  bytes_transferred = xio::write(s, sb,
      xio::transfer_exactly(42), error);
  ASIO_CHECK(bytes_transferred == 42);
  ASIO_CHECK(s.check_buffers(buffers, 42));
  ASIO_CHECK(!error);

  s.reset();
  data.assign(write_data, sizeof(write_data));
  s.next_write_length(10);
  error = xio::error_code();
  bytes_transferred = xio::write(s, sb,
      xio::transfer_exactly(42), error);
  ASIO_CHECK(bytes_transferred == 42);
  ASIO_CHECK(s.check_buffers(buffers, 42));
  ASIO_CHECK(!error);

  s.reset();
  data.assign(write_data, sizeof(write_data));
  bytes_transferred = xio::write(s, sb,
      old_style_transfer_all, error);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));
  ASIO_CHECK(!error);

  s.reset();
  data.assign(write_data, sizeof(write_data));
  s.next_write_length(1);
  error = xio::error_code();
  bytes_transferred = xio::write(s, sb,
      old_style_transfer_all, error);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));
  ASIO_CHECK(!error);

  s.reset();
  data.assign(write_data, sizeof(write_data));
  s.next_write_length(10);
  error = xio::error_code();
  bytes_transferred = xio::write(s, sb,
      old_style_transfer_all, error);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));
  ASIO_CHECK(!error);

  s.reset();
  data.assign(write_data, sizeof(write_data));
  bytes_transferred = xio::write(s, sb, short_transfer(), error);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));
  ASIO_CHECK(!error);

  s.reset();
  data.assign(write_data, sizeof(write_data));
  s.next_write_length(1);
  error = xio::error_code();
  bytes_transferred = xio::write(s, sb, short_transfer(), error);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));
  ASIO_CHECK(!error);

  s.reset();
  data.assign(write_data, sizeof(write_data));
  s.next_write_length(10);
  error = xio::error_code();
  bytes_transferred = xio::write(s, sb, short_transfer(), error);
  ASIO_CHECK(bytes_transferred == sizeof(write_data));
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));
  ASIO_CHECK(!error);
}

void async_write_handler(const xio::error_code& e,
    size_t bytes_transferred, size_t expected_bytes_transferred, bool* called)
{
  *called = true;
  ASIO_CHECK(!e);
  ASIO_CHECK(bytes_transferred == expected_bytes_transferred);
}

void test_3_arg_const_buffer_async_write()
{
  namespace bindns = std;
  using bindns::placeholders::_1;
  using bindns::placeholders::_2;

  xio::io_context ioc;
  test_stream s(ioc);
  xio::const_buffer buffers
    = xio::buffer(write_data, sizeof(write_data));

  s.reset();
  bool called = false;
  xio::async_write(s, buffers,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write(s, buffers,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write(s, buffers,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  int i = xio::async_write(s, buffers, archetypes::lazy_handler());
  ASIO_CHECK(i == 42);
  ioc.restart();
  ioc.run();
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  called = false;
  xio::async_write(s, buffers)(
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));
}

void test_3_arg_mutable_buffer_async_write()
{
  namespace bindns = std;
  using bindns::placeholders::_1;
  using bindns::placeholders::_2;

  xio::io_context ioc;
  test_stream s(ioc);
  xio::mutable_buffer buffers
    = xio::buffer(mutable_write_data, sizeof(mutable_write_data));

  s.reset();
  bool called = false;
  xio::async_write(s, buffers,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(mutable_write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, sizeof(mutable_write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write(s, buffers,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(mutable_write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, sizeof(mutable_write_data)));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write(s, buffers,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(mutable_write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, sizeof(mutable_write_data)));

  s.reset();
  int i = xio::async_write(s, buffers, archetypes::lazy_handler());
  ASIO_CHECK(i == 42);
  ioc.restart();
  ioc.run();
  ASIO_CHECK(s.check_buffers(buffers, sizeof(mutable_write_data)));

  s.reset();
  called = false;
  xio::async_write(s, buffers)(
      bindns::bind(async_write_handler,
        _1, _2, sizeof(mutable_write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, sizeof(mutable_write_data)));
}

void test_3_arg_boost_array_buffers_async_write()
{
  namespace bindns = std;
  using bindns::placeholders::_1;
  using bindns::placeholders::_2;

#if defined(ASIO_HAS_BOOST_ARRAY)
  xio::io_context ioc;
  test_stream s(ioc);
  boost::array<xio::const_buffer, 2> buffers = { {
    xio::buffer(write_data, 32),
    xio::buffer(write_data) + 32 } };

  s.reset();
  bool called = false;
  xio::async_write(s, buffers,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write(s, buffers,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write(s, buffers,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  int i = xio::async_write(s, buffers, archetypes::lazy_handler());
  ASIO_CHECK(i == 42);
  ioc.restart();
  ioc.run();
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  called = false;
  xio::async_write(s, buffers)(
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));
#endif // defined(ASIO_HAS_BOOST_ARRAY)
}

void test_3_arg_std_array_buffers_async_write()
{
  namespace bindns = std;
  using bindns::placeholders::_1;
  using bindns::placeholders::_2;

  xio::io_context ioc;
  test_stream s(ioc);
  std::array<xio::const_buffer, 2> buffers = { {
    xio::buffer(write_data, 32),
    xio::buffer(write_data) + 32 } };

  s.reset();
  bool called = false;
  xio::async_write(s, buffers,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write(s, buffers,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write(s, buffers,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  int i = xio::async_write(s, buffers, archetypes::lazy_handler());
  ASIO_CHECK(i == 42);
  ioc.restart();
  ioc.run();
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  called = false;
  xio::async_write(s, buffers)(
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));
}

void test_3_arg_vector_buffers_async_write()
{
  namespace bindns = std;
  using bindns::placeholders::_1;
  using bindns::placeholders::_2;

  xio::io_context ioc;
  test_stream s(ioc);
  std::vector<xio::const_buffer> buffers;
  buffers.push_back(xio::buffer(write_data, 32));
  buffers.push_back(xio::buffer(write_data, 39) + 32);
  buffers.push_back(xio::buffer(write_data) + 39);

  s.reset();
  bool called = false;
  xio::async_write(s, buffers,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write(s, buffers,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write(s, buffers,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  int i = xio::async_write(s, buffers, archetypes::lazy_handler());
  ASIO_CHECK(i == 42);
  ioc.restart();
  ioc.run();
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  called = false;
  xio::async_write(s, buffers)(
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));
}

void test_3_arg_dynamic_string_async_write()
{
  namespace bindns = std;
  using bindns::placeholders::_1;
  using bindns::placeholders::_2;

  xio::io_context ioc;
  test_stream s(ioc);
  std::string data;
  xio::dynamic_string_buffer<char, std::string::traits_type,
    std::string::allocator_type> sb
      = xio::dynamic_buffer(data, sizeof(write_data));
  xio::const_buffer buffers
    = xio::buffer(write_data, sizeof(write_data));

  s.reset();
  data.assign(write_data, sizeof(write_data));
  bool called = false;
  xio::async_write(s, sb,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  data.assign(write_data, sizeof(write_data));
  s.next_write_length(1);
  called = false;
  xio::async_write(s, sb,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  data.assign(write_data, sizeof(write_data));
  s.next_write_length(10);
  called = false;
  xio::async_write(s, sb,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  data.assign(write_data, sizeof(write_data));
  int i = xio::async_write(s, sb, archetypes::lazy_handler());
  ASIO_CHECK(i == 42);
  ioc.restart();
  ioc.run();
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  data.assign(write_data, sizeof(write_data));
  called = false;
  xio::async_write(s, sb)(
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));
}

void test_3_arg_streambuf_async_write()
{
#if !defined(ASIO_NO_DYNAMIC_BUFFER_V1)
  namespace bindns = std;
  using bindns::placeholders::_1;
  using bindns::placeholders::_2;

  xio::io_context ioc;
  test_stream s(ioc);
  xio::streambuf sb;
  xio::const_buffer buffers
    = xio::buffer(write_data, sizeof(write_data));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  bool called = false;
  xio::async_write(s, sb,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  s.next_write_length(1);
  called = false;
  xio::async_write(s, sb,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  s.next_write_length(10);
  called = false;
  xio::async_write(s, sb,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  int i = xio::async_write(s, sb, archetypes::lazy_handler());
  ASIO_CHECK(i == 42);
  ioc.restart();
  ioc.run();
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  s.next_write_length(10);
  called = false;
  xio::async_write(s, sb)(
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));
#endif // !defined(ASIO_NO_DYNAMIC_BUFFER_V1)
}

void test_4_arg_const_buffer_async_write()
{
  namespace bindns = std;
  using bindns::placeholders::_1;
  using bindns::placeholders::_2;

  xio::io_context ioc;
  test_stream s(ioc);
  xio::const_buffer buffers
    = xio::buffer(write_data, sizeof(write_data));

  s.reset();
  bool called = false;
  xio::async_write(s, buffers, xio::transfer_all(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write(s, buffers, xio::transfer_all(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write(s, buffers, xio::transfer_all(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  called = false;
  xio::async_write(s, buffers, xio::transfer_at_least(1),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write(s, buffers, xio::transfer_at_least(1),
      bindns::bind(async_write_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, 1));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write(s, buffers, xio::transfer_at_least(1),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, 10));

  s.reset();
  called = false;
  xio::async_write(s, buffers, xio::transfer_at_least(10),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write(s, buffers, xio::transfer_at_least(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, 10));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write(s, buffers, xio::transfer_at_least(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, 10));

  s.reset();
  called = false;
  xio::async_write(s, buffers, xio::transfer_at_least(42),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write(s, buffers, xio::transfer_at_least(42),
      bindns::bind(async_write_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, 42));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write(s, buffers, xio::transfer_at_least(42),
      bindns::bind(async_write_handler,
        _1, _2, 50, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, 50));

  s.reset();
  called = false;
  xio::async_write(s, buffers, xio::transfer_exactly(1),
      bindns::bind(async_write_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, 1));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write(s, buffers, xio::transfer_exactly(1),
      bindns::bind(async_write_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, 1));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write(s, buffers, xio::transfer_exactly(1),
      bindns::bind(async_write_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, 1));

  s.reset();
  called = false;
  xio::async_write(s, buffers, xio::transfer_exactly(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, 10));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write(s, buffers, xio::transfer_exactly(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, 10));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write(s, buffers, xio::transfer_exactly(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, 10));

  s.reset();
  called = false;
  xio::async_write(s, buffers, xio::transfer_exactly(42),
      bindns::bind(async_write_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, 42));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write(s, buffers, xio::transfer_exactly(42),
      bindns::bind(async_write_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, 42));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write(s, buffers, xio::transfer_exactly(42),
      bindns::bind(async_write_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, 42));

  s.reset();
  called = false;
  xio::async_write(s, buffers, old_style_transfer_all,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write(s, buffers, old_style_transfer_all,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write(s, buffers, old_style_transfer_all,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  called = false;
  xio::async_write(s, buffers, short_transfer(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write(s, buffers, short_transfer(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write(s, buffers, short_transfer(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  int i = xio::async_write(s, buffers, short_transfer(),
      archetypes::lazy_handler());
  ASIO_CHECK(i == 42);
  ioc.restart();
  ioc.run();
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  called = false;
  xio::async_write(s, buffers, short_transfer())(
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));
}

void test_4_arg_mutable_buffer_async_write()
{
  namespace bindns = std;
  using bindns::placeholders::_1;
  using bindns::placeholders::_2;

  xio::io_context ioc;
  test_stream s(ioc);
  xio::mutable_buffer buffers
    = xio::buffer(mutable_write_data, sizeof(mutable_write_data));

  s.reset();
  bool called = false;
  xio::async_write(s, buffers, xio::transfer_all(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(mutable_write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, sizeof(mutable_write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write(s, buffers, xio::transfer_all(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(mutable_write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, sizeof(mutable_write_data)));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write(s, buffers, xio::transfer_all(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(mutable_write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, sizeof(mutable_write_data)));

  s.reset();
  called = false;
  xio::async_write(s, buffers, xio::transfer_at_least(1),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(mutable_write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, sizeof(mutable_write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write(s, buffers, xio::transfer_at_least(1),
      bindns::bind(async_write_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, 1));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write(s, buffers, xio::transfer_at_least(1),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, 10));

  s.reset();
  called = false;
  xio::async_write(s, buffers, xio::transfer_at_least(10),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(mutable_write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, sizeof(mutable_write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write(s, buffers, xio::transfer_at_least(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, 10));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write(s, buffers, xio::transfer_at_least(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, 10));

  s.reset();
  called = false;
  xio::async_write(s, buffers, xio::transfer_at_least(42),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(mutable_write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, sizeof(mutable_write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write(s, buffers, xio::transfer_at_least(42),
      bindns::bind(async_write_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, 42));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write(s, buffers, xio::transfer_at_least(42),
      bindns::bind(async_write_handler,
        _1, _2, 50, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, 50));

  s.reset();
  called = false;
  xio::async_write(s, buffers, xio::transfer_exactly(1),
      bindns::bind(async_write_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, 1));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write(s, buffers, xio::transfer_exactly(1),
      bindns::bind(async_write_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, 1));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write(s, buffers, xio::transfer_exactly(1),
      bindns::bind(async_write_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, 1));

  s.reset();
  called = false;
  xio::async_write(s, buffers, xio::transfer_exactly(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, 10));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write(s, buffers, xio::transfer_exactly(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, 10));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write(s, buffers, xio::transfer_exactly(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, 10));

  s.reset();
  called = false;
  xio::async_write(s, buffers, xio::transfer_exactly(42),
      bindns::bind(async_write_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, 42));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write(s, buffers, xio::transfer_exactly(42),
      bindns::bind(async_write_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, 42));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write(s, buffers, xio::transfer_exactly(42),
      bindns::bind(async_write_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, 42));

  s.reset();
  called = false;
  xio::async_write(s, buffers, old_style_transfer_all,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(mutable_write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, sizeof(mutable_write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write(s, buffers, old_style_transfer_all,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(mutable_write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, sizeof(mutable_write_data)));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write(s, buffers, old_style_transfer_all,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(mutable_write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, sizeof(mutable_write_data)));

  s.reset();
  called = false;
  xio::async_write(s, buffers, short_transfer(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(mutable_write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, sizeof(mutable_write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write(s, buffers, short_transfer(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(mutable_write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, sizeof(mutable_write_data)));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write(s, buffers, short_transfer(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(mutable_write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, sizeof(mutable_write_data)));

  s.reset();
  int i = xio::async_write(s, buffers, short_transfer(),
      archetypes::lazy_handler());
  ASIO_CHECK(i == 42);
  ioc.restart();
  ioc.run();
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  called = false;
  xio::async_write(s, buffers, short_transfer())(
      bindns::bind(async_write_handler,
        _1, _2, sizeof(mutable_write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, sizeof(mutable_write_data)));
}

void test_4_arg_boost_array_buffers_async_write()
{
  namespace bindns = std;
  using bindns::placeholders::_1;
  using bindns::placeholders::_2;

#if defined(ASIO_HAS_BOOST_ARRAY)
  xio::io_context ioc;
  test_stream s(ioc);
  boost::array<xio::const_buffer, 2> buffers = { {
    xio::buffer(write_data, 32),
    xio::buffer(write_data) + 32 } };

  s.reset();
  bool called = false;
  xio::async_write(s, buffers, xio::transfer_all(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write(s, buffers, xio::transfer_all(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write(s, buffers, xio::transfer_all(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  called = false;
  xio::async_write(s, buffers, xio::transfer_at_least(1),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write(s, buffers, xio::transfer_at_least(1),
      bindns::bind(async_write_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, 1));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write(s, buffers, xio::transfer_at_least(1),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, 10));

  s.reset();
  called = false;
  xio::async_write(s, buffers, xio::transfer_at_least(10),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write(s, buffers, xio::transfer_at_least(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, 10));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write(s, buffers, xio::transfer_at_least(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, 10));

  s.reset();
  called = false;
  xio::async_write(s, buffers, xio::transfer_at_least(42),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write(s, buffers, xio::transfer_at_least(42),
      bindns::bind(async_write_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, 42));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write(s, buffers, xio::transfer_at_least(42),
      bindns::bind(async_write_handler,
        _1, _2, 50, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, 50));

  s.reset();
  called = false;
  xio::async_write(s, buffers, xio::transfer_exactly(1),
      bindns::bind(async_write_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, 1));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write(s, buffers, xio::transfer_exactly(1),
      bindns::bind(async_write_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, 1));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write(s, buffers, xio::transfer_exactly(1),
      bindns::bind(async_write_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, 1));

  s.reset();
  called = false;
  xio::async_write(s, buffers, xio::transfer_exactly(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, 10));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write(s, buffers, xio::transfer_exactly(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, 10));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write(s, buffers, xio::transfer_exactly(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, 10));

  s.reset();
  called = false;
  xio::async_write(s, buffers, xio::transfer_exactly(42),
      bindns::bind(async_write_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, 42));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write(s, buffers, xio::transfer_exactly(42),
      bindns::bind(async_write_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, 42));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write(s, buffers, xio::transfer_exactly(42),
      bindns::bind(async_write_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, 42));

  s.reset();
  called = false;
  xio::async_write(s, buffers, old_style_transfer_all,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write(s, buffers, old_style_transfer_all,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write(s, buffers, old_style_transfer_all,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  called = false;
  xio::async_write(s, buffers, short_transfer(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write(s, buffers, short_transfer(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write(s, buffers, short_transfer(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  int i = xio::async_write(s, buffers, short_transfer(),
      archetypes::lazy_handler());
  ASIO_CHECK(i == 42);
  ioc.restart();
  ioc.run();
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  called = false;
  xio::async_write(s, buffers, short_transfer())(
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));
#endif // defined(ASIO_HAS_BOOST_ARRAY)
}

void test_4_arg_std_array_buffers_async_write()
{
  namespace bindns = std;
  using bindns::placeholders::_1;
  using bindns::placeholders::_2;

  xio::io_context ioc;
  test_stream s(ioc);
  std::array<xio::const_buffer, 2> buffers = { {
    xio::buffer(write_data, 32),
    xio::buffer(write_data) + 32 } };

  s.reset();
  bool called = false;
  xio::async_write(s, buffers, xio::transfer_all(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write(s, buffers, xio::transfer_all(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write(s, buffers, xio::transfer_all(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  called = false;
  xio::async_write(s, buffers, xio::transfer_at_least(1),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write(s, buffers, xio::transfer_at_least(1),
      bindns::bind(async_write_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, 1));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write(s, buffers, xio::transfer_at_least(1),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, 10));

  s.reset();
  called = false;
  xio::async_write(s, buffers, xio::transfer_at_least(10),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write(s, buffers, xio::transfer_at_least(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, 10));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write(s, buffers, xio::transfer_at_least(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, 10));

  s.reset();
  called = false;
  xio::async_write(s, buffers, xio::transfer_at_least(42),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write(s, buffers, xio::transfer_at_least(42),
      bindns::bind(async_write_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, 42));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write(s, buffers, xio::transfer_at_least(42),
      bindns::bind(async_write_handler,
        _1, _2, 50, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, 50));

  s.reset();
  called = false;
  xio::async_write(s, buffers, xio::transfer_exactly(1),
      bindns::bind(async_write_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, 1));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write(s, buffers, xio::transfer_exactly(1),
      bindns::bind(async_write_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, 1));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write(s, buffers, xio::transfer_exactly(1),
      bindns::bind(async_write_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, 1));

  s.reset();
  called = false;
  xio::async_write(s, buffers, xio::transfer_exactly(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, 10));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write(s, buffers, xio::transfer_exactly(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, 10));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write(s, buffers, xio::transfer_exactly(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, 10));

  s.reset();
  called = false;
  xio::async_write(s, buffers, xio::transfer_exactly(42),
      bindns::bind(async_write_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, 42));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write(s, buffers, xio::transfer_exactly(42),
      bindns::bind(async_write_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, 42));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write(s, buffers, xio::transfer_exactly(42),
      bindns::bind(async_write_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, 42));

  s.reset();
  called = false;
  xio::async_write(s, buffers, old_style_transfer_all,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write(s, buffers, old_style_transfer_all,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write(s, buffers, old_style_transfer_all,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  called = false;
  xio::async_write(s, buffers, short_transfer(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write(s, buffers, short_transfer(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write(s, buffers, short_transfer(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  int i = xio::async_write(s, buffers, short_transfer(),
      archetypes::lazy_handler());
  ASIO_CHECK(i == 42);
  ioc.restart();
  ioc.run();
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  called = false;
  xio::async_write(s, buffers, short_transfer())(
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));
}

void test_4_arg_vector_buffers_async_write()
{
  namespace bindns = std;
  using bindns::placeholders::_1;
  using bindns::placeholders::_2;

  xio::io_context ioc;
  test_stream s(ioc);
  std::vector<xio::const_buffer> buffers;
  buffers.push_back(xio::buffer(write_data, 32));
  buffers.push_back(xio::buffer(write_data, 39) + 32);
  buffers.push_back(xio::buffer(write_data) + 39);

  s.reset();
  bool called = false;
  xio::async_write(s, buffers, xio::transfer_all(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write(s, buffers, xio::transfer_all(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write(s, buffers, xio::transfer_all(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  called = false;
  xio::async_write(s, buffers, xio::transfer_at_least(1),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write(s, buffers, xio::transfer_at_least(1),
      bindns::bind(async_write_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, 1));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write(s, buffers, xio::transfer_at_least(1),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, 10));

  s.reset();
  called = false;
  xio::async_write(s, buffers, xio::transfer_at_least(10),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write(s, buffers, xio::transfer_at_least(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, 10));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write(s, buffers, xio::transfer_at_least(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, 10));

  s.reset();
  called = false;
  xio::async_write(s, buffers, xio::transfer_at_least(42),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write(s, buffers, xio::transfer_at_least(42),
      bindns::bind(async_write_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, 42));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write(s, buffers, xio::transfer_at_least(42),
      bindns::bind(async_write_handler,
        _1, _2, 50, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, 50));

  s.reset();
  called = false;
  xio::async_write(s, buffers, xio::transfer_exactly(1),
      bindns::bind(async_write_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, 1));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write(s, buffers, xio::transfer_exactly(1),
      bindns::bind(async_write_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, 1));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write(s, buffers, xio::transfer_exactly(1),
      bindns::bind(async_write_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, 1));

  s.reset();
  called = false;
  xio::async_write(s, buffers, xio::transfer_exactly(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, 10));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write(s, buffers, xio::transfer_exactly(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, 10));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write(s, buffers, xio::transfer_exactly(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, 10));

  s.reset();
  called = false;
  xio::async_write(s, buffers, xio::transfer_exactly(42),
      bindns::bind(async_write_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, 42));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write(s, buffers, xio::transfer_exactly(42),
      bindns::bind(async_write_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, 42));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write(s, buffers, xio::transfer_exactly(42),
      bindns::bind(async_write_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, 42));

  s.reset();
  called = false;
  xio::async_write(s, buffers, old_style_transfer_all,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write(s, buffers, old_style_transfer_all,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write(s, buffers, old_style_transfer_all,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  called = false;
  xio::async_write(s, buffers, short_transfer(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write(s, buffers, short_transfer(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write(s, buffers, short_transfer(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  int i = xio::async_write(s, buffers, short_transfer(),
      archetypes::lazy_handler());
  ASIO_CHECK(i == 42);
  ioc.restart();
  ioc.run();
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  called = false;
  xio::async_write(s, buffers, short_transfer())(
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));
}

void test_4_arg_dynamic_string_async_write()
{
  namespace bindns = std;
  using bindns::placeholders::_1;
  using bindns::placeholders::_2;

  xio::io_context ioc;
  test_stream s(ioc);
  std::string data;
  xio::dynamic_string_buffer<char, std::string::traits_type,
    std::string::allocator_type> sb
      = xio::dynamic_buffer(data, sizeof(write_data));
  xio::const_buffer buffers
    = xio::buffer(write_data, sizeof(write_data));

  s.reset();
  data.assign(write_data, sizeof(write_data));
  bool called = false;
  xio::async_write(s, sb, xio::transfer_all(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  data.assign(write_data, sizeof(write_data));
  s.next_write_length(1);
  called = false;
  xio::async_write(s, sb, xio::transfer_all(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  data.assign(write_data, sizeof(write_data));
  s.next_write_length(10);
  called = false;
  xio::async_write(s, sb, xio::transfer_all(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  data.assign(write_data, sizeof(write_data));
  called = false;
  xio::async_write(s, sb, xio::transfer_at_least(1),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  data.assign(write_data, sizeof(write_data));
  s.next_write_length(1);
  called = false;
  xio::async_write(s, sb, xio::transfer_at_least(1),
      bindns::bind(async_write_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, 1));

  s.reset();
  data.assign(write_data, sizeof(write_data));
  s.next_write_length(10);
  called = false;
  xio::async_write(s, sb, xio::transfer_at_least(1),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, 10));

  s.reset();
  data.assign(write_data, sizeof(write_data));
  called = false;
  xio::async_write(s, sb, xio::transfer_at_least(10),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  data.assign(write_data, sizeof(write_data));
  s.next_write_length(1);
  called = false;
  xio::async_write(s, sb, xio::transfer_at_least(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, 10));

  s.reset();
  data.assign(write_data, sizeof(write_data));
  s.next_write_length(10);
  called = false;
  xio::async_write(s, sb, xio::transfer_at_least(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, 10));

  s.reset();
  data.assign(write_data, sizeof(write_data));
  called = false;
  xio::async_write(s, sb, xio::transfer_at_least(42),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  data.assign(write_data, sizeof(write_data));
  s.next_write_length(1);
  called = false;
  xio::async_write(s, sb, xio::transfer_at_least(42),
      bindns::bind(async_write_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, 42));

  s.reset();
  data.assign(write_data, sizeof(write_data));
  s.next_write_length(10);
  called = false;
  xio::async_write(s, sb, xio::transfer_at_least(42),
      bindns::bind(async_write_handler,
        _1, _2, 50, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, 50));

  s.reset();
  data.assign(write_data, sizeof(write_data));
  called = false;
  xio::async_write(s, sb, xio::transfer_exactly(1),
      bindns::bind(async_write_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, 1));

  s.reset();
  data.assign(write_data, sizeof(write_data));
  s.next_write_length(1);
  called = false;
  xio::async_write(s, sb, xio::transfer_exactly(1),
      bindns::bind(async_write_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, 1));

  s.reset();
  data.assign(write_data, sizeof(write_data));
  s.next_write_length(10);
  called = false;
  xio::async_write(s, sb, xio::transfer_exactly(1),
      bindns::bind(async_write_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, 1));

  s.reset();
  data.assign(write_data, sizeof(write_data));
  called = false;
  xio::async_write(s, sb, xio::transfer_exactly(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, 10));

  s.reset();
  data.assign(write_data, sizeof(write_data));
  s.next_write_length(1);
  called = false;
  xio::async_write(s, sb, xio::transfer_exactly(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, 10));

  s.reset();
  data.assign(write_data, sizeof(write_data));
  s.next_write_length(10);
  called = false;
  xio::async_write(s, sb, xio::transfer_exactly(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, 10));

  s.reset();
  data.assign(write_data, sizeof(write_data));
  called = false;
  xio::async_write(s, sb, xio::transfer_exactly(42),
      bindns::bind(async_write_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, 42));

  s.reset();
  data.assign(write_data, sizeof(write_data));
  s.next_write_length(1);
  called = false;
  xio::async_write(s, sb, xio::transfer_exactly(42),
      bindns::bind(async_write_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, 42));

  s.reset();
  data.assign(write_data, sizeof(write_data));
  s.next_write_length(10);
  called = false;
  xio::async_write(s, sb, xio::transfer_exactly(42),
      bindns::bind(async_write_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, 42));

  s.reset();
  data.assign(write_data, sizeof(write_data));
  called = false;
  xio::async_write(s, sb, old_style_transfer_all,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  data.assign(write_data, sizeof(write_data));
  s.next_write_length(1);
  called = false;
  xio::async_write(s, sb, old_style_transfer_all,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  data.assign(write_data, sizeof(write_data));
  s.next_write_length(10);
  called = false;
  xio::async_write(s, sb, old_style_transfer_all,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  data.assign(write_data, sizeof(write_data));
  called = false;
  xio::async_write(s, sb, short_transfer(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  data.assign(write_data, sizeof(write_data));
  s.next_write_length(1);
  called = false;
  xio::async_write(s, sb, short_transfer(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  data.assign(write_data, sizeof(write_data));
  s.next_write_length(10);
  called = false;
  xio::async_write(s, sb, short_transfer(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  data.assign(write_data, sizeof(write_data));
  int i = xio::async_write(s, sb, short_transfer(),
      archetypes::lazy_handler());
  ASIO_CHECK(i == 42);
  ioc.restart();
  ioc.run();
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  data.assign(write_data, sizeof(write_data));
  called = false;
  xio::async_write(s, sb, short_transfer())(
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));
}

void test_4_arg_streambuf_async_write()
{
#if !defined(ASIO_NO_DYNAMIC_BUFFER_V1)
  namespace bindns = std;
  using bindns::placeholders::_1;
  using bindns::placeholders::_2;

  xio::io_context ioc;
  test_stream s(ioc);
  xio::streambuf sb;
  xio::const_buffer buffers
    = xio::buffer(write_data, sizeof(write_data));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  bool called = false;
  xio::async_write(s, sb, xio::transfer_all(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  s.next_write_length(1);
  called = false;
  xio::async_write(s, sb, xio::transfer_all(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  s.next_write_length(10);
  called = false;
  xio::async_write(s, sb, xio::transfer_all(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  called = false;
  xio::async_write(s, sb, xio::transfer_at_least(1),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  s.next_write_length(1);
  called = false;
  xio::async_write(s, sb, xio::transfer_at_least(1),
      bindns::bind(async_write_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, 1));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  s.next_write_length(10);
  called = false;
  xio::async_write(s, sb, xio::transfer_at_least(1),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, 10));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  called = false;
  xio::async_write(s, sb, xio::transfer_at_least(10),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  s.next_write_length(1);
  called = false;
  xio::async_write(s, sb, xio::transfer_at_least(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, 10));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  s.next_write_length(10);
  called = false;
  xio::async_write(s, sb, xio::transfer_at_least(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, 10));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  called = false;
  xio::async_write(s, sb, xio::transfer_at_least(42),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  s.next_write_length(1);
  called = false;
  xio::async_write(s, sb, xio::transfer_at_least(42),
      bindns::bind(async_write_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, 42));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  s.next_write_length(10);
  called = false;
  xio::async_write(s, sb, xio::transfer_at_least(42),
      bindns::bind(async_write_handler,
        _1, _2, 50, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, 50));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  called = false;
  xio::async_write(s, sb, xio::transfer_exactly(1),
      bindns::bind(async_write_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, 1));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  s.next_write_length(1);
  called = false;
  xio::async_write(s, sb, xio::transfer_exactly(1),
      bindns::bind(async_write_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, 1));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  s.next_write_length(10);
  called = false;
  xio::async_write(s, sb, xio::transfer_exactly(1),
      bindns::bind(async_write_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, 1));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  called = false;
  xio::async_write(s, sb, xio::transfer_exactly(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, 10));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  s.next_write_length(1);
  called = false;
  xio::async_write(s, sb, xio::transfer_exactly(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, 10));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  s.next_write_length(10);
  called = false;
  xio::async_write(s, sb, xio::transfer_exactly(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, 10));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  called = false;
  xio::async_write(s, sb, xio::transfer_exactly(42),
      bindns::bind(async_write_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, 42));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  s.next_write_length(1);
  called = false;
  xio::async_write(s, sb, xio::transfer_exactly(42),
      bindns::bind(async_write_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, 42));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  s.next_write_length(10);
  called = false;
  xio::async_write(s, sb, xio::transfer_exactly(42),
      bindns::bind(async_write_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, 42));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  called = false;
  xio::async_write(s, sb, old_style_transfer_all,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  s.next_write_length(1);
  called = false;
  xio::async_write(s, sb, old_style_transfer_all,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  s.next_write_length(10);
  called = false;
  xio::async_write(s, sb, old_style_transfer_all,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  called = false;
  xio::async_write(s, sb, short_transfer(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  s.next_write_length(1);
  called = false;
  xio::async_write(s, sb, short_transfer(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  s.next_write_length(10);
  called = false;
  xio::async_write(s, sb, short_transfer(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  int i = xio::async_write(s, sb, short_transfer(),
      archetypes::lazy_handler());
  ASIO_CHECK(i == 42);
  ioc.restart();
  ioc.run();
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  called = false;
  xio::async_write(s, sb, short_transfer())(
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  ASIO_CHECK(called);
  ASIO_CHECK(s.check_buffers(buffers, sizeof(write_data)));
#endif // !defined(ASIO_NO_DYNAMIC_BUFFER_V1)
}

ASIO_TEST_SUITE
(
  "write",
  ASIO_TEST_CASE(test_2_arg_zero_buffers_write)
  ASIO_TEST_CASE(test_2_arg_const_buffer_write)
  ASIO_TEST_CASE(test_2_arg_mutable_buffer_write)
  ASIO_TEST_CASE(test_2_arg_vector_buffers_write)
  ASIO_TEST_CASE(test_2_arg_dynamic_string_write)
  ASIO_TEST_CASE(test_3_arg_nothrow_zero_buffers_write)
  ASIO_TEST_CASE(test_3_arg_nothrow_const_buffer_write)
  ASIO_TEST_CASE(test_3_arg_nothrow_mutable_buffer_write)
  ASIO_TEST_CASE(test_3_arg_nothrow_vector_buffers_write)
  ASIO_TEST_CASE(test_3_arg_nothrow_dynamic_string_write)
  ASIO_TEST_CASE(test_3_arg_const_buffer_write)
  ASIO_TEST_CASE(test_3_arg_mutable_buffer_write)
  ASIO_TEST_CASE(test_3_arg_vector_buffers_write)
  ASIO_TEST_CASE(test_3_arg_dynamic_string_write)
  ASIO_TEST_CASE(test_4_arg_const_buffer_write)
  ASIO_TEST_CASE(test_4_arg_mutable_buffer_write)
  ASIO_TEST_CASE(test_4_arg_vector_buffers_write)
  ASIO_TEST_CASE(test_4_arg_dynamic_string_write)
  ASIO_TEST_CASE(test_3_arg_const_buffer_async_write)
  ASIO_TEST_CASE(test_3_arg_mutable_buffer_async_write)
  ASIO_TEST_CASE(test_3_arg_boost_array_buffers_async_write)
  ASIO_TEST_CASE(test_3_arg_std_array_buffers_async_write)
  ASIO_TEST_CASE(test_3_arg_vector_buffers_async_write)
  ASIO_TEST_CASE(test_3_arg_dynamic_string_async_write)
  ASIO_TEST_CASE(test_3_arg_streambuf_async_write)
  ASIO_TEST_CASE(test_4_arg_const_buffer_async_write)
  ASIO_TEST_CASE(test_4_arg_mutable_buffer_async_write)
  ASIO_TEST_CASE(test_4_arg_boost_array_buffers_async_write)
  ASIO_TEST_CASE(test_4_arg_std_array_buffers_async_write)
  ASIO_TEST_CASE(test_4_arg_vector_buffers_async_write)
  ASIO_TEST_CASE(test_4_arg_dynamic_string_async_write)
  ASIO_TEST_CASE(test_4_arg_streambuf_async_write)
)
