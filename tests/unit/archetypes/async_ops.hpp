//
// archetypes/async_ops.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ARCHETYPES_ASYNC_OPS_HPP
#define ARCHETYPES_ASYNC_OPS_HPP

#include <functional>
#include <xio/associated_allocator.h>
#include <xio/associated_executor.h>
#include <xio/async_result.h>
#include <xio/bind_allocator.h>
#include <xio/error.h>
#include <xio/post.h>

namespace archetypes {

namespace bindns = std;

struct initiate_op_0
{
  template <typename Handler>
  void operator()(Handler&& handler)
  {
    typename xio::associated_allocator<Handler>::type a
      = xio::get_associated_allocator(handler);

    typename xio::associated_executor<Handler>::type ex
      = xio::get_associated_executor(handler);

    xio::post(ex,
        xio::bind_allocator(a, static_cast<Handler&&>(handler)));
  }
};

template <typename CompletionToken>
auto async_op_0(CompletionToken&& token)
  -> decltype(
    xio::async_initiate<CompletionToken, void()>(
      initiate_op_0(), token))
{
  return xio::async_initiate<CompletionToken, void()>(
      initiate_op_0(), token);
}

struct initiate_op_ec_0
{
  template <typename Handler>
  void operator()(Handler&& handler, bool ok)
  {
    typename xio::associated_allocator<Handler>::type a
      = xio::get_associated_allocator(handler);

    typename xio::associated_executor<Handler>::type ex
      = xio::get_associated_executor(handler);

    if (ok)
    {
      xio::post(ex,
          xio::bind_allocator(a,
            bindns::bind(static_cast<Handler&&>(handler),
              xio::error_code())));
    }
    else
    {
      xio::post(ex,
          xio::bind_allocator(a,
            bindns::bind(static_cast<Handler&&>(handler),
              xio::error_code(
                xio::error::operation_aborted))));
    }
  }
};

template <typename CompletionToken>
auto async_op_ec_0(bool ok, CompletionToken&& token)
  -> decltype(
    xio::async_initiate<CompletionToken,
      void(xio::error_code)>(
        initiate_op_ec_0(), token, ok))
{
  return xio::async_initiate<CompletionToken,
    void(xio::error_code)>(
      initiate_op_ec_0(), token, ok);
}

struct initiate_op_ex_0
{
  template <typename Handler>
  void operator()(Handler&& handler, bool ok)
  {
    typename xio::associated_allocator<Handler>::type a
      = xio::get_associated_allocator(handler);

    typename xio::associated_executor<Handler>::type ex
      = xio::get_associated_executor(handler);

    if (ok)
    {
      xio::post(ex,
          xio::bind_allocator(a,
            bindns::bind(static_cast<Handler&&>(handler),
              std::exception_ptr())));
    }
    else
    {
      xio::post(ex,
          xio::bind_allocator(a,
            bindns::bind(static_cast<Handler&&>(handler),
              std::make_exception_ptr(std::runtime_error("blah")))));
    }
  }
};

template <typename CompletionToken>
auto async_op_ex_0(bool ok, CompletionToken&& token)
  -> decltype(
    xio::async_initiate<CompletionToken,
      void(std::exception_ptr)>(
        initiate_op_ex_0(), token, ok))
{
  return xio::async_initiate<CompletionToken,
    void(std::exception_ptr)>(
      initiate_op_ex_0(), token, ok);
}

struct initiate_op_1
{
  template <typename Handler>
  void operator()(Handler&& handler)
  {
    typename xio::associated_allocator<Handler>::type a
      = xio::get_associated_allocator(handler);

    typename xio::associated_executor<Handler>::type ex
      = xio::get_associated_executor(handler);

    xio::post(ex,
        xio::bind_allocator(a,
          bindns::bind(static_cast<Handler&&>(handler), 42)));
  }
};

template <typename CompletionToken>
auto async_op_1(CompletionToken&& token)
  -> decltype(
    xio::async_initiate<CompletionToken, void(int)>(
      initiate_op_1(), token))
{
  return xio::async_initiate<CompletionToken, void(int)>(
      initiate_op_1(), token);
}

struct initiate_op_ec_1
{
  template <typename Handler>
  void operator()(Handler&& handler, bool ok)
  {
    typename xio::associated_allocator<Handler>::type a
      = xio::get_associated_allocator(handler);

    typename xio::associated_executor<Handler>::type ex
      = xio::get_associated_executor(handler);

    if (ok)
    {
      xio::post(ex,
          xio::bind_allocator(a,
            bindns::bind(static_cast<Handler&&>(handler),
              xio::error_code(), 42)));
    }
    else
    {
      xio::post(ex,
          xio::bind_allocator(a,
            bindns::bind(static_cast<Handler&&>(handler),
              xio::error_code(
                xio::error::operation_aborted), 0)));
    }
  }
};

template <typename CompletionToken>
auto async_op_ec_1(bool ok, CompletionToken&& token)
  -> decltype(
    xio::async_initiate<CompletionToken,
      void(xio::error_code, int)>(
        initiate_op_ec_1(), token, ok))
{
  return xio::async_initiate<CompletionToken,
    void(xio::error_code, int)>(
      initiate_op_ec_1(), token, ok);
}

struct initiate_op_ex_1
{
  template <typename Handler>
  void operator()(Handler&& handler, bool ok)
  {
    typename xio::associated_allocator<Handler>::type a
      = xio::get_associated_allocator(handler);

    typename xio::associated_executor<Handler>::type ex
      = xio::get_associated_executor(handler);

    if (ok)
    {
      xio::post(ex,
          xio::bind_allocator(a,
            bindns::bind(static_cast<Handler&&>(handler),
              std::exception_ptr(), 42)));
    }
    else
    {
      xio::post(ex,
          xio::bind_allocator(a,
            bindns::bind(static_cast<Handler&&>(handler),
              std::make_exception_ptr(std::runtime_error("blah")), 0)));
    }
  }
};

template <typename CompletionToken>
auto async_op_ex_1(bool ok, CompletionToken&& token)
  -> decltype(
    xio::async_initiate<CompletionToken,
      void(std::exception_ptr, int)>(
        initiate_op_ex_1(), token, ok))
{
  return xio::async_initiate<CompletionToken,
    void(std::exception_ptr, int)>(
      initiate_op_ex_1(), token, ok);
}

struct initiate_op_2
{
  template <typename Handler>
  void operator()(Handler&& handler)
  {
    typename xio::associated_allocator<Handler>::type a
      = xio::get_associated_allocator(handler);

    typename xio::associated_executor<Handler>::type ex
      = xio::get_associated_executor(handler);

    xio::post(ex,
        xio::bind_allocator(a,
          bindns::bind(static_cast<Handler&&>(handler), 42, 2.0)));
  }
};

template <typename CompletionToken>
auto async_op_2(CompletionToken&& token)
  -> decltype(
    xio::async_initiate<CompletionToken, void(int, double)>(
      initiate_op_2(), token))
{
  return xio::async_initiate<CompletionToken, void(int, double)>(
      initiate_op_2(), token);
}

struct initiate_op_ec_2
{
  template <typename Handler>
  void operator()(Handler&& handler, bool ok)
  {
    typename xio::associated_allocator<Handler>::type a
      = xio::get_associated_allocator(handler);

    typename xio::associated_executor<Handler>::type ex
      = xio::get_associated_executor(handler);

    if (ok)
    {
      xio::post(ex,
          xio::bind_allocator(a,
            bindns::bind(static_cast<Handler&&>(handler),
              xio::error_code(), 42, 2.0)));
    }
    else
    {
      xio::post(ex,
          xio::bind_allocator(a,
            bindns::bind(static_cast<Handler&&>(handler),
              xio::error_code(xio::error::operation_aborted),
              0, 0.0)));
    }
  }
};

template <typename CompletionToken>
auto async_op_ec_2(bool ok, CompletionToken&& token)
  -> decltype(
    xio::async_initiate<CompletionToken,
      void(xio::error_code, int, double)>(
        initiate_op_ec_2(), token, ok))
{
  return xio::async_initiate<CompletionToken,
    void(xio::error_code, int, double)>(
      initiate_op_ec_2(), token, ok);
}

struct initiate_op_ex_2
{
  template <typename Handler>
  void operator()(Handler&& handler, bool ok)
  {
    typename xio::associated_allocator<Handler>::type a
      = xio::get_associated_allocator(handler);

    typename xio::associated_executor<Handler>::type ex
      = xio::get_associated_executor(handler);

    if (ok)
    {
      xio::post(ex,
          xio::bind_allocator(a,
            bindns::bind(static_cast<Handler&&>(handler),
              std::exception_ptr(), 42, 2.0)));
    }
    else
    {
      xio::post(ex,
          xio::bind_allocator(a,
            bindns::bind(static_cast<Handler&&>(handler),
              std::make_exception_ptr(std::runtime_error("blah")), 0, 0.0)));
    }
  }
};

template <typename CompletionToken>
auto async_op_ex_2(bool ok, CompletionToken&& token)
  -> decltype(
    xio::async_initiate<CompletionToken,
      void(std::exception_ptr, int, double)>(
        initiate_op_ex_2(), token, ok))
{
  return xio::async_initiate<CompletionToken,
    void(std::exception_ptr, int, double)>(
      initiate_op_ex_2(), token, ok);
}

struct initiate_op_3
{
  template <typename Handler>
  void operator()(Handler&& handler)
  {
    typename xio::associated_allocator<Handler>::type a
      = xio::get_associated_allocator(handler);

    typename xio::associated_executor<Handler>::type ex
      = xio::get_associated_executor(handler);

    xio::post(ex,
        xio::bind_allocator(a,
          bindns::bind(static_cast<Handler&&>(handler), 42, 2.0, 'a')));
  }
};

template <typename CompletionToken>
auto async_op_3(CompletionToken&& token)
  -> decltype(
    xio::async_initiate<CompletionToken, void(int, double, char)>(
      initiate_op_3(), token))
{
  return xio::async_initiate<CompletionToken, void(int, double, char)>(
      initiate_op_3(), token);
}

struct initiate_op_ec_3
{
  template <typename Handler>
  void operator()(Handler&& handler, bool ok)
  {
    typename xio::associated_allocator<Handler>::type a
      = xio::get_associated_allocator(handler);

    typename xio::associated_executor<Handler>::type ex
      = xio::get_associated_executor(handler);

    if (ok)
    {
      xio::post(ex,
          xio::bind_allocator(a,
            bindns::bind(static_cast<Handler&&>(handler),
              xio::error_code(), 42, 2.0, 'a')));
    }
    else
    {
      xio::post(ex,
          xio::bind_allocator(a,
            bindns::bind(static_cast<Handler&&>(handler),
              xio::error_code(xio::error::operation_aborted),
              0, 0.0, 'z')));
    }
  }
};

template <typename CompletionToken>
auto async_op_ec_3(bool ok, CompletionToken&& token)
  -> decltype(
    xio::async_initiate<CompletionToken,
      void(xio::error_code, int, double, char)>(
        initiate_op_ec_3(), token, ok))
{
  return xio::async_initiate<CompletionToken,
    void(xio::error_code, int, double, char)>(
      initiate_op_ec_3(), token, ok);
}

struct initiate_op_ex_3
{
  template <typename Handler>
  void operator()(Handler&& handler, bool ok)
  {
    typename xio::associated_allocator<Handler>::type a
      = xio::get_associated_allocator(handler);

    typename xio::associated_executor<Handler>::type ex
      = xio::get_associated_executor(handler);

    if (ok)
    {
      xio::post(ex,
          xio::bind_allocator(a,
            bindns::bind(static_cast<Handler&&>(handler),
              std::exception_ptr(), 42, 2.0, 'a')));
    }
    else
    {
      xio::post(ex,
          xio::bind_allocator(a,
            bindns::bind(static_cast<Handler&&>(handler),
              std::make_exception_ptr(std::runtime_error("blah")),
              0, 0.0, 'z')));
    }
  }
};

template <typename CompletionToken>
auto async_op_ex_3(bool ok, CompletionToken&& token)
  -> decltype(
    xio::async_initiate<CompletionToken,
      void(std::exception_ptr, int, double, char)>(
        initiate_op_ex_3(), token, ok))
{
  return xio::async_initiate<CompletionToken,
    void(std::exception_ptr, int, double, char)>(
      initiate_op_ex_3(), token, ok);
}

} // namespace archetypes

#endif // ARCHETYPES_ASYNC_OPS_HPP
