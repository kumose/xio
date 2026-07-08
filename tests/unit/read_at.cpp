//
// read_at.cpp
// ~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//



// Test that header file is self-contained.
#include <xio/read_at.h>

#include <array>
#include <cstring>
#include <functional>
#include "archetypes/async_result.hpp"
#include <xio/io_context.h>
#include <xio/post.h>
#include <xio/streambuf.h>
#include "unit_test.hpp"

using namespace std; // For memcmp, memcpy and memset.

class test_random_access_device
{
public:
  typedef xio::io_context::executor_type executor_type;

  test_random_access_device(xio::io_context& io_context)
    : io_context_(io_context),
      length_(0),
      next_read_length_(0)
  {
  }

  executor_type get_executor() noexcept
  {
    return io_context_.get_executor();
  }

  void reset(const void* data, size_t length)
  {
    XIO_CHECK(length <= max_length);

    length_ = 0;
    while (length_ + length < max_length)
    {
      memcpy(data_ + length_, data, length);
      length_ += length;
    }

    next_read_length_ = length;
  }

  void next_read_length(size_t length)
  {
    next_read_length_ = length;
  }

  template <typename Iterator>
  bool check_buffers(uint64_t offset,
      Iterator begin, Iterator end, size_t length)
  {
    if (offset + length > max_length)
      return false;

    Iterator iter = begin;
    size_t checked_length = 0;
    for (; iter != end && checked_length < length; ++iter)
    {
      size_t buffer_length = xio::buffer_size(*iter);
      if (buffer_length > length - checked_length)
        buffer_length = length - checked_length;
      if (memcmp(data_ + offset + checked_length,
            iter->data(), buffer_length) != 0)
        return false;
      checked_length += buffer_length;
    }

    return true;
  }

  template <typename Const_Buffers>
  bool check_buffers(uint64_t offset,
      const Const_Buffers& buffers, size_t length)
  {
    return check_buffers(offset, xio::buffer_sequence_begin(buffers),
        xio::buffer_sequence_end(buffers), length);
  }

  template <typename Mutable_Buffers>
  size_t read_some_at(uint64_t offset,
      const Mutable_Buffers& buffers)
  {
    return xio::buffer_copy(buffers,
        xio::buffer(data_, length_) + offset,
        next_read_length_);
  }

  template <typename Mutable_Buffers>
  size_t read_some_at(uint64_t offset,
      const Mutable_Buffers& buffers, xio::error_code& ec)
  {
    ec = xio::error_code();
    return read_some_at(offset, buffers);
  }

  template <typename Mutable_Buffers, typename Handler>
  void async_read_some_at(uint64_t offset,
      const Mutable_Buffers& buffers, Handler&& handler)
  {
    size_t bytes_transferred = read_some_at(offset, buffers);
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
  size_t next_read_length_;
};

static const char read_data[]
  = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

void test_3_arg_mutable_buffer_read_at()
{
  xio::io_context ioc;
  test_random_access_device s(ioc);
  char read_buf[sizeof(read_data)];
  xio::mutable_buffer buffers
    = xio::buffer(read_buf, sizeof(read_buf));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  size_t bytes_transferred = xio::read_at(s, 0, buffers);
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 1234, buffers);
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 0, buffers);
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 1234, buffers);
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 0, buffers);
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 1234, buffers);
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(read_data)));
}

void test_3_arg_vector_buffers_read_at()
{
  xio::io_context ioc;
  test_random_access_device s(ioc);
  char read_buf[sizeof(read_data)];
  std::vector<xio::mutable_buffer> buffers;
  buffers.push_back(xio::buffer(read_buf, 32));
  buffers.push_back(xio::buffer(read_buf) + 32);

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  size_t bytes_transferred = xio::read_at(s, 0, buffers);
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 1234, buffers);
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 0, buffers);
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 1234, buffers);
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 0, buffers);
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 1234, buffers);
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(read_data)));
}

void test_3_arg_streambuf_read_at()
{
  xio::io_context ioc;
  test_random_access_device s(ioc);
  xio::streambuf sb(sizeof(read_data));

  s.reset(read_data, sizeof(read_data));
  sb.consume(sb.size());
  size_t bytes_transferred = xio::read_at(s, 0, sb);
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(sb.size() == sizeof(read_data));
  XIO_CHECK(s.check_buffers(0, sb.data(), sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  sb.consume(sb.size());
  bytes_transferred = xio::read_at(s, 1234, sb);
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(sb.size() == sizeof(read_data));
  XIO_CHECK(s.check_buffers(1234, sb.data(), sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb.consume(sb.size());
  bytes_transferred = xio::read_at(s, 0, sb);
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(sb.size() == sizeof(read_data));
  XIO_CHECK(s.check_buffers(0, sb.data(), sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb.consume(sb.size());
  bytes_transferred = xio::read_at(s, 1234, sb);
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(sb.size() == sizeof(read_data));
  XIO_CHECK(s.check_buffers(1234, sb.data(), sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb.consume(sb.size());
  bytes_transferred = xio::read_at(s, 0, sb);
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(sb.size() == sizeof(read_data));
  XIO_CHECK(s.check_buffers(0, sb.data(), sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb.consume(sb.size());
  bytes_transferred = xio::read_at(s, 1234, sb);
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(sb.size() == sizeof(read_data));
  XIO_CHECK(s.check_buffers(1234, sb.data(), sizeof(read_data)));
}

void test_4_arg_nothrow_mutable_buffer_read_at()
{
  xio::io_context ioc;
  test_random_access_device s(ioc);
  char read_buf[sizeof(read_data)];
  xio::mutable_buffer buffers
    = xio::buffer(read_buf, sizeof(read_buf));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  xio::error_code error;
  size_t bytes_transferred = xio::read_at(s, 0, buffers, error);
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(read_data)));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 1234, buffers, error);
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(read_data)));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 0, buffers, error);
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(read_data)));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 1234, buffers, error);
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(read_data)));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 0, buffers, error);
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(read_data)));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 1234, buffers, error);
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(read_data)));
  XIO_CHECK(!error);
}

void test_4_arg_nothrow_vector_buffers_read_at()
{
  xio::io_context ioc;
  test_random_access_device s(ioc);
  char read_buf[sizeof(read_data)];
  std::vector<xio::mutable_buffer> buffers;
  buffers.push_back(xio::buffer(read_buf, 32));
  buffers.push_back(xio::buffer(read_buf) + 32);

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  xio::error_code error;
  size_t bytes_transferred = xio::read_at(s, 0, buffers, error);
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(read_data)));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 1234, buffers, error);
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(read_data)));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 0, buffers, error);
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(read_data)));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 1234, buffers, error);
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(read_data)));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 0, buffers, error);
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(read_data)));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 1234, buffers, error);
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(read_data)));
  XIO_CHECK(!error);
}

void test_4_arg_nothrow_streambuf_read_at()
{
  xio::io_context ioc;
  test_random_access_device s(ioc);
  xio::streambuf sb(sizeof(read_data));

  s.reset(read_data, sizeof(read_data));
  sb.consume(sb.size());
  xio::error_code error;
  size_t bytes_transferred = xio::read_at(s, 0, sb, error);
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(sb.size() == sizeof(read_data));
  XIO_CHECK(s.check_buffers(0, sb.data(), sizeof(read_data)));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  sb.consume(sb.size());
  bytes_transferred = xio::read_at(s, 1234, sb, error);
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(sb.size() == sizeof(read_data));
  XIO_CHECK(s.check_buffers(1234, sb.data(), sizeof(read_data)));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb.consume(sb.size());
  bytes_transferred = xio::read_at(s, 0, sb, error);
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(sb.size() == sizeof(read_data));
  XIO_CHECK(s.check_buffers(0, sb.data(), sizeof(read_data)));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb.consume(sb.size());
  bytes_transferred = xio::read_at(s, 1234, sb, error);
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(sb.size() == sizeof(read_data));
  XIO_CHECK(s.check_buffers(1234, sb.data(), sizeof(read_data)));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb.consume(sb.size());
  bytes_transferred = xio::read_at(s, 0, sb, error);
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(sb.size() == sizeof(read_data));
  XIO_CHECK(s.check_buffers(0, sb.data(), sizeof(read_data)));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb.consume(sb.size());
  bytes_transferred = xio::read_at(s, 1234, sb, error);
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(sb.size() == sizeof(read_data));
  XIO_CHECK(s.check_buffers(1234, sb.data(), sizeof(read_data)));
  XIO_CHECK(!error);
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

void test_4_arg_mutable_buffer_read_at()
{
  xio::io_context ioc;
  test_random_access_device s(ioc);
  char read_buf[sizeof(read_data)];
  xio::mutable_buffer buffers
    = xio::buffer(read_buf, sizeof(read_buf));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  size_t bytes_transferred = xio::read_at(s, 0, buffers,
      xio::transfer_all());
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 1234, buffers,
      xio::transfer_all());
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 0, buffers,
      xio::transfer_all());
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 1234, buffers,
      xio::transfer_all());
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 0, buffers,
      xio::transfer_all());
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 1234, buffers,
      xio::transfer_all());
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 0, buffers,
      xio::transfer_at_least(1));
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 1234, buffers,
      xio::transfer_at_least(1));
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 0, buffers,
      xio::transfer_at_least(1));
  XIO_CHECK(bytes_transferred == 1);
  XIO_CHECK(s.check_buffers(0, buffers, 1));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 1234, buffers,
      xio::transfer_at_least(1));
  XIO_CHECK(bytes_transferred == 1);
  XIO_CHECK(s.check_buffers(1234, buffers, 1));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 0, buffers,
      xio::transfer_at_least(1));
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 1234, buffers,
      xio::transfer_at_least(1));
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 0, buffers,
      xio::transfer_at_least(10));
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 1234, buffers,
      xio::transfer_at_least(10));
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 0, buffers,
      xio::transfer_at_least(10));
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 1234, buffers,
      xio::transfer_at_least(10));
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 0, buffers,
      xio::transfer_at_least(10));
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 1234, buffers,
      xio::transfer_at_least(10));
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 0, buffers,
      xio::transfer_at_least(42));
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 1234, buffers,
      xio::transfer_at_least(42));
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 0, buffers,
      xio::transfer_at_least(42));
  XIO_CHECK(bytes_transferred == 42);
  XIO_CHECK(s.check_buffers(0, buffers, 42));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 1234, buffers,
      xio::transfer_at_least(42));
  XIO_CHECK(bytes_transferred == 42);
  XIO_CHECK(s.check_buffers(1234, buffers, 42));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 0, buffers,
      xio::transfer_at_least(42));
  XIO_CHECK(bytes_transferred == 50);
  XIO_CHECK(s.check_buffers(0, buffers, 50));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 1234, buffers,
      xio::transfer_at_least(42));
  XIO_CHECK(bytes_transferred == 50);
  XIO_CHECK(s.check_buffers(1234, buffers, 50));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 0, buffers,
      xio::transfer_exactly(1));
  XIO_CHECK(bytes_transferred == 1);
  XIO_CHECK(s.check_buffers(0, buffers, 1));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 1234, buffers,
      xio::transfer_exactly(1));
  XIO_CHECK(bytes_transferred == 1);
  XIO_CHECK(s.check_buffers(1234, buffers, 1));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 0, buffers,
      xio::transfer_exactly(1));
  XIO_CHECK(bytes_transferred == 1);
  XIO_CHECK(s.check_buffers(0, buffers, 1));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 1234, buffers,
      xio::transfer_exactly(1));
  XIO_CHECK(bytes_transferred == 1);
  XIO_CHECK(s.check_buffers(1234, buffers, 1));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 0, buffers,
      xio::transfer_exactly(1));
  XIO_CHECK(bytes_transferred == 1);
  XIO_CHECK(s.check_buffers(0, buffers, 1));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 1234, buffers,
      xio::transfer_exactly(1));
  XIO_CHECK(bytes_transferred == 1);
  XIO_CHECK(s.check_buffers(1234, buffers, 1));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 0, buffers,
      xio::transfer_exactly(10));
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 1234, buffers,
      xio::transfer_exactly(10));
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 0, buffers,
      xio::transfer_exactly(10));
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 1234, buffers,
      xio::transfer_exactly(10));
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 0, buffers,
      xio::transfer_exactly(10));
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 1234, buffers,
      xio::transfer_exactly(10));
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 0, buffers,
      xio::transfer_exactly(42));
  XIO_CHECK(bytes_transferred == 42);
  XIO_CHECK(s.check_buffers(0, buffers, 42));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 1234, buffers,
      xio::transfer_exactly(42));
  XIO_CHECK(bytes_transferred == 42);
  XIO_CHECK(s.check_buffers(1234, buffers, 42));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 0, buffers,
      xio::transfer_exactly(42));
  XIO_CHECK(bytes_transferred == 42);
  XIO_CHECK(s.check_buffers(0, buffers, 42));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 1234, buffers,
      xio::transfer_exactly(42));
  XIO_CHECK(bytes_transferred == 42);
  XIO_CHECK(s.check_buffers(1234, buffers, 42));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 0, buffers,
      xio::transfer_exactly(42));
  XIO_CHECK(bytes_transferred == 42);
  XIO_CHECK(s.check_buffers(0, buffers, 42));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 1234, buffers,
      xio::transfer_exactly(42));
  XIO_CHECK(bytes_transferred == 42);
  XIO_CHECK(s.check_buffers(1234, buffers, 42));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 0, buffers,
      old_style_transfer_all);
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 1234, buffers,
      old_style_transfer_all);
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 0, buffers,
      old_style_transfer_all);
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 1234, buffers,
      old_style_transfer_all);
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 0, buffers,
      old_style_transfer_all);
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 1234, buffers,
      old_style_transfer_all);
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 0, buffers, short_transfer());
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 1234, buffers, short_transfer());
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 0, buffers, short_transfer());
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 1234, buffers, short_transfer());
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 0, buffers, short_transfer());
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 1234, buffers, short_transfer());
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(read_data)));
}

void test_4_arg_vector_buffers_read_at()
{
  xio::io_context ioc;
  test_random_access_device s(ioc);
  char read_buf[sizeof(read_data)];
  std::vector<xio::mutable_buffer> buffers;
  buffers.push_back(xio::buffer(read_buf, 32));
  buffers.push_back(xio::buffer(read_buf) + 32);

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  size_t bytes_transferred = xio::read_at(s, 0, buffers,
      xio::transfer_all());
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 1234, buffers,
      xio::transfer_all());
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 0, buffers,
      xio::transfer_all());
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 1234, buffers,
      xio::transfer_all());
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 0, buffers,
      xio::transfer_all());
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 1234, buffers,
      xio::transfer_all());
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 0, buffers,
      xio::transfer_at_least(1));
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 1234, buffers,
      xio::transfer_at_least(1));
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 0, buffers,
      xio::transfer_at_least(1));
  XIO_CHECK(bytes_transferred == 1);
  XIO_CHECK(s.check_buffers(0, buffers, 1));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 1234, buffers,
      xio::transfer_at_least(1));
  XIO_CHECK(bytes_transferred == 1);
  XIO_CHECK(s.check_buffers(1234, buffers, 1));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 0, buffers,
      xio::transfer_at_least(1));
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 1234, buffers,
      xio::transfer_at_least(1));
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 0, buffers,
      xio::transfer_at_least(10));
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 1234, buffers,
      xio::transfer_at_least(10));
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 0, buffers,
      xio::transfer_at_least(10));
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 1234, buffers,
      xio::transfer_at_least(10));
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 0, buffers,
      xio::transfer_at_least(10));
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 1234, buffers,
      xio::transfer_at_least(10));
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 0, buffers,
      xio::transfer_at_least(42));
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 1234, buffers,
      xio::transfer_at_least(42));
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 0, buffers,
      xio::transfer_at_least(42));
  XIO_CHECK(bytes_transferred == 42);
  XIO_CHECK(s.check_buffers(0, buffers, 42));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 1234, buffers,
      xio::transfer_at_least(42));
  XIO_CHECK(bytes_transferred == 42);
  XIO_CHECK(s.check_buffers(1234, buffers, 42));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 0, buffers,
      xio::transfer_at_least(42));
  XIO_CHECK(bytes_transferred == 50);
  XIO_CHECK(s.check_buffers(0, buffers, 50));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 1234, buffers,
      xio::transfer_at_least(42));
  XIO_CHECK(bytes_transferred == 50);
  XIO_CHECK(s.check_buffers(1234, buffers, 50));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 0, buffers,
      xio::transfer_exactly(1));
  XIO_CHECK(bytes_transferred == 1);
  XIO_CHECK(s.check_buffers(0, buffers, 1));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 1234, buffers,
      xio::transfer_exactly(1));
  XIO_CHECK(bytes_transferred == 1);
  XIO_CHECK(s.check_buffers(1234, buffers, 1));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 0, buffers,
      xio::transfer_exactly(1));
  XIO_CHECK(bytes_transferred == 1);
  XIO_CHECK(s.check_buffers(0, buffers, 1));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 1234, buffers,
      xio::transfer_exactly(1));
  XIO_CHECK(bytes_transferred == 1);
  XIO_CHECK(s.check_buffers(1234, buffers, 1));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 0, buffers,
      xio::transfer_exactly(1));
  XIO_CHECK(bytes_transferred == 1);
  XIO_CHECK(s.check_buffers(0, buffers, 1));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 1234, buffers,
      xio::transfer_exactly(1));
  XIO_CHECK(bytes_transferred == 1);
  XIO_CHECK(s.check_buffers(1234, buffers, 1));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 0, buffers,
      xio::transfer_exactly(10));
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 1234, buffers,
      xio::transfer_exactly(10));
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 0, buffers,
      xio::transfer_exactly(10));
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 1234, buffers,
      xio::transfer_exactly(10));
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 0, buffers,
      xio::transfer_exactly(10));
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 1234, buffers,
      xio::transfer_exactly(10));
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 0, buffers,
      xio::transfer_exactly(42));
  XIO_CHECK(bytes_transferred == 42);
  XIO_CHECK(s.check_buffers(0, buffers, 42));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 1234, buffers,
      xio::transfer_exactly(42));
  XIO_CHECK(bytes_transferred == 42);
  XIO_CHECK(s.check_buffers(1234, buffers, 42));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 0, buffers,
      xio::transfer_exactly(42));
  XIO_CHECK(bytes_transferred == 42);
  XIO_CHECK(s.check_buffers(0, buffers, 42));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 1234, buffers,
      xio::transfer_exactly(42));
  XIO_CHECK(bytes_transferred == 42);
  XIO_CHECK(s.check_buffers(1234, buffers, 42));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 0, buffers,
      xio::transfer_exactly(42));
  XIO_CHECK(bytes_transferred == 42);
  XIO_CHECK(s.check_buffers(0, buffers, 42));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 1234, buffers,
      xio::transfer_exactly(42));
  XIO_CHECK(bytes_transferred == 42);
  XIO_CHECK(s.check_buffers(1234, buffers, 42));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 0, buffers,
      old_style_transfer_all);
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 1234, buffers,
      old_style_transfer_all);
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 0, buffers,
      old_style_transfer_all);
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 1234, buffers,
      old_style_transfer_all);
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 0, buffers,
      old_style_transfer_all);
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 1234, buffers,
      old_style_transfer_all);
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 0, buffers, short_transfer());
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 1234, buffers, short_transfer());
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 0, buffers, short_transfer());
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 1234, buffers, short_transfer());
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 0, buffers, short_transfer());
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 1234, buffers, short_transfer());
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(read_data)));
}

void test_4_arg_streambuf_read_at()
{
  xio::io_context ioc;
  test_random_access_device s(ioc);
  xio::streambuf sb(sizeof(read_data));

  s.reset(read_data, sizeof(read_data));
  sb.consume(sb.size());
  size_t bytes_transferred = xio::read_at(s, 0, sb,
      xio::transfer_all());
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(sb.size() == sizeof(read_data));
  XIO_CHECK(s.check_buffers(0, sb.data(), sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  sb.consume(sb.size());
  bytes_transferred = xio::read_at(s, 1234, sb,
      xio::transfer_all());
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(sb.size() == sizeof(read_data));
  XIO_CHECK(s.check_buffers(1234, sb.data(), sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb.consume(sb.size());
  bytes_transferred = xio::read_at(s, 0, sb,
      xio::transfer_all());
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(sb.size() == sizeof(read_data));
  XIO_CHECK(s.check_buffers(0, sb.data(), sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb.consume(sb.size());
  bytes_transferred = xio::read_at(s, 1234, sb,
      xio::transfer_all());
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(sb.size() == sizeof(read_data));
  XIO_CHECK(s.check_buffers(1234, sb.data(), sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb.consume(sb.size());
  bytes_transferred = xio::read_at(s, 0, sb,
      xio::transfer_all());
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(sb.size() == sizeof(read_data));
  XIO_CHECK(s.check_buffers(0, sb.data(), sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb.consume(sb.size());
  bytes_transferred = xio::read_at(s, 1234, sb,
      xio::transfer_all());
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(sb.size() == sizeof(read_data));
  XIO_CHECK(s.check_buffers(1234, sb.data(), sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  sb.consume(sb.size());
  bytes_transferred = xio::read_at(s, 0, sb,
      xio::transfer_at_least(1));
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(sb.size() == sizeof(read_data));
  XIO_CHECK(s.check_buffers(0, sb.data(), sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  sb.consume(sb.size());
  bytes_transferred = xio::read_at(s, 1234, sb,
      xio::transfer_at_least(1));
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(sb.size() == sizeof(read_data));
  XIO_CHECK(s.check_buffers(1234, sb.data(), sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb.consume(sb.size());
  bytes_transferred = xio::read_at(s, 0, sb,
      xio::transfer_at_least(1));
  XIO_CHECK(bytes_transferred == 1);
  XIO_CHECK(sb.size() == 1);
  XIO_CHECK(s.check_buffers(0, sb.data(), 1));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb.consume(sb.size());
  bytes_transferred = xio::read_at(s, 1234, sb,
      xio::transfer_at_least(1));
  XIO_CHECK(bytes_transferred == 1);
  XIO_CHECK(sb.size() == 1);
  XIO_CHECK(s.check_buffers(1234, sb.data(), 1));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb.consume(sb.size());
  bytes_transferred = xio::read_at(s, 0, sb,
      xio::transfer_at_least(1));
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(sb.size() == 10);
  XIO_CHECK(s.check_buffers(0, sb.data(), 10));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb.consume(sb.size());
  bytes_transferred = xio::read_at(s, 1234, sb,
      xio::transfer_at_least(1));
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(sb.size() == 10);
  XIO_CHECK(s.check_buffers(1234, sb.data(), 10));

  s.reset(read_data, sizeof(read_data));
  sb.consume(sb.size());
  bytes_transferred = xio::read_at(s, 0, sb,
      xio::transfer_at_least(10));
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(sb.size() == sizeof(read_data));
  XIO_CHECK(s.check_buffers(0, sb.data(), sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  sb.consume(sb.size());
  bytes_transferred = xio::read_at(s, 1234, sb,
      xio::transfer_at_least(10));
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(sb.size() == sizeof(read_data));
  XIO_CHECK(s.check_buffers(1234, sb.data(), sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb.consume(sb.size());
  bytes_transferred = xio::read_at(s, 0, sb,
      xio::transfer_at_least(10));
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(sb.size() == 10);
  XIO_CHECK(s.check_buffers(0, sb.data(), 10));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb.consume(sb.size());
  bytes_transferred = xio::read_at(s, 1234, sb,
      xio::transfer_at_least(10));
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(sb.size() == 10);
  XIO_CHECK(s.check_buffers(1234, sb.data(), 10));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb.consume(sb.size());
  bytes_transferred = xio::read_at(s, 0, sb,
      xio::transfer_at_least(10));
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(sb.size() == 10);
  XIO_CHECK(s.check_buffers(0, sb.data(), 10));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb.consume(sb.size());
  bytes_transferred = xio::read_at(s, 1234, sb,
      xio::transfer_at_least(10));
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(sb.size() == 10);
  XIO_CHECK(s.check_buffers(1234, sb.data(), 10));

  s.reset(read_data, sizeof(read_data));
  sb.consume(sb.size());
  bytes_transferred = xio::read_at(s, 0, sb,
      xio::transfer_at_least(42));
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(sb.size() == sizeof(read_data));
  XIO_CHECK(s.check_buffers(0, sb.data(), sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  sb.consume(sb.size());
  bytes_transferred = xio::read_at(s, 1234, sb,
      xio::transfer_at_least(42));
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(sb.size() == sizeof(read_data));
  XIO_CHECK(s.check_buffers(1234, sb.data(), sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb.consume(sb.size());
  bytes_transferred = xio::read_at(s, 0, sb,
      xio::transfer_at_least(42));
  XIO_CHECK(bytes_transferred == 42);
  XIO_CHECK(sb.size() == 42);
  XIO_CHECK(s.check_buffers(0, sb.data(), 42));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb.consume(sb.size());
  bytes_transferred = xio::read_at(s, 1234, sb,
      xio::transfer_at_least(42));
  XIO_CHECK(bytes_transferred == 42);
  XIO_CHECK(sb.size() == 42);
  XIO_CHECK(s.check_buffers(1234, sb.data(), 42));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb.consume(sb.size());
  bytes_transferred = xio::read_at(s, 0, sb,
      xio::transfer_at_least(42));
  XIO_CHECK(bytes_transferred == 50);
  XIO_CHECK(sb.size() == 50);
  XIO_CHECK(s.check_buffers(0, sb.data(), 50));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb.consume(sb.size());
  bytes_transferred = xio::read_at(s, 1234, sb,
      xio::transfer_at_least(42));
  XIO_CHECK(bytes_transferred == 50);
  XIO_CHECK(sb.size() == 50);
  XIO_CHECK(s.check_buffers(1234, sb.data(), 50));

  s.reset(read_data, sizeof(read_data));
  sb.consume(sb.size());
  bytes_transferred = xio::read_at(s, 0, sb,
      xio::transfer_exactly(1));
  XIO_CHECK(bytes_transferred == 1);
  XIO_CHECK(sb.size() == 1);
  XIO_CHECK(s.check_buffers(0, sb.data(), 1));

  s.reset(read_data, sizeof(read_data));
  sb.consume(sb.size());
  bytes_transferred = xio::read_at(s, 1234, sb,
      xio::transfer_exactly(1));
  XIO_CHECK(bytes_transferred == 1);
  XIO_CHECK(sb.size() == 1);
  XIO_CHECK(s.check_buffers(1234, sb.data(), 1));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb.consume(sb.size());
  bytes_transferred = xio::read_at(s, 0, sb,
      xio::transfer_exactly(1));
  XIO_CHECK(bytes_transferred == 1);
  XIO_CHECK(sb.size() == 1);
  XIO_CHECK(s.check_buffers(0, sb.data(), 1));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb.consume(sb.size());
  bytes_transferred = xio::read_at(s, 1234, sb,
      xio::transfer_exactly(1));
  XIO_CHECK(bytes_transferred == 1);
  XIO_CHECK(sb.size() == 1);
  XIO_CHECK(s.check_buffers(1234, sb.data(), 1));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb.consume(sb.size());
  bytes_transferred = xio::read_at(s, 0, sb,
      xio::transfer_exactly(1));
  XIO_CHECK(bytes_transferred == 1);
  XIO_CHECK(sb.size() == 1);
  XIO_CHECK(s.check_buffers(0, sb.data(), 1));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb.consume(sb.size());
  bytes_transferred = xio::read_at(s, 1234, sb,
      xio::transfer_exactly(1));
  XIO_CHECK(bytes_transferred == 1);
  XIO_CHECK(sb.size() == 1);
  XIO_CHECK(s.check_buffers(1234, sb.data(), 1));

  s.reset(read_data, sizeof(read_data));
  sb.consume(sb.size());
  bytes_transferred = xio::read_at(s, 0, sb,
      xio::transfer_exactly(10));
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(sb.size() == 10);
  XIO_CHECK(s.check_buffers(0, sb.data(), 10));

  s.reset(read_data, sizeof(read_data));
  sb.consume(sb.size());
  bytes_transferred = xio::read_at(s, 1234, sb,
      xio::transfer_exactly(10));
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(sb.size() == 10);
  XIO_CHECK(s.check_buffers(1234, sb.data(), 10));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb.consume(sb.size());
  bytes_transferred = xio::read_at(s, 0, sb,
      xio::transfer_exactly(10));
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(sb.size() == 10);
  XIO_CHECK(s.check_buffers(0, sb.data(), 10));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb.consume(sb.size());
  bytes_transferred = xio::read_at(s, 1234, sb,
      xio::transfer_exactly(10));
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(sb.size() == 10);
  XIO_CHECK(s.check_buffers(1234, sb.data(), 10));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb.consume(sb.size());
  bytes_transferred = xio::read_at(s, 0, sb,
      xio::transfer_exactly(10));
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(sb.size() == 10);
  XIO_CHECK(s.check_buffers(0, sb.data(), 10));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb.consume(sb.size());
  bytes_transferred = xio::read_at(s, 1234, sb,
      xio::transfer_exactly(10));
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(sb.size() == 10);
  XIO_CHECK(s.check_buffers(1234, sb.data(), 10));

  s.reset(read_data, sizeof(read_data));
  sb.consume(sb.size());
  bytes_transferred = xio::read_at(s, 0, sb,
      xio::transfer_exactly(42));
  XIO_CHECK(bytes_transferred == 42);
  XIO_CHECK(sb.size() == 42);
  XIO_CHECK(s.check_buffers(0, sb.data(), 42));

  s.reset(read_data, sizeof(read_data));
  sb.consume(sb.size());
  bytes_transferred = xio::read_at(s, 1234, sb,
      xio::transfer_exactly(42));
  XIO_CHECK(bytes_transferred == 42);
  XIO_CHECK(sb.size() == 42);
  XIO_CHECK(s.check_buffers(1234, sb.data(), 42));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb.consume(sb.size());
  bytes_transferred = xio::read_at(s, 0, sb,
      xio::transfer_exactly(42));
  XIO_CHECK(bytes_transferred == 42);
  XIO_CHECK(sb.size() == 42);
  XIO_CHECK(s.check_buffers(0, sb.data(), 42));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb.consume(sb.size());
  bytes_transferred = xio::read_at(s, 1234, sb,
      xio::transfer_exactly(42));
  XIO_CHECK(bytes_transferred == 42);
  XIO_CHECK(sb.size() == 42);
  XIO_CHECK(s.check_buffers(1234, sb.data(), 42));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb.consume(sb.size());
  bytes_transferred = xio::read_at(s, 0, sb,
      xio::transfer_exactly(42));
  XIO_CHECK(bytes_transferred == 42);
  XIO_CHECK(sb.size() == 42);
  XIO_CHECK(s.check_buffers(0, sb.data(), 42));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb.consume(sb.size());
  bytes_transferred = xio::read_at(s, 1234, sb,
      xio::transfer_exactly(42));
  XIO_CHECK(bytes_transferred == 42);
  XIO_CHECK(sb.size() == 42);
  XIO_CHECK(s.check_buffers(1234, sb.data(), 42));

  s.reset(read_data, sizeof(read_data));
  sb.consume(sb.size());
  bytes_transferred = xio::read_at(s, 0, sb,
      old_style_transfer_all);
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(sb.size() == sizeof(read_data));
  XIO_CHECK(s.check_buffers(0, sb.data(), sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  sb.consume(sb.size());
  bytes_transferred = xio::read_at(s, 1234, sb,
      old_style_transfer_all);
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(sb.size() == sizeof(read_data));
  XIO_CHECK(s.check_buffers(1234, sb.data(), sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb.consume(sb.size());
  bytes_transferred = xio::read_at(s, 0, sb,
      old_style_transfer_all);
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(sb.size() == sizeof(read_data));
  XIO_CHECK(s.check_buffers(0, sb.data(), sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb.consume(sb.size());
  bytes_transferred = xio::read_at(s, 1234, sb,
      old_style_transfer_all);
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(sb.size() == sizeof(read_data));
  XIO_CHECK(s.check_buffers(1234, sb.data(), sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb.consume(sb.size());
  bytes_transferred = xio::read_at(s, 0, sb,
      old_style_transfer_all);
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(sb.size() == sizeof(read_data));
  XIO_CHECK(s.check_buffers(0, sb.data(), sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb.consume(sb.size());
  bytes_transferred = xio::read_at(s, 1234, sb,
      old_style_transfer_all);
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(sb.size() == sizeof(read_data));
  XIO_CHECK(s.check_buffers(1234, sb.data(), sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  sb.consume(sb.size());
  bytes_transferred = xio::read_at(s, 0, sb, short_transfer());
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(sb.size() == sizeof(read_data));
  XIO_CHECK(s.check_buffers(0, sb.data(), sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  sb.consume(sb.size());
  bytes_transferred = xio::read_at(s, 1234, sb, short_transfer());
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(sb.size() == sizeof(read_data));
  XIO_CHECK(s.check_buffers(1234, sb.data(), sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb.consume(sb.size());
  bytes_transferred = xio::read_at(s, 0, sb, short_transfer());
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(sb.size() == sizeof(read_data));
  XIO_CHECK(s.check_buffers(0, sb.data(), sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb.consume(sb.size());
  bytes_transferred = xio::read_at(s, 1234, sb, short_transfer());
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(sb.size() == sizeof(read_data));
  XIO_CHECK(s.check_buffers(1234, sb.data(), sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb.consume(sb.size());
  bytes_transferred = xio::read_at(s, 0, sb, short_transfer());
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(sb.size() == sizeof(read_data));
  XIO_CHECK(s.check_buffers(0, sb.data(), sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb.consume(sb.size());
  bytes_transferred = xio::read_at(s, 1234, sb, short_transfer());
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(sb.size() == sizeof(read_data));
  XIO_CHECK(s.check_buffers(1234, sb.data(), sizeof(read_data)));
}

void test_5_arg_mutable_buffer_read_at()
{
  xio::io_context ioc;
  test_random_access_device s(ioc);
  char read_buf[sizeof(read_data)];
  xio::mutable_buffer buffers
    = xio::buffer(read_buf, sizeof(read_buf));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  xio::error_code error;
  size_t bytes_transferred = xio::read_at(s, 0, buffers,
      xio::transfer_all(), error);
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(read_data)));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 1234, buffers,
      xio::transfer_all(), error);
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(read_data)));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 0, buffers,
      xio::transfer_all(), error);
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(read_data)));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 1234, buffers,
      xio::transfer_all(), error);
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(read_data)));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 0, buffers,
      xio::transfer_all(), error);
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(read_data)));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 1234, buffers,
      xio::transfer_all(), error);
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(read_data)));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 0, buffers,
      xio::transfer_at_least(1), error);
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(read_data)));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 1234, buffers,
      xio::transfer_at_least(1), error);
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(read_data)));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 0, buffers,
      xio::transfer_at_least(1), error);
  XIO_CHECK(bytes_transferred == 1);
  XIO_CHECK(s.check_buffers(0, buffers, 1));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 1234, buffers,
      xio::transfer_at_least(1), error);
  XIO_CHECK(bytes_transferred == 1);
  XIO_CHECK(s.check_buffers(1234, buffers, 1));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 0, buffers,
      xio::transfer_at_least(1), error);
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(s.check_buffers(0, buffers, 10));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 1234, buffers,
      xio::transfer_at_least(1), error);
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(s.check_buffers(1234, buffers, 10));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 0, buffers,
      xio::transfer_at_least(10), error);
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(read_data)));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 1234, buffers,
      xio::transfer_at_least(10), error);
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(read_data)));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 0, buffers,
      xio::transfer_at_least(10), error);
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(s.check_buffers(0, buffers, 10));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 1234, buffers,
      xio::transfer_at_least(10), error);
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(s.check_buffers(1234, buffers, 10));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 0, buffers,
      xio::transfer_at_least(10), error);
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(s.check_buffers(0, buffers, 10));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 1234, buffers,
      xio::transfer_at_least(10), error);
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(s.check_buffers(1234, buffers, 10));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 0, buffers,
      xio::transfer_at_least(42), error);
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(read_data)));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 1234, buffers,
      xio::transfer_at_least(42), error);
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(read_data)));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 0, buffers,
      xio::transfer_at_least(42), error);
  XIO_CHECK(bytes_transferred == 42);
  XIO_CHECK(s.check_buffers(0, buffers, 42));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 1234, buffers,
      xio::transfer_at_least(42), error);
  XIO_CHECK(bytes_transferred == 42);
  XIO_CHECK(s.check_buffers(1234, buffers, 42));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 0, buffers,
      xio::transfer_at_least(42), error);
  XIO_CHECK(bytes_transferred == 50);
  XIO_CHECK(s.check_buffers(0, buffers, 50));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 1234, buffers,
      xio::transfer_at_least(42), error);
  XIO_CHECK(bytes_transferred == 50);
  XIO_CHECK(s.check_buffers(1234, buffers, 50));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 0, buffers,
      xio::transfer_exactly(1), error);
  XIO_CHECK(bytes_transferred == 1);
  XIO_CHECK(s.check_buffers(0, buffers, 1));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 1234, buffers,
      xio::transfer_exactly(1), error);
  XIO_CHECK(bytes_transferred == 1);
  XIO_CHECK(s.check_buffers(1234, buffers, 1));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 0, buffers,
      xio::transfer_exactly(1), error);
  XIO_CHECK(bytes_transferred == 1);
  XIO_CHECK(s.check_buffers(0, buffers, 1));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 1234, buffers,
      xio::transfer_exactly(1), error);
  XIO_CHECK(bytes_transferred == 1);
  XIO_CHECK(s.check_buffers(1234, buffers, 1));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 0, buffers,
      xio::transfer_exactly(1), error);
  XIO_CHECK(bytes_transferred == 1);
  XIO_CHECK(s.check_buffers(0, buffers, 1));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 1234, buffers,
      xio::transfer_exactly(1), error);
  XIO_CHECK(bytes_transferred == 1);
  XIO_CHECK(s.check_buffers(1234, buffers, 1));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 0, buffers,
      xio::transfer_exactly(10), error);
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(s.check_buffers(0, buffers, 10));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 1234, buffers,
      xio::transfer_exactly(10), error);
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(s.check_buffers(1234, buffers, 10));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 0, buffers,
      xio::transfer_exactly(10), error);
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(s.check_buffers(0, buffers, 10));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 1234, buffers,
      xio::transfer_exactly(10), error);
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(s.check_buffers(1234, buffers, 10));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 0, buffers,
      xio::transfer_exactly(10), error);
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(s.check_buffers(0, buffers, 10));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 1234, buffers,
      xio::transfer_exactly(10), error);
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(s.check_buffers(1234, buffers, 10));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 0, buffers,
      xio::transfer_exactly(42), error);
  XIO_CHECK(bytes_transferred == 42);
  XIO_CHECK(s.check_buffers(0, buffers, 42));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 1234, buffers,
      xio::transfer_exactly(42), error);
  XIO_CHECK(bytes_transferred == 42);
  XIO_CHECK(s.check_buffers(1234, buffers, 42));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 0, buffers,
      xio::transfer_exactly(42), error);
  XIO_CHECK(bytes_transferred == 42);
  XIO_CHECK(s.check_buffers(0, buffers, 42));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 1234, buffers,
      xio::transfer_exactly(42), error);
  XIO_CHECK(bytes_transferred == 42);
  XIO_CHECK(s.check_buffers(1234, buffers, 42));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 0, buffers,
      xio::transfer_exactly(42), error);
  XIO_CHECK(bytes_transferred == 42);
  XIO_CHECK(s.check_buffers(0, buffers, 42));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 1234, buffers,
      xio::transfer_exactly(42), error);
  XIO_CHECK(bytes_transferred == 42);
  XIO_CHECK(s.check_buffers(1234, buffers, 42));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 0, buffers,
      old_style_transfer_all, error);
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(read_data)));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 1234, buffers,
      old_style_transfer_all, error);
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(read_data)));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 0, buffers,
      old_style_transfer_all, error);
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(read_data)));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 1234, buffers,
      old_style_transfer_all, error);
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(read_data)));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 0, buffers,
      old_style_transfer_all, error);
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(read_data)));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 1234, buffers,
      old_style_transfer_all, error);
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(read_data)));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 0, buffers,
      short_transfer(), error);
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(read_data)));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 1234, buffers,
      short_transfer(), error);
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(read_data)));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 0, buffers,
      short_transfer(), error);
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(read_data)));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 1234, buffers,
      short_transfer(), error);
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(read_data)));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 0, buffers,
      short_transfer(), error);
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(read_data)));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 1234, buffers,
      short_transfer(), error);
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(read_data)));
  XIO_CHECK(!error);
}

void test_5_arg_vector_buffers_read_at()
{
  xio::io_context ioc;
  test_random_access_device s(ioc);
  char read_buf[sizeof(read_data)];
  std::vector<xio::mutable_buffer> buffers;
  buffers.push_back(xio::buffer(read_buf, 32));
  buffers.push_back(xio::buffer(read_buf) + 32);

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  xio::error_code error;
  size_t bytes_transferred = xio::read_at(s, 0, buffers,
      xio::transfer_all(), error);
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(read_data)));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 1234, buffers,
      xio::transfer_all(), error);
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(read_data)));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 0, buffers,
      xio::transfer_all(), error);
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(read_data)));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 1234, buffers,
      xio::transfer_all(), error);
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(read_data)));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 0, buffers,
      xio::transfer_all(), error);
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(read_data)));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 1234, buffers,
      xio::transfer_all(), error);
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(read_data)));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 0, buffers,
      xio::transfer_at_least(1), error);
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(read_data)));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 1234, buffers,
      xio::transfer_at_least(1), error);
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(read_data)));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 0, buffers,
      xio::transfer_at_least(1), error);
  XIO_CHECK(bytes_transferred == 1);
  XIO_CHECK(s.check_buffers(0, buffers, 1));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 1234, buffers,
      xio::transfer_at_least(1), error);
  XIO_CHECK(bytes_transferred == 1);
  XIO_CHECK(s.check_buffers(1234, buffers, 1));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 0, buffers,
      xio::transfer_at_least(1), error);
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(s.check_buffers(0, buffers, 10));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 1234, buffers,
      xio::transfer_at_least(1), error);
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(s.check_buffers(1234, buffers, 10));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 0, buffers,
      xio::transfer_at_least(10), error);
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(read_data)));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 1234, buffers,
      xio::transfer_at_least(10), error);
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(read_data)));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 0, buffers,
      xio::transfer_at_least(10), error);
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(s.check_buffers(0, buffers, 10));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 1234, buffers,
      xio::transfer_at_least(10), error);
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(s.check_buffers(1234, buffers, 10));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 0, buffers,
      xio::transfer_at_least(10), error);
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(s.check_buffers(0, buffers, 10));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 1234, buffers,
      xio::transfer_at_least(10), error);
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(s.check_buffers(1234, buffers, 10));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 0, buffers,
      xio::transfer_at_least(42), error);
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(read_data)));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 1234, buffers,
      xio::transfer_at_least(42), error);
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(read_data)));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 0, buffers,
      xio::transfer_at_least(42), error);
  XIO_CHECK(bytes_transferred == 42);
  XIO_CHECK(s.check_buffers(0, buffers, 42));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 1234, buffers,
      xio::transfer_at_least(42), error);
  XIO_CHECK(bytes_transferred == 42);
  XIO_CHECK(s.check_buffers(1234, buffers, 42));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 0, buffers,
      xio::transfer_at_least(42), error);
  XIO_CHECK(bytes_transferred == 50);
  XIO_CHECK(s.check_buffers(0, buffers, 50));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 1234, buffers,
      xio::transfer_at_least(42), error);
  XIO_CHECK(bytes_transferred == 50);
  XIO_CHECK(s.check_buffers(1234, buffers, 50));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 0, buffers,
      xio::transfer_exactly(1), error);
  XIO_CHECK(bytes_transferred == 1);
  XIO_CHECK(s.check_buffers(0, buffers, 1));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 1234, buffers,
      xio::transfer_exactly(1), error);
  XIO_CHECK(bytes_transferred == 1);
  XIO_CHECK(s.check_buffers(1234, buffers, 1));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 0, buffers,
      xio::transfer_exactly(1), error);
  XIO_CHECK(bytes_transferred == 1);
  XIO_CHECK(s.check_buffers(0, buffers, 1));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 1234, buffers,
      xio::transfer_exactly(1), error);
  XIO_CHECK(bytes_transferred == 1);
  XIO_CHECK(s.check_buffers(1234, buffers, 1));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 0, buffers,
      xio::transfer_exactly(1), error);
  XIO_CHECK(bytes_transferred == 1);
  XIO_CHECK(s.check_buffers(0, buffers, 1));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 1234, buffers,
      xio::transfer_exactly(1), error);
  XIO_CHECK(bytes_transferred == 1);
  XIO_CHECK(s.check_buffers(1234, buffers, 1));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 0, buffers,
      xio::transfer_exactly(10), error);
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(s.check_buffers(0, buffers, 10));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 1234, buffers,
      xio::transfer_exactly(10), error);
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(s.check_buffers(1234, buffers, 10));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 0, buffers,
      xio::transfer_exactly(10), error);
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(s.check_buffers(0, buffers, 10));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 1234, buffers,
      xio::transfer_exactly(10), error);
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(s.check_buffers(1234, buffers, 10));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 0, buffers,
      xio::transfer_exactly(10), error);
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(s.check_buffers(0, buffers, 10));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 1234, buffers,
      xio::transfer_exactly(10), error);
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(s.check_buffers(1234, buffers, 10));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 0, buffers,
      xio::transfer_exactly(42), error);
  XIO_CHECK(bytes_transferred == 42);
  XIO_CHECK(s.check_buffers(0, buffers, 42));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 1234, buffers,
      xio::transfer_exactly(42), error);
  XIO_CHECK(bytes_transferred == 42);
  XIO_CHECK(s.check_buffers(1234, buffers, 42));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 0, buffers,
      xio::transfer_exactly(42), error);
  XIO_CHECK(bytes_transferred == 42);
  XIO_CHECK(s.check_buffers(0, buffers, 42));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 1234, buffers,
      xio::transfer_exactly(42), error);
  XIO_CHECK(bytes_transferred == 42);
  XIO_CHECK(s.check_buffers(1234, buffers, 42));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 0, buffers,
      xio::transfer_exactly(42), error);
  XIO_CHECK(bytes_transferred == 42);
  XIO_CHECK(s.check_buffers(0, buffers, 42));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 1234, buffers,
      xio::transfer_exactly(42), error);
  XIO_CHECK(bytes_transferred == 42);
  XIO_CHECK(s.check_buffers(1234, buffers, 42));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 0, buffers,
      old_style_transfer_all, error);
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(read_data)));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 1234, buffers,
      old_style_transfer_all, error);
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(read_data)));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 0, buffers,
      old_style_transfer_all, error);
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(read_data)));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 1234, buffers,
      old_style_transfer_all, error);
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(read_data)));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 0, buffers,
      old_style_transfer_all, error);
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(read_data)));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 1234, buffers,
      old_style_transfer_all, error);
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(read_data)));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 0, buffers,
      short_transfer(), error);
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(read_data)));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  bytes_transferred = xio::read_at(s, 1234, buffers,
      short_transfer(), error);
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(read_data)));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 0, buffers,
      short_transfer(), error);
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(read_data)));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 1234, buffers,
      short_transfer(), error);
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(read_data)));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 0, buffers,
      short_transfer(), error);
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(read_data)));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 1234, buffers,
      short_transfer(), error);
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(read_data)));
  XIO_CHECK(!error);
}

void test_5_arg_streambuf_read_at()
{
  xio::io_context ioc;
  test_random_access_device s(ioc);
  xio::streambuf sb(sizeof(read_data));

  s.reset(read_data, sizeof(read_data));
  sb.consume(sb.size());
  xio::error_code error;
  size_t bytes_transferred = xio::read_at(s, 0, sb,
      xio::transfer_all(), error);
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(sb.size() == sizeof(read_data));
  XIO_CHECK(s.check_buffers(0, sb.data(), sizeof(read_data)));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  sb.consume(sb.size());
  bytes_transferred = xio::read_at(s, 1234, sb,
      xio::transfer_all(), error);
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(sb.size() == sizeof(read_data));
  XIO_CHECK(s.check_buffers(1234, sb.data(), sizeof(read_data)));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb.consume(sb.size());
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 0, sb,
      xio::transfer_all(), error);
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(sb.size() == sizeof(read_data));
  XIO_CHECK(s.check_buffers(0, sb.data(), sizeof(read_data)));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb.consume(sb.size());
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 1234, sb,
      xio::transfer_all(), error);
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(sb.size() == sizeof(read_data));
  XIO_CHECK(s.check_buffers(1234, sb.data(), sizeof(read_data)));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb.consume(sb.size());
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 0, sb,
      xio::transfer_all(), error);
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(sb.size() == sizeof(read_data));
  XIO_CHECK(s.check_buffers(0, sb.data(), sizeof(read_data)));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb.consume(sb.size());
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 1234, sb,
      xio::transfer_all(), error);
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(sb.size() == sizeof(read_data));
  XIO_CHECK(s.check_buffers(1234, sb.data(), sizeof(read_data)));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  sb.consume(sb.size());
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 0, sb,
      xio::transfer_at_least(1), error);
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(sb.size() == sizeof(read_data));
  XIO_CHECK(s.check_buffers(0, sb.data(), sizeof(read_data)));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  sb.consume(sb.size());
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 1234, sb,
      xio::transfer_at_least(1), error);
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(sb.size() == sizeof(read_data));
  XIO_CHECK(s.check_buffers(1234, sb.data(), sizeof(read_data)));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb.consume(sb.size());
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 0, sb,
      xio::transfer_at_least(1), error);
  XIO_CHECK(bytes_transferred == 1);
  XIO_CHECK(sb.size() == 1);
  XIO_CHECK(s.check_buffers(0, sb.data(), 1));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb.consume(sb.size());
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 1234, sb,
      xio::transfer_at_least(1), error);
  XIO_CHECK(bytes_transferred == 1);
  XIO_CHECK(sb.size() == 1);
  XIO_CHECK(s.check_buffers(1234, sb.data(), 1));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb.consume(sb.size());
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 0, sb,
      xio::transfer_at_least(1), error);
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(sb.size() == 10);
  XIO_CHECK(s.check_buffers(0, sb.data(), 10));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb.consume(sb.size());
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 1234, sb,
      xio::transfer_at_least(1), error);
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(sb.size() == 10);
  XIO_CHECK(s.check_buffers(1234, sb.data(), 10));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  sb.consume(sb.size());
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 0, sb,
      xio::transfer_at_least(10), error);
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(sb.size() == sizeof(read_data));
  XIO_CHECK(s.check_buffers(0, sb.data(), sizeof(read_data)));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  sb.consume(sb.size());
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 1234, sb,
      xio::transfer_at_least(10), error);
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(sb.size() == sizeof(read_data));
  XIO_CHECK(s.check_buffers(1234, sb.data(), sizeof(read_data)));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb.consume(sb.size());
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 0, sb,
      xio::transfer_at_least(10), error);
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(sb.size() == 10);
  XIO_CHECK(s.check_buffers(0, sb.data(), 10));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb.consume(sb.size());
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 1234, sb,
      xio::transfer_at_least(10), error);
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(sb.size() == 10);
  XIO_CHECK(s.check_buffers(1234, sb.data(), 10));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb.consume(sb.size());
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 0, sb,
      xio::transfer_at_least(10), error);
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(sb.size() == 10);
  XIO_CHECK(s.check_buffers(0, sb.data(), 10));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb.consume(sb.size());
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 1234, sb,
      xio::transfer_at_least(10), error);
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(sb.size() == 10);
  XIO_CHECK(s.check_buffers(1234, sb.data(), 10));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  sb.consume(sb.size());
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 0, sb,
      xio::transfer_at_least(42), error);
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(sb.size() == sizeof(read_data));
  XIO_CHECK(s.check_buffers(0, sb.data(), sizeof(read_data)));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  sb.consume(sb.size());
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 1234, sb,
      xio::transfer_at_least(42), error);
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(sb.size() == sizeof(read_data));
  XIO_CHECK(s.check_buffers(1234, sb.data(), sizeof(read_data)));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb.consume(sb.size());
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 0, sb,
      xio::transfer_at_least(42), error);
  XIO_CHECK(bytes_transferred == 42);
  XIO_CHECK(sb.size() == 42);
  XIO_CHECK(s.check_buffers(0, sb.data(), 42));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb.consume(sb.size());
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 1234, sb,
      xio::transfer_at_least(42), error);
  XIO_CHECK(bytes_transferred == 42);
  XIO_CHECK(sb.size() == 42);
  XIO_CHECK(s.check_buffers(1234, sb.data(), 42));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb.consume(sb.size());
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 0, sb,
      xio::transfer_at_least(42), error);
  XIO_CHECK(bytes_transferred == 50);
  XIO_CHECK(sb.size() == 50);
  XIO_CHECK(s.check_buffers(0, sb.data(), 50));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb.consume(sb.size());
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 1234, sb,
      xio::transfer_at_least(42), error);
  XIO_CHECK(bytes_transferred == 50);
  XIO_CHECK(sb.size() == 50);
  XIO_CHECK(s.check_buffers(1234, sb.data(), 50));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  sb.consume(sb.size());
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 0, sb,
      xio::transfer_exactly(1), error);
  XIO_CHECK(bytes_transferred == 1);
  XIO_CHECK(sb.size() == 1);
  XIO_CHECK(s.check_buffers(0, sb.data(), 1));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  sb.consume(sb.size());
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 1234, sb,
      xio::transfer_exactly(1), error);
  XIO_CHECK(bytes_transferred == 1);
  XIO_CHECK(sb.size() == 1);
  XIO_CHECK(s.check_buffers(1234, sb.data(), 1));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb.consume(sb.size());
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 0, sb,
      xio::transfer_exactly(1), error);
  XIO_CHECK(bytes_transferred == 1);
  XIO_CHECK(sb.size() == 1);
  XIO_CHECK(s.check_buffers(0, sb.data(), 1));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb.consume(sb.size());
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 1234, sb,
      xio::transfer_exactly(1), error);
  XIO_CHECK(bytes_transferred == 1);
  XIO_CHECK(sb.size() == 1);
  XIO_CHECK(s.check_buffers(1234, sb.data(), 1));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb.consume(sb.size());
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 0, sb,
      xio::transfer_exactly(1), error);
  XIO_CHECK(bytes_transferred == 1);
  XIO_CHECK(sb.size() == 1);
  XIO_CHECK(s.check_buffers(0, sb.data(), 1));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb.consume(sb.size());
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 1234, sb,
      xio::transfer_exactly(1), error);
  XIO_CHECK(bytes_transferred == 1);
  XIO_CHECK(sb.size() == 1);
  XIO_CHECK(s.check_buffers(1234, sb.data(), 1));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  sb.consume(sb.size());
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 0, sb,
      xio::transfer_exactly(10), error);
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(sb.size() == 10);
  XIO_CHECK(s.check_buffers(0, sb.data(), 10));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  sb.consume(sb.size());
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 1234, sb,
      xio::transfer_exactly(10), error);
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(sb.size() == 10);
  XIO_CHECK(s.check_buffers(1234, sb.data(), 10));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb.consume(sb.size());
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 0, sb,
      xio::transfer_exactly(10), error);
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(sb.size() == 10);
  XIO_CHECK(s.check_buffers(0, sb.data(), 10));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb.consume(sb.size());
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 1234, sb,
      xio::transfer_exactly(10), error);
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(sb.size() == 10);
  XIO_CHECK(s.check_buffers(1234, sb.data(), 10));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb.consume(sb.size());
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 0, sb,
      xio::transfer_exactly(10), error);
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(sb.size() == 10);
  XIO_CHECK(s.check_buffers(0, sb.data(), 10));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb.consume(sb.size());
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 1234, sb,
      xio::transfer_exactly(10), error);
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(sb.size() == 10);
  XIO_CHECK(s.check_buffers(1234, sb.data(), 10));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  sb.consume(sb.size());
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 0, sb,
      xio::transfer_exactly(42), error);
  XIO_CHECK(bytes_transferred == 42);
  XIO_CHECK(sb.size() == 42);
  XIO_CHECK(s.check_buffers(0, sb.data(), 42));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  sb.consume(sb.size());
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 1234, sb,
      xio::transfer_exactly(42), error);
  XIO_CHECK(bytes_transferred == 42);
  XIO_CHECK(sb.size() == 42);
  XIO_CHECK(s.check_buffers(1234, sb.data(), 42));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb.consume(sb.size());
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 0, sb,
      xio::transfer_exactly(42), error);
  XIO_CHECK(bytes_transferred == 42);
  XIO_CHECK(sb.size() == 42);
  XIO_CHECK(s.check_buffers(0, sb.data(), 42));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb.consume(sb.size());
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 1234, sb,
      xio::transfer_exactly(42), error);
  XIO_CHECK(bytes_transferred == 42);
  XIO_CHECK(sb.size() == 42);
  XIO_CHECK(s.check_buffers(1234, sb.data(), 42));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb.consume(sb.size());
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 0, sb,
      xio::transfer_exactly(42), error);
  XIO_CHECK(bytes_transferred == 42);
  XIO_CHECK(sb.size() == 42);
  XIO_CHECK(s.check_buffers(0, sb.data(), 42));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb.consume(sb.size());
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 1234, sb,
      xio::transfer_exactly(42), error);
  XIO_CHECK(bytes_transferred == 42);
  XIO_CHECK(sb.size() == 42);
  XIO_CHECK(s.check_buffers(1234, sb.data(), 42));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  sb.consume(sb.size());
  bytes_transferred = xio::read_at(s, 0, sb,
      old_style_transfer_all, error);
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(sb.size() == sizeof(read_data));
  XIO_CHECK(s.check_buffers(0, sb.data(), sizeof(read_data)));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  sb.consume(sb.size());
  bytes_transferred = xio::read_at(s, 1234, sb,
      old_style_transfer_all, error);
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(sb.size() == sizeof(read_data));
  XIO_CHECK(s.check_buffers(1234, sb.data(), sizeof(read_data)));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb.consume(sb.size());
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 0, sb,
      old_style_transfer_all, error);
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(sb.size() == sizeof(read_data));
  XIO_CHECK(s.check_buffers(0, sb.data(), sizeof(read_data)));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb.consume(sb.size());
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 1234, sb,
      old_style_transfer_all, error);
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(sb.size() == sizeof(read_data));
  XIO_CHECK(s.check_buffers(1234, sb.data(), sizeof(read_data)));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb.consume(sb.size());
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 0, sb,
      old_style_transfer_all, error);
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(sb.size() == sizeof(read_data));
  XIO_CHECK(s.check_buffers(0, sb.data(), sizeof(read_data)));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb.consume(sb.size());
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 1234, sb,
      old_style_transfer_all, error);
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(sb.size() == sizeof(read_data));
  XIO_CHECK(s.check_buffers(1234, sb.data(), sizeof(read_data)));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  sb.consume(sb.size());
  bytes_transferred = xio::read_at(s, 0, sb,
      short_transfer(), error);
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(sb.size() == sizeof(read_data));
  XIO_CHECK(s.check_buffers(0, sb.data(), sizeof(read_data)));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  sb.consume(sb.size());
  bytes_transferred = xio::read_at(s, 1234, sb,
      short_transfer(), error);
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(sb.size() == sizeof(read_data));
  XIO_CHECK(s.check_buffers(1234, sb.data(), sizeof(read_data)));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb.consume(sb.size());
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 0, sb,
      short_transfer(), error);
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(sb.size() == sizeof(read_data));
  XIO_CHECK(s.check_buffers(0, sb.data(), sizeof(read_data)));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb.consume(sb.size());
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 1234, sb,
      short_transfer(), error);
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(sb.size() == sizeof(read_data));
  XIO_CHECK(s.check_buffers(1234, sb.data(), sizeof(read_data)));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb.consume(sb.size());
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 0, sb,
      short_transfer(), error);
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(sb.size() == sizeof(read_data));
  XIO_CHECK(s.check_buffers(0, sb.data(), sizeof(read_data)));
  XIO_CHECK(!error);

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb.consume(sb.size());
  error = xio::error_code();
  bytes_transferred = xio::read_at(s, 1234, sb,
      short_transfer(), error);
  XIO_CHECK(bytes_transferred == sizeof(read_data));
  XIO_CHECK(sb.size() == sizeof(read_data));
  XIO_CHECK(s.check_buffers(1234, sb.data(), sizeof(read_data)));
  XIO_CHECK(!error);
}

void async_read_handler(const xio::error_code& e,
    size_t bytes_transferred, size_t expected_bytes_transferred, bool* called)
{
  *called = true;
  XIO_CHECK(!e);
  XIO_CHECK(bytes_transferred == expected_bytes_transferred);
}

void test_4_arg_mutable_buffer_async_read_at()
{
  namespace bindns = std;
  using bindns::placeholders::_1;
  using bindns::placeholders::_2;

  xio::io_context ioc;
  test_random_access_device s(ioc);
  char read_buf[sizeof(read_data)];
  xio::mutable_buffer buffers
    = xio::buffer(read_buf, sizeof(read_buf));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  bool called = false;
  xio::async_read_at(s, 0, buffers,
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 1234, buffers,
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 0, buffers,
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 1234, buffers,
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 0, buffers,
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 1234, buffers,
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  int i = xio::async_read_at(s, 1234, buffers,
      archetypes::lazy_handler());
  XIO_CHECK(i == 42);
  ioc.restart();
  ioc.run();
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 1234, buffers)(
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(read_data)));
}

void test_4_arg_boost_array_buffers_async_read_at()
{
  namespace bindns = std;
  using bindns::placeholders::_1;
  using bindns::placeholders::_2;

}

void test_4_arg_std_array_buffers_async_read_at()
{
  namespace bindns = std;
  using bindns::placeholders::_1;
  using bindns::placeholders::_2;

  xio::io_context ioc;
  test_random_access_device s(ioc);
  char read_buf[sizeof(read_data)];
  std::array<xio::mutable_buffer, 2> buffers = { {
    xio::buffer(read_buf, 32),
    xio::buffer(read_buf) + 32 } };

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  bool called = false;
  xio::async_read_at(s, 0, buffers,
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 1234, buffers,
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 0, buffers,
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 1234, buffers,
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 0, buffers,
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 1234, buffers,
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  int i = xio::async_read_at(s, 1234, buffers,
      archetypes::lazy_handler());
  XIO_CHECK(i == 42);
  ioc.restart();
  ioc.run();
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 1234, buffers)(
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(read_data)));
}

void test_4_arg_vector_buffers_async_read_at()
{
  namespace bindns = std;
  using bindns::placeholders::_1;
  using bindns::placeholders::_2;

  xio::io_context ioc;
  test_random_access_device s(ioc);
  char read_buf[sizeof(read_data)];
  std::vector<xio::mutable_buffer> buffers;
  buffers.push_back(xio::buffer(read_buf, 32));
  buffers.push_back(xio::buffer(read_buf) + 32);

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  bool called = false;
  xio::async_read_at(s, 0, buffers,
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 1234, buffers,
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 0, buffers,
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 1234, buffers,
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 0, buffers,
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 1234, buffers,
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  int i = xio::async_read_at(s, 1234, buffers,
      archetypes::lazy_handler());
  XIO_CHECK(i == 42);
  ioc.restart();
  ioc.run();
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 1234, buffers)(
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(read_data)));
}

void test_4_arg_streambuf_async_read_at()
{
  namespace bindns = std;
  using bindns::placeholders::_1;
  using bindns::placeholders::_2;

  xio::io_context ioc;
  test_random_access_device s(ioc);
  xio::streambuf sb(sizeof(read_data));

  s.reset(read_data, sizeof(read_data));
  sb.consume(sb.size());
  bool called = false;
  xio::async_read_at(s, 0, sb,
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(sb.size() == sizeof(read_data));
  XIO_CHECK(s.check_buffers(0, sb.data(), sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  sb.consume(sb.size());
  called = false;
  xio::async_read_at(s, 1234, sb,
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(sb.size() == sizeof(read_data));
  XIO_CHECK(s.check_buffers(1234, sb.data(), sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb.consume(sb.size());
  called = false;
  xio::async_read_at(s, 0, sb,
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(sb.size() == sizeof(read_data));
  XIO_CHECK(s.check_buffers(0, sb.data(), sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb.consume(sb.size());
  called = false;
  xio::async_read_at(s, 1234, sb,
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(sb.size() == sizeof(read_data));
  XIO_CHECK(s.check_buffers(1234, sb.data(), sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb.consume(sb.size());
  called = false;
  xio::async_read_at(s, 0, sb,
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(sb.size() == sizeof(read_data));
  XIO_CHECK(s.check_buffers(0, sb.data(), sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb.consume(sb.size());
  called = false;
  xio::async_read_at(s, 1234, sb,
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(sb.size() == sizeof(read_data));
  XIO_CHECK(s.check_buffers(1234, sb.data(), sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  sb.consume(sb.size());
  int i = xio::async_read_at(s, 1234, sb,
      archetypes::lazy_handler());
  XIO_CHECK(i == 42);
  ioc.restart();
  ioc.run();
  XIO_CHECK(s.check_buffers(1234, sb.data(), sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb.consume(sb.size());
  called = false;
  xio::async_read_at(s, 1234, sb)(
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(sb.size() == sizeof(read_data));
  XIO_CHECK(s.check_buffers(1234, sb.data(), sizeof(read_data)));
}

void test_5_arg_mutable_buffer_async_read_at()
{
  namespace bindns = std;
  using bindns::placeholders::_1;
  using bindns::placeholders::_2;

  xio::io_context ioc;
  test_random_access_device s(ioc);
  char read_buf[sizeof(read_data)];
  xio::mutable_buffer buffers
    = xio::buffer(read_buf, sizeof(read_buf));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  bool called = false;
  xio::async_read_at(s, 0, buffers,
      xio::transfer_all(),
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 1234, buffers,
      xio::transfer_all(),
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 0, buffers,
      xio::transfer_all(),
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 1234, buffers,
      xio::transfer_all(),
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 0, buffers,
      xio::transfer_all(),
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 1234, buffers,
      xio::transfer_all(),
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 0, buffers,
      xio::transfer_at_least(1),
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 1234, buffers,
      xio::transfer_at_least(1),
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 0, buffers,
      xio::transfer_at_least(1),
      bindns::bind(async_read_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, 1));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 1234, buffers,
      xio::transfer_at_least(1),
      bindns::bind(async_read_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, 1));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 0, buffers,
      xio::transfer_at_least(1),
      bindns::bind(async_read_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 1234, buffers,
      xio::transfer_at_least(1),
      bindns::bind(async_read_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 0, buffers,
      xio::transfer_at_least(10),
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 1234, buffers,
      xio::transfer_at_least(10),
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 0, buffers,
      xio::transfer_at_least(10),
      bindns::bind(async_read_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 1234, buffers,
      xio::transfer_at_least(10),
      bindns::bind(async_read_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 0, buffers,
      xio::transfer_at_least(10),
      bindns::bind(async_read_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 1234, buffers,
      xio::transfer_at_least(10),
      bindns::bind(async_read_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 0, buffers,
      xio::transfer_at_least(42),
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 1234, buffers,
      xio::transfer_at_least(42),
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 0, buffers,
      xio::transfer_at_least(42),
      bindns::bind(async_read_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, 42));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 1234, buffers,
      xio::transfer_at_least(42),
      bindns::bind(async_read_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, 42));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 0, buffers,
      xio::transfer_at_least(42),
      bindns::bind(async_read_handler,
        _1, _2, 50, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, 50));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 1234, buffers,
      xio::transfer_at_least(42),
      bindns::bind(async_read_handler,
        _1, _2, 50, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, 50));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 0, buffers,
      xio::transfer_exactly(1),
      bindns::bind(async_read_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, 1));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 1234, buffers,
      xio::transfer_exactly(1),
      bindns::bind(async_read_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, 1));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 0, buffers,
      xio::transfer_exactly(1),
      bindns::bind(async_read_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, 1));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 1234, buffers,
      xio::transfer_exactly(1),
      bindns::bind(async_read_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, 1));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 0, buffers,
      xio::transfer_exactly(1),
      bindns::bind(async_read_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, 1));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 1234, buffers,
      xio::transfer_exactly(1),
      bindns::bind(async_read_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, 1));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 0, buffers,
      xio::transfer_exactly(10),
      bindns::bind(async_read_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 1234, buffers,
      xio::transfer_exactly(10),
      bindns::bind(async_read_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 0, buffers,
      xio::transfer_exactly(10),
      bindns::bind(async_read_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 1234, buffers,
      xio::transfer_exactly(10),
      bindns::bind(async_read_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 0, buffers,
      xio::transfer_exactly(10),
      bindns::bind(async_read_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 1234, buffers,
      xio::transfer_exactly(10),
      bindns::bind(async_read_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 0, buffers,
      xio::transfer_exactly(42),
      bindns::bind(async_read_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, 42));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 1234, buffers,
      xio::transfer_exactly(42),
      bindns::bind(async_read_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, 42));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 0, buffers,
      xio::transfer_exactly(42),
      bindns::bind(async_read_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, 42));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 1234, buffers,
      xio::transfer_exactly(42),
      bindns::bind(async_read_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, 42));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 0, buffers,
      xio::transfer_exactly(42),
      bindns::bind(async_read_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, 42));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 1234, buffers,
      xio::transfer_exactly(42),
      bindns::bind(async_read_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, 42));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 0, buffers, old_style_transfer_all,
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 1234, buffers, old_style_transfer_all,
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 0, buffers, old_style_transfer_all,
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 1234, buffers, old_style_transfer_all,
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 0, buffers, old_style_transfer_all,
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 1234, buffers, old_style_transfer_all,
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 0, buffers, short_transfer(),
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 1234, buffers, short_transfer(),
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 0, buffers, short_transfer(),
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 1234, buffers, short_transfer(),
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 0, buffers, short_transfer(),
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 1234, buffers, short_transfer(),
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  int i = xio::async_read_at(s, 1234, buffers,
      short_transfer(), archetypes::lazy_handler());
  XIO_CHECK(i == 42);
  ioc.restart();
  ioc.run();
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 1234, buffers, short_transfer())(
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(read_data)));
}

void test_5_arg_boost_array_buffers_async_read_at()
{
  namespace bindns = std;
  using bindns::placeholders::_1;
  using bindns::placeholders::_2;

}

void test_5_arg_std_array_buffers_async_read_at()
{
  namespace bindns = std;
  using bindns::placeholders::_1;
  using bindns::placeholders::_2;

  xio::io_context ioc;
  test_random_access_device s(ioc);
  char read_buf[sizeof(read_data)];
  std::array<xio::mutable_buffer, 2> buffers = { {
    xio::buffer(read_buf, 32),
    xio::buffer(read_buf) + 32 } };

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  bool called = false;
  xio::async_read_at(s, 0, buffers,
      xio::transfer_all(),
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 1234, buffers,
      xio::transfer_all(),
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 0, buffers,
      xio::transfer_all(),
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 1234, buffers,
      xio::transfer_all(),
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 0, buffers,
      xio::transfer_all(),
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 1234, buffers,
      xio::transfer_all(),
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 0, buffers,
      xio::transfer_at_least(1),
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 1234, buffers,
      xio::transfer_at_least(1),
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 0, buffers,
      xio::transfer_at_least(1),
      bindns::bind(async_read_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, 1));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 1234, buffers,
      xio::transfer_at_least(1),
      bindns::bind(async_read_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, 1));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 0, buffers,
      xio::transfer_at_least(1),
      bindns::bind(async_read_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 1234, buffers,
      xio::transfer_at_least(1),
      bindns::bind(async_read_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 0, buffers,
      xio::transfer_at_least(10),
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 1234, buffers,
      xio::transfer_at_least(10),
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 0, buffers,
      xio::transfer_at_least(10),
      bindns::bind(async_read_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 1234, buffers,
      xio::transfer_at_least(10),
      bindns::bind(async_read_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 0, buffers,
      xio::transfer_at_least(10),
      bindns::bind(async_read_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 1234, buffers,
      xio::transfer_at_least(10),
      bindns::bind(async_read_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 0, buffers,
      xio::transfer_at_least(42),
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 1234, buffers,
      xio::transfer_at_least(42),
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 0, buffers,
      xio::transfer_at_least(42),
      bindns::bind(async_read_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, 42));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 1234, buffers,
      xio::transfer_at_least(42),
      bindns::bind(async_read_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, 42));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 0, buffers,
      xio::transfer_at_least(42),
      bindns::bind(async_read_handler,
        _1, _2, 50, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, 50));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 1234, buffers,
      xio::transfer_at_least(42),
      bindns::bind(async_read_handler,
        _1, _2, 50, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, 50));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 0, buffers,
      xio::transfer_exactly(1),
      bindns::bind(async_read_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, 1));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 1234, buffers,
      xio::transfer_exactly(1),
      bindns::bind(async_read_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, 1));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 0, buffers,
      xio::transfer_exactly(1),
      bindns::bind(async_read_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, 1));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 1234, buffers,
      xio::transfer_exactly(1),
      bindns::bind(async_read_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, 1));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 0, buffers,
      xio::transfer_exactly(1),
      bindns::bind(async_read_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, 1));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 1234, buffers,
      xio::transfer_exactly(1),
      bindns::bind(async_read_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, 1));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 0, buffers,
      xio::transfer_exactly(10),
      bindns::bind(async_read_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 1234, buffers,
      xio::transfer_exactly(10),
      bindns::bind(async_read_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 0, buffers,
      xio::transfer_exactly(10),
      bindns::bind(async_read_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 1234, buffers,
      xio::transfer_exactly(10),
      bindns::bind(async_read_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 0, buffers,
      xio::transfer_exactly(10),
      bindns::bind(async_read_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 1234, buffers,
      xio::transfer_exactly(10),
      bindns::bind(async_read_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 0, buffers,
      xio::transfer_exactly(42),
      bindns::bind(async_read_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, 42));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 1234, buffers,
      xio::transfer_exactly(42),
      bindns::bind(async_read_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, 42));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 0, buffers,
      xio::transfer_exactly(42),
      bindns::bind(async_read_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, 42));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 1234, buffers,
      xio::transfer_exactly(42),
      bindns::bind(async_read_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, 42));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 0, buffers,
      xio::transfer_exactly(42),
      bindns::bind(async_read_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, 42));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 1234, buffers,
      xio::transfer_exactly(42),
      bindns::bind(async_read_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, 42));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 0, buffers, old_style_transfer_all,
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 1234, buffers, old_style_transfer_all,
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 0, buffers, old_style_transfer_all,
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 1234, buffers, old_style_transfer_all,
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 0, buffers, old_style_transfer_all,
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 1234, buffers, old_style_transfer_all,
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 0, buffers, short_transfer(),
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 1234, buffers, short_transfer(),
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 0, buffers, short_transfer(),
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 1234, buffers, short_transfer(),
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 0, buffers, short_transfer(),
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 1234, buffers, short_transfer(),
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  int i = xio::async_read_at(s, 1234, buffers,
      short_transfer(), archetypes::lazy_handler());
  XIO_CHECK(i == 42);
  ioc.restart();
  ioc.run();
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 1234, buffers, short_transfer())(
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(read_data)));
}

void test_5_arg_vector_buffers_async_read_at()
{
  namespace bindns = std;
  using bindns::placeholders::_1;
  using bindns::placeholders::_2;

  xio::io_context ioc;
  test_random_access_device s(ioc);
  char read_buf[sizeof(read_data)];
  std::vector<xio::mutable_buffer> buffers;
  buffers.push_back(xio::buffer(read_buf, 32));
  buffers.push_back(xio::buffer(read_buf) + 32);

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  bool called = false;
  xio::async_read_at(s, 0, buffers,
      xio::transfer_all(),
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 1234, buffers,
      xio::transfer_all(),
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 0, buffers,
      xio::transfer_all(),
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 1234, buffers,
      xio::transfer_all(),
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 0, buffers,
      xio::transfer_all(),
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 1234, buffers,
      xio::transfer_all(),
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 0, buffers,
      xio::transfer_at_least(1),
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 1234, buffers,
      xio::transfer_at_least(1),
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 0, buffers,
      xio::transfer_at_least(1),
      bindns::bind(async_read_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, 1));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 1234, buffers,
      xio::transfer_at_least(1),
      bindns::bind(async_read_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, 1));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 0, buffers,
      xio::transfer_at_least(1),
      bindns::bind(async_read_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 1234, buffers,
      xio::transfer_at_least(1),
      bindns::bind(async_read_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 0, buffers,
      xio::transfer_at_least(10),
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 1234, buffers,
      xio::transfer_at_least(10),
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 0, buffers,
      xio::transfer_at_least(10),
      bindns::bind(async_read_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 1234, buffers,
      xio::transfer_at_least(10),
      bindns::bind(async_read_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 0, buffers,
      xio::transfer_at_least(10),
      bindns::bind(async_read_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 1234, buffers,
      xio::transfer_at_least(10),
      bindns::bind(async_read_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 0, buffers,
      xio::transfer_at_least(42),
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 1234, buffers,
      xio::transfer_at_least(42),
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 0, buffers,
      xio::transfer_at_least(42),
      bindns::bind(async_read_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, 42));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 1234, buffers,
      xio::transfer_at_least(42),
      bindns::bind(async_read_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, 42));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 0, buffers,
      xio::transfer_at_least(42),
      bindns::bind(async_read_handler,
        _1, _2, 50, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, 50));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 1234, buffers,
      xio::transfer_at_least(42),
      bindns::bind(async_read_handler,
        _1, _2, 50, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, 50));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 0, buffers,
      xio::transfer_exactly(1),
      bindns::bind(async_read_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, 1));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 1234, buffers,
      xio::transfer_exactly(1),
      bindns::bind(async_read_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, 1));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 0, buffers,
      xio::transfer_exactly(1),
      bindns::bind(async_read_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, 1));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 1234, buffers,
      xio::transfer_exactly(1),
      bindns::bind(async_read_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, 1));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 0, buffers,
      xio::transfer_exactly(1),
      bindns::bind(async_read_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, 1));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 1234, buffers,
      xio::transfer_exactly(1),
      bindns::bind(async_read_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, 1));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 0, buffers,
      xio::transfer_exactly(10),
      bindns::bind(async_read_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 1234, buffers,
      xio::transfer_exactly(10),
      bindns::bind(async_read_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 0, buffers,
      xio::transfer_exactly(10),
      bindns::bind(async_read_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 1234, buffers,
      xio::transfer_exactly(10),
      bindns::bind(async_read_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 0, buffers,
      xio::transfer_exactly(10),
      bindns::bind(async_read_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 1234, buffers,
      xio::transfer_exactly(10),
      bindns::bind(async_read_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 0, buffers,
      xio::transfer_exactly(42),
      bindns::bind(async_read_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, 42));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 1234, buffers,
      xio::transfer_exactly(42),
      bindns::bind(async_read_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, 42));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 0, buffers,
      xio::transfer_exactly(42),
      bindns::bind(async_read_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, 42));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 1234, buffers,
      xio::transfer_exactly(42),
      bindns::bind(async_read_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, 42));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 0, buffers,
      xio::transfer_exactly(42),
      bindns::bind(async_read_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, 42));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 1234, buffers,
      xio::transfer_exactly(42),
      bindns::bind(async_read_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, 42));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 0, buffers, old_style_transfer_all,
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 1234, buffers, old_style_transfer_all,
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 0, buffers, old_style_transfer_all,
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 1234, buffers, old_style_transfer_all,
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 0, buffers, old_style_transfer_all,
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 1234, buffers, old_style_transfer_all,
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 0, buffers, short_transfer(),
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 1234, buffers, short_transfer(),
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 0, buffers, short_transfer(),
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 1234, buffers, short_transfer(),
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 0, buffers, short_transfer(),
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 1234, buffers, short_transfer(),
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  memset(read_buf, 0, sizeof(read_buf));
  int i = xio::async_read_at(s, 1234, buffers,
      short_transfer(), archetypes::lazy_handler());
  XIO_CHECK(i == 42);
  ioc.restart();
  ioc.run();
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  memset(read_buf, 0, sizeof(read_buf));
  called = false;
  xio::async_read_at(s, 1234, buffers, short_transfer())(
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(read_data)));
}

void test_5_arg_streambuf_async_read_at()
{
  namespace bindns = std;
  using bindns::placeholders::_1;
  using bindns::placeholders::_2;

  xio::io_context ioc;
  test_random_access_device s(ioc);
  xio::streambuf sb(sizeof(read_data));

  s.reset(read_data, sizeof(read_data));
  sb.consume(sb.size());
  bool called = false;
  xio::async_read_at(s, 0, sb,
      xio::transfer_all(),
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(sb.size() == sizeof(read_data));
  XIO_CHECK(s.check_buffers(0, sb.data(), sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  sb.consume(sb.size());
  called = false;
  xio::async_read_at(s, 1234, sb,
      xio::transfer_all(),
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(sb.size() == sizeof(read_data));
  XIO_CHECK(s.check_buffers(1234, sb.data(), sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb.consume(sb.size());
  called = false;
  xio::async_read_at(s, 0, sb,
      xio::transfer_all(),
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(sb.size() == sizeof(read_data));
  XIO_CHECK(s.check_buffers(0, sb.data(), sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb.consume(sb.size());
  called = false;
  xio::async_read_at(s, 1234, sb,
      xio::transfer_all(),
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(sb.size() == sizeof(read_data));
  XIO_CHECK(s.check_buffers(1234, sb.data(), sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb.consume(sb.size());
  called = false;
  xio::async_read_at(s, 0, sb,
      xio::transfer_all(),
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(sb.size() == sizeof(read_data));
  XIO_CHECK(s.check_buffers(0, sb.data(), sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb.consume(sb.size());
  called = false;
  xio::async_read_at(s, 1234, sb,
      xio::transfer_all(),
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(sb.size() == sizeof(read_data));
  XIO_CHECK(s.check_buffers(1234, sb.data(), sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  sb.consume(sb.size());
  called = false;
  xio::async_read_at(s, 0, sb,
      xio::transfer_at_least(1),
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(sb.size() == sizeof(read_data));
  XIO_CHECK(s.check_buffers(0, sb.data(), sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  sb.consume(sb.size());
  called = false;
  xio::async_read_at(s, 1234, sb,
      xio::transfer_at_least(1),
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(sb.size() == sizeof(read_data));
  XIO_CHECK(s.check_buffers(1234, sb.data(), sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb.consume(sb.size());
  called = false;
  xio::async_read_at(s, 0, sb,
      xio::transfer_at_least(1),
      bindns::bind(async_read_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(sb.size() == 1);
  XIO_CHECK(s.check_buffers(0, sb.data(), 1));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb.consume(sb.size());
  called = false;
  xio::async_read_at(s, 1234, sb,
      xio::transfer_at_least(1),
      bindns::bind(async_read_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(sb.size() == 1);
  XIO_CHECK(s.check_buffers(1234, sb.data(), 1));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb.consume(sb.size());
  called = false;
  xio::async_read_at(s, 0, sb,
      xio::transfer_at_least(1),
      bindns::bind(async_read_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(sb.size() == 10);
  XIO_CHECK(s.check_buffers(0, sb.data(), 10));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb.consume(sb.size());
  called = false;
  xio::async_read_at(s, 1234, sb,
      xio::transfer_at_least(1),
      bindns::bind(async_read_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(sb.size() == 10);
  XIO_CHECK(s.check_buffers(1234, sb.data(), 10));

  s.reset(read_data, sizeof(read_data));
  sb.consume(sb.size());
  called = false;
  xio::async_read_at(s, 0, sb,
      xio::transfer_at_least(10),
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(sb.size() == sizeof(read_data));
  XIO_CHECK(s.check_buffers(0, sb.data(), sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  sb.consume(sb.size());
  called = false;
  xio::async_read_at(s, 1234, sb,
      xio::transfer_at_least(10),
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(sb.size() == sizeof(read_data));
  XIO_CHECK(s.check_buffers(1234, sb.data(), sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb.consume(sb.size());
  called = false;
  xio::async_read_at(s, 0, sb,
      xio::transfer_at_least(10),
      bindns::bind(async_read_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(sb.size() == 10);
  XIO_CHECK(s.check_buffers(0, sb.data(), 10));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb.consume(sb.size());
  called = false;
  xio::async_read_at(s, 1234, sb,
      xio::transfer_at_least(10),
      bindns::bind(async_read_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(sb.size() == 10);
  XIO_CHECK(s.check_buffers(1234, sb.data(), 10));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb.consume(sb.size());
  called = false;
  xio::async_read_at(s, 0, sb,
      xio::transfer_at_least(10),
      bindns::bind(async_read_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(sb.size() == 10);
  XIO_CHECK(s.check_buffers(0, sb.data(), 10));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb.consume(sb.size());
  called = false;
  xio::async_read_at(s, 1234, sb,
      xio::transfer_at_least(10),
      bindns::bind(async_read_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(sb.size() == 10);
  XIO_CHECK(s.check_buffers(1234, sb.data(), 10));

  s.reset(read_data, sizeof(read_data));
  sb.consume(sb.size());
  called = false;
  xio::async_read_at(s, 0, sb,
      xio::transfer_at_least(42),
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(sb.size() == sizeof(read_data));
  XIO_CHECK(s.check_buffers(0, sb.data(), sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  sb.consume(sb.size());
  called = false;
  xio::async_read_at(s, 1234, sb,
      xio::transfer_at_least(42),
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(sb.size() == sizeof(read_data));
  XIO_CHECK(s.check_buffers(1234, sb.data(), sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb.consume(sb.size());
  called = false;
  xio::async_read_at(s, 0, sb,
      xio::transfer_at_least(42),
      bindns::bind(async_read_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(sb.size() == 42);
  XIO_CHECK(s.check_buffers(0, sb.data(), 42));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb.consume(sb.size());
  called = false;
  xio::async_read_at(s, 1234, sb,
      xio::transfer_at_least(42),
      bindns::bind(async_read_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(sb.size() == 42);
  XIO_CHECK(s.check_buffers(1234, sb.data(), 42));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb.consume(sb.size());
  called = false;
  xio::async_read_at(s, 0, sb,
      xio::transfer_at_least(42),
      bindns::bind(async_read_handler,
        _1, _2, 50, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(sb.size() == 50);
  XIO_CHECK(s.check_buffers(0, sb.data(), 50));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb.consume(sb.size());
  called = false;
  xio::async_read_at(s, 1234, sb,
      xio::transfer_at_least(42),
      bindns::bind(async_read_handler,
        _1, _2, 50, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(sb.size() == 50);
  XIO_CHECK(s.check_buffers(1234, sb.data(), 50));

  s.reset(read_data, sizeof(read_data));
  sb.consume(sb.size());
  called = false;
  xio::async_read_at(s, 0, sb,
      xio::transfer_exactly(1),
      bindns::bind(async_read_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(sb.size() == 1);
  XIO_CHECK(s.check_buffers(0, sb.data(), 1));

  s.reset(read_data, sizeof(read_data));
  sb.consume(sb.size());
  called = false;
  xio::async_read_at(s, 1234, sb,
      xio::transfer_exactly(1),
      bindns::bind(async_read_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(sb.size() == 1);
  XIO_CHECK(s.check_buffers(1234, sb.data(), 1));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb.consume(sb.size());
  called = false;
  xio::async_read_at(s, 0, sb,
      xio::transfer_exactly(1),
      bindns::bind(async_read_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(sb.size() == 1);
  XIO_CHECK(s.check_buffers(0, sb.data(), 1));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb.consume(sb.size());
  called = false;
  xio::async_read_at(s, 1234, sb,
      xio::transfer_exactly(1),
      bindns::bind(async_read_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(sb.size() == 1);
  XIO_CHECK(s.check_buffers(1234, sb.data(), 1));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb.consume(sb.size());
  called = false;
  xio::async_read_at(s, 0, sb,
      xio::transfer_exactly(1),
      bindns::bind(async_read_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(sb.size() == 1);
  XIO_CHECK(s.check_buffers(0, sb.data(), 1));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb.consume(sb.size());
  called = false;
  xio::async_read_at(s, 1234, sb,
      xio::transfer_exactly(1),
      bindns::bind(async_read_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(sb.size() == 1);
  XIO_CHECK(s.check_buffers(1234, sb.data(), 1));

  s.reset(read_data, sizeof(read_data));
  sb.consume(sb.size());
  called = false;
  xio::async_read_at(s, 0, sb,
      xio::transfer_exactly(10),
      bindns::bind(async_read_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(sb.size() == 10);
  XIO_CHECK(s.check_buffers(0, sb.data(), 10));

  s.reset(read_data, sizeof(read_data));
  sb.consume(sb.size());
  called = false;
  xio::async_read_at(s, 1234, sb,
      xio::transfer_exactly(10),
      bindns::bind(async_read_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(sb.size() == 10);
  XIO_CHECK(s.check_buffers(1234, sb.data(), 10));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb.consume(sb.size());
  called = false;
  xio::async_read_at(s, 0, sb,
      xio::transfer_exactly(10),
      bindns::bind(async_read_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(sb.size() == 10);
  XIO_CHECK(s.check_buffers(0, sb.data(), 10));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb.consume(sb.size());
  called = false;
  xio::async_read_at(s, 1234, sb,
      xio::transfer_exactly(10),
      bindns::bind(async_read_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(sb.size() == 10);
  XIO_CHECK(s.check_buffers(1234, sb.data(), 10));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb.consume(sb.size());
  called = false;
  xio::async_read_at(s, 0, sb,
      xio::transfer_exactly(10),
      bindns::bind(async_read_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(sb.size() == 10);
  XIO_CHECK(s.check_buffers(0, sb.data(), 10));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb.consume(sb.size());
  called = false;
  xio::async_read_at(s, 1234, sb,
      xio::transfer_exactly(10),
      bindns::bind(async_read_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(sb.size() == 10);
  XIO_CHECK(s.check_buffers(1234, sb.data(), 10));

  s.reset(read_data, sizeof(read_data));
  sb.consume(sb.size());
  called = false;
  xio::async_read_at(s, 0, sb,
      xio::transfer_exactly(42),
      bindns::bind(async_read_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(sb.size() == 42);
  XIO_CHECK(s.check_buffers(0, sb.data(), 42));

  s.reset(read_data, sizeof(read_data));
  sb.consume(sb.size());
  called = false;
  xio::async_read_at(s, 1234, sb,
      xio::transfer_exactly(42),
      bindns::bind(async_read_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(sb.size() == 42);
  XIO_CHECK(s.check_buffers(1234, sb.data(), 42));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb.consume(sb.size());
  called = false;
  xio::async_read_at(s, 0, sb,
      xio::transfer_exactly(42),
      bindns::bind(async_read_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(sb.size() == 42);
  XIO_CHECK(s.check_buffers(0, sb.data(), 42));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb.consume(sb.size());
  called = false;
  xio::async_read_at(s, 1234, sb,
      xio::transfer_exactly(42),
      bindns::bind(async_read_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(sb.size() == 42);
  XIO_CHECK(s.check_buffers(1234, sb.data(), 42));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb.consume(sb.size());
  called = false;
  xio::async_read_at(s, 0, sb,
      xio::transfer_exactly(42),
      bindns::bind(async_read_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(sb.size() == 42);
  XIO_CHECK(s.check_buffers(0, sb.data(), 42));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb.consume(sb.size());
  called = false;
  xio::async_read_at(s, 1234, sb,
      xio::transfer_exactly(42),
      bindns::bind(async_read_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(sb.size() == 42);
  XIO_CHECK(s.check_buffers(1234, sb.data(), 42));

  s.reset(read_data, sizeof(read_data));
  sb.consume(sb.size());
  called = false;
  xio::async_read_at(s, 0, sb, old_style_transfer_all,
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(sb.size() == sizeof(read_data));
  XIO_CHECK(s.check_buffers(0, sb.data(), sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  sb.consume(sb.size());
  called = false;
  xio::async_read_at(s, 1234, sb, old_style_transfer_all,
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(sb.size() == sizeof(read_data));
  XIO_CHECK(s.check_buffers(1234, sb.data(), sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb.consume(sb.size());
  called = false;
  xio::async_read_at(s, 0, sb, old_style_transfer_all,
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(sb.size() == sizeof(read_data));
  XIO_CHECK(s.check_buffers(0, sb.data(), sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb.consume(sb.size());
  called = false;
  xio::async_read_at(s, 1234, sb, old_style_transfer_all,
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(sb.size() == sizeof(read_data));
  XIO_CHECK(s.check_buffers(1234, sb.data(), sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb.consume(sb.size());
  called = false;
  xio::async_read_at(s, 0, sb, old_style_transfer_all,
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(sb.size() == sizeof(read_data));
  XIO_CHECK(s.check_buffers(0, sb.data(), sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb.consume(sb.size());
  called = false;
  xio::async_read_at(s, 1234, sb, old_style_transfer_all,
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(sb.size() == sizeof(read_data));
  XIO_CHECK(s.check_buffers(1234, sb.data(), sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  sb.consume(sb.size());
  called = false;
  xio::async_read_at(s, 0, sb, short_transfer(),
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(sb.size() == sizeof(read_data));
  XIO_CHECK(s.check_buffers(0, sb.data(), sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  sb.consume(sb.size());
  called = false;
  xio::async_read_at(s, 1234, sb, short_transfer(),
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(sb.size() == sizeof(read_data));
  XIO_CHECK(s.check_buffers(1234, sb.data(), sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb.consume(sb.size());
  called = false;
  xio::async_read_at(s, 0, sb, short_transfer(),
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(sb.size() == sizeof(read_data));
  XIO_CHECK(s.check_buffers(0, sb.data(), sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(1);
  sb.consume(sb.size());
  called = false;
  xio::async_read_at(s, 1234, sb, short_transfer(),
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(sb.size() == sizeof(read_data));
  XIO_CHECK(s.check_buffers(1234, sb.data(), sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb.consume(sb.size());
  called = false;
  xio::async_read_at(s, 0, sb, short_transfer(),
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(sb.size() == sizeof(read_data));
  XIO_CHECK(s.check_buffers(0, sb.data(), sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb.consume(sb.size());
  called = false;
  xio::async_read_at(s, 1234, sb, short_transfer(),
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(sb.size() == sizeof(read_data));
  XIO_CHECK(s.check_buffers(1234, sb.data(), sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  sb.consume(sb.size());
  int i = xio::async_read_at(s, 1234, sb,
      short_transfer(), archetypes::lazy_handler());
  XIO_CHECK(i == 42);
  ioc.restart();
  ioc.run();
  XIO_CHECK(s.check_buffers(1234, sb.data(), sizeof(read_data)));

  s.reset(read_data, sizeof(read_data));
  s.next_read_length(10);
  sb.consume(sb.size());
  called = false;
  xio::async_read_at(s, 1234, sb, short_transfer())(
      bindns::bind(async_read_handler,
        _1, _2, sizeof(read_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(sb.size() == sizeof(read_data));
  XIO_CHECK(s.check_buffers(1234, sb.data(), sizeof(read_data)));
}

XIO_TEST_SUITE
(
  "read_at",
  XIO_TEST_CASE(test_3_arg_mutable_buffer_read_at)
  XIO_TEST_CASE(test_3_arg_vector_buffers_read_at)
  XIO_TEST_CASE(test_3_arg_streambuf_read_at)
  XIO_TEST_CASE(test_4_arg_nothrow_mutable_buffer_read_at)
  XIO_TEST_CASE(test_4_arg_nothrow_vector_buffers_read_at)
  XIO_TEST_CASE(test_4_arg_nothrow_streambuf_read_at)
  XIO_TEST_CASE(test_4_arg_mutable_buffer_read_at)
  XIO_TEST_CASE(test_4_arg_vector_buffers_read_at)
  XIO_TEST_CASE(test_4_arg_streambuf_read_at)
  XIO_TEST_CASE(test_5_arg_mutable_buffer_read_at)
  XIO_TEST_CASE(test_5_arg_vector_buffers_read_at)
  XIO_TEST_CASE(test_5_arg_streambuf_read_at)
  XIO_TEST_CASE(test_4_arg_mutable_buffer_async_read_at)
  XIO_TEST_CASE(test_4_arg_boost_array_buffers_async_read_at)
  XIO_TEST_CASE(test_4_arg_std_array_buffers_async_read_at)
  XIO_TEST_CASE(test_4_arg_vector_buffers_async_read_at)
  XIO_TEST_CASE(test_4_arg_streambuf_async_read_at)
  XIO_TEST_CASE(test_5_arg_mutable_buffer_async_read_at)
  XIO_TEST_CASE(test_5_arg_boost_array_buffers_async_read_at)
  XIO_TEST_CASE(test_5_arg_std_array_buffers_async_read_at)
  XIO_TEST_CASE(test_5_arg_vector_buffers_async_read_at)
  XIO_TEST_CASE(test_5_arg_streambuf_async_read_at)
)
