//
// write_at.cpp
// ~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//



// Test that header file is self-contained.
#include <xio/write_at.h>

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
      length_(max_length),
      next_write_length_(max_length)
  {
    memset(data_, 0, max_length);
  }

  executor_type get_executor() noexcept
  {
    return io_context_.get_executor();
  }

  void reset()
  {
    memset(data_, 0, max_length);
    next_write_length_ = max_length;
  }

  void next_write_length(size_t length)
  {
    next_write_length_ = length;
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

  template <typename Const_Buffers>
  size_t write_some_at(uint64_t offset,
      const Const_Buffers& buffers)
  {
    return xio::buffer_copy(
        xio::buffer(data_, length_) + offset,
        buffers, next_write_length_);
  }

  template <typename Const_Buffers>
  size_t write_some_at(uint64_t offset,
      const Const_Buffers& buffers, xio::error_code& ec)
  {
    ec = xio::error_code();
    return write_some_at(offset, buffers);
  }

  template <typename Const_Buffers, typename Handler>
  void async_write_some_at(uint64_t offset,
      const Const_Buffers& buffers, Handler&& handler)
  {
    size_t bytes_transferred = write_some_at(offset, buffers);
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
  size_t next_write_length_;
};

static const char write_data[]
  = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
static char mutable_write_data[]
  = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

void test_3_arg_const_buffer_write_at()
{
  xio::io_context ioc;
  test_random_access_device s(ioc);
  xio::const_buffer buffers
    = xio::buffer(write_data, sizeof(write_data));

  s.reset();
  size_t bytes_transferred = xio::write_at(s, 0, buffers);
  XIO_CHECK(bytes_transferred == sizeof(write_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  bytes_transferred = xio::write_at(s, 1234, buffers);
  XIO_CHECK(bytes_transferred == sizeof(write_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = xio::write_at(s, 0, buffers);
  XIO_CHECK(bytes_transferred == sizeof(write_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = xio::write_at(s, 1234, buffers);
  XIO_CHECK(bytes_transferred == sizeof(write_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = xio::write_at(s, 0, buffers);
  XIO_CHECK(bytes_transferred == sizeof(write_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = xio::write_at(s, 1234, buffers);
  XIO_CHECK(bytes_transferred == sizeof(write_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));
}

void test_3_arg_mutable_buffer_write_at()
{
  xio::io_context ioc;
  test_random_access_device s(ioc);
  xio::mutable_buffer buffers
    = xio::buffer(mutable_write_data, sizeof(mutable_write_data));

  s.reset();
  size_t bytes_transferred = xio::write_at(s, 0, buffers);
  XIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(mutable_write_data)));

  s.reset();
  bytes_transferred = xio::write_at(s, 1234, buffers);
  XIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(mutable_write_data)));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = xio::write_at(s, 0, buffers);
  XIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(mutable_write_data)));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = xio::write_at(s, 1234, buffers);
  XIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(mutable_write_data)));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = xio::write_at(s, 0, buffers);
  XIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(mutable_write_data)));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = xio::write_at(s, 1234, buffers);
  XIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(mutable_write_data)));
}

void test_3_arg_vector_buffers_write_at()
{
  xio::io_context ioc;
  test_random_access_device s(ioc);
  std::vector<xio::const_buffer> buffers;
  buffers.push_back(xio::buffer(write_data, 32));
  buffers.push_back(xio::buffer(write_data) + 32);

  s.reset();
  size_t bytes_transferred = xio::write_at(s, 0, buffers);
  XIO_CHECK(bytes_transferred == sizeof(write_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  bytes_transferred = xio::write_at(s, 1234, buffers);
  XIO_CHECK(bytes_transferred == sizeof(write_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = xio::write_at(s, 0, buffers);
  XIO_CHECK(bytes_transferred == sizeof(write_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = xio::write_at(s, 1234, buffers);
  XIO_CHECK(bytes_transferred == sizeof(write_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = xio::write_at(s, 0, buffers);
  XIO_CHECK(bytes_transferred == sizeof(write_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = xio::write_at(s, 1234, buffers);
  XIO_CHECK(bytes_transferred == sizeof(write_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));
}

void test_4_arg_nothrow_const_buffer_write_at()
{
  xio::io_context ioc;
  test_random_access_device s(ioc);
  xio::const_buffer buffers
    = xio::buffer(write_data, sizeof(write_data));

  s.reset();
  xio::error_code error;
  size_t bytes_transferred = xio::write_at(s, 0, buffers, error);
  XIO_CHECK(bytes_transferred == sizeof(write_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));
  XIO_CHECK(!error);

  s.reset();
  bytes_transferred = xio::write_at(s, 1234, buffers, error);
  XIO_CHECK(bytes_transferred == sizeof(write_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));
  XIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  bytes_transferred = xio::write_at(s, 0, buffers, error);
  XIO_CHECK(bytes_transferred == sizeof(write_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));
  XIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  bytes_transferred = xio::write_at(s, 1234, buffers, error);
  XIO_CHECK(bytes_transferred == sizeof(write_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));
  XIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  bytes_transferred = xio::write_at(s, 0, buffers, error);
  XIO_CHECK(bytes_transferred == sizeof(write_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));
  XIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  bytes_transferred = xio::write_at(s, 1234, buffers, error);
  XIO_CHECK(bytes_transferred == sizeof(write_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));
  XIO_CHECK(!error);
}

void test_4_arg_nothrow_mutable_buffer_write_at()
{
  xio::io_context ioc;
  test_random_access_device s(ioc);
  xio::mutable_buffer buffers
    = xio::buffer(mutable_write_data, sizeof(mutable_write_data));

  s.reset();
  xio::error_code error;
  size_t bytes_transferred = xio::write_at(s, 0, buffers, error);
  XIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(mutable_write_data)));
  XIO_CHECK(!error);

  s.reset();
  bytes_transferred = xio::write_at(s, 1234, buffers, error);
  XIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(mutable_write_data)));
  XIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  bytes_transferred = xio::write_at(s, 0, buffers, error);
  XIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(mutable_write_data)));
  XIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  bytes_transferred = xio::write_at(s, 1234, buffers, error);
  XIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(mutable_write_data)));
  XIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  bytes_transferred = xio::write_at(s, 0, buffers, error);
  XIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(mutable_write_data)));
  XIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  bytes_transferred = xio::write_at(s, 1234, buffers, error);
  XIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(mutable_write_data)));
  XIO_CHECK(!error);
}

void test_4_arg_nothrow_vector_buffers_write_at()
{
  xio::io_context ioc;
  test_random_access_device s(ioc);
  std::vector<xio::const_buffer> buffers;
  buffers.push_back(xio::buffer(write_data, 32));
  buffers.push_back(xio::buffer(write_data) + 32);

  s.reset();
  xio::error_code error;
  size_t bytes_transferred = xio::write_at(s, 0, buffers, error);
  XIO_CHECK(bytes_transferred == sizeof(write_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));
  XIO_CHECK(!error);

  s.reset();
  bytes_transferred = xio::write_at(s, 1234, buffers, error);
  XIO_CHECK(bytes_transferred == sizeof(write_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));
  XIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  bytes_transferred = xio::write_at(s, 0, buffers, error);
  XIO_CHECK(bytes_transferred == sizeof(write_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));
  XIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  bytes_transferred = xio::write_at(s, 1234, buffers, error);
  XIO_CHECK(bytes_transferred == sizeof(write_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));
  XIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  bytes_transferred = xio::write_at(s, 0, buffers, error);
  XIO_CHECK(bytes_transferred == sizeof(write_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));
  XIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  bytes_transferred = xio::write_at(s, 1234, buffers, error);
  XIO_CHECK(bytes_transferred == sizeof(write_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));
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

void test_4_arg_const_buffer_write_at()
{
  xio::io_context ioc;
  test_random_access_device s(ioc);
  xio::const_buffer buffers
    = xio::buffer(write_data, sizeof(write_data));

  s.reset();
  size_t bytes_transferred = xio::write_at(s, 0, buffers,
      xio::transfer_all());
  XIO_CHECK(bytes_transferred == sizeof(write_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  bytes_transferred = xio::write_at(s, 1234, buffers,
      xio::transfer_all());
  XIO_CHECK(bytes_transferred == sizeof(write_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = xio::write_at(s, 0, buffers,
      xio::transfer_all());
  XIO_CHECK(bytes_transferred == sizeof(write_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = xio::write_at(s, 1234, buffers,
      xio::transfer_all());
  XIO_CHECK(bytes_transferred == sizeof(write_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = xio::write_at(s, 0, buffers,
      xio::transfer_all());
  XIO_CHECK(bytes_transferred == sizeof(write_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = xio::write_at(s, 1234, buffers,
      xio::transfer_all());
  XIO_CHECK(bytes_transferred == sizeof(write_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  bytes_transferred = xio::write_at(s, 0, buffers,
      xio::transfer_at_least(1));
  XIO_CHECK(bytes_transferred == sizeof(write_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  bytes_transferred = xio::write_at(s, 1234, buffers,
      xio::transfer_at_least(1));
  XIO_CHECK(bytes_transferred == sizeof(write_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = xio::write_at(s, 0, buffers,
      xio::transfer_at_least(1));
  XIO_CHECK(bytes_transferred == 1);
  XIO_CHECK(s.check_buffers(0, buffers, 1));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = xio::write_at(s, 1234, buffers,
      xio::transfer_at_least(1));
  XIO_CHECK(bytes_transferred == 1);
  XIO_CHECK(s.check_buffers(1234, buffers, 1));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = xio::write_at(s, 0, buffers,
      xio::transfer_at_least(1));
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = xio::write_at(s, 1234, buffers,
      xio::transfer_at_least(1));
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset();
  bytes_transferred = xio::write_at(s, 0, buffers,
      xio::transfer_at_least(10));
  XIO_CHECK(bytes_transferred == sizeof(write_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  bytes_transferred = xio::write_at(s, 1234, buffers,
      xio::transfer_at_least(10));
  XIO_CHECK(bytes_transferred == sizeof(write_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = xio::write_at(s, 0, buffers,
      xio::transfer_at_least(10));
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = xio::write_at(s, 1234, buffers,
      xio::transfer_at_least(10));
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = xio::write_at(s, 0, buffers,
      xio::transfer_at_least(10));
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = xio::write_at(s, 1234, buffers,
      xio::transfer_at_least(10));
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset();
  bytes_transferred = xio::write_at(s, 0, buffers,
      xio::transfer_at_least(42));
  XIO_CHECK(bytes_transferred == sizeof(write_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  bytes_transferred = xio::write_at(s, 1234, buffers,
      xio::transfer_at_least(42));
  XIO_CHECK(bytes_transferred == sizeof(write_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = xio::write_at(s, 0, buffers,
      xio::transfer_at_least(42));
  XIO_CHECK(bytes_transferred == 42);
  XIO_CHECK(s.check_buffers(0, buffers, 42));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = xio::write_at(s, 1234, buffers,
      xio::transfer_at_least(42));
  XIO_CHECK(bytes_transferred == 42);
  XIO_CHECK(s.check_buffers(1234, buffers, 42));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = xio::write_at(s, 0, buffers,
      xio::transfer_at_least(42));
  XIO_CHECK(bytes_transferred == 50);
  XIO_CHECK(s.check_buffers(0, buffers, 50));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = xio::write_at(s, 1234, buffers,
      xio::transfer_at_least(42));
  XIO_CHECK(bytes_transferred == 50);
  XIO_CHECK(s.check_buffers(1234, buffers, 50));

  s.reset();
  bytes_transferred = xio::write_at(s, 0, buffers,
      xio::transfer_exactly(1));
  XIO_CHECK(bytes_transferred == 1);
  XIO_CHECK(s.check_buffers(0, buffers, 1));

  s.reset();
  bytes_transferred = xio::write_at(s, 1234, buffers,
      xio::transfer_exactly(1));
  XIO_CHECK(bytes_transferred == 1);
  XIO_CHECK(s.check_buffers(1234, buffers, 1));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = xio::write_at(s, 0, buffers,
      xio::transfer_exactly(1));
  XIO_CHECK(bytes_transferred == 1);
  XIO_CHECK(s.check_buffers(0, buffers, 1));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = xio::write_at(s, 1234, buffers,
      xio::transfer_exactly(1));
  XIO_CHECK(bytes_transferred == 1);
  XIO_CHECK(s.check_buffers(1234, buffers, 1));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = xio::write_at(s, 0, buffers,
      xio::transfer_exactly(1));
  XIO_CHECK(bytes_transferred == 1);
  XIO_CHECK(s.check_buffers(0, buffers, 1));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = xio::write_at(s, 1234, buffers,
      xio::transfer_exactly(1));
  XIO_CHECK(bytes_transferred == 1);
  XIO_CHECK(s.check_buffers(1234, buffers, 1));

  s.reset();
  bytes_transferred = xio::write_at(s, 0, buffers,
      xio::transfer_exactly(10));
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset();
  bytes_transferred = xio::write_at(s, 1234, buffers,
      xio::transfer_exactly(10));
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = xio::write_at(s, 0, buffers,
      xio::transfer_exactly(10));
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = xio::write_at(s, 1234, buffers,
      xio::transfer_exactly(10));
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = xio::write_at(s, 0, buffers,
      xio::transfer_exactly(10));
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = xio::write_at(s, 1234, buffers,
      xio::transfer_exactly(10));
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset();
  bytes_transferred = xio::write_at(s, 0, buffers,
      xio::transfer_exactly(42));
  XIO_CHECK(bytes_transferred == 42);
  XIO_CHECK(s.check_buffers(0, buffers, 42));

  s.reset();
  bytes_transferred = xio::write_at(s, 1234, buffers,
      xio::transfer_exactly(42));
  XIO_CHECK(bytes_transferred == 42);
  XIO_CHECK(s.check_buffers(1234, buffers, 42));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = xio::write_at(s, 0, buffers,
      xio::transfer_exactly(42));
  XIO_CHECK(bytes_transferred == 42);
  XIO_CHECK(s.check_buffers(0, buffers, 42));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = xio::write_at(s, 1234, buffers,
      xio::transfer_exactly(42));
  XIO_CHECK(bytes_transferred == 42);
  XIO_CHECK(s.check_buffers(1234, buffers, 42));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = xio::write_at(s, 0, buffers,
      xio::transfer_exactly(42));
  XIO_CHECK(bytes_transferred == 42);
  XIO_CHECK(s.check_buffers(0, buffers, 42));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = xio::write_at(s, 1234, buffers,
      xio::transfer_exactly(42));
  XIO_CHECK(bytes_transferred == 42);
  XIO_CHECK(s.check_buffers(1234, buffers, 42));

  s.reset();
  bytes_transferred = xio::write_at(s, 0, buffers,
      old_style_transfer_all);
  XIO_CHECK(bytes_transferred == sizeof(write_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  bytes_transferred = xio::write_at(s, 1234, buffers,
      old_style_transfer_all);
  XIO_CHECK(bytes_transferred == sizeof(write_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = xio::write_at(s, 0, buffers,
      old_style_transfer_all);
  XIO_CHECK(bytes_transferred == sizeof(write_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = xio::write_at(s, 1234, buffers,
      old_style_transfer_all);
  XIO_CHECK(bytes_transferred == sizeof(write_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = xio::write_at(s, 0, buffers,
      old_style_transfer_all);
  XIO_CHECK(bytes_transferred == sizeof(write_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = xio::write_at(s, 1234, buffers,
      old_style_transfer_all);
  XIO_CHECK(bytes_transferred == sizeof(write_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  bytes_transferred = xio::write_at(s, 0, buffers, short_transfer());
  XIO_CHECK(bytes_transferred == sizeof(write_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  bytes_transferred = xio::write_at(s, 1234, buffers, short_transfer());
  XIO_CHECK(bytes_transferred == sizeof(write_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = xio::write_at(s, 0, buffers, short_transfer());
  XIO_CHECK(bytes_transferred == sizeof(write_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = xio::write_at(s, 1234, buffers, short_transfer());
  XIO_CHECK(bytes_transferred == sizeof(write_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = xio::write_at(s, 0, buffers, short_transfer());
  XIO_CHECK(bytes_transferred == sizeof(write_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = xio::write_at(s, 1234, buffers, short_transfer());
  XIO_CHECK(bytes_transferred == sizeof(write_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));
}

void test_4_arg_mutable_buffer_write_at()
{
  xio::io_context ioc;
  test_random_access_device s(ioc);
  xio::mutable_buffer buffers
    = xio::buffer(mutable_write_data, sizeof(mutable_write_data));

  s.reset();
  size_t bytes_transferred = xio::write_at(s, 0, buffers,
      xio::transfer_all());
  XIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(mutable_write_data)));

  s.reset();
  bytes_transferred = xio::write_at(s, 1234, buffers,
      xio::transfer_all());
  XIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(mutable_write_data)));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = xio::write_at(s, 0, buffers,
      xio::transfer_all());
  XIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(mutable_write_data)));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = xio::write_at(s, 1234, buffers,
      xio::transfer_all());
  XIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(mutable_write_data)));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = xio::write_at(s, 0, buffers,
      xio::transfer_all());
  XIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(mutable_write_data)));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = xio::write_at(s, 1234, buffers,
      xio::transfer_all());
  XIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(mutable_write_data)));

  s.reset();
  bytes_transferred = xio::write_at(s, 0, buffers,
      xio::transfer_at_least(1));
  XIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(mutable_write_data)));

  s.reset();
  bytes_transferred = xio::write_at(s, 1234, buffers,
      xio::transfer_at_least(1));
  XIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(mutable_write_data)));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = xio::write_at(s, 0, buffers,
      xio::transfer_at_least(1));
  XIO_CHECK(bytes_transferred == 1);
  XIO_CHECK(s.check_buffers(0, buffers, 1));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = xio::write_at(s, 1234, buffers,
      xio::transfer_at_least(1));
  XIO_CHECK(bytes_transferred == 1);
  XIO_CHECK(s.check_buffers(1234, buffers, 1));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = xio::write_at(s, 0, buffers,
      xio::transfer_at_least(1));
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = xio::write_at(s, 1234, buffers,
      xio::transfer_at_least(1));
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset();
  bytes_transferred = xio::write_at(s, 0, buffers,
      xio::transfer_at_least(10));
  XIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(mutable_write_data)));

  s.reset();
  bytes_transferred = xio::write_at(s, 1234, buffers,
      xio::transfer_at_least(10));
  XIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(mutable_write_data)));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = xio::write_at(s, 0, buffers,
      xio::transfer_at_least(10));
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = xio::write_at(s, 1234, buffers,
      xio::transfer_at_least(10));
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = xio::write_at(s, 0, buffers,
      xio::transfer_at_least(10));
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = xio::write_at(s, 1234, buffers,
      xio::transfer_at_least(10));
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset();
  bytes_transferred = xio::write_at(s, 0, buffers,
      xio::transfer_at_least(42));
  XIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(mutable_write_data)));

  s.reset();
  bytes_transferred = xio::write_at(s, 1234, buffers,
      xio::transfer_at_least(42));
  XIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(mutable_write_data)));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = xio::write_at(s, 0, buffers,
      xio::transfer_at_least(42));
  XIO_CHECK(bytes_transferred == 42);
  XIO_CHECK(s.check_buffers(0, buffers, 42));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = xio::write_at(s, 1234, buffers,
      xio::transfer_at_least(42));
  XIO_CHECK(bytes_transferred == 42);
  XIO_CHECK(s.check_buffers(1234, buffers, 42));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = xio::write_at(s, 0, buffers,
      xio::transfer_at_least(42));
  XIO_CHECK(bytes_transferred == 50);
  XIO_CHECK(s.check_buffers(0, buffers, 50));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = xio::write_at(s, 1234, buffers,
      xio::transfer_at_least(42));
  XIO_CHECK(bytes_transferred == 50);
  XIO_CHECK(s.check_buffers(1234, buffers, 50));

  s.reset();
  bytes_transferred = xio::write_at(s, 0, buffers,
      xio::transfer_exactly(1));
  XIO_CHECK(bytes_transferred == 1);
  XIO_CHECK(s.check_buffers(0, buffers, 1));

  s.reset();
  bytes_transferred = xio::write_at(s, 1234, buffers,
      xio::transfer_exactly(1));
  XIO_CHECK(bytes_transferred == 1);
  XIO_CHECK(s.check_buffers(1234, buffers, 1));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = xio::write_at(s, 0, buffers,
      xio::transfer_exactly(1));
  XIO_CHECK(bytes_transferred == 1);
  XIO_CHECK(s.check_buffers(0, buffers, 1));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = xio::write_at(s, 1234, buffers,
      xio::transfer_exactly(1));
  XIO_CHECK(bytes_transferred == 1);
  XIO_CHECK(s.check_buffers(1234, buffers, 1));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = xio::write_at(s, 0, buffers,
      xio::transfer_exactly(1));
  XIO_CHECK(bytes_transferred == 1);
  XIO_CHECK(s.check_buffers(0, buffers, 1));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = xio::write_at(s, 1234, buffers,
      xio::transfer_exactly(1));
  XIO_CHECK(bytes_transferred == 1);
  XIO_CHECK(s.check_buffers(1234, buffers, 1));

  s.reset();
  bytes_transferred = xio::write_at(s, 0, buffers,
      xio::transfer_exactly(10));
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset();
  bytes_transferred = xio::write_at(s, 1234, buffers,
      xio::transfer_exactly(10));
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = xio::write_at(s, 0, buffers,
      xio::transfer_exactly(10));
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = xio::write_at(s, 1234, buffers,
      xio::transfer_exactly(10));
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = xio::write_at(s, 0, buffers,
      xio::transfer_exactly(10));
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = xio::write_at(s, 1234, buffers,
      xio::transfer_exactly(10));
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset();
  bytes_transferred = xio::write_at(s, 0, buffers,
      xio::transfer_exactly(42));
  XIO_CHECK(bytes_transferred == 42);
  XIO_CHECK(s.check_buffers(0, buffers, 42));

  s.reset();
  bytes_transferred = xio::write_at(s, 1234, buffers,
      xio::transfer_exactly(42));
  XIO_CHECK(bytes_transferred == 42);
  XIO_CHECK(s.check_buffers(1234, buffers, 42));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = xio::write_at(s, 0, buffers,
      xio::transfer_exactly(42));
  XIO_CHECK(bytes_transferred == 42);
  XIO_CHECK(s.check_buffers(0, buffers, 42));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = xio::write_at(s, 1234, buffers,
      xio::transfer_exactly(42));
  XIO_CHECK(bytes_transferred == 42);
  XIO_CHECK(s.check_buffers(1234, buffers, 42));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = xio::write_at(s, 0, buffers,
      xio::transfer_exactly(42));
  XIO_CHECK(bytes_transferred == 42);
  XIO_CHECK(s.check_buffers(0, buffers, 42));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = xio::write_at(s, 1234, buffers,
      xio::transfer_exactly(42));
  XIO_CHECK(bytes_transferred == 42);
  XIO_CHECK(s.check_buffers(1234, buffers, 42));

  s.reset();
  bytes_transferred = xio::write_at(s, 0, buffers,
      old_style_transfer_all);
  XIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(mutable_write_data)));

  s.reset();
  bytes_transferred = xio::write_at(s, 1234, buffers,
      old_style_transfer_all);
  XIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(mutable_write_data)));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = xio::write_at(s, 0, buffers,
      old_style_transfer_all);
  XIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(mutable_write_data)));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = xio::write_at(s, 1234, buffers,
      old_style_transfer_all);
  XIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(mutable_write_data)));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = xio::write_at(s, 0, buffers,
      old_style_transfer_all);
  XIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(mutable_write_data)));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = xio::write_at(s, 1234, buffers,
      old_style_transfer_all);
  XIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(mutable_write_data)));

  s.reset();
  bytes_transferred = xio::write_at(s, 0, buffers, short_transfer());
  XIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(mutable_write_data)));

  s.reset();
  bytes_transferred = xio::write_at(s, 1234, buffers, short_transfer());
  XIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(mutable_write_data)));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = xio::write_at(s, 0, buffers, short_transfer());
  XIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(mutable_write_data)));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = xio::write_at(s, 1234, buffers, short_transfer());
  XIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(mutable_write_data)));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = xio::write_at(s, 0, buffers, short_transfer());
  XIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(mutable_write_data)));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = xio::write_at(s, 1234, buffers, short_transfer());
  XIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(mutable_write_data)));
}

void test_4_arg_vector_buffers_write_at()
{
  xio::io_context ioc;
  test_random_access_device s(ioc);
  std::vector<xio::const_buffer> buffers;
  buffers.push_back(xio::buffer(write_data, 32));
  buffers.push_back(xio::buffer(write_data) + 32);

  s.reset();
  size_t bytes_transferred = xio::write_at(s, 0, buffers,
      xio::transfer_all());
  XIO_CHECK(bytes_transferred == sizeof(write_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  bytes_transferred = xio::write_at(s, 1234, buffers,
      xio::transfer_all());
  XIO_CHECK(bytes_transferred == sizeof(write_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = xio::write_at(s, 0, buffers,
      xio::transfer_all());
  XIO_CHECK(bytes_transferred == sizeof(write_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = xio::write_at(s, 1234, buffers,
      xio::transfer_all());
  XIO_CHECK(bytes_transferred == sizeof(write_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = xio::write_at(s, 0, buffers,
      xio::transfer_all());
  XIO_CHECK(bytes_transferred == sizeof(write_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = xio::write_at(s, 1234, buffers,
      xio::transfer_all());
  XIO_CHECK(bytes_transferred == sizeof(write_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  bytes_transferred = xio::write_at(s, 0, buffers,
      xio::transfer_at_least(1));
  XIO_CHECK(bytes_transferred == sizeof(write_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  bytes_transferred = xio::write_at(s, 1234, buffers,
      xio::transfer_at_least(1));
  XIO_CHECK(bytes_transferred == sizeof(write_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = xio::write_at(s, 0, buffers,
      xio::transfer_at_least(1));
  XIO_CHECK(bytes_transferred == 1);
  XIO_CHECK(s.check_buffers(0, buffers, 1));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = xio::write_at(s, 1234, buffers,
      xio::transfer_at_least(1));
  XIO_CHECK(bytes_transferred == 1);
  XIO_CHECK(s.check_buffers(1234, buffers, 1));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = xio::write_at(s, 0, buffers,
      xio::transfer_at_least(1));
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = xio::write_at(s, 1234, buffers,
      xio::transfer_at_least(1));
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset();
  bytes_transferred = xio::write_at(s, 0, buffers,
      xio::transfer_at_least(10));
  XIO_CHECK(bytes_transferred == sizeof(write_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  bytes_transferred = xio::write_at(s, 1234, buffers,
      xio::transfer_at_least(10));
  XIO_CHECK(bytes_transferred == sizeof(write_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = xio::write_at(s, 0, buffers,
      xio::transfer_at_least(10));
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = xio::write_at(s, 1234, buffers,
      xio::transfer_at_least(10));
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = xio::write_at(s, 0, buffers,
      xio::transfer_at_least(10));
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = xio::write_at(s, 1234, buffers,
      xio::transfer_at_least(10));
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset();
  bytes_transferred = xio::write_at(s, 0, buffers,
      xio::transfer_at_least(42));
  XIO_CHECK(bytes_transferred == sizeof(write_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  bytes_transferred = xio::write_at(s, 1234, buffers,
      xio::transfer_at_least(42));
  XIO_CHECK(bytes_transferred == sizeof(write_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = xio::write_at(s, 0, buffers,
      xio::transfer_at_least(42));
  XIO_CHECK(bytes_transferred == 42);
  XIO_CHECK(s.check_buffers(0, buffers, 42));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = xio::write_at(s, 1234, buffers,
      xio::transfer_at_least(42));
  XIO_CHECK(bytes_transferred == 42);
  XIO_CHECK(s.check_buffers(1234, buffers, 42));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = xio::write_at(s, 0, buffers,
      xio::transfer_at_least(42));
  XIO_CHECK(bytes_transferred == 50);
  XIO_CHECK(s.check_buffers(0, buffers, 50));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = xio::write_at(s, 1234, buffers,
      xio::transfer_at_least(42));
  XIO_CHECK(bytes_transferred == 50);
  XIO_CHECK(s.check_buffers(1234, buffers, 50));

  s.reset();
  bytes_transferred = xio::write_at(s, 0, buffers,
      xio::transfer_exactly(1));
  XIO_CHECK(bytes_transferred == 1);
  XIO_CHECK(s.check_buffers(0, buffers, 1));

  s.reset();
  bytes_transferred = xio::write_at(s, 1234, buffers,
      xio::transfer_exactly(1));
  XIO_CHECK(bytes_transferred == 1);
  XIO_CHECK(s.check_buffers(1234, buffers, 1));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = xio::write_at(s, 0, buffers,
      xio::transfer_exactly(1));
  XIO_CHECK(bytes_transferred == 1);
  XIO_CHECK(s.check_buffers(0, buffers, 1));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = xio::write_at(s, 1234, buffers,
      xio::transfer_exactly(1));
  XIO_CHECK(bytes_transferred == 1);
  XIO_CHECK(s.check_buffers(1234, buffers, 1));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = xio::write_at(s, 0, buffers,
      xio::transfer_exactly(1));
  XIO_CHECK(bytes_transferred == 1);
  XIO_CHECK(s.check_buffers(0, buffers, 1));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = xio::write_at(s, 1234, buffers,
      xio::transfer_exactly(1));
  XIO_CHECK(bytes_transferred == 1);
  XIO_CHECK(s.check_buffers(1234, buffers, 1));

  s.reset();
  bytes_transferred = xio::write_at(s, 0, buffers,
      xio::transfer_exactly(10));
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset();
  bytes_transferred = xio::write_at(s, 1234, buffers,
      xio::transfer_exactly(10));
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = xio::write_at(s, 0, buffers,
      xio::transfer_exactly(10));
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = xio::write_at(s, 1234, buffers,
      xio::transfer_exactly(10));
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = xio::write_at(s, 0, buffers,
      xio::transfer_exactly(10));
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = xio::write_at(s, 1234, buffers,
      xio::transfer_exactly(10));
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset();
  bytes_transferred = xio::write_at(s, 0, buffers,
      xio::transfer_exactly(42));
  XIO_CHECK(bytes_transferred == 42);
  XIO_CHECK(s.check_buffers(0, buffers, 42));

  s.reset();
  bytes_transferred = xio::write_at(s, 1234, buffers,
      xio::transfer_exactly(42));
  XIO_CHECK(bytes_transferred == 42);
  XIO_CHECK(s.check_buffers(1234, buffers, 42));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = xio::write_at(s, 0, buffers,
      xio::transfer_exactly(42));
  XIO_CHECK(bytes_transferred == 42);
  XIO_CHECK(s.check_buffers(0, buffers, 42));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = xio::write_at(s, 1234, buffers,
      xio::transfer_exactly(42));
  XIO_CHECK(bytes_transferred == 42);
  XIO_CHECK(s.check_buffers(1234, buffers, 42));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = xio::write_at(s, 0, buffers,
      xio::transfer_exactly(42));
  XIO_CHECK(bytes_transferred == 42);
  XIO_CHECK(s.check_buffers(0, buffers, 42));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = xio::write_at(s, 1234, buffers,
      xio::transfer_exactly(42));
  XIO_CHECK(bytes_transferred == 42);
  XIO_CHECK(s.check_buffers(1234, buffers, 42));

  s.reset();
  bytes_transferred = xio::write_at(s, 0, buffers,
      old_style_transfer_all);
  XIO_CHECK(bytes_transferred == sizeof(write_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  bytes_transferred = xio::write_at(s, 1234, buffers,
      old_style_transfer_all);
  XIO_CHECK(bytes_transferred == sizeof(write_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = xio::write_at(s, 0, buffers,
      old_style_transfer_all);
  XIO_CHECK(bytes_transferred == sizeof(write_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = xio::write_at(s, 1234, buffers,
      old_style_transfer_all);
  XIO_CHECK(bytes_transferred == sizeof(write_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = xio::write_at(s, 0, buffers,
      old_style_transfer_all);
  XIO_CHECK(bytes_transferred == sizeof(write_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = xio::write_at(s, 1234, buffers,
      old_style_transfer_all);
  XIO_CHECK(bytes_transferred == sizeof(write_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  bytes_transferred = xio::write_at(s, 0, buffers, short_transfer());
  XIO_CHECK(bytes_transferred == sizeof(write_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  bytes_transferred = xio::write_at(s, 1234, buffers, short_transfer());
  XIO_CHECK(bytes_transferred == sizeof(write_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = xio::write_at(s, 0, buffers, short_transfer());
  XIO_CHECK(bytes_transferred == sizeof(write_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  bytes_transferred = xio::write_at(s, 1234, buffers, short_transfer());
  XIO_CHECK(bytes_transferred == sizeof(write_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = xio::write_at(s, 0, buffers, short_transfer());
  XIO_CHECK(bytes_transferred == sizeof(write_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  bytes_transferred = xio::write_at(s, 1234, buffers, short_transfer());
  XIO_CHECK(bytes_transferred == sizeof(write_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));
}

void test_5_arg_const_buffer_write_at()
{
  xio::io_context ioc;
  test_random_access_device s(ioc);
  xio::const_buffer buffers
    = xio::buffer(write_data, sizeof(write_data));

  s.reset();
  xio::error_code error;
  size_t bytes_transferred = xio::write_at(s, 0, buffers,
      xio::transfer_all(), error);
  XIO_CHECK(bytes_transferred == sizeof(write_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));
  XIO_CHECK(!error);

  s.reset();
  bytes_transferred = xio::write_at(s, 1234, buffers,
      xio::transfer_all(), error);
  XIO_CHECK(bytes_transferred == sizeof(write_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));
  XIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 0, buffers,
      xio::transfer_all(), error);
  XIO_CHECK(bytes_transferred == sizeof(write_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));
  XIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 1234, buffers,
      xio::transfer_all(), error);
  XIO_CHECK(bytes_transferred == sizeof(write_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));
  XIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 0, buffers,
      xio::transfer_all(), error);
  XIO_CHECK(bytes_transferred == sizeof(write_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));
  XIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 1234, buffers,
      xio::transfer_all(), error);
  XIO_CHECK(bytes_transferred == sizeof(write_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));
  XIO_CHECK(!error);

  s.reset();
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 0, buffers,
      xio::transfer_at_least(1), error);
  XIO_CHECK(bytes_transferred == sizeof(write_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));
  XIO_CHECK(!error);

  s.reset();
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 1234, buffers,
      xio::transfer_at_least(1), error);
  XIO_CHECK(bytes_transferred == sizeof(write_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));
  XIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 0, buffers,
      xio::transfer_at_least(1), error);
  XIO_CHECK(bytes_transferred == 1);
  XIO_CHECK(s.check_buffers(0, buffers, 1));
  XIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 1234, buffers,
      xio::transfer_at_least(1), error);
  XIO_CHECK(bytes_transferred == 1);
  XIO_CHECK(s.check_buffers(1234, buffers, 1));
  XIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 0, buffers,
      xio::transfer_at_least(1), error);
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(s.check_buffers(0, buffers, 10));
  XIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 1234, buffers,
      xio::transfer_at_least(1), error);
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(s.check_buffers(1234, buffers, 10));
  XIO_CHECK(!error);

  s.reset();
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 0, buffers,
      xio::transfer_at_least(10), error);
  XIO_CHECK(bytes_transferred == sizeof(write_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));
  XIO_CHECK(!error);

  s.reset();
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 1234, buffers,
      xio::transfer_at_least(10), error);
  XIO_CHECK(bytes_transferred == sizeof(write_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));
  XIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 0, buffers,
      xio::transfer_at_least(10), error);
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(s.check_buffers(0, buffers, 10));
  XIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 1234, buffers,
      xio::transfer_at_least(10), error);
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(s.check_buffers(1234, buffers, 10));
  XIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 0, buffers,
      xio::transfer_at_least(10), error);
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(s.check_buffers(0, buffers, 10));
  XIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 1234, buffers,
      xio::transfer_at_least(10), error);
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(s.check_buffers(1234, buffers, 10));
  XIO_CHECK(!error);

  s.reset();
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 0, buffers,
      xio::transfer_at_least(42), error);
  XIO_CHECK(bytes_transferred == sizeof(write_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));
  XIO_CHECK(!error);

  s.reset();
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 1234, buffers,
      xio::transfer_at_least(42), error);
  XIO_CHECK(bytes_transferred == sizeof(write_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));
  XIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 0, buffers,
      xio::transfer_at_least(42), error);
  XIO_CHECK(bytes_transferred == 42);
  XIO_CHECK(s.check_buffers(0, buffers, 42));
  XIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 1234, buffers,
      xio::transfer_at_least(42), error);
  XIO_CHECK(bytes_transferred == 42);
  XIO_CHECK(s.check_buffers(1234, buffers, 42));
  XIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 0, buffers,
      xio::transfer_at_least(42), error);
  XIO_CHECK(bytes_transferred == 50);
  XIO_CHECK(s.check_buffers(0, buffers, 50));
  XIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 1234, buffers,
      xio::transfer_at_least(42), error);
  XIO_CHECK(bytes_transferred == 50);
  XIO_CHECK(s.check_buffers(1234, buffers, 50));
  XIO_CHECK(!error);

  s.reset();
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 0, buffers,
      xio::transfer_exactly(1), error);
  XIO_CHECK(bytes_transferred == 1);
  XIO_CHECK(s.check_buffers(0, buffers, 1));
  XIO_CHECK(!error);

  s.reset();
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 1234, buffers,
      xio::transfer_exactly(1), error);
  XIO_CHECK(bytes_transferred == 1);
  XIO_CHECK(s.check_buffers(1234, buffers, 1));
  XIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 0, buffers,
      xio::transfer_exactly(1), error);
  XIO_CHECK(bytes_transferred == 1);
  XIO_CHECK(s.check_buffers(0, buffers, 1));
  XIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 1234, buffers,
      xio::transfer_exactly(1), error);
  XIO_CHECK(bytes_transferred == 1);
  XIO_CHECK(s.check_buffers(1234, buffers, 1));
  XIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 0, buffers,
      xio::transfer_exactly(1), error);
  XIO_CHECK(bytes_transferred == 1);
  XIO_CHECK(s.check_buffers(0, buffers, 1));
  XIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 1234, buffers,
      xio::transfer_exactly(1), error);
  XIO_CHECK(bytes_transferred == 1);
  XIO_CHECK(s.check_buffers(1234, buffers, 1));
  XIO_CHECK(!error);

  s.reset();
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 0, buffers,
      xio::transfer_exactly(10), error);
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(s.check_buffers(0, buffers, 10));
  XIO_CHECK(!error);

  s.reset();
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 1234, buffers,
      xio::transfer_exactly(10), error);
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(s.check_buffers(1234, buffers, 10));
  XIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 0, buffers,
      xio::transfer_exactly(10), error);
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(s.check_buffers(0, buffers, 10));
  XIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 1234, buffers,
      xio::transfer_exactly(10), error);
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(s.check_buffers(1234, buffers, 10));
  XIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 0, buffers,
      xio::transfer_exactly(10), error);
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(s.check_buffers(0, buffers, 10));
  XIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 1234, buffers,
      xio::transfer_exactly(10), error);
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(s.check_buffers(1234, buffers, 10));
  XIO_CHECK(!error);

  s.reset();
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 0, buffers,
      xio::transfer_exactly(42), error);
  XIO_CHECK(bytes_transferred == 42);
  XIO_CHECK(s.check_buffers(0, buffers, 42));
  XIO_CHECK(!error);

  s.reset();
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 1234, buffers,
      xio::transfer_exactly(42), error);
  XIO_CHECK(bytes_transferred == 42);
  XIO_CHECK(s.check_buffers(1234, buffers, 42));
  XIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 0, buffers,
      xio::transfer_exactly(42), error);
  XIO_CHECK(bytes_transferred == 42);
  XIO_CHECK(s.check_buffers(0, buffers, 42));
  XIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 1234, buffers,
      xio::transfer_exactly(42), error);
  XIO_CHECK(bytes_transferred == 42);
  XIO_CHECK(s.check_buffers(1234, buffers, 42));
  XIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 0, buffers,
      xio::transfer_exactly(42), error);
  XIO_CHECK(bytes_transferred == 42);
  XIO_CHECK(s.check_buffers(0, buffers, 42));
  XIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 1234, buffers,
      xio::transfer_exactly(42), error);
  XIO_CHECK(bytes_transferred == 42);
  XIO_CHECK(s.check_buffers(1234, buffers, 42));
  XIO_CHECK(!error);

  s.reset();
  bytes_transferred = xio::write_at(s, 0, buffers,
      old_style_transfer_all, error);
  XIO_CHECK(bytes_transferred == sizeof(write_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));
  XIO_CHECK(!error);

  s.reset();
  bytes_transferred = xio::write_at(s, 1234, buffers,
      old_style_transfer_all, error);
  XIO_CHECK(bytes_transferred == sizeof(write_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));
  XIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 0, buffers,
      old_style_transfer_all, error);
  XIO_CHECK(bytes_transferred == sizeof(write_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));
  XIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 1234, buffers,
      old_style_transfer_all, error);
  XIO_CHECK(bytes_transferred == sizeof(write_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));
  XIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 0, buffers,
      old_style_transfer_all, error);
  XIO_CHECK(bytes_transferred == sizeof(write_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));
  XIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 1234, buffers,
      old_style_transfer_all, error);
  XIO_CHECK(bytes_transferred == sizeof(write_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));
  XIO_CHECK(!error);

  s.reset();
  bytes_transferred = xio::write_at(s, 0, buffers,
      short_transfer(), error);
  XIO_CHECK(bytes_transferred == sizeof(write_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));
  XIO_CHECK(!error);

  s.reset();
  bytes_transferred = xio::write_at(s, 1234, buffers,
      short_transfer(), error);
  XIO_CHECK(bytes_transferred == sizeof(write_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));
  XIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 0, buffers,
      short_transfer(), error);
  XIO_CHECK(bytes_transferred == sizeof(write_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));
  XIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 1234, buffers,
      short_transfer(), error);
  XIO_CHECK(bytes_transferred == sizeof(write_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));
  XIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 0, buffers,
      short_transfer(), error);
  XIO_CHECK(bytes_transferred == sizeof(write_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));
  XIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 1234, buffers,
      short_transfer(), error);
  XIO_CHECK(bytes_transferred == sizeof(write_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));
  XIO_CHECK(!error);
}

void test_5_arg_mutable_buffer_write_at()
{
  xio::io_context ioc;
  test_random_access_device s(ioc);
  xio::mutable_buffer buffers
    = xio::buffer(mutable_write_data, sizeof(mutable_write_data));

  s.reset();
  xio::error_code error;
  size_t bytes_transferred = xio::write_at(s, 0, buffers,
      xio::transfer_all(), error);
  XIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(mutable_write_data)));
  XIO_CHECK(!error);

  s.reset();
  bytes_transferred = xio::write_at(s, 1234, buffers,
      xio::transfer_all(), error);
  XIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(mutable_write_data)));
  XIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 0, buffers,
      xio::transfer_all(), error);
  XIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(mutable_write_data)));
  XIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 1234, buffers,
      xio::transfer_all(), error);
  XIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(mutable_write_data)));
  XIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 0, buffers,
      xio::transfer_all(), error);
  XIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(mutable_write_data)));
  XIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 1234, buffers,
      xio::transfer_all(), error);
  XIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(mutable_write_data)));
  XIO_CHECK(!error);

  s.reset();
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 0, buffers,
      xio::transfer_at_least(1), error);
  XIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(mutable_write_data)));
  XIO_CHECK(!error);

  s.reset();
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 1234, buffers,
      xio::transfer_at_least(1), error);
  XIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(mutable_write_data)));
  XIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 0, buffers,
      xio::transfer_at_least(1), error);
  XIO_CHECK(bytes_transferred == 1);
  XIO_CHECK(s.check_buffers(0, buffers, 1));
  XIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 1234, buffers,
      xio::transfer_at_least(1), error);
  XIO_CHECK(bytes_transferred == 1);
  XIO_CHECK(s.check_buffers(1234, buffers, 1));
  XIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 0, buffers,
      xio::transfer_at_least(1), error);
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(s.check_buffers(0, buffers, 10));
  XIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 1234, buffers,
      xio::transfer_at_least(1), error);
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(s.check_buffers(1234, buffers, 10));
  XIO_CHECK(!error);

  s.reset();
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 0, buffers,
      xio::transfer_at_least(10), error);
  XIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(mutable_write_data)));
  XIO_CHECK(!error);

  s.reset();
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 1234, buffers,
      xio::transfer_at_least(10), error);
  XIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(mutable_write_data)));
  XIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 0, buffers,
      xio::transfer_at_least(10), error);
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(s.check_buffers(0, buffers, 10));
  XIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 1234, buffers,
      xio::transfer_at_least(10), error);
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(s.check_buffers(1234, buffers, 10));
  XIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 0, buffers,
      xio::transfer_at_least(10), error);
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(s.check_buffers(0, buffers, 10));
  XIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 1234, buffers,
      xio::transfer_at_least(10), error);
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(s.check_buffers(1234, buffers, 10));
  XIO_CHECK(!error);

  s.reset();
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 0, buffers,
      xio::transfer_at_least(42), error);
  XIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(mutable_write_data)));
  XIO_CHECK(!error);

  s.reset();
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 1234, buffers,
      xio::transfer_at_least(42), error);
  XIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(mutable_write_data)));
  XIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 0, buffers,
      xio::transfer_at_least(42), error);
  XIO_CHECK(bytes_transferred == 42);
  XIO_CHECK(s.check_buffers(0, buffers, 42));
  XIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 1234, buffers,
      xio::transfer_at_least(42), error);
  XIO_CHECK(bytes_transferred == 42);
  XIO_CHECK(s.check_buffers(1234, buffers, 42));
  XIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 0, buffers,
      xio::transfer_at_least(42), error);
  XIO_CHECK(bytes_transferred == 50);
  XIO_CHECK(s.check_buffers(0, buffers, 50));
  XIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 1234, buffers,
      xio::transfer_at_least(42), error);
  XIO_CHECK(bytes_transferred == 50);
  XIO_CHECK(s.check_buffers(1234, buffers, 50));
  XIO_CHECK(!error);

  s.reset();
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 0, buffers,
      xio::transfer_exactly(1), error);
  XIO_CHECK(bytes_transferred == 1);
  XIO_CHECK(s.check_buffers(0, buffers, 1));
  XIO_CHECK(!error);

  s.reset();
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 1234, buffers,
      xio::transfer_exactly(1), error);
  XIO_CHECK(bytes_transferred == 1);
  XIO_CHECK(s.check_buffers(1234, buffers, 1));
  XIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 0, buffers,
      xio::transfer_exactly(1), error);
  XIO_CHECK(bytes_transferred == 1);
  XIO_CHECK(s.check_buffers(0, buffers, 1));
  XIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 1234, buffers,
      xio::transfer_exactly(1), error);
  XIO_CHECK(bytes_transferred == 1);
  XIO_CHECK(s.check_buffers(1234, buffers, 1));
  XIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 0, buffers,
      xio::transfer_exactly(1), error);
  XIO_CHECK(bytes_transferred == 1);
  XIO_CHECK(s.check_buffers(0, buffers, 1));
  XIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 1234, buffers,
      xio::transfer_exactly(1), error);
  XIO_CHECK(bytes_transferred == 1);
  XIO_CHECK(s.check_buffers(1234, buffers, 1));
  XIO_CHECK(!error);

  s.reset();
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 0, buffers,
      xio::transfer_exactly(10), error);
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(s.check_buffers(0, buffers, 10));
  XIO_CHECK(!error);

  s.reset();
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 1234, buffers,
      xio::transfer_exactly(10), error);
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(s.check_buffers(1234, buffers, 10));
  XIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 0, buffers,
      xio::transfer_exactly(10), error);
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(s.check_buffers(0, buffers, 10));
  XIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 1234, buffers,
      xio::transfer_exactly(10), error);
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(s.check_buffers(1234, buffers, 10));
  XIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 0, buffers,
      xio::transfer_exactly(10), error);
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(s.check_buffers(0, buffers, 10));
  XIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 1234, buffers,
      xio::transfer_exactly(10), error);
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(s.check_buffers(1234, buffers, 10));
  XIO_CHECK(!error);

  s.reset();
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 0, buffers,
      xio::transfer_exactly(42), error);
  XIO_CHECK(bytes_transferred == 42);
  XIO_CHECK(s.check_buffers(0, buffers, 42));
  XIO_CHECK(!error);

  s.reset();
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 1234, buffers,
      xio::transfer_exactly(42), error);
  XIO_CHECK(bytes_transferred == 42);
  XIO_CHECK(s.check_buffers(1234, buffers, 42));
  XIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 0, buffers,
      xio::transfer_exactly(42), error);
  XIO_CHECK(bytes_transferred == 42);
  XIO_CHECK(s.check_buffers(0, buffers, 42));
  XIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 1234, buffers,
      xio::transfer_exactly(42), error);
  XIO_CHECK(bytes_transferred == 42);
  XIO_CHECK(s.check_buffers(1234, buffers, 42));
  XIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 0, buffers,
      xio::transfer_exactly(42), error);
  XIO_CHECK(bytes_transferred == 42);
  XIO_CHECK(s.check_buffers(0, buffers, 42));
  XIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 1234, buffers,
      xio::transfer_exactly(42), error);
  XIO_CHECK(bytes_transferred == 42);
  XIO_CHECK(s.check_buffers(1234, buffers, 42));
  XIO_CHECK(!error);

  s.reset();
  bytes_transferred = xio::write_at(s, 0, buffers,
      old_style_transfer_all, error);
  XIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(mutable_write_data)));
  XIO_CHECK(!error);

  s.reset();
  bytes_transferred = xio::write_at(s, 1234, buffers,
      old_style_transfer_all, error);
  XIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(mutable_write_data)));
  XIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 0, buffers,
      old_style_transfer_all, error);
  XIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(mutable_write_data)));
  XIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 1234, buffers,
      old_style_transfer_all, error);
  XIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(mutable_write_data)));
  XIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 0, buffers,
      old_style_transfer_all, error);
  XIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(mutable_write_data)));
  XIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 1234, buffers,
      old_style_transfer_all, error);
  XIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(mutable_write_data)));
  XIO_CHECK(!error);

  s.reset();
  bytes_transferred = xio::write_at(s, 0, buffers,
      short_transfer(), error);
  XIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(mutable_write_data)));
  XIO_CHECK(!error);

  s.reset();
  bytes_transferred = xio::write_at(s, 1234, buffers,
      short_transfer(), error);
  XIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(mutable_write_data)));
  XIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 0, buffers,
      short_transfer(), error);
  XIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(mutable_write_data)));
  XIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 1234, buffers,
      short_transfer(), error);
  XIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(mutable_write_data)));
  XIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 0, buffers,
      short_transfer(), error);
  XIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(mutable_write_data)));
  XIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 1234, buffers,
      short_transfer(), error);
  XIO_CHECK(bytes_transferred == sizeof(mutable_write_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(mutable_write_data)));
  XIO_CHECK(!error);
}

void test_5_arg_vector_buffers_write_at()
{
  xio::io_context ioc;
  test_random_access_device s(ioc);
  std::vector<xio::const_buffer> buffers;
  buffers.push_back(xio::buffer(write_data, 32));
  buffers.push_back(xio::buffer(write_data) + 32);

  s.reset();
  xio::error_code error;
  size_t bytes_transferred = xio::write_at(s, 0, buffers,
      xio::transfer_all(), error);
  XIO_CHECK(bytes_transferred == sizeof(write_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));
  XIO_CHECK(!error);

  s.reset();
  bytes_transferred = xio::write_at(s, 1234, buffers,
      xio::transfer_all(), error);
  XIO_CHECK(bytes_transferred == sizeof(write_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));
  XIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 0, buffers,
      xio::transfer_all(), error);
  XIO_CHECK(bytes_transferred == sizeof(write_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));
  XIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 1234, buffers,
      xio::transfer_all(), error);
  XIO_CHECK(bytes_transferred == sizeof(write_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));
  XIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 0, buffers,
      xio::transfer_all(), error);
  XIO_CHECK(bytes_transferred == sizeof(write_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));
  XIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 1234, buffers,
      xio::transfer_all(), error);
  XIO_CHECK(bytes_transferred == sizeof(write_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));
  XIO_CHECK(!error);

  s.reset();
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 0, buffers,
      xio::transfer_at_least(1), error);
  XIO_CHECK(bytes_transferred == sizeof(write_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));
  XIO_CHECK(!error);

  s.reset();
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 1234, buffers,
      xio::transfer_at_least(1), error);
  XIO_CHECK(bytes_transferred == sizeof(write_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));
  XIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 0, buffers,
      xio::transfer_at_least(1), error);
  XIO_CHECK(bytes_transferred == 1);
  XIO_CHECK(s.check_buffers(0, buffers, 1));
  XIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 1234, buffers,
      xio::transfer_at_least(1), error);
  XIO_CHECK(bytes_transferred == 1);
  XIO_CHECK(s.check_buffers(1234, buffers, 1));
  XIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 0, buffers,
      xio::transfer_at_least(1), error);
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(s.check_buffers(0, buffers, 10));
  XIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 1234, buffers,
      xio::transfer_at_least(1), error);
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(s.check_buffers(1234, buffers, 10));
  XIO_CHECK(!error);

  s.reset();
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 0, buffers,
      xio::transfer_at_least(10), error);
  XIO_CHECK(bytes_transferred == sizeof(write_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));
  XIO_CHECK(!error);

  s.reset();
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 1234, buffers,
      xio::transfer_at_least(10), error);
  XIO_CHECK(bytes_transferred == sizeof(write_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));
  XIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 0, buffers,
      xio::transfer_at_least(10), error);
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(s.check_buffers(0, buffers, 10));
  XIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 1234, buffers,
      xio::transfer_at_least(10), error);
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(s.check_buffers(1234, buffers, 10));
  XIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 0, buffers,
      xio::transfer_at_least(10), error);
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(s.check_buffers(0, buffers, 10));
  XIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 1234, buffers,
      xio::transfer_at_least(10), error);
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(s.check_buffers(1234, buffers, 10));
  XIO_CHECK(!error);

  s.reset();
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 0, buffers,
      xio::transfer_at_least(42), error);
  XIO_CHECK(bytes_transferred == sizeof(write_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));
  XIO_CHECK(!error);

  s.reset();
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 1234, buffers,
      xio::transfer_at_least(42), error);
  XIO_CHECK(bytes_transferred == sizeof(write_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));
  XIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 0, buffers,
      xio::transfer_at_least(42), error);
  XIO_CHECK(bytes_transferred == 42);
  XIO_CHECK(s.check_buffers(0, buffers, 42));
  XIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 1234, buffers,
      xio::transfer_at_least(42), error);
  XIO_CHECK(bytes_transferred == 42);
  XIO_CHECK(s.check_buffers(1234, buffers, 42));
  XIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 0, buffers,
      xio::transfer_at_least(42), error);
  XIO_CHECK(bytes_transferred == 50);
  XIO_CHECK(s.check_buffers(0, buffers, 50));
  XIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 1234, buffers,
      xio::transfer_at_least(42), error);
  XIO_CHECK(bytes_transferred == 50);
  XIO_CHECK(s.check_buffers(1234, buffers, 50));
  XIO_CHECK(!error);

  s.reset();
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 0, buffers,
      xio::transfer_exactly(1), error);
  XIO_CHECK(bytes_transferred == 1);
  XIO_CHECK(s.check_buffers(0, buffers, 1));
  XIO_CHECK(!error);

  s.reset();
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 1234, buffers,
      xio::transfer_exactly(1), error);
  XIO_CHECK(bytes_transferred == 1);
  XIO_CHECK(s.check_buffers(1234, buffers, 1));
  XIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 0, buffers,
      xio::transfer_exactly(1), error);
  XIO_CHECK(bytes_transferred == 1);
  XIO_CHECK(s.check_buffers(0, buffers, 1));
  XIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 1234, buffers,
      xio::transfer_exactly(1), error);
  XIO_CHECK(bytes_transferred == 1);
  XIO_CHECK(s.check_buffers(1234, buffers, 1));
  XIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 0, buffers,
      xio::transfer_exactly(1), error);
  XIO_CHECK(bytes_transferred == 1);
  XIO_CHECK(s.check_buffers(0, buffers, 1));
  XIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 1234, buffers,
      xio::transfer_exactly(1), error);
  XIO_CHECK(bytes_transferred == 1);
  XIO_CHECK(s.check_buffers(1234, buffers, 1));
  XIO_CHECK(!error);

  s.reset();
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 0, buffers,
      xio::transfer_exactly(10), error);
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(s.check_buffers(0, buffers, 10));
  XIO_CHECK(!error);

  s.reset();
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 1234, buffers,
      xio::transfer_exactly(10), error);
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(s.check_buffers(1234, buffers, 10));
  XIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 0, buffers,
      xio::transfer_exactly(10), error);
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(s.check_buffers(0, buffers, 10));
  XIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 1234, buffers,
      xio::transfer_exactly(10), error);
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(s.check_buffers(1234, buffers, 10));
  XIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 0, buffers,
      xio::transfer_exactly(10), error);
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(s.check_buffers(0, buffers, 10));
  XIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 1234, buffers,
      xio::transfer_exactly(10), error);
  XIO_CHECK(bytes_transferred == 10);
  XIO_CHECK(s.check_buffers(1234, buffers, 10));
  XIO_CHECK(!error);

  s.reset();
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 0, buffers,
      xio::transfer_exactly(42), error);
  XIO_CHECK(bytes_transferred == 42);
  XIO_CHECK(s.check_buffers(0, buffers, 42));
  XIO_CHECK(!error);

  s.reset();
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 1234, buffers,
      xio::transfer_exactly(42), error);
  XIO_CHECK(bytes_transferred == 42);
  XIO_CHECK(s.check_buffers(1234, buffers, 42));
  XIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 0, buffers,
      xio::transfer_exactly(42), error);
  XIO_CHECK(bytes_transferred == 42);
  XIO_CHECK(s.check_buffers(0, buffers, 42));
  XIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 1234, buffers,
      xio::transfer_exactly(42), error);
  XIO_CHECK(bytes_transferred == 42);
  XIO_CHECK(s.check_buffers(1234, buffers, 42));
  XIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 0, buffers,
      xio::transfer_exactly(42), error);
  XIO_CHECK(bytes_transferred == 42);
  XIO_CHECK(s.check_buffers(0, buffers, 42));
  XIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 1234, buffers,
      xio::transfer_exactly(42), error);
  XIO_CHECK(bytes_transferred == 42);
  XIO_CHECK(s.check_buffers(1234, buffers, 42));
  XIO_CHECK(!error);

  s.reset();
  bytes_transferred = xio::write_at(s, 0, buffers,
      old_style_transfer_all, error);
  XIO_CHECK(bytes_transferred == sizeof(write_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));
  XIO_CHECK(!error);

  s.reset();
  bytes_transferred = xio::write_at(s, 1234, buffers,
      old_style_transfer_all, error);
  XIO_CHECK(bytes_transferred == sizeof(write_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));
  XIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 0, buffers,
      old_style_transfer_all, error);
  XIO_CHECK(bytes_transferred == sizeof(write_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));
  XIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 1234, buffers,
      old_style_transfer_all, error);
  XIO_CHECK(bytes_transferred == sizeof(write_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));
  XIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 0, buffers,
      old_style_transfer_all, error);
  XIO_CHECK(bytes_transferred == sizeof(write_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));
  XIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 1234, buffers,
      old_style_transfer_all, error);
  XIO_CHECK(bytes_transferred == sizeof(write_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));
  XIO_CHECK(!error);

  s.reset();
  bytes_transferred = xio::write_at(s, 0, buffers,
      short_transfer(), error);
  XIO_CHECK(bytes_transferred == sizeof(write_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));
  XIO_CHECK(!error);

  s.reset();
  bytes_transferred = xio::write_at(s, 1234, buffers,
      short_transfer(), error);
  XIO_CHECK(bytes_transferred == sizeof(write_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));
  XIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 0, buffers,
      short_transfer(), error);
  XIO_CHECK(bytes_transferred == sizeof(write_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));
  XIO_CHECK(!error);

  s.reset();
  s.next_write_length(1);
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 1234, buffers,
      short_transfer(), error);
  XIO_CHECK(bytes_transferred == sizeof(write_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));
  XIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 0, buffers,
      short_transfer(), error);
  XIO_CHECK(bytes_transferred == sizeof(write_data));
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));
  XIO_CHECK(!error);

  s.reset();
  s.next_write_length(10);
  error = xio::error_code();
  bytes_transferred = xio::write_at(s, 1234, buffers,
      short_transfer(), error);
  XIO_CHECK(bytes_transferred == sizeof(write_data));
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));
  XIO_CHECK(!error);
}

void async_write_handler(const xio::error_code& e,
    size_t bytes_transferred, size_t expected_bytes_transferred, bool* called)
{
  *called = true;
  XIO_CHECK(!e);
  XIO_CHECK(bytes_transferred == expected_bytes_transferred);
}

void test_4_arg_const_buffer_async_write_at()
{
  namespace bindns = std;
  using bindns::placeholders::_1;
  using bindns::placeholders::_2;

  xio::io_context ioc;
  test_random_access_device s(ioc);
  xio::const_buffer buffers
    = xio::buffer(write_data, sizeof(write_data));

  s.reset();
  bool called = false;
  xio::async_write_at(s, 0, buffers,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  called = false;
  xio::async_write_at(s, 1234, buffers,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write_at(s, 0, buffers,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write_at(s, 1234, buffers,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write_at(s, 0, buffers,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write_at(s, 1234, buffers,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write_at(s, 0, buffers,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  int i = xio::async_write_at(s, 0, buffers,
      archetypes::lazy_handler());
  XIO_CHECK(i == 42);
  ioc.restart();
  ioc.run();
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write_at(s, 0, buffers)(
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));
}

void test_4_arg_mutable_buffer_async_write_at()
{
  namespace bindns = std;
  using bindns::placeholders::_1;
  using bindns::placeholders::_2;

  xio::io_context ioc;
  test_random_access_device s(ioc);
  xio::mutable_buffer buffers
    = xio::buffer(mutable_write_data, sizeof(mutable_write_data));

  s.reset();
  bool called = false;
  xio::async_write_at(s, 0, buffers,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(mutable_write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(mutable_write_data)));

  s.reset();
  called = false;
  xio::async_write_at(s, 1234, buffers,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(mutable_write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(mutable_write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write_at(s, 0, buffers,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(mutable_write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(mutable_write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write_at(s, 1234, buffers,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(mutable_write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(mutable_write_data)));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write_at(s, 0, buffers,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(mutable_write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(mutable_write_data)));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write_at(s, 1234, buffers,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(mutable_write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(mutable_write_data)));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write_at(s, 0, buffers,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(mutable_write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(mutable_write_data)));

  s.reset();
  int i = xio::async_write_at(s, 0, buffers,
      archetypes::lazy_handler());
  XIO_CHECK(i == 42);
  ioc.restart();
  ioc.run();
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write_at(s, 0, buffers)(
      bindns::bind(async_write_handler,
        _1, _2, sizeof(mutable_write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(mutable_write_data)));
}

void test_4_arg_boost_array_buffers_async_write_at()
{
  namespace bindns = std;
  using bindns::placeholders::_1;
  using bindns::placeholders::_2;

}

void test_4_arg_std_array_buffers_async_write_at()
{
  namespace bindns = std;
  using bindns::placeholders::_1;
  using bindns::placeholders::_2;

  xio::io_context ioc;
  test_random_access_device s(ioc);
  std::array<xio::const_buffer, 2> buffers = { {
    xio::buffer(write_data, 32),
    xio::buffer(write_data) + 32 } };

  s.reset();
  bool called = false;
  xio::async_write_at(s, 0, buffers,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  called = false;
  xio::async_write_at(s, 1234, buffers,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write_at(s, 0, buffers,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write_at(s, 1234, buffers,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write_at(s, 0, buffers,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write_at(s, 1234, buffers,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write_at(s, 0, buffers,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  int i = xio::async_write_at(s, 0, buffers,
      archetypes::lazy_handler());
  XIO_CHECK(i == 42);
  ioc.restart();
  ioc.run();
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write_at(s, 0, buffers)(
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));
}

void test_4_arg_vector_buffers_async_write_at()
{
  namespace bindns = std;
  using bindns::placeholders::_1;
  using bindns::placeholders::_2;

  xio::io_context ioc;
  test_random_access_device s(ioc);
  std::vector<xio::const_buffer> buffers;
  buffers.push_back(xio::buffer(write_data, 32));
  buffers.push_back(xio::buffer(write_data) + 32);

  s.reset();
  bool called = false;
  xio::async_write_at(s, 0, buffers,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  called = false;
  xio::async_write_at(s, 1234, buffers,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write_at(s, 0, buffers,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write_at(s, 1234, buffers,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write_at(s, 0, buffers,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write_at(s, 1234, buffers,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write_at(s, 0, buffers,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  int i = xio::async_write_at(s, 0, buffers,
      archetypes::lazy_handler());
  XIO_CHECK(i == 42);
  ioc.restart();
  ioc.run();
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write_at(s, 0, buffers)(
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));
}

void test_4_arg_streambuf_async_write_at()
{
  namespace bindns = std;
  using bindns::placeholders::_1;
  using bindns::placeholders::_2;

  xio::io_context ioc;
  test_random_access_device s(ioc);
  xio::streambuf sb;
  xio::const_buffer buffers
    = xio::buffer(write_data, sizeof(write_data));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  bool called = false;
  xio::async_write_at(s, 0, sb,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  called = false;
  xio::async_write_at(s, 1234, sb,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  s.next_write_length(1);
  called = false;
  xio::async_write_at(s, 0, sb,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  s.next_write_length(1);
  called = false;
  xio::async_write_at(s, 1234, sb,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  s.next_write_length(10);
  called = false;
  xio::async_write_at(s, 0, sb,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  s.next_write_length(10);
  called = false;
  xio::async_write_at(s, 1234, sb,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  s.next_write_length(10);
  called = false;
  xio::async_write_at(s, 0, sb,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  int i = xio::async_write_at(s, 0, sb,
      archetypes::lazy_handler());
  XIO_CHECK(i == 42);
  ioc.restart();
  ioc.run();
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  s.next_write_length(10);
  called = false;
  xio::async_write_at(s, 0, sb)(
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));
}

void test_5_arg_const_buffer_async_write_at()
{
  namespace bindns = std;
  using bindns::placeholders::_1;
  using bindns::placeholders::_2;

  xio::io_context ioc;
  test_random_access_device s(ioc);
  xio::const_buffer buffers
    = xio::buffer(write_data, sizeof(write_data));

  s.reset();
  bool called = false;
  xio::async_write_at(s, 0, buffers,
      xio::transfer_all(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  called = false;
  xio::async_write_at(s, 1234, buffers,
      xio::transfer_all(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write_at(s, 0, buffers,
      xio::transfer_all(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write_at(s, 1234, buffers,
      xio::transfer_all(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write_at(s, 0, buffers,
      xio::transfer_all(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write_at(s, 1234, buffers,
      xio::transfer_all(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  called = false;
  xio::async_write_at(s, 0, buffers,
      xio::transfer_at_least(1),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  called = false;
  xio::async_write_at(s, 1234, buffers,
      xio::transfer_at_least(1),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write_at(s, 0, buffers,
      xio::transfer_at_least(1),
      bindns::bind(async_write_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, 1));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write_at(s, 1234, buffers,
      xio::transfer_at_least(1),
      bindns::bind(async_write_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, 1));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write_at(s, 0, buffers,
      xio::transfer_at_least(1),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write_at(s, 1234, buffers,
      xio::transfer_at_least(1),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset();
  called = false;
  xio::async_write_at(s, 0, buffers,
      xio::transfer_at_least(10),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  called = false;
  xio::async_write_at(s, 1234, buffers,
      xio::transfer_at_least(10),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write_at(s, 0, buffers,
      xio::transfer_at_least(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write_at(s, 1234, buffers,
      xio::transfer_at_least(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write_at(s, 0, buffers,
      xio::transfer_at_least(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write_at(s, 1234, buffers,
      xio::transfer_at_least(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset();
  called = false;
  xio::async_write_at(s, 0, buffers,
      xio::transfer_at_least(42),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  called = false;
  xio::async_write_at(s, 1234, buffers,
      xio::transfer_at_least(42),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write_at(s, 0, buffers,
      xio::transfer_at_least(42),
      bindns::bind(async_write_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, 42));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write_at(s, 1234, buffers,
      xio::transfer_at_least(42),
      bindns::bind(async_write_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, 42));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write_at(s, 0, buffers,
      xio::transfer_at_least(42),
      bindns::bind(async_write_handler,
        _1, _2, 50, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, 50));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write_at(s, 1234, buffers,
      xio::transfer_at_least(42),
      bindns::bind(async_write_handler,
        _1, _2, 50, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, 50));

  s.reset();
  called = false;
  xio::async_write_at(s, 0, buffers,
      xio::transfer_exactly(1),
      bindns::bind(async_write_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, 1));

  s.reset();
  called = false;
  xio::async_write_at(s, 1234, buffers,
      xio::transfer_exactly(1),
      bindns::bind(async_write_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, 1));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write_at(s, 0, buffers,
      xio::transfer_exactly(1),
      bindns::bind(async_write_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, 1));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write_at(s, 1234, buffers,
      xio::transfer_exactly(1),
      bindns::bind(async_write_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, 1));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write_at(s, 0, buffers,
      xio::transfer_exactly(1),
      bindns::bind(async_write_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, 1));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write_at(s, 1234, buffers,
      xio::transfer_exactly(1),
      bindns::bind(async_write_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, 1));

  s.reset();
  called = false;
  xio::async_write_at(s, 0, buffers,
      xio::transfer_exactly(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset();
  called = false;
  xio::async_write_at(s, 1234, buffers,
      xio::transfer_exactly(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write_at(s, 0, buffers,
      xio::transfer_exactly(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write_at(s, 1234, buffers,
      xio::transfer_exactly(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write_at(s, 0, buffers,
      xio::transfer_exactly(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write_at(s, 1234, buffers,
      xio::transfer_exactly(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset();
  called = false;
  xio::async_write_at(s, 0, buffers,
      xio::transfer_exactly(42),
      bindns::bind(async_write_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, 42));

  s.reset();
  called = false;
  xio::async_write_at(s, 1234, buffers,
      xio::transfer_exactly(42),
      bindns::bind(async_write_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, 42));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write_at(s, 0, buffers,
      xio::transfer_exactly(42),
      bindns::bind(async_write_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, 42));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write_at(s, 1234, buffers,
      xio::transfer_exactly(42),
      bindns::bind(async_write_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, 42));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write_at(s, 0, buffers,
      xio::transfer_exactly(42),
      bindns::bind(async_write_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, 42));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write_at(s, 1234, buffers,
      xio::transfer_exactly(42),
      bindns::bind(async_write_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, 42));

  s.reset();
  called = false;
  xio::async_write_at(s, 0, buffers, old_style_transfer_all,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  called = false;
  xio::async_write_at(s, 1234, buffers, old_style_transfer_all,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write_at(s, 0, buffers, old_style_transfer_all,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write_at(s, 1234, buffers, old_style_transfer_all,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write_at(s, 0, buffers, old_style_transfer_all,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write_at(s, 1234, buffers, old_style_transfer_all,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  called = false;
  xio::async_write_at(s, 0, buffers, short_transfer(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  called = false;
  xio::async_write_at(s, 1234, buffers, short_transfer(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write_at(s, 0, buffers, short_transfer(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write_at(s, 1234, buffers, short_transfer(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write_at(s, 0, buffers, short_transfer(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write_at(s, 1234, buffers, short_transfer(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  int i = xio::async_write_at(s, 0, buffers, short_transfer(),
      archetypes::lazy_handler());
  XIO_CHECK(i == 42);
  ioc.restart();
  ioc.run();
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write_at(s, 1234, buffers, short_transfer())(
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));
}

void test_5_arg_mutable_buffer_async_write_at()
{
  namespace bindns = std;
  using bindns::placeholders::_1;
  using bindns::placeholders::_2;

  xio::io_context ioc;
  test_random_access_device s(ioc);
  xio::mutable_buffer buffers
    = xio::buffer(mutable_write_data, sizeof(mutable_write_data));

  s.reset();
  bool called = false;
  xio::async_write_at(s, 0, buffers,
      xio::transfer_all(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(mutable_write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(mutable_write_data)));

  s.reset();
  called = false;
  xio::async_write_at(s, 1234, buffers,
      xio::transfer_all(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(mutable_write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(mutable_write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write_at(s, 0, buffers,
      xio::transfer_all(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(mutable_write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(mutable_write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write_at(s, 1234, buffers,
      xio::transfer_all(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(mutable_write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(mutable_write_data)));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write_at(s, 0, buffers,
      xio::transfer_all(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(mutable_write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(mutable_write_data)));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write_at(s, 1234, buffers,
      xio::transfer_all(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(mutable_write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(mutable_write_data)));

  s.reset();
  called = false;
  xio::async_write_at(s, 0, buffers,
      xio::transfer_at_least(1),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(mutable_write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(mutable_write_data)));

  s.reset();
  called = false;
  xio::async_write_at(s, 1234, buffers,
      xio::transfer_at_least(1),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(mutable_write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(mutable_write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write_at(s, 0, buffers,
      xio::transfer_at_least(1),
      bindns::bind(async_write_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, 1));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write_at(s, 1234, buffers,
      xio::transfer_at_least(1),
      bindns::bind(async_write_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, 1));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write_at(s, 0, buffers,
      xio::transfer_at_least(1),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write_at(s, 1234, buffers,
      xio::transfer_at_least(1),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset();
  called = false;
  xio::async_write_at(s, 0, buffers,
      xio::transfer_at_least(10),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(mutable_write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(mutable_write_data)));

  s.reset();
  called = false;
  xio::async_write_at(s, 1234, buffers,
      xio::transfer_at_least(10),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(mutable_write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(mutable_write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write_at(s, 0, buffers,
      xio::transfer_at_least(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write_at(s, 1234, buffers,
      xio::transfer_at_least(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write_at(s, 0, buffers,
      xio::transfer_at_least(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write_at(s, 1234, buffers,
      xio::transfer_at_least(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset();
  called = false;
  xio::async_write_at(s, 0, buffers,
      xio::transfer_at_least(42),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(mutable_write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(mutable_write_data)));

  s.reset();
  called = false;
  xio::async_write_at(s, 1234, buffers,
      xio::transfer_at_least(42),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(mutable_write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(mutable_write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write_at(s, 0, buffers,
      xio::transfer_at_least(42),
      bindns::bind(async_write_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, 42));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write_at(s, 1234, buffers,
      xio::transfer_at_least(42),
      bindns::bind(async_write_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, 42));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write_at(s, 0, buffers,
      xio::transfer_at_least(42),
      bindns::bind(async_write_handler,
        _1, _2, 50, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, 50));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write_at(s, 1234, buffers,
      xio::transfer_at_least(42),
      bindns::bind(async_write_handler,
        _1, _2, 50, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, 50));

  s.reset();
  called = false;
  xio::async_write_at(s, 0, buffers,
      xio::transfer_exactly(1),
      bindns::bind(async_write_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, 1));

  s.reset();
  called = false;
  xio::async_write_at(s, 1234, buffers,
      xio::transfer_exactly(1),
      bindns::bind(async_write_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, 1));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write_at(s, 0, buffers,
      xio::transfer_exactly(1),
      bindns::bind(async_write_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, 1));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write_at(s, 1234, buffers,
      xio::transfer_exactly(1),
      bindns::bind(async_write_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, 1));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write_at(s, 0, buffers,
      xio::transfer_exactly(1),
      bindns::bind(async_write_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, 1));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write_at(s, 1234, buffers,
      xio::transfer_exactly(1),
      bindns::bind(async_write_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, 1));

  s.reset();
  called = false;
  xio::async_write_at(s, 0, buffers,
      xio::transfer_exactly(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset();
  called = false;
  xio::async_write_at(s, 1234, buffers,
      xio::transfer_exactly(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write_at(s, 0, buffers,
      xio::transfer_exactly(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write_at(s, 1234, buffers,
      xio::transfer_exactly(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write_at(s, 0, buffers,
      xio::transfer_exactly(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write_at(s, 1234, buffers,
      xio::transfer_exactly(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset();
  called = false;
  xio::async_write_at(s, 0, buffers,
      xio::transfer_exactly(42),
      bindns::bind(async_write_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, 42));

  s.reset();
  called = false;
  xio::async_write_at(s, 1234, buffers,
      xio::transfer_exactly(42),
      bindns::bind(async_write_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, 42));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write_at(s, 0, buffers,
      xio::transfer_exactly(42),
      bindns::bind(async_write_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, 42));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write_at(s, 1234, buffers,
      xio::transfer_exactly(42),
      bindns::bind(async_write_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, 42));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write_at(s, 0, buffers,
      xio::transfer_exactly(42),
      bindns::bind(async_write_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, 42));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write_at(s, 1234, buffers,
      xio::transfer_exactly(42),
      bindns::bind(async_write_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, 42));

  s.reset();
  called = false;
  xio::async_write_at(s, 0, buffers, old_style_transfer_all,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(mutable_write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(mutable_write_data)));

  s.reset();
  called = false;
  xio::async_write_at(s, 1234, buffers, old_style_transfer_all,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(mutable_write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(mutable_write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write_at(s, 0, buffers, old_style_transfer_all,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(mutable_write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(mutable_write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write_at(s, 1234, buffers, old_style_transfer_all,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(mutable_write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(mutable_write_data)));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write_at(s, 0, buffers, old_style_transfer_all,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(mutable_write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(mutable_write_data)));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write_at(s, 1234, buffers, old_style_transfer_all,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(mutable_write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(mutable_write_data)));

  s.reset();
  called = false;
  xio::async_write_at(s, 0, buffers, short_transfer(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(mutable_write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(mutable_write_data)));

  s.reset();
  called = false;
  xio::async_write_at(s, 1234, buffers, short_transfer(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(mutable_write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(mutable_write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write_at(s, 0, buffers, short_transfer(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(mutable_write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(mutable_write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write_at(s, 1234, buffers, short_transfer(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(mutable_write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(mutable_write_data)));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write_at(s, 0, buffers, short_transfer(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(mutable_write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(mutable_write_data)));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write_at(s, 1234, buffers, short_transfer(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(mutable_write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(mutable_write_data)));

  s.reset();
  int i = xio::async_write_at(s, 0, buffers, short_transfer(),
      archetypes::lazy_handler());
  XIO_CHECK(i == 42);
  ioc.restart();
  ioc.run();
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write_at(s, 1234, buffers, short_transfer())(
      bindns::bind(async_write_handler,
        _1, _2, sizeof(mutable_write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(mutable_write_data)));
}

void test_5_arg_boost_array_buffers_async_write_at()
{
  namespace bindns = std;
  using bindns::placeholders::_1;
  using bindns::placeholders::_2;

}

void test_5_arg_std_array_buffers_async_write_at()
{
  namespace bindns = std;
  using bindns::placeholders::_1;
  using bindns::placeholders::_2;

  xio::io_context ioc;
  test_random_access_device s(ioc);
  std::array<xio::const_buffer, 2> buffers = { {
    xio::buffer(write_data, 32),
    xio::buffer(write_data) + 32 } };

  s.reset();
  bool called = false;
  xio::async_write_at(s, 0, buffers,
      xio::transfer_all(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  called = false;
  xio::async_write_at(s, 1234, buffers,
      xio::transfer_all(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write_at(s, 0, buffers,
      xio::transfer_all(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write_at(s, 1234, buffers,
      xio::transfer_all(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write_at(s, 0, buffers,
      xio::transfer_all(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write_at(s, 1234, buffers,
      xio::transfer_all(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  called = false;
  xio::async_write_at(s, 0, buffers,
      xio::transfer_at_least(1),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  called = false;
  xio::async_write_at(s, 1234, buffers,
      xio::transfer_at_least(1),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write_at(s, 0, buffers,
      xio::transfer_at_least(1),
      bindns::bind(async_write_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, 1));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write_at(s, 1234, buffers,
      xio::transfer_at_least(1),
      bindns::bind(async_write_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, 1));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write_at(s, 0, buffers,
      xio::transfer_at_least(1),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write_at(s, 1234, buffers,
      xio::transfer_at_least(1),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset();
  called = false;
  xio::async_write_at(s, 0, buffers,
      xio::transfer_at_least(10),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  called = false;
  xio::async_write_at(s, 1234, buffers,
      xio::transfer_at_least(10),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write_at(s, 0, buffers,
      xio::transfer_at_least(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write_at(s, 1234, buffers,
      xio::transfer_at_least(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write_at(s, 0, buffers,
      xio::transfer_at_least(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write_at(s, 1234, buffers,
      xio::transfer_at_least(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset();
  called = false;
  xio::async_write_at(s, 0, buffers,
      xio::transfer_at_least(42),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  called = false;
  xio::async_write_at(s, 1234, buffers,
      xio::transfer_at_least(42),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write_at(s, 0, buffers,
      xio::transfer_at_least(42),
      bindns::bind(async_write_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, 42));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write_at(s, 1234, buffers,
      xio::transfer_at_least(42),
      bindns::bind(async_write_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, 42));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write_at(s, 0, buffers,
      xio::transfer_at_least(42),
      bindns::bind(async_write_handler,
        _1, _2, 50, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, 50));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write_at(s, 1234, buffers,
      xio::transfer_at_least(42),
      bindns::bind(async_write_handler,
        _1, _2, 50, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, 50));

  s.reset();
  called = false;
  xio::async_write_at(s, 0, buffers,
      xio::transfer_exactly(1),
      bindns::bind(async_write_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, 1));

  s.reset();
  called = false;
  xio::async_write_at(s, 1234, buffers,
      xio::transfer_exactly(1),
      bindns::bind(async_write_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, 1));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write_at(s, 0, buffers,
      xio::transfer_exactly(1),
      bindns::bind(async_write_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, 1));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write_at(s, 1234, buffers,
      xio::transfer_exactly(1),
      bindns::bind(async_write_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, 1));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write_at(s, 0, buffers,
      xio::transfer_exactly(1),
      bindns::bind(async_write_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, 1));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write_at(s, 1234, buffers,
      xio::transfer_exactly(1),
      bindns::bind(async_write_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, 1));

  s.reset();
  called = false;
  xio::async_write_at(s, 0, buffers,
      xio::transfer_exactly(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset();
  called = false;
  xio::async_write_at(s, 1234, buffers,
      xio::transfer_exactly(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write_at(s, 0, buffers,
      xio::transfer_exactly(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write_at(s, 1234, buffers,
      xio::transfer_exactly(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write_at(s, 0, buffers,
      xio::transfer_exactly(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write_at(s, 1234, buffers,
      xio::transfer_exactly(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset();
  called = false;
  xio::async_write_at(s, 0, buffers,
      xio::transfer_exactly(42),
      bindns::bind(async_write_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, 42));

  s.reset();
  called = false;
  xio::async_write_at(s, 1234, buffers,
      xio::transfer_exactly(42),
      bindns::bind(async_write_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, 42));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write_at(s, 0, buffers,
      xio::transfer_exactly(42),
      bindns::bind(async_write_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, 42));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write_at(s, 1234, buffers,
      xio::transfer_exactly(42),
      bindns::bind(async_write_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, 42));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write_at(s, 0, buffers,
      xio::transfer_exactly(42),
      bindns::bind(async_write_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, 42));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write_at(s, 1234, buffers,
      xio::transfer_exactly(42),
      bindns::bind(async_write_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, 42));

  s.reset();
  called = false;
  xio::async_write_at(s, 0, buffers, old_style_transfer_all,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  called = false;
  xio::async_write_at(s, 1234, buffers, old_style_transfer_all,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write_at(s, 0, buffers, old_style_transfer_all,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write_at(s, 1234, buffers, old_style_transfer_all,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write_at(s, 0, buffers, old_style_transfer_all,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write_at(s, 1234, buffers, old_style_transfer_all,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  called = false;
  xio::async_write_at(s, 0, buffers, short_transfer(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  called = false;
  xio::async_write_at(s, 1234, buffers, short_transfer(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write_at(s, 0, buffers, short_transfer(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write_at(s, 1234, buffers, short_transfer(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write_at(s, 0, buffers, short_transfer(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write_at(s, 1234, buffers, short_transfer(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  int i = xio::async_write_at(s, 0, buffers, short_transfer(),
      archetypes::lazy_handler());
  XIO_CHECK(i == 42);
  ioc.restart();
  ioc.run();
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write_at(s, 1234, buffers, short_transfer())(
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));
}

void test_5_arg_vector_buffers_async_write_at()
{
  namespace bindns = std;
  using bindns::placeholders::_1;
  using bindns::placeholders::_2;

  xio::io_context ioc;
  test_random_access_device s(ioc);
  std::vector<xio::const_buffer> buffers;
  buffers.push_back(xio::buffer(write_data, 32));
  buffers.push_back(xio::buffer(write_data) + 32);

  s.reset();
  bool called = false;
  xio::async_write_at(s, 0, buffers,
      xio::transfer_all(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  called = false;
  xio::async_write_at(s, 1234, buffers,
      xio::transfer_all(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write_at(s, 0, buffers,
      xio::transfer_all(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write_at(s, 1234, buffers,
      xio::transfer_all(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write_at(s, 0, buffers,
      xio::transfer_all(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write_at(s, 1234, buffers,
      xio::transfer_all(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  called = false;
  xio::async_write_at(s, 0, buffers,
      xio::transfer_at_least(1),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  called = false;
  xio::async_write_at(s, 1234, buffers,
      xio::transfer_at_least(1),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write_at(s, 0, buffers,
      xio::transfer_at_least(1),
      bindns::bind(async_write_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, 1));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write_at(s, 1234, buffers,
      xio::transfer_at_least(1),
      bindns::bind(async_write_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, 1));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write_at(s, 0, buffers,
      xio::transfer_at_least(1),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write_at(s, 1234, buffers,
      xio::transfer_at_least(1),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset();
  called = false;
  xio::async_write_at(s, 0, buffers,
      xio::transfer_at_least(10),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  called = false;
  xio::async_write_at(s, 1234, buffers,
      xio::transfer_at_least(10),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write_at(s, 0, buffers,
      xio::transfer_at_least(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write_at(s, 1234, buffers,
      xio::transfer_at_least(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write_at(s, 0, buffers,
      xio::transfer_at_least(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write_at(s, 1234, buffers,
      xio::transfer_at_least(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset();
  called = false;
  xio::async_write_at(s, 0, buffers,
      xio::transfer_at_least(42),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  called = false;
  xio::async_write_at(s, 1234, buffers,
      xio::transfer_at_least(42),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write_at(s, 0, buffers,
      xio::transfer_at_least(42),
      bindns::bind(async_write_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, 42));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write_at(s, 1234, buffers,
      xio::transfer_at_least(42),
      bindns::bind(async_write_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, 42));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write_at(s, 0, buffers,
      xio::transfer_at_least(42),
      bindns::bind(async_write_handler,
        _1, _2, 50, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, 50));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write_at(s, 1234, buffers,
      xio::transfer_at_least(42),
      bindns::bind(async_write_handler,
        _1, _2, 50, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, 50));

  s.reset();
  called = false;
  xio::async_write_at(s, 0, buffers,
      xio::transfer_exactly(1),
      bindns::bind(async_write_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, 1));

  s.reset();
  called = false;
  xio::async_write_at(s, 1234, buffers,
      xio::transfer_exactly(1),
      bindns::bind(async_write_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, 1));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write_at(s, 0, buffers,
      xio::transfer_exactly(1),
      bindns::bind(async_write_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, 1));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write_at(s, 1234, buffers,
      xio::transfer_exactly(1),
      bindns::bind(async_write_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, 1));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write_at(s, 0, buffers,
      xio::transfer_exactly(1),
      bindns::bind(async_write_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, 1));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write_at(s, 1234, buffers,
      xio::transfer_exactly(1),
      bindns::bind(async_write_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, 1));

  s.reset();
  called = false;
  xio::async_write_at(s, 0, buffers,
      xio::transfer_exactly(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset();
  called = false;
  xio::async_write_at(s, 1234, buffers,
      xio::transfer_exactly(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write_at(s, 0, buffers,
      xio::transfer_exactly(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write_at(s, 1234, buffers,
      xio::transfer_exactly(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write_at(s, 0, buffers,
      xio::transfer_exactly(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write_at(s, 1234, buffers,
      xio::transfer_exactly(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset();
  called = false;
  xio::async_write_at(s, 0, buffers,
      xio::transfer_exactly(42),
      bindns::bind(async_write_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, 42));

  s.reset();
  called = false;
  xio::async_write_at(s, 1234, buffers,
      xio::transfer_exactly(42),
      bindns::bind(async_write_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, 42));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write_at(s, 0, buffers,
      xio::transfer_exactly(42),
      bindns::bind(async_write_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, 42));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write_at(s, 1234, buffers,
      xio::transfer_exactly(42),
      bindns::bind(async_write_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, 42));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write_at(s, 0, buffers,
      xio::transfer_exactly(42),
      bindns::bind(async_write_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, 42));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write_at(s, 1234, buffers,
      xio::transfer_exactly(42),
      bindns::bind(async_write_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, 42));

  s.reset();
  called = false;
  xio::async_write_at(s, 0, buffers, old_style_transfer_all,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  called = false;
  xio::async_write_at(s, 1234, buffers, old_style_transfer_all,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write_at(s, 0, buffers, old_style_transfer_all,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write_at(s, 1234, buffers, old_style_transfer_all,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write_at(s, 0, buffers, old_style_transfer_all,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write_at(s, 1234, buffers, old_style_transfer_all,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  called = false;
  xio::async_write_at(s, 0, buffers, short_transfer(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  called = false;
  xio::async_write_at(s, 1234, buffers, short_transfer(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write_at(s, 0, buffers, short_transfer(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(1);
  called = false;
  xio::async_write_at(s, 1234, buffers, short_transfer(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write_at(s, 0, buffers, short_transfer(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write_at(s, 1234, buffers, short_transfer(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  int i = xio::async_write_at(s, 0, buffers, short_transfer(),
      archetypes::lazy_handler());
  XIO_CHECK(i == 42);
  ioc.restart();
  ioc.run();
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  s.next_write_length(10);
  called = false;
  xio::async_write_at(s, 1234, buffers, short_transfer())(
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));
}

void test_5_arg_streambuf_async_write_at()
{
  namespace bindns = std;
  using bindns::placeholders::_1;
  using bindns::placeholders::_2;

  xio::io_context ioc;
  test_random_access_device s(ioc);
  xio::streambuf sb;
  xio::const_buffer buffers
    = xio::buffer(write_data, sizeof(write_data));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  bool called = false;
  xio::async_write_at(s, 0, sb,
      xio::transfer_all(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  called = false;
  xio::async_write_at(s, 1234, sb,
      xio::transfer_all(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  s.next_write_length(1);
  called = false;
  xio::async_write_at(s, 0, sb,
      xio::transfer_all(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  s.next_write_length(1);
  called = false;
  xio::async_write_at(s, 1234, sb,
      xio::transfer_all(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  s.next_write_length(10);
  called = false;
  xio::async_write_at(s, 0, sb,
      xio::transfer_all(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  s.next_write_length(10);
  called = false;
  xio::async_write_at(s, 1234, sb,
      xio::transfer_all(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  called = false;
  xio::async_write_at(s, 0, sb,
      xio::transfer_at_least(1),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  called = false;
  xio::async_write_at(s, 1234, sb,
      xio::transfer_at_least(1),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  s.next_write_length(1);
  called = false;
  xio::async_write_at(s, 0, sb,
      xio::transfer_at_least(1),
      bindns::bind(async_write_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, 1));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  s.next_write_length(1);
  called = false;
  xio::async_write_at(s, 1234, sb,
      xio::transfer_at_least(1),
      bindns::bind(async_write_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, 1));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  s.next_write_length(10);
  called = false;
  xio::async_write_at(s, 0, sb,
      xio::transfer_at_least(1),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  s.next_write_length(10);
  called = false;
  xio::async_write_at(s, 1234, sb,
      xio::transfer_at_least(1),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  called = false;
  xio::async_write_at(s, 0, sb,
      xio::transfer_at_least(10),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  called = false;
  xio::async_write_at(s, 1234, sb,
      xio::transfer_at_least(10),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  s.next_write_length(1);
  called = false;
  xio::async_write_at(s, 0, sb,
      xio::transfer_at_least(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  s.next_write_length(1);
  called = false;
  xio::async_write_at(s, 1234, sb,
      xio::transfer_at_least(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  s.next_write_length(10);
  called = false;
  xio::async_write_at(s, 0, sb,
      xio::transfer_at_least(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  s.next_write_length(10);
  called = false;
  xio::async_write_at(s, 1234, sb,
      xio::transfer_at_least(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  called = false;
  xio::async_write_at(s, 0, sb,
      xio::transfer_at_least(42),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  called = false;
  xio::async_write_at(s, 1234, sb,
      xio::transfer_at_least(42),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  s.next_write_length(1);
  called = false;
  xio::async_write_at(s, 0, sb,
      xio::transfer_at_least(42),
      bindns::bind(async_write_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, 42));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  s.next_write_length(1);
  called = false;
  xio::async_write_at(s, 1234, sb,
      xio::transfer_at_least(42),
      bindns::bind(async_write_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, 42));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  s.next_write_length(10);
  called = false;
  xio::async_write_at(s, 0, sb,
      xio::transfer_at_least(42),
      bindns::bind(async_write_handler,
        _1, _2, 50, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, 50));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  s.next_write_length(10);
  called = false;
  xio::async_write_at(s, 1234, sb,
      xio::transfer_at_least(42),
      bindns::bind(async_write_handler,
        _1, _2, 50, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, 50));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  called = false;
  xio::async_write_at(s, 0, sb,
      xio::transfer_exactly(1),
      bindns::bind(async_write_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, 1));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  called = false;
  xio::async_write_at(s, 1234, sb,
      xio::transfer_exactly(1),
      bindns::bind(async_write_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, 1));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  s.next_write_length(1);
  called = false;
  xio::async_write_at(s, 0, sb,
      xio::transfer_exactly(1),
      bindns::bind(async_write_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, 1));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  s.next_write_length(1);
  called = false;
  xio::async_write_at(s, 1234, sb,
      xio::transfer_exactly(1),
      bindns::bind(async_write_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, 1));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  s.next_write_length(10);
  called = false;
  xio::async_write_at(s, 0, sb,
      xio::transfer_exactly(1),
      bindns::bind(async_write_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, 1));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  s.next_write_length(10);
  called = false;
  xio::async_write_at(s, 1234, sb,
      xio::transfer_exactly(1),
      bindns::bind(async_write_handler,
        _1, _2, 1, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, 1));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  called = false;
  xio::async_write_at(s, 0, sb,
      xio::transfer_exactly(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  called = false;
  xio::async_write_at(s, 1234, sb,
      xio::transfer_exactly(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  s.next_write_length(1);
  called = false;
  xio::async_write_at(s, 0, sb,
      xio::transfer_exactly(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  s.next_write_length(1);
  called = false;
  xio::async_write_at(s, 1234, sb,
      xio::transfer_exactly(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  s.next_write_length(10);
  called = false;
  xio::async_write_at(s, 0, sb,
      xio::transfer_exactly(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, 10));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  s.next_write_length(10);
  called = false;
  xio::async_write_at(s, 1234, sb,
      xio::transfer_exactly(10),
      bindns::bind(async_write_handler,
        _1, _2, 10, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, 10));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  called = false;
  xio::async_write_at(s, 0, sb,
      xio::transfer_exactly(42),
      bindns::bind(async_write_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, 42));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  called = false;
  xio::async_write_at(s, 1234, sb,
      xio::transfer_exactly(42),
      bindns::bind(async_write_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, 42));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  s.next_write_length(1);
  called = false;
  xio::async_write_at(s, 0, sb,
      xio::transfer_exactly(42),
      bindns::bind(async_write_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, 42));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  s.next_write_length(1);
  called = false;
  xio::async_write_at(s, 1234, sb,
      xio::transfer_exactly(42),
      bindns::bind(async_write_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, 42));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  s.next_write_length(10);
  called = false;
  xio::async_write_at(s, 0, sb,
      xio::transfer_exactly(42),
      bindns::bind(async_write_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, 42));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  s.next_write_length(10);
  called = false;
  xio::async_write_at(s, 1234, sb,
      xio::transfer_exactly(42),
      bindns::bind(async_write_handler,
        _1, _2, 42, &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, 42));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  called = false;
  xio::async_write_at(s, 0, sb, old_style_transfer_all,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  called = false;
  xio::async_write_at(s, 1234, sb, old_style_transfer_all,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  s.next_write_length(1);
  called = false;
  xio::async_write_at(s, 0, sb, old_style_transfer_all,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  s.next_write_length(1);
  called = false;
  xio::async_write_at(s, 1234, sb, old_style_transfer_all,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  s.next_write_length(10);
  called = false;
  xio::async_write_at(s, 0, sb, old_style_transfer_all,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  s.next_write_length(10);
  called = false;
  xio::async_write_at(s, 1234, sb, old_style_transfer_all,
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  called = false;
  xio::async_write_at(s, 0, sb, short_transfer(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  called = false;
  xio::async_write_at(s, 1234, sb, short_transfer(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  s.next_write_length(1);
  called = false;
  xio::async_write_at(s, 0, sb, short_transfer(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  s.next_write_length(1);
  called = false;
  xio::async_write_at(s, 1234, sb, short_transfer(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  s.next_write_length(10);
  called = false;
  xio::async_write_at(s, 0, sb, short_transfer(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  s.next_write_length(10);
  called = false;
  xio::async_write_at(s, 1234, sb, short_transfer(),
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  int i = xio::async_write_at(s, 0, sb, short_transfer(),
      archetypes::lazy_handler());
  XIO_CHECK(i == 42);
  ioc.restart();
  ioc.run();
  XIO_CHECK(s.check_buffers(0, buffers, sizeof(write_data)));

  s.reset();
  sb.consume(sb.size());
  sb.sputn(write_data, sizeof(write_data));
  s.next_write_length(10);
  called = false;
  xio::async_write_at(s, 1234, sb, short_transfer())(
      bindns::bind(async_write_handler,
        _1, _2, sizeof(write_data), &called));
  ioc.restart();
  ioc.run();
  XIO_CHECK(called);
  XIO_CHECK(s.check_buffers(1234, buffers, sizeof(write_data)));
}

XIO_TEST_SUITE
(
  "write_at",
  XIO_TEST_CASE(test_3_arg_const_buffer_write_at)
  XIO_TEST_CASE(test_3_arg_mutable_buffer_write_at)
  XIO_TEST_CASE(test_3_arg_vector_buffers_write_at)
  XIO_TEST_CASE(test_4_arg_nothrow_const_buffer_write_at)
  XIO_TEST_CASE(test_4_arg_nothrow_mutable_buffer_write_at)
  XIO_TEST_CASE(test_4_arg_nothrow_vector_buffers_write_at)
  XIO_TEST_CASE(test_4_arg_const_buffer_write_at)
  XIO_TEST_CASE(test_4_arg_mutable_buffer_write_at)
  XIO_TEST_CASE(test_4_arg_vector_buffers_write_at)
  XIO_TEST_CASE(test_5_arg_const_buffer_write_at)
  XIO_TEST_CASE(test_5_arg_mutable_buffer_write_at)
  XIO_TEST_CASE(test_5_arg_vector_buffers_write_at)
  XIO_TEST_CASE(test_4_arg_const_buffer_async_write_at)
  XIO_TEST_CASE(test_4_arg_mutable_buffer_async_write_at)
  XIO_TEST_CASE(test_4_arg_boost_array_buffers_async_write_at)
  XIO_TEST_CASE(test_4_arg_std_array_buffers_async_write_at)
  XIO_TEST_CASE(test_4_arg_vector_buffers_async_write_at)
  XIO_TEST_CASE(test_4_arg_streambuf_async_write_at)
  XIO_TEST_CASE(test_5_arg_const_buffer_async_write_at)
  XIO_TEST_CASE(test_5_arg_mutable_buffer_async_write_at)
  XIO_TEST_CASE(test_5_arg_boost_array_buffers_async_write_at)
  XIO_TEST_CASE(test_5_arg_std_array_buffers_async_write_at)
  XIO_TEST_CASE(test_5_arg_vector_buffers_async_write_at)
  XIO_TEST_CASE(test_5_arg_streambuf_async_write_at)
)
