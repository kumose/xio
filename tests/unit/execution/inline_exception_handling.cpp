//
// inline_exception_handling.cpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//



// Test that header file is self-contained.
#include <xio/execution/inline_exception_handling.h>

#include <xio/prefer.h>
#include <xio/query.h>
#include <xio/require.h>
#include "../unit_test.hpp"

namespace exec = xio::execution;

typedef exec::inline_exception_handling_t s;
typedef exec::inline_exception_handling_t::propagate_t n1;
typedef exec::inline_exception_handling_t::capture_t n2;
typedef exec::inline_exception_handling_t::terminate_t n3;

struct ex_nq_nr
{
  template <typename F>
  void execute(const F&) const
  {
  }

  friend bool operator==(const ex_nq_nr&, const ex_nq_nr&) noexcept
  {
    return true;
  }

  friend bool operator!=(const ex_nq_nr&, const ex_nq_nr&) noexcept
  {
    return false;
  }
};

#if !defined(XIO_HAS_DEDUCED_EXECUTION_IS_EXECUTOR_TRAIT)

namespace xio {
namespace execution {

template <>
struct is_executor<ex_nq_nr> : std::true_type
{
};

} // namespace execution
} // namespace xio

#endif // !defined(XIO_HAS_DEDUCED_EXECUTION_IS_EXECUTOR_TRAIT)

template <typename ResultType, typename ParamType, typename Result>
struct ex_cq_nr
{
  static constexpr ResultType query(ParamType) noexcept
  {
    return Result();
  }

  template <typename F>
  void execute(const F&) const
  {
  }

  friend bool operator==(const ex_cq_nr&, const ex_cq_nr&) noexcept
  {
    return true;
  }

  friend bool operator!=(const ex_cq_nr&, const ex_cq_nr&) noexcept
  {
    return false;
  }
};

#if !defined(XIO_HAS_DEDUCED_EXECUTION_IS_EXECUTOR_TRAIT)

namespace xio {
namespace execution {

template <typename ResultType, typename ParamType, typename Result>
struct is_executor<ex_cq_nr<ResultType, ParamType, Result> >
  : std::true_type
{
};

} // namespace execution
} // namespace xio

#endif // !defined(XIO_HAS_DEDUCED_EXECUTION_IS_EXECUTOR_TRAIT)

namespace xio {
namespace traits {

#if !defined(XIO_HAS_DEDUCED_QUERY_STATIC_CONSTEXPR_MEMBER_TRAIT)

template <typename ResultType, typename ParamType,
  typename Result, typename Param>
struct query_static_constexpr_member<
  ex_cq_nr<ResultType, ParamType, Result>, Param,
  typename std::enable_if<
    std::is_convertible<Param, ParamType>::value
  >::type>
{
  static constexpr bool is_valid = true;
  static constexpr bool is_noexcept = true;

  typedef Result result_type; // Must return raw result type.

  static constexpr result_type value()
  {
    return Result();
  }
};

#endif // !defined(XIO_HAS_DEDUCED_QUERY_STATIC_CONSTEXPR_MEMBER_TRAIT)

} // namespace traits
} // namespace xio

template <typename ResultType, typename ParamType, typename Result>
struct ex_mq_nr
{
  ResultType query(ParamType) const noexcept
  {
    return Result();
  }

  template <typename F>
  void execute(const F&) const
  {
  }

  friend bool operator==(const ex_mq_nr&, const ex_mq_nr&) noexcept
  {
    return true;
  }

  friend bool operator!=(const ex_mq_nr&, const ex_mq_nr&) noexcept
  {
    return false;
  }
};

#if !defined(XIO_HAS_DEDUCED_EXECUTION_IS_EXECUTOR_TRAIT)

namespace xio {
namespace execution {

template <typename ResultType, typename ParamType, typename Result>
struct is_executor<ex_mq_nr<ResultType, ParamType, Result> >
  : std::true_type
{
};

} // namespace execution
} // namespace xio

#endif // !defined(XIO_HAS_DEDUCED_EXECUTION_IS_EXECUTOR_TRAIT)

namespace xio {
namespace traits {

#if !defined(XIO_HAS_DEDUCED_QUERY_MEMBER_TRAIT)

template <typename ResultType, typename ParamType,
  typename Result, typename Param>
struct query_member<
  ex_mq_nr<ResultType, ParamType, Result>, Param,
  typename std::enable_if<
    std::is_convertible<Param, ParamType>::value
  >::type>
{
  static constexpr bool is_valid = true;
  static constexpr bool is_noexcept = true;

  typedef ResultType result_type;
};

#endif // !defined(XIO_HAS_DEDUCED_QUERY_MEMBER_TRAIT)

} // namespace traits
} // namespace xio

template <typename ResultType, typename ParamType, typename Result>
struct ex_fq_nr
{
  friend ResultType query(const ex_fq_nr&, ParamType) noexcept
  {
    return Result();
  }

  template <typename F>
  void execute(const F&) const
  {
  }

  friend bool operator==(const ex_fq_nr&, const ex_fq_nr&) noexcept
  {
    return true;
  }

  friend bool operator!=(const ex_fq_nr&, const ex_fq_nr&) noexcept
  {
    return false;
  }
};

#if !defined(XIO_HAS_DEDUCED_EXECUTION_IS_EXECUTOR_TRAIT)

namespace xio {
namespace execution {

template <typename ResultType, typename ParamType, typename Result>
struct is_executor<ex_fq_nr<ResultType, ParamType, Result> >
  : std::true_type
{
};

} // namespace execution
} // namespace xio

#endif // !defined(XIO_HAS_DEDUCED_EXECUTION_IS_EXECUTOR_TRAIT)

namespace xio {
namespace traits {

#if !defined(XIO_HAS_DEDUCED_QUERY_FREE_TRAIT)

template <typename ResultType, typename ParamType,
  typename Result, typename Param>
struct query_free<
  ex_fq_nr<ResultType, ParamType, Result>, Param,
  typename std::enable_if<
    std::is_convertible<Param, ParamType>::value
  >::type>
{
  static constexpr bool is_valid = true;
  static constexpr bool is_noexcept = true;

  typedef ResultType result_type;
};

#endif // !defined(XIO_HAS_DEDUCED_QUERY_FREE_TRAIT)

} // namespace traits
} // namespace xio

template <typename CurrentType, typename OtherType>
struct ex_mq_mr
{
  CurrentType query(CurrentType) const noexcept
  {
    return CurrentType();
  }

  CurrentType query(OtherType) const noexcept
  {
    return CurrentType();
  }

  ex_mq_mr<CurrentType, OtherType> require(
      CurrentType) const noexcept
  {
    return ex_mq_mr<CurrentType, OtherType>();
  }

  ex_mq_mr<OtherType, CurrentType> require(
      OtherType) const noexcept
  {
    return ex_mq_mr<OtherType, CurrentType>();
  }

  template <typename F>
  void execute(const F&) const
  {
  }

  friend bool operator==(const ex_mq_mr&, const ex_mq_mr&) noexcept
  {
    return true;
  }

  friend bool operator!=(const ex_mq_mr&, const ex_mq_mr&) noexcept
  {
    return false;
  }
};

template <typename CurrentType>
struct ex_mq_mr<CurrentType, CurrentType>
{
  CurrentType query(CurrentType) const noexcept
  {
    return CurrentType();
  }

  ex_mq_mr<CurrentType, CurrentType> require(
      CurrentType) const noexcept
  {
    return ex_mq_mr<CurrentType, CurrentType>();
  }

  template <typename F>
  void execute(const F&) const
  {
  }

  friend bool operator==(const ex_mq_mr&, const ex_mq_mr&) noexcept
  {
    return true;
  }

  friend bool operator!=(const ex_mq_mr&, const ex_mq_mr&) noexcept
  {
    return false;
  }
};

#if !defined(XIO_HAS_DEDUCED_EXECUTION_IS_EXECUTOR_TRAIT)

namespace xio {
namespace execution {

template <typename CurrentType, typename OtherType>
struct is_executor<ex_mq_mr<CurrentType, OtherType> >
  : std::true_type
{
};

} // namespace execution
} // namespace xio

#endif // !defined(XIO_HAS_DEDUCED_EXECUTION_IS_EXECUTOR_TRAIT)

namespace xio {
namespace traits {

#if !defined(XIO_HAS_DEDUCED_QUERY_MEMBER_TRAIT)

template <typename CurrentType, typename OtherType, typename Param>
struct query_member<
  ex_mq_mr<CurrentType, OtherType>, Param,
  typename std::enable_if<
    std::is_convertible<Param, CurrentType>::value
      || std::is_convertible<Param, OtherType>::value
  >::type>
{
  static constexpr bool is_valid = true;
  static constexpr bool is_noexcept = true;

  typedef CurrentType result_type;
};

#endif // !defined(XIO_HAS_DEDUCED_QUERY_MEMBER_TRAIT)

#if !defined(XIO_HAS_DEDUCED_REQUIRE_MEMBER_TRAIT)

template <typename CurrentType, typename OtherType, typename Param>
struct require_member<
  ex_mq_mr<CurrentType, OtherType>, Param,
  typename std::enable_if<
    std::is_convertible<Param, CurrentType>::value
  >::type>
{
  static constexpr bool is_valid = true;
  static constexpr bool is_noexcept = true;

  typedef ex_mq_mr<CurrentType, OtherType> result_type;
};

template <typename CurrentType, typename OtherType, typename Param>
struct require_member<
  ex_mq_mr<CurrentType, OtherType>, Param,
  typename std::enable_if<
    std::is_convertible<Param, OtherType>::value
      && !std::is_same<CurrentType, OtherType>::value
  >::type>
{
  static constexpr bool is_valid = true;
  static constexpr bool is_noexcept = true;

  typedef ex_mq_mr<OtherType, CurrentType> result_type;
};

#endif // !defined(XIO_HAS_DEDUCED_REQUIRE_MEMBER_TRAIT)

} // namespace traits
} // namespace xio

template <typename CurrentType, typename OtherType>
struct ex_fq_fr
{
  friend CurrentType query(const ex_fq_fr&, CurrentType) noexcept
  {
    return CurrentType();
  }

  friend CurrentType query(const ex_fq_fr&, OtherType) noexcept
  {
    return CurrentType();
  }

  friend ex_fq_fr<CurrentType, OtherType> require(
      const ex_fq_fr&, CurrentType) noexcept
  {
    return ex_fq_fr<CurrentType, OtherType>();
  }

  friend ex_fq_fr<OtherType, CurrentType> require(
      const ex_fq_fr&, OtherType) noexcept
  {
    return ex_fq_fr<OtherType, CurrentType>();
  }

  friend ex_fq_fr<CurrentType, OtherType> prefer(
      const ex_fq_fr&, CurrentType) noexcept
  {
    return ex_fq_fr<CurrentType, OtherType>();
  }

  friend ex_fq_fr<OtherType, CurrentType> prefer(
      const ex_fq_fr&, OtherType) noexcept
  {
    return ex_fq_fr<OtherType, CurrentType>();
  }

  template <typename F>
  void execute(const F&) const
  {
  }

  friend bool operator==(const ex_fq_fr&, const ex_fq_fr&) noexcept
  {
    return true;
  }

  friend bool operator!=(const ex_fq_fr&, const ex_fq_fr&) noexcept
  {
    return false;
  }
};

template <typename CurrentType>
struct ex_fq_fr<CurrentType, CurrentType>
{
  friend CurrentType query(const ex_fq_fr&, CurrentType) noexcept
  {
    return CurrentType();
  }

  friend ex_fq_fr<CurrentType, CurrentType> require(
      const ex_fq_fr&, CurrentType) noexcept
  {
    return ex_fq_fr<CurrentType, CurrentType>();
  }

  friend ex_fq_fr<CurrentType, CurrentType> prefer(
      const ex_fq_fr&, CurrentType) noexcept
  {
    return ex_fq_fr<CurrentType, CurrentType>();
  }

  template <typename F>
  void execute(const F&) const
  {
  }

  friend bool operator==(const ex_fq_fr&, const ex_fq_fr&) noexcept
  {
    return true;
  }

  friend bool operator!=(const ex_fq_fr&, const ex_fq_fr&) noexcept
  {
    return false;
  }
};

#if !defined(XIO_HAS_DEDUCED_EXECUTION_IS_EXECUTOR_TRAIT)

namespace xio {
namespace execution {

template <typename CurrentType, typename OtherType>
struct is_executor<ex_fq_fr<CurrentType, OtherType> >
  : std::true_type
{
};

} // namespace execution
} // namespace xio

#endif // !defined(XIO_HAS_DEDUCED_EXECUTION_IS_EXECUTOR_TRAIT)

namespace xio {
namespace traits {

#if !defined(XIO_HAS_DEDUCED_QUERY_FREE_TRAIT)

template <typename CurrentType, typename OtherType, typename Param>
struct query_free<
  ex_fq_fr<CurrentType, OtherType>, Param,
  typename std::enable_if<
    std::is_convertible<Param, CurrentType>::value
      || std::is_convertible<Param, OtherType>::value
  >::type>
{
  static constexpr bool is_valid = true;
  static constexpr bool is_noexcept = true;

  typedef CurrentType result_type;
};

#endif // !defined(XIO_HAS_DEDUCED_QUERY_FREE_TRAIT)

#if !defined(XIO_HAS_DEDUCED_REQUIRE_FREE_TRAIT)

template <typename CurrentType, typename OtherType, typename Param>
struct require_free<
  ex_fq_fr<CurrentType, OtherType>, Param,
  typename std::enable_if<
    std::is_convertible<Param, CurrentType>::value
  >::type>
{
  static constexpr bool is_valid = true;
  static constexpr bool is_noexcept = true;

  typedef ex_fq_fr<CurrentType, OtherType> result_type;
};

template <typename CurrentType, typename OtherType, typename Param>
struct require_free<
  ex_fq_fr<CurrentType, OtherType>, Param,
  typename std::enable_if<
    std::is_convertible<Param, OtherType>::value
      && !std::is_same<CurrentType, OtherType>::value
  >::type>
{
  static constexpr bool is_valid = true;
  static constexpr bool is_noexcept = true;

  typedef ex_fq_fr<OtherType, CurrentType> result_type;
};

#endif // !defined(XIO_HAS_DEDUCED_REQUIRE_FREE_TRAIT)

#if !defined(XIO_HAS_DEDUCED_PREFER_FREE_TRAIT)

template <typename CurrentType, typename OtherType, typename Param>
struct prefer_free<
  ex_fq_fr<CurrentType, OtherType>, Param,
  typename std::enable_if<
    std::is_convertible<Param, CurrentType>::value
  >::type>
{
  static constexpr bool is_valid = true;
  static constexpr bool is_noexcept = true;

  typedef ex_fq_fr<CurrentType, OtherType> result_type;
};

template <typename CurrentType, typename OtherType, typename Param>
struct prefer_free<
  ex_fq_fr<CurrentType, OtherType>, Param,
  typename std::enable_if<
    std::is_convertible<Param, OtherType>::value
      && !std::is_same<CurrentType, OtherType>::value
  >::type>
{
  static constexpr bool is_valid = true;
  static constexpr bool is_noexcept = true;

  typedef ex_fq_fr<OtherType, CurrentType> result_type;
};

#endif // !defined(XIO_HAS_DEDUCED_PREFER_FREE_TRAIT)

} // namespace traits
} // namespace xio

template <typename Executor, typename Param, bool ExpectedResult>
void test_can_query()
{
  constexpr bool b1 =
    xio::can_query<Executor, Param>::value;
  XIO_CHECK(b1 == ExpectedResult);

  constexpr bool b2 =
    xio::can_query<const Executor, Param>::value;
  XIO_CHECK(b2 == ExpectedResult);

  constexpr bool b3 =
    xio::can_query<Executor&, Param>::value;
  XIO_CHECK(b3 == ExpectedResult);

  constexpr bool b4 =
    xio::can_query<const Executor&, Param>::value;
  XIO_CHECK(b4 == ExpectedResult);
}

template <typename Executor, typename Param, typename ExpectedResult>
void test_query()
{
  exec::inline_exception_handling_t result1 = xio::query(Executor(), Param());
  XIO_CHECK(result1 == ExpectedResult());

  Executor ex1 = {};
  exec::inline_exception_handling_t result2 = xio::query(ex1, Param());
  XIO_CHECK(result2 == ExpectedResult());

  const Executor ex2 = {};
  exec::inline_exception_handling_t result3 = xio::query(ex2, Param());
  XIO_CHECK(result3 == ExpectedResult());
}

template <typename Executor, typename Param, typename ExpectedResult>
void test_constexpr_query()
{
  constexpr Executor ex1 = {};
  constexpr exec::inline_exception_handling_t result1 = xio::query(ex1, Param());
  XIO_CHECK(result1 == ExpectedResult());
}

template <typename Executor, typename Param, bool ExpectedResult>
void test_can_require()
{
  constexpr bool b1 =
    xio::can_require<Executor, Param>::value;
  XIO_CHECK(b1 == ExpectedResult);

  constexpr bool b2 =
    xio::can_require<const Executor, Param>::value;
  XIO_CHECK(b2 == ExpectedResult);

  constexpr bool b3 =
    xio::can_require<Executor&, Param>::value;
  XIO_CHECK(b3 == ExpectedResult);

  constexpr bool b4 =
    xio::can_require<const Executor&, Param>::value;
  XIO_CHECK(b4 == ExpectedResult);
}

template <typename Executor, typename Param, typename ExpectedResult>
void test_require()
{
  XIO_CHECK(
      xio::query(
        xio::require(Executor(), Param()),
        Param()) == ExpectedResult());

  Executor ex1 = {};
  XIO_CHECK(
      xio::query(
        xio::require(ex1, Param()),
        Param()) == ExpectedResult());

  const Executor ex2 = {};
  XIO_CHECK(
      xio::query(
        xio::require(ex2, Param()),
        Param()) == ExpectedResult());
}

template <typename Executor, typename Param, bool ExpectedResult>
void test_can_prefer()
{
  constexpr bool b1 =
    xio::can_prefer<Executor, Param>::value;
  XIO_CHECK(b1 == ExpectedResult);

  constexpr bool b2 =
    xio::can_prefer<const Executor, Param>::value;
  XIO_CHECK(b2 == ExpectedResult);

  constexpr bool b3 =
    xio::can_prefer<Executor&, Param>::value;
  XIO_CHECK(b3 == ExpectedResult);

  constexpr bool b4 =
    xio::can_prefer<const Executor&, Param>::value;
  XIO_CHECK(b4 == ExpectedResult);
}

template <typename Executor, typename Param, typename ExpectedResult>
void test_prefer()
{
  XIO_CHECK(
      s(xio::query(
        xio::prefer(Executor(), Param()),
          s())) == s(ExpectedResult()));

  Executor ex1 = {};
  XIO_CHECK(
      s(xio::query(
        xio::prefer(ex1, Param()),
          s())) == s(ExpectedResult()));

  const Executor ex2 = {};
  XIO_CHECK(
      s(xio::query(
        xio::prefer(ex2, Param()),
          s())) == s(ExpectedResult()));
}

void test_vars()
{
  XIO_CHECK(s() == exec::inline_exception_handling);
  XIO_CHECK(s() != exec::inline_exception_handling.propagate);
  XIO_CHECK(s() != exec::inline_exception_handling.capture);
  XIO_CHECK(s() != exec::inline_exception_handling.terminate);
  XIO_CHECK(n1() == exec::inline_exception_handling.propagate);
  XIO_CHECK(n1() != exec::inline_exception_handling.capture);
  XIO_CHECK(n1() != exec::inline_exception_handling.terminate);
  XIO_CHECK(n2() == exec::inline_exception_handling.capture);
  XIO_CHECK(n2() != exec::inline_exception_handling.propagate);
  XIO_CHECK(n2() != exec::inline_exception_handling.terminate);
  XIO_CHECK(n3() == exec::inline_exception_handling.terminate);
  XIO_CHECK(n3() != exec::inline_exception_handling.propagate);
  XIO_CHECK(n3() != exec::inline_exception_handling.capture);
}

XIO_TEST_SUITE
(
  "inline_exception_handling",

  XIO_TEST_CASE3(test_can_query<ex_nq_nr, s, true>)
  XIO_TEST_CASE3(test_can_query<ex_nq_nr, n1, true>)
  XIO_TEST_CASE3(test_can_query<ex_nq_nr, n2, false>)
  XIO_TEST_CASE3(test_can_query<ex_nq_nr, n3, false>)

  XIO_TEST_CASE3(test_query<ex_nq_nr, s, n1>)
  XIO_TEST_CASE3(test_query<ex_nq_nr, n1, n1>)

  XIO_TEST_CASE3(test_constexpr_query<ex_nq_nr, s, n1>)
  XIO_TEST_CASE3(test_constexpr_query<ex_nq_nr, n1, n1>)

  XIO_TEST_CASE3(test_can_require<ex_nq_nr, s, false>)
  XIO_TEST_CASE3(test_can_require<ex_nq_nr, n1, true>)
  XIO_TEST_CASE3(test_can_require<ex_nq_nr, n2, false>)
  XIO_TEST_CASE3(test_can_require<ex_nq_nr, n3, false>)

  XIO_TEST_CASE3(test_require<ex_nq_nr, n1, n1>)

  XIO_TEST_CASE3(test_can_prefer<ex_nq_nr, s, false>)
  XIO_TEST_CASE3(test_can_prefer<ex_nq_nr, n1, true>)
  XIO_TEST_CASE3(test_can_prefer<ex_nq_nr, n2, false>)
  XIO_TEST_CASE3(test_can_prefer<ex_nq_nr, n3, true>)

  XIO_TEST_CASE3(test_prefer<ex_nq_nr, n1, n1>)
  XIO_TEST_CASE3(test_prefer<ex_nq_nr, n3, n1>)

  XIO_TEST_CASE5(test_can_query<ex_cq_nr<s, s, n1>, s, true>)
  XIO_TEST_CASE5(test_can_query<ex_cq_nr<s, s, n1>, n1, true>)
  XIO_TEST_CASE5(test_can_query<ex_cq_nr<s, s, n1>, n2, true>)
  XIO_TEST_CASE5(test_can_query<ex_cq_nr<s, s, n1>, n3, true>)
  XIO_TEST_CASE5(test_can_query<ex_cq_nr<s, s, n2>, s, true>)
  XIO_TEST_CASE5(test_can_query<ex_cq_nr<s, s, n2>, n1, true>)
  XIO_TEST_CASE5(test_can_query<ex_cq_nr<s, s, n2>, n2, true>)
  XIO_TEST_CASE5(test_can_query<ex_cq_nr<s, s, n2>, n3, true>)
  XIO_TEST_CASE5(test_can_query<ex_cq_nr<s, s, n3>, s, true>)
  XIO_TEST_CASE5(test_can_query<ex_cq_nr<s, s, n3>, n1, true>)
  XIO_TEST_CASE5(test_can_query<ex_cq_nr<s, s, n3>, n2, true>)
  XIO_TEST_CASE5(test_can_query<ex_cq_nr<s, s, n3>, n3, true>)
  XIO_TEST_CASE5(test_can_query<ex_cq_nr<s, n1, n1>, s, true>)
  XIO_TEST_CASE5(test_can_query<ex_cq_nr<s, n1, n1>, n1, true>)
  XIO_TEST_CASE5(test_can_query<ex_cq_nr<s, n1, n1>, n2, false>)
  XIO_TEST_CASE5(test_can_query<ex_cq_nr<s, n1, n1>, n3, false>)
  XIO_TEST_CASE5(test_can_query<ex_cq_nr<s, n1, n2>, s, true>)
  XIO_TEST_CASE5(test_can_query<ex_cq_nr<s, n1, n2>, n1, true>)
  XIO_TEST_CASE5(test_can_query<ex_cq_nr<s, n1, n2>, n2, false>)
  XIO_TEST_CASE5(test_can_query<ex_cq_nr<s, n1, n2>, n3, false>)
  XIO_TEST_CASE5(test_can_query<ex_cq_nr<s, n1, n3>, s, true>)
  XIO_TEST_CASE5(test_can_query<ex_cq_nr<s, n1, n3>, n1, true>)
  XIO_TEST_CASE5(test_can_query<ex_cq_nr<s, n1, n3>, n2, false>)
  XIO_TEST_CASE5(test_can_query<ex_cq_nr<s, n1, n3>, n3, false>)
  XIO_TEST_CASE5(test_can_query<ex_cq_nr<s, n2, n1>, s, true>)
  XIO_TEST_CASE5(test_can_query<ex_cq_nr<s, n2, n1>, n1, false>)
  XIO_TEST_CASE5(test_can_query<ex_cq_nr<s, n2, n1>, n2, true>)
  XIO_TEST_CASE5(test_can_query<ex_cq_nr<s, n2, n1>, n3, false>)
  XIO_TEST_CASE5(test_can_query<ex_cq_nr<s, n2, n2>, s, true>)
  XIO_TEST_CASE5(test_can_query<ex_cq_nr<s, n2, n2>, n1, false>)
  XIO_TEST_CASE5(test_can_query<ex_cq_nr<s, n2, n2>, n2, true>)
  XIO_TEST_CASE5(test_can_query<ex_cq_nr<s, n2, n2>, n3, false>)
  XIO_TEST_CASE5(test_can_query<ex_cq_nr<s, n2, n3>, s, true>)
  XIO_TEST_CASE5(test_can_query<ex_cq_nr<s, n2, n3>, n1, false>)
  XIO_TEST_CASE5(test_can_query<ex_cq_nr<s, n2, n3>, n2, true>)
  XIO_TEST_CASE5(test_can_query<ex_cq_nr<s, n2, n3>, n3, false>)
  XIO_TEST_CASE5(test_can_query<ex_cq_nr<s, n3, n1>, s, true>)
  XIO_TEST_CASE5(test_can_query<ex_cq_nr<s, n3, n1>, n1, false>)
  XIO_TEST_CASE5(test_can_query<ex_cq_nr<s, n3, n1>, n2, false>)
  XIO_TEST_CASE5(test_can_query<ex_cq_nr<s, n3, n1>, n3, true>)
  XIO_TEST_CASE5(test_can_query<ex_cq_nr<s, n3, n2>, s, true>)
  XIO_TEST_CASE5(test_can_query<ex_cq_nr<s, n3, n2>, n1, false>)
  XIO_TEST_CASE5(test_can_query<ex_cq_nr<s, n3, n2>, n2, false>)
  XIO_TEST_CASE5(test_can_query<ex_cq_nr<s, n3, n2>, n3, true>)
  XIO_TEST_CASE5(test_can_query<ex_cq_nr<s, n3, n3>, s, true>)
  XIO_TEST_CASE5(test_can_query<ex_cq_nr<s, n3, n3>, n1, false>)
  XIO_TEST_CASE5(test_can_query<ex_cq_nr<s, n3, n3>, n2, false>)
  XIO_TEST_CASE5(test_can_query<ex_cq_nr<s, n3, n3>, n3, true>)
  XIO_TEST_CASE5(test_can_query<ex_cq_nr<n1, s, n1>, s, true>)
  XIO_TEST_CASE5(test_can_query<ex_cq_nr<n1, s, n1>, n1, true>)
  XIO_TEST_CASE5(test_can_query<ex_cq_nr<n1, s, n1>, n2, true>)
  XIO_TEST_CASE5(test_can_query<ex_cq_nr<n1, s, n1>, n3, true>)
  XIO_TEST_CASE5(test_can_query<ex_cq_nr<n2, s, n2>, s, true>)
  XIO_TEST_CASE5(test_can_query<ex_cq_nr<n2, s, n2>, n1, true>)
  XIO_TEST_CASE5(test_can_query<ex_cq_nr<n2, s, n2>, n2, true>)
  XIO_TEST_CASE5(test_can_query<ex_cq_nr<n2, s, n2>, n3, true>)
  XIO_TEST_CASE5(test_can_query<ex_cq_nr<n3, s, n3>, s, true>)
  XIO_TEST_CASE5(test_can_query<ex_cq_nr<n3, s, n3>, n1, true>)
  XIO_TEST_CASE5(test_can_query<ex_cq_nr<n3, s, n3>, n2, true>)
  XIO_TEST_CASE5(test_can_query<ex_cq_nr<n3, s, n3>, n3, true>)

  XIO_TEST_CASE5(test_query<ex_cq_nr<s, s, n1>, s, n1>)
  XIO_TEST_CASE5(test_query<ex_cq_nr<s, s, n1>, n1, n1>)
  XIO_TEST_CASE5(test_query<ex_cq_nr<s, s, n1>, n2, n1>)
  XIO_TEST_CASE5(test_query<ex_cq_nr<s, s, n1>, n3, n1>)
  XIO_TEST_CASE5(test_query<ex_cq_nr<s, s, n2>, s, n2>)
  XIO_TEST_CASE5(test_query<ex_cq_nr<s, s, n2>, n1, n2>)
  XIO_TEST_CASE5(test_query<ex_cq_nr<s, s, n2>, n2, n2>)
  XIO_TEST_CASE5(test_query<ex_cq_nr<s, s, n2>, n3, n2>)
  XIO_TEST_CASE5(test_query<ex_cq_nr<s, s, n3>, s, n3>)
  XIO_TEST_CASE5(test_query<ex_cq_nr<s, s, n3>, n1, n3>)
  XIO_TEST_CASE5(test_query<ex_cq_nr<s, s, n3>, n2, n3>)
  XIO_TEST_CASE5(test_query<ex_cq_nr<s, s, n3>, n3, n3>)
  XIO_TEST_CASE5(test_query<ex_cq_nr<s, n1, n1>, s, n1>)
  XIO_TEST_CASE5(test_query<ex_cq_nr<s, n1, n1>, n1, n1>)
  XIO_TEST_CASE5(test_query<ex_cq_nr<s, n1, n2>, s, n2>)
  XIO_TEST_CASE5(test_query<ex_cq_nr<s, n1, n2>, n1, n2>)
  XIO_TEST_CASE5(test_query<ex_cq_nr<s, n1, n3>, s, n3>)
  XIO_TEST_CASE5(test_query<ex_cq_nr<s, n1, n3>, n1, n3>)
  XIO_TEST_CASE5(test_query<ex_cq_nr<s, n2, n1>, s, n1>)
  XIO_TEST_CASE5(test_query<ex_cq_nr<s, n2, n1>, n2, n1>)
  XIO_TEST_CASE5(test_query<ex_cq_nr<s, n2, n2>, s, n2>)
  XIO_TEST_CASE5(test_query<ex_cq_nr<s, n2, n2>, n2, n2>)
  XIO_TEST_CASE5(test_query<ex_cq_nr<s, n2, n3>, s, n3>)
  XIO_TEST_CASE5(test_query<ex_cq_nr<s, n2, n3>, n2, n3>)
  XIO_TEST_CASE5(test_query<ex_cq_nr<s, n3, n1>, s, n1>)
  XIO_TEST_CASE5(test_query<ex_cq_nr<s, n3, n1>, n3, n1>)
  XIO_TEST_CASE5(test_query<ex_cq_nr<s, n3, n2>, s, n2>)
  XIO_TEST_CASE5(test_query<ex_cq_nr<s, n3, n2>, n3, n2>)
  XIO_TEST_CASE5(test_query<ex_cq_nr<s, n3, n3>, s, n3>)
  XIO_TEST_CASE5(test_query<ex_cq_nr<s, n3, n3>, n3, n3>)
  XIO_TEST_CASE5(test_query<ex_cq_nr<n1, s, n1>, s, n1>)
  XIO_TEST_CASE5(test_query<ex_cq_nr<n1, s, n1>, n1, n1>)
  XIO_TEST_CASE5(test_query<ex_cq_nr<n1, s, n1>, n2, n1>)
  XIO_TEST_CASE5(test_query<ex_cq_nr<n1, s, n1>, n3, n1>)
  XIO_TEST_CASE5(test_query<ex_cq_nr<n2, s, n2>, s, n2>)
  XIO_TEST_CASE5(test_query<ex_cq_nr<n2, s, n2>, n1, n2>)
  XIO_TEST_CASE5(test_query<ex_cq_nr<n2, s, n2>, n2, n2>)
  XIO_TEST_CASE5(test_query<ex_cq_nr<n2, s, n2>, n3, n2>)
  XIO_TEST_CASE5(test_query<ex_cq_nr<n3, s, n3>, s, n3>)
  XIO_TEST_CASE5(test_query<ex_cq_nr<n3, s, n3>, n1, n3>)
  XIO_TEST_CASE5(test_query<ex_cq_nr<n3, s, n3>, n2, n3>)
  XIO_TEST_CASE5(test_query<ex_cq_nr<n3, s, n3>, n3, n3>)

  XIO_TEST_CASE5(test_constexpr_query<ex_cq_nr<s, s, n1>, s, n1>)
  XIO_TEST_CASE5(test_constexpr_query<ex_cq_nr<s, s, n1>, n1, n1>)
  XIO_TEST_CASE5(test_constexpr_query<ex_cq_nr<s, s, n1>, n2, n1>)
  XIO_TEST_CASE5(test_constexpr_query<ex_cq_nr<s, s, n1>, n3, n1>)
  XIO_TEST_CASE5(test_constexpr_query<ex_cq_nr<s, s, n2>, s, n2>)
  XIO_TEST_CASE5(test_constexpr_query<ex_cq_nr<s, s, n2>, n1, n2>)
  XIO_TEST_CASE5(test_constexpr_query<ex_cq_nr<s, s, n2>, n2, n2>)
  XIO_TEST_CASE5(test_constexpr_query<ex_cq_nr<s, s, n2>, n3, n2>)
  XIO_TEST_CASE5(test_constexpr_query<ex_cq_nr<s, s, n3>, s, n3>)
  XIO_TEST_CASE5(test_constexpr_query<ex_cq_nr<s, s, n3>, n1, n3>)
  XIO_TEST_CASE5(test_constexpr_query<ex_cq_nr<s, s, n3>, n2, n3>)
  XIO_TEST_CASE5(test_constexpr_query<ex_cq_nr<s, s, n3>, n3, n3>)
  XIO_TEST_CASE5(test_constexpr_query<ex_cq_nr<s, n1, n1>, s, n1>)
  XIO_TEST_CASE5(test_constexpr_query<ex_cq_nr<s, n1, n1>, n1, n1>)
  XIO_TEST_CASE5(test_constexpr_query<ex_cq_nr<s, n1, n2>, s, n2>)
  XIO_TEST_CASE5(test_constexpr_query<ex_cq_nr<s, n1, n2>, n1, n2>)
  XIO_TEST_CASE5(test_constexpr_query<ex_cq_nr<s, n1, n3>, s, n3>)
  XIO_TEST_CASE5(test_constexpr_query<ex_cq_nr<s, n1, n3>, n1, n3>)
  XIO_TEST_CASE5(test_constexpr_query<ex_cq_nr<s, n2, n1>, s, n1>)
  XIO_TEST_CASE5(test_constexpr_query<ex_cq_nr<s, n2, n1>, n2, n1>)
  XIO_TEST_CASE5(test_constexpr_query<ex_cq_nr<s, n2, n2>, s, n2>)
  XIO_TEST_CASE5(test_constexpr_query<ex_cq_nr<s, n2, n2>, n2, n2>)
  XIO_TEST_CASE5(test_constexpr_query<ex_cq_nr<s, n2, n3>, s, n3>)
  XIO_TEST_CASE5(test_constexpr_query<ex_cq_nr<s, n2, n3>, n2, n3>)
  XIO_TEST_CASE5(test_constexpr_query<ex_cq_nr<s, n3, n1>, s, n1>)
  XIO_TEST_CASE5(test_constexpr_query<ex_cq_nr<s, n3, n1>, n3, n1>)
  XIO_TEST_CASE5(test_constexpr_query<ex_cq_nr<s, n3, n2>, s, n2>)
  XIO_TEST_CASE5(test_constexpr_query<ex_cq_nr<s, n3, n2>, n3, n2>)
  XIO_TEST_CASE5(test_constexpr_query<ex_cq_nr<s, n3, n3>, s, n3>)
  XIO_TEST_CASE5(test_constexpr_query<ex_cq_nr<s, n3, n3>, n3, n3>)
  XIO_TEST_CASE5(test_constexpr_query<ex_cq_nr<n1, s, n1>, s, n1>)
  XIO_TEST_CASE5(test_constexpr_query<ex_cq_nr<n1, s, n1>, n1, n1>)
  XIO_TEST_CASE5(test_constexpr_query<ex_cq_nr<n1, s, n1>, n2, n1>)
  XIO_TEST_CASE5(test_constexpr_query<ex_cq_nr<n1, s, n1>, n3, n1>)
  XIO_TEST_CASE5(test_constexpr_query<ex_cq_nr<n2, s, n2>, s, n2>)
  XIO_TEST_CASE5(test_constexpr_query<ex_cq_nr<n2, s, n2>, n1, n2>)
  XIO_TEST_CASE5(test_constexpr_query<ex_cq_nr<n2, s, n2>, n2, n2>)
  XIO_TEST_CASE5(test_constexpr_query<ex_cq_nr<n2, s, n2>, n3, n2>)
  XIO_TEST_CASE5(test_constexpr_query<ex_cq_nr<n3, s, n3>, s, n3>)
  XIO_TEST_CASE5(test_constexpr_query<ex_cq_nr<n3, s, n3>, n1, n3>)
  XIO_TEST_CASE5(test_constexpr_query<ex_cq_nr<n3, s, n3>, n2, n3>)
  XIO_TEST_CASE5(test_constexpr_query<ex_cq_nr<n3, s, n3>, n3, n3>)

  XIO_TEST_CASE5(test_can_require<ex_cq_nr<s, s, n1>, s, false>)
  XIO_TEST_CASE5(test_can_require<ex_cq_nr<s, s, n1>, n1, true>)
  XIO_TEST_CASE5(test_can_require<ex_cq_nr<s, s, n1>, n2, false>)
  XIO_TEST_CASE5(test_can_require<ex_cq_nr<s, s, n1>, n3, false>)
  XIO_TEST_CASE5(test_can_require<ex_cq_nr<s, s, n2>, s, false>)
  XIO_TEST_CASE5(test_can_require<ex_cq_nr<s, s, n2>, n1, false>)
  XIO_TEST_CASE5(test_can_require<ex_cq_nr<s, s, n2>, n2, true>)
  XIO_TEST_CASE5(test_can_require<ex_cq_nr<s, s, n2>, n3, false>)
  XIO_TEST_CASE5(test_can_require<ex_cq_nr<s, s, n3>, s, false>)
  XIO_TEST_CASE5(test_can_require<ex_cq_nr<s, s, n3>, n1, false>)
  XIO_TEST_CASE5(test_can_require<ex_cq_nr<s, s, n3>, n2, false>)
  XIO_TEST_CASE5(test_can_require<ex_cq_nr<s, s, n3>, n3, true>)
  XIO_TEST_CASE5(test_can_require<ex_cq_nr<s, n1, n1>, s, false>)
  XIO_TEST_CASE5(test_can_require<ex_cq_nr<s, n1, n1>, n1, true>)
  XIO_TEST_CASE5(test_can_require<ex_cq_nr<s, n1, n1>, n2, false>)
  XIO_TEST_CASE5(test_can_require<ex_cq_nr<s, n1, n1>, n3, false>)
  XIO_TEST_CASE5(test_can_require<ex_cq_nr<s, n1, n2>, s, false>)
  XIO_TEST_CASE5(test_can_require<ex_cq_nr<s, n1, n2>, n1, false>)
  XIO_TEST_CASE5(test_can_require<ex_cq_nr<s, n1, n2>, n2, false>)
  XIO_TEST_CASE5(test_can_require<ex_cq_nr<s, n1, n2>, n3, false>)
  XIO_TEST_CASE5(test_can_require<ex_cq_nr<s, n1, n3>, s, false>)
  XIO_TEST_CASE5(test_can_require<ex_cq_nr<s, n1, n3>, n1, false>)
  XIO_TEST_CASE5(test_can_require<ex_cq_nr<s, n1, n3>, n2, false>)
  XIO_TEST_CASE5(test_can_require<ex_cq_nr<s, n1, n3>, n3, false>)
  XIO_TEST_CASE5(test_can_require<ex_cq_nr<s, n2, n1>, s, false>)
  XIO_TEST_CASE5(test_can_require<ex_cq_nr<s, n2, n1>, n1, false>)
  XIO_TEST_CASE5(test_can_require<ex_cq_nr<s, n2, n1>, n2, false>)
  XIO_TEST_CASE5(test_can_require<ex_cq_nr<s, n2, n1>, n3, false>)
  XIO_TEST_CASE5(test_can_require<ex_cq_nr<s, n2, n2>, s, false>)
  XIO_TEST_CASE5(test_can_require<ex_cq_nr<s, n2, n2>, n1, false>)
  XIO_TEST_CASE5(test_can_require<ex_cq_nr<s, n2, n2>, n2, true>)
  XIO_TEST_CASE5(test_can_require<ex_cq_nr<s, n2, n2>, n3, false>)
  XIO_TEST_CASE5(test_can_require<ex_cq_nr<s, n2, n3>, s, false>)
  XIO_TEST_CASE5(test_can_require<ex_cq_nr<s, n2, n3>, n1, false>)
  XIO_TEST_CASE5(test_can_require<ex_cq_nr<s, n2, n3>, n2, false>)
  XIO_TEST_CASE5(test_can_require<ex_cq_nr<s, n2, n3>, n3, false>)
  XIO_TEST_CASE5(test_can_require<ex_cq_nr<s, n3, n1>, s, false>)
  XIO_TEST_CASE5(test_can_require<ex_cq_nr<s, n3, n1>, n1, false>)
  XIO_TEST_CASE5(test_can_require<ex_cq_nr<s, n3, n1>, n2, false>)
  XIO_TEST_CASE5(test_can_require<ex_cq_nr<s, n3, n1>, n3, false>)
  XIO_TEST_CASE5(test_can_require<ex_cq_nr<s, n3, n2>, s, false>)
  XIO_TEST_CASE5(test_can_require<ex_cq_nr<s, n3, n2>, n1, false>)
  XIO_TEST_CASE5(test_can_require<ex_cq_nr<s, n3, n2>, n2, false>)
  XIO_TEST_CASE5(test_can_require<ex_cq_nr<s, n3, n2>, n3, false>)
  XIO_TEST_CASE5(test_can_require<ex_cq_nr<s, n3, n3>, s, false>)
  XIO_TEST_CASE5(test_can_require<ex_cq_nr<s, n3, n3>, n1, false>)
  XIO_TEST_CASE5(test_can_require<ex_cq_nr<s, n3, n3>, n2, false>)
  XIO_TEST_CASE5(test_can_require<ex_cq_nr<s, n3, n3>, n3, true>)
  XIO_TEST_CASE5(test_can_require<ex_cq_nr<n1, s, n1>, s, false>)
  XIO_TEST_CASE5(test_can_require<ex_cq_nr<n1, s, n1>, n1, true>)
  XIO_TEST_CASE5(test_can_require<ex_cq_nr<n1, s, n1>, n2, false>)
  XIO_TEST_CASE5(test_can_require<ex_cq_nr<n1, s, n1>, n3, false>)
  XIO_TEST_CASE5(test_can_require<ex_cq_nr<n2, s, n2>, s, false>)
  XIO_TEST_CASE5(test_can_require<ex_cq_nr<n2, s, n2>, n1, false>)
  XIO_TEST_CASE5(test_can_require<ex_cq_nr<n2, s, n2>, n2, true>)
  XIO_TEST_CASE5(test_can_require<ex_cq_nr<n2, s, n2>, n3, false>)
  XIO_TEST_CASE5(test_can_require<ex_cq_nr<n3, s, n3>, s, false>)
  XIO_TEST_CASE5(test_can_require<ex_cq_nr<n3, s, n3>, n1, false>)
  XIO_TEST_CASE5(test_can_require<ex_cq_nr<n3, s, n3>, n2, false>)
  XIO_TEST_CASE5(test_can_require<ex_cq_nr<n3, s, n3>, n3, true>)

  XIO_TEST_CASE5(test_require<ex_cq_nr<s, s, n1>, n1, n1>)
  XIO_TEST_CASE5(test_require<ex_cq_nr<s, s, n2>, n2, n2>)
  XIO_TEST_CASE5(test_require<ex_cq_nr<s, s, n3>, n3, n3>)
  XIO_TEST_CASE5(test_require<ex_cq_nr<s, n1, n1>, n1, n1>)
  XIO_TEST_CASE5(test_require<ex_cq_nr<s, n2, n2>, n2, n2>)
  XIO_TEST_CASE5(test_require<ex_cq_nr<s, n3, n3>, n3, n3>)
  XIO_TEST_CASE5(test_require<ex_cq_nr<n1, s, n1>, n1, n1>)
  XIO_TEST_CASE5(test_require<ex_cq_nr<n2, s, n2>, n2, n2>)
  XIO_TEST_CASE5(test_require<ex_cq_nr<n3, s, n3>, n3, n3>)

  XIO_TEST_CASE5(test_can_prefer<ex_cq_nr<s, s, n1>, s, false>)
  XIO_TEST_CASE5(test_can_prefer<ex_cq_nr<s, s, n1>, n1, true>)
  XIO_TEST_CASE5(test_can_prefer<ex_cq_nr<s, s, n1>, n2, false>)
  XIO_TEST_CASE5(test_can_prefer<ex_cq_nr<s, s, n1>, n3, true>)
  XIO_TEST_CASE5(test_can_prefer<ex_cq_nr<s, s, n2>, s, false>)
  XIO_TEST_CASE5(test_can_prefer<ex_cq_nr<s, s, n2>, n1, true>)
  XIO_TEST_CASE5(test_can_prefer<ex_cq_nr<s, s, n2>, n2, false>)
  XIO_TEST_CASE5(test_can_prefer<ex_cq_nr<s, s, n2>, n3, true>)
  XIO_TEST_CASE5(test_can_prefer<ex_cq_nr<s, s, n3>, s, false>)
  XIO_TEST_CASE5(test_can_prefer<ex_cq_nr<s, s, n3>, n1, true>)
  XIO_TEST_CASE5(test_can_prefer<ex_cq_nr<s, s, n3>, n2, false>)
  XIO_TEST_CASE5(test_can_prefer<ex_cq_nr<s, s, n3>, n3, true>)
  XIO_TEST_CASE5(test_can_prefer<ex_cq_nr<s, n1, n1>, s, false>)
  XIO_TEST_CASE5(test_can_prefer<ex_cq_nr<s, n1, n1>, n1, true>)
  XIO_TEST_CASE5(test_can_prefer<ex_cq_nr<s, n1, n1>, n2, false>)
  XIO_TEST_CASE5(test_can_prefer<ex_cq_nr<s, n1, n1>, n3, true>)
  XIO_TEST_CASE5(test_can_prefer<ex_cq_nr<s, n1, n2>, s, false>)
  XIO_TEST_CASE5(test_can_prefer<ex_cq_nr<s, n1, n2>, n1, true>)
  XIO_TEST_CASE5(test_can_prefer<ex_cq_nr<s, n1, n2>, n2, false>)
  XIO_TEST_CASE5(test_can_prefer<ex_cq_nr<s, n1, n2>, n3, true>)
  XIO_TEST_CASE5(test_can_prefer<ex_cq_nr<s, n1, n3>, s, false>)
  XIO_TEST_CASE5(test_can_prefer<ex_cq_nr<s, n1, n3>, n1, true>)
  XIO_TEST_CASE5(test_can_prefer<ex_cq_nr<s, n1, n3>, n2, false>)
  XIO_TEST_CASE5(test_can_prefer<ex_cq_nr<s, n1, n3>, n3, true>)
  XIO_TEST_CASE5(test_can_prefer<ex_cq_nr<s, n2, n1>, s, false>)
  XIO_TEST_CASE5(test_can_prefer<ex_cq_nr<s, n2, n1>, n1, true>)
  XIO_TEST_CASE5(test_can_prefer<ex_cq_nr<s, n2, n1>, n2, false>)
  XIO_TEST_CASE5(test_can_prefer<ex_cq_nr<s, n2, n1>, n3, true>)
  XIO_TEST_CASE5(test_can_prefer<ex_cq_nr<s, n2, n2>, s, false>)
  XIO_TEST_CASE5(test_can_prefer<ex_cq_nr<s, n2, n2>, n1, true>)
  XIO_TEST_CASE5(test_can_prefer<ex_cq_nr<s, n2, n2>, n2, false>)
  XIO_TEST_CASE5(test_can_prefer<ex_cq_nr<s, n2, n2>, n3, true>)
  XIO_TEST_CASE5(test_can_prefer<ex_cq_nr<s, n2, n3>, s, false>)
  XIO_TEST_CASE5(test_can_prefer<ex_cq_nr<s, n2, n3>, n1, true>)
  XIO_TEST_CASE5(test_can_prefer<ex_cq_nr<s, n2, n3>, n2, false>)
  XIO_TEST_CASE5(test_can_prefer<ex_cq_nr<s, n2, n3>, n3, true>)
  XIO_TEST_CASE5(test_can_prefer<ex_cq_nr<s, n3, n1>, s, false>)
  XIO_TEST_CASE5(test_can_prefer<ex_cq_nr<s, n3, n1>, n1, true>)
  XIO_TEST_CASE5(test_can_prefer<ex_cq_nr<s, n3, n1>, n2, false>)
  XIO_TEST_CASE5(test_can_prefer<ex_cq_nr<s, n3, n1>, n3, true>)
  XIO_TEST_CASE5(test_can_prefer<ex_cq_nr<s, n3, n2>, s, false>)
  XIO_TEST_CASE5(test_can_prefer<ex_cq_nr<s, n3, n2>, n1, true>)
  XIO_TEST_CASE5(test_can_prefer<ex_cq_nr<s, n3, n2>, n2, false>)
  XIO_TEST_CASE5(test_can_prefer<ex_cq_nr<s, n3, n2>, n3, true>)
  XIO_TEST_CASE5(test_can_prefer<ex_cq_nr<s, n3, n3>, s, false>)
  XIO_TEST_CASE5(test_can_prefer<ex_cq_nr<s, n3, n3>, n1, true>)
  XIO_TEST_CASE5(test_can_prefer<ex_cq_nr<s, n3, n3>, n2, false>)
  XIO_TEST_CASE5(test_can_prefer<ex_cq_nr<s, n3, n3>, n3, true>)
  XIO_TEST_CASE5(test_can_prefer<ex_cq_nr<n1, s, n1>, s, false>)
  XIO_TEST_CASE5(test_can_prefer<ex_cq_nr<n1, s, n1>, n1, true>)
  XIO_TEST_CASE5(test_can_prefer<ex_cq_nr<n1, s, n1>, n2, false>)
  XIO_TEST_CASE5(test_can_prefer<ex_cq_nr<n1, s, n1>, n3, true>)
  XIO_TEST_CASE5(test_can_prefer<ex_cq_nr<n2, s, n2>, s, false>)
  XIO_TEST_CASE5(test_can_prefer<ex_cq_nr<n2, s, n2>, n1, true>)
  XIO_TEST_CASE5(test_can_prefer<ex_cq_nr<n2, s, n2>, n2, false>)
  XIO_TEST_CASE5(test_can_prefer<ex_cq_nr<n2, s, n2>, n3, true>)
  XIO_TEST_CASE5(test_can_prefer<ex_cq_nr<n3, s, n3>, s, false>)
  XIO_TEST_CASE5(test_can_prefer<ex_cq_nr<n3, s, n3>, n1, true>)
  XIO_TEST_CASE5(test_can_prefer<ex_cq_nr<n3, s, n3>, n2, false>)
  XIO_TEST_CASE5(test_can_prefer<ex_cq_nr<n3, s, n3>, n3, true>)

  XIO_TEST_CASE5(test_prefer<ex_cq_nr<s, s, n1>, n1, n1>)
  XIO_TEST_CASE5(test_prefer<ex_cq_nr<s, s, n1>, n3, n1>)
  XIO_TEST_CASE5(test_prefer<ex_cq_nr<s, s, n2>, n1, n2>)
  XIO_TEST_CASE5(test_prefer<ex_cq_nr<s, s, n2>, n3, n2>)
  XIO_TEST_CASE5(test_prefer<ex_cq_nr<s, s, n3>, n1, n3>)
  XIO_TEST_CASE5(test_prefer<ex_cq_nr<s, s, n3>, n3, n3>)
  XIO_TEST_CASE5(test_prefer<ex_cq_nr<s, n1, n1>, n1, n1>)
  XIO_TEST_CASE5(test_prefer<ex_cq_nr<s, n1, n1>, n3, n1>)
  XIO_TEST_CASE5(test_prefer<ex_cq_nr<s, n1, n2>, n1, n2>)
  XIO_TEST_CASE5(test_prefer<ex_cq_nr<s, n1, n2>, n3, n2>)
  XIO_TEST_CASE5(test_prefer<ex_cq_nr<s, n1, n3>, n1, n3>)
  XIO_TEST_CASE5(test_prefer<ex_cq_nr<s, n1, n3>, n3, n3>)
  XIO_TEST_CASE5(test_prefer<ex_cq_nr<s, n2, n1>, n1, n1>)
  XIO_TEST_CASE5(test_prefer<ex_cq_nr<s, n2, n1>, n3, n1>)
  XIO_TEST_CASE5(test_prefer<ex_cq_nr<s, n2, n2>, n1, n2>)
  XIO_TEST_CASE5(test_prefer<ex_cq_nr<s, n2, n2>, n3, n2>)
  XIO_TEST_CASE5(test_prefer<ex_cq_nr<s, n2, n3>, n1, n3>)
  XIO_TEST_CASE5(test_prefer<ex_cq_nr<s, n2, n3>, n3, n3>)
  XIO_TEST_CASE5(test_prefer<ex_cq_nr<s, n3, n1>, n1, n1>)
  XIO_TEST_CASE5(test_prefer<ex_cq_nr<s, n3, n1>, n3, n1>)
  XIO_TEST_CASE5(test_prefer<ex_cq_nr<s, n3, n2>, n1, n2>)
  XIO_TEST_CASE5(test_prefer<ex_cq_nr<s, n3, n2>, n3, n2>)
  XIO_TEST_CASE5(test_prefer<ex_cq_nr<s, n3, n3>, n1, n3>)
  XIO_TEST_CASE5(test_prefer<ex_cq_nr<s, n3, n3>, n3, n3>)
  XIO_TEST_CASE5(test_prefer<ex_cq_nr<n1, s, n1>, n1, n1>)
  XIO_TEST_CASE5(test_prefer<ex_cq_nr<n1, s, n1>, n3, n1>)
  XIO_TEST_CASE5(test_prefer<ex_cq_nr<n2, s, n2>, n1, n2>)
  XIO_TEST_CASE5(test_prefer<ex_cq_nr<n2, s, n2>, n3, n2>)
  XIO_TEST_CASE5(test_prefer<ex_cq_nr<n3, s, n3>, n1, n3>)
  XIO_TEST_CASE5(test_prefer<ex_cq_nr<n3, s, n3>, n3, n3>)

  XIO_TEST_CASE5(test_can_query<ex_mq_nr<s, s, n1>, s, true>)
  XIO_TEST_CASE5(test_can_query<ex_mq_nr<s, s, n1>, n1, true>)
  XIO_TEST_CASE5(test_can_query<ex_mq_nr<s, s, n1>, n2, true>)
  XIO_TEST_CASE5(test_can_query<ex_mq_nr<s, s, n1>, n3, true>)
  XIO_TEST_CASE5(test_can_query<ex_mq_nr<s, s, n2>, s, true>)
  XIO_TEST_CASE5(test_can_query<ex_mq_nr<s, s, n2>, n1, true>)
  XIO_TEST_CASE5(test_can_query<ex_mq_nr<s, s, n2>, n2, true>)
  XIO_TEST_CASE5(test_can_query<ex_mq_nr<s, s, n2>, n3, true>)
  XIO_TEST_CASE5(test_can_query<ex_mq_nr<s, s, n3>, s, true>)
  XIO_TEST_CASE5(test_can_query<ex_mq_nr<s, s, n3>, n1, true>)
  XIO_TEST_CASE5(test_can_query<ex_mq_nr<s, s, n3>, n2, true>)
  XIO_TEST_CASE5(test_can_query<ex_mq_nr<s, s, n3>, n3, true>)
  XIO_TEST_CASE5(test_can_query<ex_mq_nr<s, n1, n1>, s, true>)
  XIO_TEST_CASE5(test_can_query<ex_mq_nr<s, n1, n1>, n1, true>)
  XIO_TEST_CASE5(test_can_query<ex_mq_nr<s, n1, n1>, n2, false>)
  XIO_TEST_CASE5(test_can_query<ex_mq_nr<s, n1, n1>, n3, false>)
  XIO_TEST_CASE5(test_can_query<ex_mq_nr<s, n1, n2>, s, true>)
  XIO_TEST_CASE5(test_can_query<ex_mq_nr<s, n1, n2>, n1, true>)
  XIO_TEST_CASE5(test_can_query<ex_mq_nr<s, n1, n2>, n2, false>)
  XIO_TEST_CASE5(test_can_query<ex_mq_nr<s, n1, n2>, n3, false>)
  XIO_TEST_CASE5(test_can_query<ex_mq_nr<s, n1, n3>, s, true>)
  XIO_TEST_CASE5(test_can_query<ex_mq_nr<s, n1, n3>, n1, true>)
  XIO_TEST_CASE5(test_can_query<ex_mq_nr<s, n1, n3>, n2, false>)
  XIO_TEST_CASE5(test_can_query<ex_mq_nr<s, n1, n3>, n3, false>)
  XIO_TEST_CASE5(test_can_query<ex_mq_nr<s, n2, n1>, s, true>)
  XIO_TEST_CASE5(test_can_query<ex_mq_nr<s, n2, n1>, n1, false>)
  XIO_TEST_CASE5(test_can_query<ex_mq_nr<s, n2, n1>, n2, true>)
  XIO_TEST_CASE5(test_can_query<ex_mq_nr<s, n2, n1>, n3, false>)
  XIO_TEST_CASE5(test_can_query<ex_mq_nr<s, n2, n2>, s, true>)
  XIO_TEST_CASE5(test_can_query<ex_mq_nr<s, n2, n2>, n1, false>)
  XIO_TEST_CASE5(test_can_query<ex_mq_nr<s, n2, n2>, n2, true>)
  XIO_TEST_CASE5(test_can_query<ex_mq_nr<s, n2, n2>, n3, false>)
  XIO_TEST_CASE5(test_can_query<ex_mq_nr<s, n2, n3>, s, true>)
  XIO_TEST_CASE5(test_can_query<ex_mq_nr<s, n2, n3>, n1, false>)
  XIO_TEST_CASE5(test_can_query<ex_mq_nr<s, n2, n3>, n2, true>)
  XIO_TEST_CASE5(test_can_query<ex_mq_nr<s, n2, n3>, n3, false>)
  XIO_TEST_CASE5(test_can_query<ex_mq_nr<s, n3, n1>, s, true>)
  XIO_TEST_CASE5(test_can_query<ex_mq_nr<s, n3, n1>, n1, false>)
  XIO_TEST_CASE5(test_can_query<ex_mq_nr<s, n3, n1>, n2, false>)
  XIO_TEST_CASE5(test_can_query<ex_mq_nr<s, n3, n1>, n3, true>)
  XIO_TEST_CASE5(test_can_query<ex_mq_nr<s, n3, n2>, s, true>)
  XIO_TEST_CASE5(test_can_query<ex_mq_nr<s, n3, n2>, n1, false>)
  XIO_TEST_CASE5(test_can_query<ex_mq_nr<s, n3, n2>, n2, false>)
  XIO_TEST_CASE5(test_can_query<ex_mq_nr<s, n3, n2>, n3, true>)
  XIO_TEST_CASE5(test_can_query<ex_mq_nr<s, n3, n3>, s, true>)
  XIO_TEST_CASE5(test_can_query<ex_mq_nr<s, n3, n3>, n1, false>)
  XIO_TEST_CASE5(test_can_query<ex_mq_nr<s, n3, n3>, n2, false>)
  XIO_TEST_CASE5(test_can_query<ex_mq_nr<s, n3, n3>, n3, true>)
  XIO_TEST_CASE5(test_can_query<ex_mq_nr<n1, s, n1>, s, true>)
  XIO_TEST_CASE5(test_can_query<ex_mq_nr<n1, s, n1>, n1, true>)
  XIO_TEST_CASE5(test_can_query<ex_mq_nr<n1, s, n1>, n2, true>)
  XIO_TEST_CASE5(test_can_query<ex_mq_nr<n1, s, n1>, n3, true>)
  XIO_TEST_CASE5(test_can_query<ex_mq_nr<n2, s, n2>, s, true>)
  XIO_TEST_CASE5(test_can_query<ex_mq_nr<n2, s, n2>, n1, true>)
  XIO_TEST_CASE5(test_can_query<ex_mq_nr<n2, s, n2>, n2, true>)
  XIO_TEST_CASE5(test_can_query<ex_mq_nr<n2, s, n2>, n3, true>)
  XIO_TEST_CASE5(test_can_query<ex_mq_nr<n3, s, n3>, s, true>)
  XIO_TEST_CASE5(test_can_query<ex_mq_nr<n3, s, n3>, n1, true>)
  XIO_TEST_CASE5(test_can_query<ex_mq_nr<n3, s, n3>, n2, true>)
  XIO_TEST_CASE5(test_can_query<ex_mq_nr<n3, s, n3>, n3, true>)

  XIO_TEST_CASE5(test_query<ex_mq_nr<s, s, n1>, s, n1>)
  XIO_TEST_CASE5(test_query<ex_mq_nr<s, s, n1>, n1, n1>)
  XIO_TEST_CASE5(test_query<ex_mq_nr<s, s, n1>, n2, n1>)
  XIO_TEST_CASE5(test_query<ex_mq_nr<s, s, n1>, n3, n1>)
  XIO_TEST_CASE5(test_query<ex_mq_nr<s, s, n2>, s, n2>)
  XIO_TEST_CASE5(test_query<ex_mq_nr<s, s, n2>, n1, n2>)
  XIO_TEST_CASE5(test_query<ex_mq_nr<s, s, n2>, n2, n2>)
  XIO_TEST_CASE5(test_query<ex_mq_nr<s, s, n2>, n3, n2>)
  XIO_TEST_CASE5(test_query<ex_mq_nr<s, s, n3>, s, n3>)
  XIO_TEST_CASE5(test_query<ex_mq_nr<s, s, n3>, n1, n3>)
  XIO_TEST_CASE5(test_query<ex_mq_nr<s, s, n3>, n2, n3>)
  XIO_TEST_CASE5(test_query<ex_mq_nr<s, s, n3>, n3, n3>)
  XIO_TEST_CASE5(test_query<ex_mq_nr<s, n1, n1>, s, n1>)
  XIO_TEST_CASE5(test_query<ex_mq_nr<s, n1, n1>, n1, n1>)
  XIO_TEST_CASE5(test_query<ex_mq_nr<s, n1, n2>, s, n2>)
  XIO_TEST_CASE5(test_query<ex_mq_nr<s, n1, n2>, n1, n2>)
  XIO_TEST_CASE5(test_query<ex_mq_nr<s, n1, n3>, s, n3>)
  XIO_TEST_CASE5(test_query<ex_mq_nr<s, n1, n3>, n1, n3>)
  XIO_TEST_CASE5(test_query<ex_mq_nr<s, n2, n1>, s, n1>)
  XIO_TEST_CASE5(test_query<ex_mq_nr<s, n2, n1>, n2, n1>)
  XIO_TEST_CASE5(test_query<ex_mq_nr<s, n2, n2>, s, n2>)
  XIO_TEST_CASE5(test_query<ex_mq_nr<s, n2, n2>, n2, n2>)
  XIO_TEST_CASE5(test_query<ex_mq_nr<s, n2, n3>, s, n3>)
  XIO_TEST_CASE5(test_query<ex_mq_nr<s, n2, n3>, n2, n3>)
  XIO_TEST_CASE5(test_query<ex_mq_nr<s, n3, n1>, s, n1>)
  XIO_TEST_CASE5(test_query<ex_mq_nr<s, n3, n1>, n3, n1>)
  XIO_TEST_CASE5(test_query<ex_mq_nr<s, n3, n2>, s, n2>)
  XIO_TEST_CASE5(test_query<ex_mq_nr<s, n3, n2>, n3, n2>)
  XIO_TEST_CASE5(test_query<ex_mq_nr<s, n3, n3>, s, n3>)
  XIO_TEST_CASE5(test_query<ex_mq_nr<s, n3, n3>, n3, n3>)
  XIO_TEST_CASE5(test_query<ex_mq_nr<n1, s, n1>, s, n1>)
  XIO_TEST_CASE5(test_query<ex_mq_nr<n1, s, n1>, n1, n1>)
  XIO_TEST_CASE5(test_query<ex_mq_nr<n1, s, n1>, n2, n1>)
  XIO_TEST_CASE5(test_query<ex_mq_nr<n1, s, n1>, n3, n1>)
  XIO_TEST_CASE5(test_query<ex_mq_nr<n2, s, n2>, s, n2>)
  XIO_TEST_CASE5(test_query<ex_mq_nr<n2, s, n2>, n1, n2>)
  XIO_TEST_CASE5(test_query<ex_mq_nr<n2, s, n2>, n2, n2>)
  XIO_TEST_CASE5(test_query<ex_mq_nr<n2, s, n2>, n3, n2>)
  XIO_TEST_CASE5(test_query<ex_mq_nr<n3, s, n3>, s, n3>)
  XIO_TEST_CASE5(test_query<ex_mq_nr<n3, s, n3>, n1, n3>)
  XIO_TEST_CASE5(test_query<ex_mq_nr<n3, s, n3>, n2, n3>)
  XIO_TEST_CASE5(test_query<ex_mq_nr<n3, s, n3>, n3, n3>)

  XIO_TEST_CASE5(test_can_require<ex_mq_nr<s, s, n1>, s, false>)
  XIO_TEST_CASE5(test_can_require<ex_mq_nr<s, s, n1>, n1, false>)
  XIO_TEST_CASE5(test_can_require<ex_mq_nr<s, s, n1>, n2, false>)
  XIO_TEST_CASE5(test_can_require<ex_mq_nr<s, s, n1>, n3, false>)
  XIO_TEST_CASE5(test_can_require<ex_mq_nr<s, s, n2>, s, false>)
  XIO_TEST_CASE5(test_can_require<ex_mq_nr<s, s, n2>, n1, false>)
  XIO_TEST_CASE5(test_can_require<ex_mq_nr<s, s, n2>, n2, false>)
  XIO_TEST_CASE5(test_can_require<ex_mq_nr<s, s, n2>, n3, false>)
  XIO_TEST_CASE5(test_can_require<ex_mq_nr<s, s, n3>, s, false>)
  XIO_TEST_CASE5(test_can_require<ex_mq_nr<s, s, n3>, n1, false>)
  XIO_TEST_CASE5(test_can_require<ex_mq_nr<s, s, n3>, n2, false>)
  XIO_TEST_CASE5(test_can_require<ex_mq_nr<s, s, n3>, n3, false>)
  XIO_TEST_CASE5(test_can_require<ex_mq_nr<s, n1, n1>, s, false>)
  XIO_TEST_CASE5(test_can_require<ex_mq_nr<s, n1, n1>, n1, false>)
  XIO_TEST_CASE5(test_can_require<ex_mq_nr<s, n1, n1>, n2, false>)
  XIO_TEST_CASE5(test_can_require<ex_mq_nr<s, n1, n1>, n3, false>)
  XIO_TEST_CASE5(test_can_require<ex_mq_nr<s, n1, n2>, s, false>)
  XIO_TEST_CASE5(test_can_require<ex_mq_nr<s, n1, n2>, n1, false>)
  XIO_TEST_CASE5(test_can_require<ex_mq_nr<s, n1, n2>, n2, false>)
  XIO_TEST_CASE5(test_can_require<ex_mq_nr<s, n1, n2>, n3, false>)
  XIO_TEST_CASE5(test_can_require<ex_mq_nr<s, n1, n3>, s, false>)
  XIO_TEST_CASE5(test_can_require<ex_mq_nr<s, n1, n3>, n1, false>)
  XIO_TEST_CASE5(test_can_require<ex_mq_nr<s, n1, n3>, n2, false>)
  XIO_TEST_CASE5(test_can_require<ex_mq_nr<s, n1, n3>, n3, false>)
  XIO_TEST_CASE5(test_can_require<ex_mq_nr<s, n2, n1>, s, false>)
  XIO_TEST_CASE5(test_can_require<ex_mq_nr<s, n2, n1>, n1, false>)
  XIO_TEST_CASE5(test_can_require<ex_mq_nr<s, n2, n1>, n2, false>)
  XIO_TEST_CASE5(test_can_require<ex_mq_nr<s, n2, n1>, n3, false>)
  XIO_TEST_CASE5(test_can_require<ex_mq_nr<s, n2, n2>, s, false>)
  XIO_TEST_CASE5(test_can_require<ex_mq_nr<s, n2, n2>, n1, false>)
  XIO_TEST_CASE5(test_can_require<ex_mq_nr<s, n2, n2>, n2, false>)
  XIO_TEST_CASE5(test_can_require<ex_mq_nr<s, n2, n2>, n3, false>)
  XIO_TEST_CASE5(test_can_require<ex_mq_nr<s, n2, n3>, s, false>)
  XIO_TEST_CASE5(test_can_require<ex_mq_nr<s, n2, n3>, n1, false>)
  XIO_TEST_CASE5(test_can_require<ex_mq_nr<s, n2, n3>, n2, false>)
  XIO_TEST_CASE5(test_can_require<ex_mq_nr<s, n2, n3>, n3, false>)
  XIO_TEST_CASE5(test_can_require<ex_mq_nr<s, n3, n1>, s, false>)
  XIO_TEST_CASE5(test_can_require<ex_mq_nr<s, n3, n1>, n1, false>)
  XIO_TEST_CASE5(test_can_require<ex_mq_nr<s, n3, n1>, n2, false>)
  XIO_TEST_CASE5(test_can_require<ex_mq_nr<s, n3, n1>, n3, false>)
  XIO_TEST_CASE5(test_can_require<ex_mq_nr<s, n3, n2>, s, false>)
  XIO_TEST_CASE5(test_can_require<ex_mq_nr<s, n3, n2>, n1, false>)
  XIO_TEST_CASE5(test_can_require<ex_mq_nr<s, n3, n2>, n2, false>)
  XIO_TEST_CASE5(test_can_require<ex_mq_nr<s, n3, n2>, n3, false>)
  XIO_TEST_CASE5(test_can_require<ex_mq_nr<s, n3, n3>, s, false>)
  XIO_TEST_CASE5(test_can_require<ex_mq_nr<s, n3, n3>, n1, false>)
  XIO_TEST_CASE5(test_can_require<ex_mq_nr<s, n3, n3>, n2, false>)
  XIO_TEST_CASE5(test_can_require<ex_mq_nr<s, n3, n3>, n3, false>)
  XIO_TEST_CASE5(test_can_require<ex_mq_nr<n1, s, n1>, s, false>)
  XIO_TEST_CASE5(test_can_require<ex_mq_nr<n1, s, n1>, n1, false>)
  XIO_TEST_CASE5(test_can_require<ex_mq_nr<n1, s, n1>, n2, false>)
  XIO_TEST_CASE5(test_can_require<ex_mq_nr<n1, s, n1>, n3, false>)
  XIO_TEST_CASE5(test_can_require<ex_mq_nr<n2, s, n2>, s, false>)
  XIO_TEST_CASE5(test_can_require<ex_mq_nr<n2, s, n2>, n1, false>)
  XIO_TEST_CASE5(test_can_require<ex_mq_nr<n2, s, n2>, n2, false>)
  XIO_TEST_CASE5(test_can_require<ex_mq_nr<n2, s, n2>, n3, false>)
  XIO_TEST_CASE5(test_can_require<ex_mq_nr<n3, s, n3>, s, false>)
  XIO_TEST_CASE5(test_can_require<ex_mq_nr<n3, s, n3>, n1, false>)
  XIO_TEST_CASE5(test_can_require<ex_mq_nr<n3, s, n3>, n2, false>)
  XIO_TEST_CASE5(test_can_require<ex_mq_nr<n3, s, n3>, n3, false>)

  XIO_TEST_CASE5(test_can_prefer<ex_mq_nr<s, s, n1>, s, false>)
  XIO_TEST_CASE5(test_can_prefer<ex_mq_nr<s, s, n1>, n1, true>)
  XIO_TEST_CASE5(test_can_prefer<ex_mq_nr<s, s, n1>, n2, false>)
  XIO_TEST_CASE5(test_can_prefer<ex_mq_nr<s, s, n1>, n3, true>)
  XIO_TEST_CASE5(test_can_prefer<ex_mq_nr<s, s, n2>, s, false>)
  XIO_TEST_CASE5(test_can_prefer<ex_mq_nr<s, s, n2>, n1, true>)
  XIO_TEST_CASE5(test_can_prefer<ex_mq_nr<s, s, n2>, n2, false>)
  XIO_TEST_CASE5(test_can_prefer<ex_mq_nr<s, s, n2>, n3, true>)
  XIO_TEST_CASE5(test_can_prefer<ex_mq_nr<s, s, n3>, s, false>)
  XIO_TEST_CASE5(test_can_prefer<ex_mq_nr<s, s, n3>, n1, true>)
  XIO_TEST_CASE5(test_can_prefer<ex_mq_nr<s, s, n3>, n2, false>)
  XIO_TEST_CASE5(test_can_prefer<ex_mq_nr<s, s, n3>, n3, true>)
  XIO_TEST_CASE5(test_can_prefer<ex_mq_nr<s, n1, n1>, s, false>)
  XIO_TEST_CASE5(test_can_prefer<ex_mq_nr<s, n1, n1>, n1, true>)
  XIO_TEST_CASE5(test_can_prefer<ex_mq_nr<s, n1, n1>, n2, false>)
  XIO_TEST_CASE5(test_can_prefer<ex_mq_nr<s, n1, n1>, n3, true>)
  XIO_TEST_CASE5(test_can_prefer<ex_mq_nr<s, n1, n2>, s, false>)
  XIO_TEST_CASE5(test_can_prefer<ex_mq_nr<s, n1, n2>, n1, true>)
  XIO_TEST_CASE5(test_can_prefer<ex_mq_nr<s, n1, n2>, n2, false>)
  XIO_TEST_CASE5(test_can_prefer<ex_mq_nr<s, n1, n2>, n3, true>)
  XIO_TEST_CASE5(test_can_prefer<ex_mq_nr<s, n1, n3>, s, false>)
  XIO_TEST_CASE5(test_can_prefer<ex_mq_nr<s, n1, n3>, n1, true>)
  XIO_TEST_CASE5(test_can_prefer<ex_mq_nr<s, n1, n3>, n2, false>)
  XIO_TEST_CASE5(test_can_prefer<ex_mq_nr<s, n1, n3>, n3, true>)
  XIO_TEST_CASE5(test_can_prefer<ex_mq_nr<s, n2, n1>, s, false>)
  XIO_TEST_CASE5(test_can_prefer<ex_mq_nr<s, n2, n1>, n1, true>)
  XIO_TEST_CASE5(test_can_prefer<ex_mq_nr<s, n2, n1>, n2, false>)
  XIO_TEST_CASE5(test_can_prefer<ex_mq_nr<s, n2, n1>, n3, true>)
  XIO_TEST_CASE5(test_can_prefer<ex_mq_nr<s, n2, n2>, s, false>)
  XIO_TEST_CASE5(test_can_prefer<ex_mq_nr<s, n2, n2>, n1, true>)
  XIO_TEST_CASE5(test_can_prefer<ex_mq_nr<s, n2, n2>, n2, false>)
  XIO_TEST_CASE5(test_can_prefer<ex_mq_nr<s, n2, n2>, n3, true>)
  XIO_TEST_CASE5(test_can_prefer<ex_mq_nr<s, n2, n3>, s, false>)
  XIO_TEST_CASE5(test_can_prefer<ex_mq_nr<s, n2, n3>, n1, true>)
  XIO_TEST_CASE5(test_can_prefer<ex_mq_nr<s, n2, n3>, n2, false>)
  XIO_TEST_CASE5(test_can_prefer<ex_mq_nr<s, n2, n3>, n3, true>)
  XIO_TEST_CASE5(test_can_prefer<ex_mq_nr<s, n3, n1>, s, false>)
  XIO_TEST_CASE5(test_can_prefer<ex_mq_nr<s, n3, n1>, n1, true>)
  XIO_TEST_CASE5(test_can_prefer<ex_mq_nr<s, n3, n1>, n2, false>)
  XIO_TEST_CASE5(test_can_prefer<ex_mq_nr<s, n3, n1>, n3, true>)
  XIO_TEST_CASE5(test_can_prefer<ex_mq_nr<s, n3, n2>, s, false>)
  XIO_TEST_CASE5(test_can_prefer<ex_mq_nr<s, n3, n2>, n1, true>)
  XIO_TEST_CASE5(test_can_prefer<ex_mq_nr<s, n3, n2>, n2, false>)
  XIO_TEST_CASE5(test_can_prefer<ex_mq_nr<s, n3, n2>, n3, true>)
  XIO_TEST_CASE5(test_can_prefer<ex_mq_nr<s, n3, n3>, s, false>)
  XIO_TEST_CASE5(test_can_prefer<ex_mq_nr<s, n3, n3>, n1, true>)
  XIO_TEST_CASE5(test_can_prefer<ex_mq_nr<s, n3, n3>, n2, false>)
  XIO_TEST_CASE5(test_can_prefer<ex_mq_nr<s, n3, n3>, n3, true>)
  XIO_TEST_CASE5(test_can_prefer<ex_mq_nr<n1, s, n1>, s, false>)
  XIO_TEST_CASE5(test_can_prefer<ex_mq_nr<n1, s, n1>, n1, true>)
  XIO_TEST_CASE5(test_can_prefer<ex_mq_nr<n1, s, n1>, n2, false>)
  XIO_TEST_CASE5(test_can_prefer<ex_mq_nr<n1, s, n1>, n3, true>)
  XIO_TEST_CASE5(test_can_prefer<ex_mq_nr<n2, s, n2>, s, false>)
  XIO_TEST_CASE5(test_can_prefer<ex_mq_nr<n2, s, n2>, n1, true>)
  XIO_TEST_CASE5(test_can_prefer<ex_mq_nr<n2, s, n2>, n2, false>)
  XIO_TEST_CASE5(test_can_prefer<ex_mq_nr<n2, s, n2>, n3, true>)
  XIO_TEST_CASE5(test_can_prefer<ex_mq_nr<n3, s, n3>, s, false>)
  XIO_TEST_CASE5(test_can_prefer<ex_mq_nr<n3, s, n3>, n1, true>)
  XIO_TEST_CASE5(test_can_prefer<ex_mq_nr<n3, s, n3>, n2, false>)
  XIO_TEST_CASE5(test_can_prefer<ex_mq_nr<n3, s, n3>, n3, true>)

  XIO_TEST_CASE5(test_prefer<ex_mq_nr<s, s, n1>, n1, n1>)
  XIO_TEST_CASE5(test_prefer<ex_mq_nr<s, s, n1>, n3, n1>)
  XIO_TEST_CASE5(test_prefer<ex_mq_nr<s, s, n2>, n1, n2>)
  XIO_TEST_CASE5(test_prefer<ex_mq_nr<s, s, n2>, n3, n2>)
  XIO_TEST_CASE5(test_prefer<ex_mq_nr<s, s, n3>, n1, n3>)
  XIO_TEST_CASE5(test_prefer<ex_mq_nr<s, s, n3>, n3, n3>)
  XIO_TEST_CASE5(test_prefer<ex_mq_nr<s, n1, n1>, n1, n1>)
  XIO_TEST_CASE5(test_prefer<ex_mq_nr<s, n1, n1>, n3, n1>)
  XIO_TEST_CASE5(test_prefer<ex_mq_nr<s, n1, n2>, n1, n2>)
  XIO_TEST_CASE5(test_prefer<ex_mq_nr<s, n1, n2>, n3, n2>)
  XIO_TEST_CASE5(test_prefer<ex_mq_nr<s, n1, n3>, n1, n3>)
  XIO_TEST_CASE5(test_prefer<ex_mq_nr<s, n1, n3>, n3, n3>)
  XIO_TEST_CASE5(test_prefer<ex_mq_nr<s, n2, n1>, n1, n1>)
  XIO_TEST_CASE5(test_prefer<ex_mq_nr<s, n2, n1>, n3, n1>)
  XIO_TEST_CASE5(test_prefer<ex_mq_nr<s, n2, n2>, n1, n2>)
  XIO_TEST_CASE5(test_prefer<ex_mq_nr<s, n2, n2>, n3, n2>)
  XIO_TEST_CASE5(test_prefer<ex_mq_nr<s, n2, n3>, n1, n3>)
  XIO_TEST_CASE5(test_prefer<ex_mq_nr<s, n2, n3>, n3, n3>)
  XIO_TEST_CASE5(test_prefer<ex_mq_nr<s, n3, n1>, n1, n1>)
  XIO_TEST_CASE5(test_prefer<ex_mq_nr<s, n3, n1>, n3, n1>)
  XIO_TEST_CASE5(test_prefer<ex_mq_nr<s, n3, n2>, n1, n2>)
  XIO_TEST_CASE5(test_prefer<ex_mq_nr<s, n3, n2>, n3, n2>)
  XIO_TEST_CASE5(test_prefer<ex_mq_nr<s, n3, n3>, n1, n3>)
  XIO_TEST_CASE5(test_prefer<ex_mq_nr<s, n3, n3>, n3, n3>)
  XIO_TEST_CASE5(test_prefer<ex_mq_nr<n1, s, n1>, n1, n1>)
  XIO_TEST_CASE5(test_prefer<ex_mq_nr<n1, s, n1>, n3, n1>)
  XIO_TEST_CASE5(test_prefer<ex_mq_nr<n2, s, n2>, n1, n2>)
  XIO_TEST_CASE5(test_prefer<ex_mq_nr<n2, s, n2>, n3, n2>)
  XIO_TEST_CASE5(test_prefer<ex_mq_nr<n3, s, n3>, n1, n3>)
  XIO_TEST_CASE5(test_prefer<ex_mq_nr<n3, s, n3>, n3, n3>)

  XIO_TEST_CASE5(test_can_query<ex_fq_nr<s, s, n1>, s, true>)
  XIO_TEST_CASE5(test_can_query<ex_fq_nr<s, s, n1>, n1, true>)
  XIO_TEST_CASE5(test_can_query<ex_fq_nr<s, s, n1>, n2, true>)
  XIO_TEST_CASE5(test_can_query<ex_fq_nr<s, s, n1>, n3, true>)
  XIO_TEST_CASE5(test_can_query<ex_fq_nr<s, s, n2>, s, true>)
  XIO_TEST_CASE5(test_can_query<ex_fq_nr<s, s, n2>, n1, true>)
  XIO_TEST_CASE5(test_can_query<ex_fq_nr<s, s, n2>, n2, true>)
  XIO_TEST_CASE5(test_can_query<ex_fq_nr<s, s, n2>, n3, true>)
  XIO_TEST_CASE5(test_can_query<ex_fq_nr<s, s, n3>, s, true>)
  XIO_TEST_CASE5(test_can_query<ex_fq_nr<s, s, n3>, n1, true>)
  XIO_TEST_CASE5(test_can_query<ex_fq_nr<s, s, n3>, n2, true>)
  XIO_TEST_CASE5(test_can_query<ex_fq_nr<s, s, n3>, n3, true>)
  XIO_TEST_CASE5(test_can_query<ex_fq_nr<s, n1, n1>, s, true>)
  XIO_TEST_CASE5(test_can_query<ex_fq_nr<s, n1, n1>, n1, true>)
  XIO_TEST_CASE5(test_can_query<ex_fq_nr<s, n1, n1>, n2, false>)
  XIO_TEST_CASE5(test_can_query<ex_fq_nr<s, n1, n1>, n3, false>)
  XIO_TEST_CASE5(test_can_query<ex_fq_nr<s, n1, n2>, s, true>)
  XIO_TEST_CASE5(test_can_query<ex_fq_nr<s, n1, n2>, n1, true>)
  XIO_TEST_CASE5(test_can_query<ex_fq_nr<s, n1, n2>, n2, false>)
  XIO_TEST_CASE5(test_can_query<ex_fq_nr<s, n1, n2>, n3, false>)
  XIO_TEST_CASE5(test_can_query<ex_fq_nr<s, n1, n3>, s, true>)
  XIO_TEST_CASE5(test_can_query<ex_fq_nr<s, n1, n3>, n1, true>)
  XIO_TEST_CASE5(test_can_query<ex_fq_nr<s, n1, n3>, n2, false>)
  XIO_TEST_CASE5(test_can_query<ex_fq_nr<s, n1, n3>, n3, false>)
  XIO_TEST_CASE5(test_can_query<ex_fq_nr<s, n2, n1>, s, true>)
  XIO_TEST_CASE5(test_can_query<ex_fq_nr<s, n2, n1>, n1, false>)
  XIO_TEST_CASE5(test_can_query<ex_fq_nr<s, n2, n1>, n2, true>)
  XIO_TEST_CASE5(test_can_query<ex_fq_nr<s, n2, n1>, n3, false>)
  XIO_TEST_CASE5(test_can_query<ex_fq_nr<s, n2, n2>, s, true>)
  XIO_TEST_CASE5(test_can_query<ex_fq_nr<s, n2, n2>, n1, false>)
  XIO_TEST_CASE5(test_can_query<ex_fq_nr<s, n2, n2>, n2, true>)
  XIO_TEST_CASE5(test_can_query<ex_fq_nr<s, n2, n2>, n3, false>)
  XIO_TEST_CASE5(test_can_query<ex_fq_nr<s, n2, n3>, s, true>)
  XIO_TEST_CASE5(test_can_query<ex_fq_nr<s, n2, n3>, n1, false>)
  XIO_TEST_CASE5(test_can_query<ex_fq_nr<s, n2, n3>, n2, true>)
  XIO_TEST_CASE5(test_can_query<ex_fq_nr<s, n2, n3>, n3, false>)
  XIO_TEST_CASE5(test_can_query<ex_fq_nr<s, n3, n1>, s, true>)
  XIO_TEST_CASE5(test_can_query<ex_fq_nr<s, n3, n1>, n1, false>)
  XIO_TEST_CASE5(test_can_query<ex_fq_nr<s, n3, n1>, n2, false>)
  XIO_TEST_CASE5(test_can_query<ex_fq_nr<s, n3, n1>, n3, true>)
  XIO_TEST_CASE5(test_can_query<ex_fq_nr<s, n3, n2>, s, true>)
  XIO_TEST_CASE5(test_can_query<ex_fq_nr<s, n3, n2>, n1, false>)
  XIO_TEST_CASE5(test_can_query<ex_fq_nr<s, n3, n2>, n2, false>)
  XIO_TEST_CASE5(test_can_query<ex_fq_nr<s, n3, n2>, n3, true>)
  XIO_TEST_CASE5(test_can_query<ex_fq_nr<s, n3, n3>, s, true>)
  XIO_TEST_CASE5(test_can_query<ex_fq_nr<s, n3, n3>, n1, false>)
  XIO_TEST_CASE5(test_can_query<ex_fq_nr<s, n3, n3>, n2, false>)
  XIO_TEST_CASE5(test_can_query<ex_fq_nr<s, n3, n3>, n3, true>)
  XIO_TEST_CASE5(test_can_query<ex_fq_nr<n1, s, n1>, s, true>)
  XIO_TEST_CASE5(test_can_query<ex_fq_nr<n1, s, n1>, n1, true>)
  XIO_TEST_CASE5(test_can_query<ex_fq_nr<n1, s, n1>, n2, true>)
  XIO_TEST_CASE5(test_can_query<ex_fq_nr<n1, s, n1>, n3, true>)
  XIO_TEST_CASE5(test_can_query<ex_fq_nr<n2, s, n2>, s, true>)
  XIO_TEST_CASE5(test_can_query<ex_fq_nr<n2, s, n2>, n1, true>)
  XIO_TEST_CASE5(test_can_query<ex_fq_nr<n2, s, n2>, n2, true>)
  XIO_TEST_CASE5(test_can_query<ex_fq_nr<n2, s, n2>, n3, true>)
  XIO_TEST_CASE5(test_can_query<ex_fq_nr<n3, s, n3>, s, true>)
  XIO_TEST_CASE5(test_can_query<ex_fq_nr<n3, s, n3>, n1, true>)
  XIO_TEST_CASE5(test_can_query<ex_fq_nr<n3, s, n3>, n2, true>)
  XIO_TEST_CASE5(test_can_query<ex_fq_nr<n3, s, n3>, n3, true>)

  XIO_TEST_CASE5(test_query<ex_fq_nr<s, s, n1>, s, n1>)
  XIO_TEST_CASE5(test_query<ex_fq_nr<s, s, n1>, n1, n1>)
  XIO_TEST_CASE5(test_query<ex_fq_nr<s, s, n1>, n2, n1>)
  XIO_TEST_CASE5(test_query<ex_fq_nr<s, s, n1>, n3, n1>)
  XIO_TEST_CASE5(test_query<ex_fq_nr<s, s, n2>, s, n2>)
  XIO_TEST_CASE5(test_query<ex_fq_nr<s, s, n2>, n1, n2>)
  XIO_TEST_CASE5(test_query<ex_fq_nr<s, s, n2>, n2, n2>)
  XIO_TEST_CASE5(test_query<ex_fq_nr<s, s, n2>, n3, n2>)
  XIO_TEST_CASE5(test_query<ex_fq_nr<s, s, n3>, s, n3>)
  XIO_TEST_CASE5(test_query<ex_fq_nr<s, s, n3>, n1, n3>)
  XIO_TEST_CASE5(test_query<ex_fq_nr<s, s, n3>, n2, n3>)
  XIO_TEST_CASE5(test_query<ex_fq_nr<s, s, n3>, n3, n3>)
  XIO_TEST_CASE5(test_query<ex_fq_nr<s, n1, n1>, s, n1>)
  XIO_TEST_CASE5(test_query<ex_fq_nr<s, n1, n1>, n1, n1>)
  XIO_TEST_CASE5(test_query<ex_fq_nr<s, n1, n2>, s, n2>)
  XIO_TEST_CASE5(test_query<ex_fq_nr<s, n1, n2>, n1, n2>)
  XIO_TEST_CASE5(test_query<ex_fq_nr<s, n1, n3>, s, n3>)
  XIO_TEST_CASE5(test_query<ex_fq_nr<s, n1, n3>, n1, n3>)
  XIO_TEST_CASE5(test_query<ex_fq_nr<s, n2, n1>, s, n1>)
  XIO_TEST_CASE5(test_query<ex_fq_nr<s, n2, n1>, n2, n1>)
  XIO_TEST_CASE5(test_query<ex_fq_nr<s, n2, n2>, s, n2>)
  XIO_TEST_CASE5(test_query<ex_fq_nr<s, n2, n2>, n2, n2>)
  XIO_TEST_CASE5(test_query<ex_fq_nr<s, n2, n3>, s, n3>)
  XIO_TEST_CASE5(test_query<ex_fq_nr<s, n2, n3>, n2, n3>)
  XIO_TEST_CASE5(test_query<ex_fq_nr<s, n3, n1>, s, n1>)
  XIO_TEST_CASE5(test_query<ex_fq_nr<s, n3, n1>, n3, n1>)
  XIO_TEST_CASE5(test_query<ex_fq_nr<s, n3, n2>, s, n2>)
  XIO_TEST_CASE5(test_query<ex_fq_nr<s, n3, n2>, n3, n2>)
  XIO_TEST_CASE5(test_query<ex_fq_nr<s, n3, n3>, s, n3>)
  XIO_TEST_CASE5(test_query<ex_fq_nr<s, n3, n3>, n3, n3>)
  XIO_TEST_CASE5(test_query<ex_fq_nr<n1, s, n1>, s, n1>)
  XIO_TEST_CASE5(test_query<ex_fq_nr<n1, s, n1>, n1, n1>)
  XIO_TEST_CASE5(test_query<ex_fq_nr<n1, s, n1>, n2, n1>)
  XIO_TEST_CASE5(test_query<ex_fq_nr<n1, s, n1>, n3, n1>)
  XIO_TEST_CASE5(test_query<ex_fq_nr<n2, s, n2>, s, n2>)
  XIO_TEST_CASE5(test_query<ex_fq_nr<n2, s, n2>, n1, n2>)
  XIO_TEST_CASE5(test_query<ex_fq_nr<n2, s, n2>, n2, n2>)
  XIO_TEST_CASE5(test_query<ex_fq_nr<n2, s, n2>, n3, n2>)
  XIO_TEST_CASE5(test_query<ex_fq_nr<n3, s, n3>, s, n3>)
  XIO_TEST_CASE5(test_query<ex_fq_nr<n3, s, n3>, n1, n3>)
  XIO_TEST_CASE5(test_query<ex_fq_nr<n3, s, n3>, n2, n3>)
  XIO_TEST_CASE5(test_query<ex_fq_nr<n3, s, n3>, n3, n3>)

  XIO_TEST_CASE5(test_can_require<ex_fq_nr<s, s, n1>, s, false>)
  XIO_TEST_CASE5(test_can_require<ex_fq_nr<s, s, n1>, n1, false>)
  XIO_TEST_CASE5(test_can_require<ex_fq_nr<s, s, n1>, n2, false>)
  XIO_TEST_CASE5(test_can_require<ex_fq_nr<s, s, n1>, n3, false>)
  XIO_TEST_CASE5(test_can_require<ex_fq_nr<s, s, n2>, s, false>)
  XIO_TEST_CASE5(test_can_require<ex_fq_nr<s, s, n2>, n1, false>)
  XIO_TEST_CASE5(test_can_require<ex_fq_nr<s, s, n2>, n2, false>)
  XIO_TEST_CASE5(test_can_require<ex_fq_nr<s, s, n2>, n3, false>)
  XIO_TEST_CASE5(test_can_require<ex_fq_nr<s, s, n3>, s, false>)
  XIO_TEST_CASE5(test_can_require<ex_fq_nr<s, s, n3>, n1, false>)
  XIO_TEST_CASE5(test_can_require<ex_fq_nr<s, s, n3>, n2, false>)
  XIO_TEST_CASE5(test_can_require<ex_fq_nr<s, s, n3>, n3, false>)
  XIO_TEST_CASE5(test_can_require<ex_fq_nr<s, n1, n1>, s, false>)
  XIO_TEST_CASE5(test_can_require<ex_fq_nr<s, n1, n1>, n1, false>)
  XIO_TEST_CASE5(test_can_require<ex_fq_nr<s, n1, n1>, n2, false>)
  XIO_TEST_CASE5(test_can_require<ex_fq_nr<s, n1, n1>, n3, false>)
  XIO_TEST_CASE5(test_can_require<ex_fq_nr<s, n1, n2>, s, false>)
  XIO_TEST_CASE5(test_can_require<ex_fq_nr<s, n1, n2>, n1, false>)
  XIO_TEST_CASE5(test_can_require<ex_fq_nr<s, n1, n2>, n2, false>)
  XIO_TEST_CASE5(test_can_require<ex_fq_nr<s, n1, n2>, n3, false>)
  XIO_TEST_CASE5(test_can_require<ex_fq_nr<s, n1, n3>, s, false>)
  XIO_TEST_CASE5(test_can_require<ex_fq_nr<s, n1, n3>, n1, false>)
  XIO_TEST_CASE5(test_can_require<ex_fq_nr<s, n1, n3>, n2, false>)
  XIO_TEST_CASE5(test_can_require<ex_fq_nr<s, n1, n3>, n3, false>)
  XIO_TEST_CASE5(test_can_require<ex_fq_nr<s, n2, n1>, s, false>)
  XIO_TEST_CASE5(test_can_require<ex_fq_nr<s, n2, n1>, n1, false>)
  XIO_TEST_CASE5(test_can_require<ex_fq_nr<s, n2, n1>, n2, false>)
  XIO_TEST_CASE5(test_can_require<ex_fq_nr<s, n2, n1>, n3, false>)
  XIO_TEST_CASE5(test_can_require<ex_fq_nr<s, n2, n2>, s, false>)
  XIO_TEST_CASE5(test_can_require<ex_fq_nr<s, n2, n2>, n1, false>)
  XIO_TEST_CASE5(test_can_require<ex_fq_nr<s, n2, n2>, n2, false>)
  XIO_TEST_CASE5(test_can_require<ex_fq_nr<s, n2, n2>, n3, false>)
  XIO_TEST_CASE5(test_can_require<ex_fq_nr<s, n2, n3>, s, false>)
  XIO_TEST_CASE5(test_can_require<ex_fq_nr<s, n2, n3>, n1, false>)
  XIO_TEST_CASE5(test_can_require<ex_fq_nr<s, n2, n3>, n2, false>)
  XIO_TEST_CASE5(test_can_require<ex_fq_nr<s, n2, n3>, n3, false>)
  XIO_TEST_CASE5(test_can_require<ex_fq_nr<s, n3, n1>, s, false>)
  XIO_TEST_CASE5(test_can_require<ex_fq_nr<s, n3, n1>, n1, false>)
  XIO_TEST_CASE5(test_can_require<ex_fq_nr<s, n3, n1>, n2, false>)
  XIO_TEST_CASE5(test_can_require<ex_fq_nr<s, n3, n1>, n3, false>)
  XIO_TEST_CASE5(test_can_require<ex_fq_nr<s, n3, n2>, s, false>)
  XIO_TEST_CASE5(test_can_require<ex_fq_nr<s, n3, n2>, n1, false>)
  XIO_TEST_CASE5(test_can_require<ex_fq_nr<s, n3, n2>, n2, false>)
  XIO_TEST_CASE5(test_can_require<ex_fq_nr<s, n3, n2>, n3, false>)
  XIO_TEST_CASE5(test_can_require<ex_fq_nr<s, n3, n3>, s, false>)
  XIO_TEST_CASE5(test_can_require<ex_fq_nr<s, n3, n3>, n1, false>)
  XIO_TEST_CASE5(test_can_require<ex_fq_nr<s, n3, n3>, n2, false>)
  XIO_TEST_CASE5(test_can_require<ex_fq_nr<s, n3, n3>, n3, false>)
  XIO_TEST_CASE5(test_can_require<ex_fq_nr<n1, s, n1>, s, false>)
  XIO_TEST_CASE5(test_can_require<ex_fq_nr<n1, s, n1>, n1, false>)
  XIO_TEST_CASE5(test_can_require<ex_fq_nr<n1, s, n1>, n2, false>)
  XIO_TEST_CASE5(test_can_require<ex_fq_nr<n1, s, n1>, n3, false>)
  XIO_TEST_CASE5(test_can_require<ex_fq_nr<n2, s, n2>, s, false>)
  XIO_TEST_CASE5(test_can_require<ex_fq_nr<n2, s, n2>, n1, false>)
  XIO_TEST_CASE5(test_can_require<ex_fq_nr<n2, s, n2>, n2, false>)
  XIO_TEST_CASE5(test_can_require<ex_fq_nr<n2, s, n2>, n3, false>)
  XIO_TEST_CASE5(test_can_require<ex_fq_nr<n3, s, n3>, s, false>)
  XIO_TEST_CASE5(test_can_require<ex_fq_nr<n3, s, n3>, n1, false>)
  XIO_TEST_CASE5(test_can_require<ex_fq_nr<n3, s, n3>, n2, false>)
  XIO_TEST_CASE5(test_can_require<ex_fq_nr<n3, s, n3>, n3, false>)

  XIO_TEST_CASE5(test_can_prefer<ex_fq_nr<s, s, n1>, s, false>)
  XIO_TEST_CASE5(test_can_prefer<ex_fq_nr<s, s, n1>, n1, true>)
  XIO_TEST_CASE5(test_can_prefer<ex_fq_nr<s, s, n1>, n2, false>)
  XIO_TEST_CASE5(test_can_prefer<ex_fq_nr<s, s, n1>, n3, true>)
  XIO_TEST_CASE5(test_can_prefer<ex_fq_nr<s, s, n2>, s, false>)
  XIO_TEST_CASE5(test_can_prefer<ex_fq_nr<s, s, n2>, n1, true>)
  XIO_TEST_CASE5(test_can_prefer<ex_fq_nr<s, s, n2>, n2, false>)
  XIO_TEST_CASE5(test_can_prefer<ex_fq_nr<s, s, n2>, n3, true>)
  XIO_TEST_CASE5(test_can_prefer<ex_fq_nr<s, s, n3>, s, false>)
  XIO_TEST_CASE5(test_can_prefer<ex_fq_nr<s, s, n3>, n1, true>)
  XIO_TEST_CASE5(test_can_prefer<ex_fq_nr<s, s, n3>, n2, false>)
  XIO_TEST_CASE5(test_can_prefer<ex_fq_nr<s, s, n3>, n3, true>)
  XIO_TEST_CASE5(test_can_prefer<ex_fq_nr<s, n1, n1>, s, false>)
  XIO_TEST_CASE5(test_can_prefer<ex_fq_nr<s, n1, n1>, n1, true>)
  XIO_TEST_CASE5(test_can_prefer<ex_fq_nr<s, n1, n1>, n2, false>)
  XIO_TEST_CASE5(test_can_prefer<ex_fq_nr<s, n1, n1>, n3, true>)
  XIO_TEST_CASE5(test_can_prefer<ex_fq_nr<s, n1, n2>, s, false>)
  XIO_TEST_CASE5(test_can_prefer<ex_fq_nr<s, n1, n2>, n1, true>)
  XIO_TEST_CASE5(test_can_prefer<ex_fq_nr<s, n1, n2>, n2, false>)
  XIO_TEST_CASE5(test_can_prefer<ex_fq_nr<s, n1, n2>, n3, true>)
  XIO_TEST_CASE5(test_can_prefer<ex_fq_nr<s, n1, n3>, s, false>)
  XIO_TEST_CASE5(test_can_prefer<ex_fq_nr<s, n1, n3>, n1, true>)
  XIO_TEST_CASE5(test_can_prefer<ex_fq_nr<s, n1, n3>, n2, false>)
  XIO_TEST_CASE5(test_can_prefer<ex_fq_nr<s, n1, n3>, n3, true>)
  XIO_TEST_CASE5(test_can_prefer<ex_fq_nr<s, n2, n1>, s, false>)
  XIO_TEST_CASE5(test_can_prefer<ex_fq_nr<s, n2, n1>, n1, true>)
  XIO_TEST_CASE5(test_can_prefer<ex_fq_nr<s, n2, n1>, n2, false>)
  XIO_TEST_CASE5(test_can_prefer<ex_fq_nr<s, n2, n1>, n3, true>)
  XIO_TEST_CASE5(test_can_prefer<ex_fq_nr<s, n2, n2>, s, false>)
  XIO_TEST_CASE5(test_can_prefer<ex_fq_nr<s, n2, n2>, n1, true>)
  XIO_TEST_CASE5(test_can_prefer<ex_fq_nr<s, n2, n2>, n2, false>)
  XIO_TEST_CASE5(test_can_prefer<ex_fq_nr<s, n2, n2>, n3, true>)
  XIO_TEST_CASE5(test_can_prefer<ex_fq_nr<s, n2, n3>, s, false>)
  XIO_TEST_CASE5(test_can_prefer<ex_fq_nr<s, n2, n3>, n1, true>)
  XIO_TEST_CASE5(test_can_prefer<ex_fq_nr<s, n2, n3>, n2, false>)
  XIO_TEST_CASE5(test_can_prefer<ex_fq_nr<s, n2, n3>, n3, true>)
  XIO_TEST_CASE5(test_can_prefer<ex_fq_nr<s, n3, n1>, s, false>)
  XIO_TEST_CASE5(test_can_prefer<ex_fq_nr<s, n3, n1>, n1, true>)
  XIO_TEST_CASE5(test_can_prefer<ex_fq_nr<s, n3, n1>, n2, false>)
  XIO_TEST_CASE5(test_can_prefer<ex_fq_nr<s, n3, n1>, n3, true>)
  XIO_TEST_CASE5(test_can_prefer<ex_fq_nr<s, n3, n2>, s, false>)
  XIO_TEST_CASE5(test_can_prefer<ex_fq_nr<s, n3, n2>, n1, true>)
  XIO_TEST_CASE5(test_can_prefer<ex_fq_nr<s, n3, n2>, n2, false>)
  XIO_TEST_CASE5(test_can_prefer<ex_fq_nr<s, n3, n2>, n3, true>)
  XIO_TEST_CASE5(test_can_prefer<ex_fq_nr<s, n3, n3>, s, false>)
  XIO_TEST_CASE5(test_can_prefer<ex_fq_nr<s, n3, n3>, n1, true>)
  XIO_TEST_CASE5(test_can_prefer<ex_fq_nr<s, n3, n3>, n2, false>)
  XIO_TEST_CASE5(test_can_prefer<ex_fq_nr<s, n3, n3>, n3, true>)
  XIO_TEST_CASE5(test_can_prefer<ex_fq_nr<n1, s, n1>, s, false>)
  XIO_TEST_CASE5(test_can_prefer<ex_fq_nr<n1, s, n1>, n1, true>)
  XIO_TEST_CASE5(test_can_prefer<ex_fq_nr<n1, s, n1>, n2, false>)
  XIO_TEST_CASE5(test_can_prefer<ex_fq_nr<n1, s, n1>, n3, true>)
  XIO_TEST_CASE5(test_can_prefer<ex_fq_nr<n2, s, n2>, s, false>)
  XIO_TEST_CASE5(test_can_prefer<ex_fq_nr<n2, s, n2>, n1, true>)
  XIO_TEST_CASE5(test_can_prefer<ex_fq_nr<n2, s, n2>, n2, false>)
  XIO_TEST_CASE5(test_can_prefer<ex_fq_nr<n2, s, n2>, n3, true>)
  XIO_TEST_CASE5(test_can_prefer<ex_fq_nr<n3, s, n3>, s, false>)
  XIO_TEST_CASE5(test_can_prefer<ex_fq_nr<n3, s, n3>, n1, true>)
  XIO_TEST_CASE5(test_can_prefer<ex_fq_nr<n3, s, n3>, n2, false>)
  XIO_TEST_CASE5(test_can_prefer<ex_fq_nr<n3, s, n3>, n3, true>)

  XIO_TEST_CASE5(test_prefer<ex_fq_nr<s, s, n1>, n1, n1>)
  XIO_TEST_CASE5(test_prefer<ex_fq_nr<s, s, n1>, n3, n1>)
  XIO_TEST_CASE5(test_prefer<ex_fq_nr<s, s, n2>, n1, n2>)
  XIO_TEST_CASE5(test_prefer<ex_fq_nr<s, s, n2>, n3, n2>)
  XIO_TEST_CASE5(test_prefer<ex_fq_nr<s, s, n3>, n1, n3>)
  XIO_TEST_CASE5(test_prefer<ex_fq_nr<s, s, n3>, n3, n3>)
  XIO_TEST_CASE5(test_prefer<ex_fq_nr<s, n1, n1>, n1, n1>)
  XIO_TEST_CASE5(test_prefer<ex_fq_nr<s, n1, n1>, n3, n1>)
  XIO_TEST_CASE5(test_prefer<ex_fq_nr<s, n1, n2>, n1, n2>)
  XIO_TEST_CASE5(test_prefer<ex_fq_nr<s, n1, n2>, n3, n2>)
  XIO_TEST_CASE5(test_prefer<ex_fq_nr<s, n1, n3>, n1, n3>)
  XIO_TEST_CASE5(test_prefer<ex_fq_nr<s, n1, n3>, n3, n3>)
  XIO_TEST_CASE5(test_prefer<ex_fq_nr<s, n2, n1>, n1, n1>)
  XIO_TEST_CASE5(test_prefer<ex_fq_nr<s, n2, n1>, n3, n1>)
  XIO_TEST_CASE5(test_prefer<ex_fq_nr<s, n2, n2>, n1, n2>)
  XIO_TEST_CASE5(test_prefer<ex_fq_nr<s, n2, n2>, n3, n2>)
  XIO_TEST_CASE5(test_prefer<ex_fq_nr<s, n2, n3>, n1, n3>)
  XIO_TEST_CASE5(test_prefer<ex_fq_nr<s, n2, n3>, n3, n3>)
  XIO_TEST_CASE5(test_prefer<ex_fq_nr<s, n3, n1>, n1, n1>)
  XIO_TEST_CASE5(test_prefer<ex_fq_nr<s, n3, n1>, n3, n1>)
  XIO_TEST_CASE5(test_prefer<ex_fq_nr<s, n3, n2>, n1, n2>)
  XIO_TEST_CASE5(test_prefer<ex_fq_nr<s, n3, n2>, n3, n2>)
  XIO_TEST_CASE5(test_prefer<ex_fq_nr<s, n3, n3>, n1, n3>)
  XIO_TEST_CASE5(test_prefer<ex_fq_nr<s, n3, n3>, n3, n3>)
  XIO_TEST_CASE5(test_prefer<ex_fq_nr<n1, s, n1>, n1, n1>)
  XIO_TEST_CASE5(test_prefer<ex_fq_nr<n1, s, n1>, n3, n1>)
  XIO_TEST_CASE5(test_prefer<ex_fq_nr<n2, s, n2>, n1, n2>)
  XIO_TEST_CASE5(test_prefer<ex_fq_nr<n2, s, n2>, n3, n2>)
  XIO_TEST_CASE5(test_prefer<ex_fq_nr<n3, s, n3>, n1, n3>)
  XIO_TEST_CASE5(test_prefer<ex_fq_nr<n3, s, n3>, n3, n3>)

  XIO_TEST_CASE4(test_can_query<ex_mq_mr<n1, n1>, s, true>)
  XIO_TEST_CASE4(test_can_query<ex_mq_mr<n1, n1>, n1, true>)
  XIO_TEST_CASE4(test_can_query<ex_mq_mr<n1, n1>, n2, false>)
  XIO_TEST_CASE4(test_can_query<ex_mq_mr<n1, n1>, n3, false>)
  XIO_TEST_CASE4(test_can_query<ex_mq_mr<n1, n2>, s, true>)
  XIO_TEST_CASE4(test_can_query<ex_mq_mr<n1, n2>, n1, true>)
  XIO_TEST_CASE4(test_can_query<ex_mq_mr<n1, n2>, n2, true>)
  XIO_TEST_CASE4(test_can_query<ex_mq_mr<n1, n2>, n3, false>)
  XIO_TEST_CASE4(test_can_query<ex_mq_mr<n1, n3>, s, true>)
  XIO_TEST_CASE4(test_can_query<ex_mq_mr<n1, n3>, n1, true>)
  XIO_TEST_CASE4(test_can_query<ex_mq_mr<n1, n3>, n2, false>)
  XIO_TEST_CASE4(test_can_query<ex_mq_mr<n1, n3>, n3, true>)
  XIO_TEST_CASE4(test_can_query<ex_mq_mr<n2, n1>, s, true>)
  XIO_TEST_CASE4(test_can_query<ex_mq_mr<n2, n1>, n1, true>)
  XIO_TEST_CASE4(test_can_query<ex_mq_mr<n2, n1>, n2, true>)
  XIO_TEST_CASE4(test_can_query<ex_mq_mr<n2, n1>, n3, false>)
  XIO_TEST_CASE4(test_can_query<ex_mq_mr<n2, n2>, s, true>)
  XIO_TEST_CASE4(test_can_query<ex_mq_mr<n2, n2>, n1, false>)
  XIO_TEST_CASE4(test_can_query<ex_mq_mr<n2, n2>, n2, true>)
  XIO_TEST_CASE4(test_can_query<ex_mq_mr<n2, n2>, n3, false>)
  XIO_TEST_CASE4(test_can_query<ex_mq_mr<n2, n3>, s, true>)
  XIO_TEST_CASE4(test_can_query<ex_mq_mr<n2, n3>, n1, false>)
  XIO_TEST_CASE4(test_can_query<ex_mq_mr<n2, n3>, n2, true>)
  XIO_TEST_CASE4(test_can_query<ex_mq_mr<n2, n3>, n3, true>)
  XIO_TEST_CASE4(test_can_query<ex_mq_mr<n3, n1>, s, true>)
  XIO_TEST_CASE4(test_can_query<ex_mq_mr<n3, n1>, n1, true>)
  XIO_TEST_CASE4(test_can_query<ex_mq_mr<n3, n1>, n2, false>)
  XIO_TEST_CASE4(test_can_query<ex_mq_mr<n3, n1>, n3, true>)
  XIO_TEST_CASE4(test_can_query<ex_mq_mr<n3, n2>, s, true>)
  XIO_TEST_CASE4(test_can_query<ex_mq_mr<n3, n2>, n1, false>)
  XIO_TEST_CASE4(test_can_query<ex_mq_mr<n3, n2>, n2, true>)
  XIO_TEST_CASE4(test_can_query<ex_mq_mr<n3, n2>, n3, true>)
  XIO_TEST_CASE4(test_can_query<ex_mq_mr<n3, n3>, s, true>)
  XIO_TEST_CASE4(test_can_query<ex_mq_mr<n3, n3>, n1, false>)
  XIO_TEST_CASE4(test_can_query<ex_mq_mr<n3, n3>, n2, false>)
  XIO_TEST_CASE4(test_can_query<ex_mq_mr<n3, n3>, n3, true>)

  XIO_TEST_CASE4(test_query<ex_mq_mr<n1, n1>, s, n1>)
  XIO_TEST_CASE4(test_query<ex_mq_mr<n1, n1>, n1, n1>)
  XIO_TEST_CASE4(test_query<ex_mq_mr<n1, n2>, s, n1>)
  XIO_TEST_CASE4(test_query<ex_mq_mr<n1, n2>, n1, n1>)
  XIO_TEST_CASE4(test_query<ex_mq_mr<n1, n3>, s, n1>)
  XIO_TEST_CASE4(test_query<ex_mq_mr<n1, n3>, n1, n1>)
  XIO_TEST_CASE4(test_query<ex_mq_mr<n2, n1>, s, n2>)
  XIO_TEST_CASE4(test_query<ex_mq_mr<n2, n1>, n2, n2>)
  XIO_TEST_CASE4(test_query<ex_mq_mr<n2, n2>, s, n2>)
  XIO_TEST_CASE4(test_query<ex_mq_mr<n2, n2>, n2, n2>)
  XIO_TEST_CASE4(test_query<ex_mq_mr<n2, n3>, s, n2>)
  XIO_TEST_CASE4(test_query<ex_mq_mr<n2, n3>, n2, n2>)
  XIO_TEST_CASE4(test_query<ex_mq_mr<n3, n1>, s, n3>)
  XIO_TEST_CASE4(test_query<ex_mq_mr<n3, n1>, n3, n3>)
  XIO_TEST_CASE4(test_query<ex_mq_mr<n3, n2>, s, n3>)
  XIO_TEST_CASE4(test_query<ex_mq_mr<n3, n2>, n3, n3>)
  XIO_TEST_CASE4(test_query<ex_mq_mr<n3, n3>, s, n3>)
  XIO_TEST_CASE4(test_query<ex_mq_mr<n3, n3>, n3, n3>)

  XIO_TEST_CASE4(test_can_require<ex_mq_mr<n1, n1>, s, false>)
  XIO_TEST_CASE4(test_can_require<ex_mq_mr<n1, n1>, n1, true>)
  XIO_TEST_CASE4(test_can_require<ex_mq_mr<n1, n1>, n2, false>)
  XIO_TEST_CASE4(test_can_require<ex_mq_mr<n1, n1>, n3, false>)
  XIO_TEST_CASE4(test_can_require<ex_mq_mr<n1, n2>, s, false>)
  XIO_TEST_CASE4(test_can_require<ex_mq_mr<n1, n2>, n1, true>)
  XIO_TEST_CASE4(test_can_require<ex_mq_mr<n1, n2>, n2, true>)
  XIO_TEST_CASE4(test_can_require<ex_mq_mr<n1, n2>, n3, false>)
  XIO_TEST_CASE4(test_can_require<ex_mq_mr<n1, n3>, s, false>)
  XIO_TEST_CASE4(test_can_require<ex_mq_mr<n1, n3>, n1, true>)
  XIO_TEST_CASE4(test_can_require<ex_mq_mr<n1, n3>, n2, false>)
  XIO_TEST_CASE4(test_can_require<ex_mq_mr<n1, n3>, n3, true>)
  XIO_TEST_CASE4(test_can_require<ex_mq_mr<n2, n1>, s, false>)
  XIO_TEST_CASE4(test_can_require<ex_mq_mr<n2, n1>, n1, true>)
  XIO_TEST_CASE4(test_can_require<ex_mq_mr<n2, n1>, n2, true>)
  XIO_TEST_CASE4(test_can_require<ex_mq_mr<n2, n1>, n3, false>)
  XIO_TEST_CASE4(test_can_require<ex_mq_mr<n2, n2>, s, false>)
  XIO_TEST_CASE4(test_can_require<ex_mq_mr<n2, n2>, n1, false>)
  XIO_TEST_CASE4(test_can_require<ex_mq_mr<n2, n2>, n2, true>)
  XIO_TEST_CASE4(test_can_require<ex_mq_mr<n2, n2>, n3, false>)
  XIO_TEST_CASE4(test_can_require<ex_mq_mr<n2, n3>, s, false>)
  XIO_TEST_CASE4(test_can_require<ex_mq_mr<n2, n3>, n1, false>)
  XIO_TEST_CASE4(test_can_require<ex_mq_mr<n2, n3>, n2, true>)
  XIO_TEST_CASE4(test_can_require<ex_mq_mr<n2, n3>, n3, true>)
  XIO_TEST_CASE4(test_can_require<ex_mq_mr<n3, n1>, s, false>)
  XIO_TEST_CASE4(test_can_require<ex_mq_mr<n3, n1>, n1, true>)
  XIO_TEST_CASE4(test_can_require<ex_mq_mr<n3, n1>, n2, false>)
  XIO_TEST_CASE4(test_can_require<ex_mq_mr<n3, n1>, n3, true>)
  XIO_TEST_CASE4(test_can_require<ex_mq_mr<n3, n2>, s, false>)
  XIO_TEST_CASE4(test_can_require<ex_mq_mr<n3, n2>, n1, false>)
  XIO_TEST_CASE4(test_can_require<ex_mq_mr<n3, n2>, n2, true>)
  XIO_TEST_CASE4(test_can_require<ex_mq_mr<n3, n2>, n3, true>)
  XIO_TEST_CASE4(test_can_require<ex_mq_mr<n3, n3>, s, false>)
  XIO_TEST_CASE4(test_can_require<ex_mq_mr<n3, n3>, n1, false>)
  XIO_TEST_CASE4(test_can_require<ex_mq_mr<n3, n3>, n2, false>)
  XIO_TEST_CASE4(test_can_require<ex_mq_mr<n3, n3>, n3, true>)

  XIO_TEST_CASE4(test_require<ex_mq_mr<n1, n1>, n1, n1>)
  XIO_TEST_CASE4(test_require<ex_mq_mr<n1, n2>, n1, n1>)
  XIO_TEST_CASE4(test_require<ex_mq_mr<n1, n2>, n2, n2>)
  XIO_TEST_CASE4(test_require<ex_mq_mr<n1, n3>, n1, n1>)
  XIO_TEST_CASE4(test_require<ex_mq_mr<n1, n3>, n3, n3>)
  XIO_TEST_CASE4(test_require<ex_mq_mr<n2, n1>, n1, n1>)
  XIO_TEST_CASE4(test_require<ex_mq_mr<n2, n1>, n2, n2>)
  XIO_TEST_CASE4(test_require<ex_mq_mr<n2, n2>, n2, n2>)
  XIO_TEST_CASE4(test_require<ex_mq_mr<n2, n3>, n2, n2>)
  XIO_TEST_CASE4(test_require<ex_mq_mr<n2, n3>, n3, n3>)
  XIO_TEST_CASE4(test_require<ex_mq_mr<n3, n1>, n1, n1>)
  XIO_TEST_CASE4(test_require<ex_mq_mr<n3, n1>, n3, n3>)
  XIO_TEST_CASE4(test_require<ex_mq_mr<n3, n2>, n2, n2>)
  XIO_TEST_CASE4(test_require<ex_mq_mr<n3, n2>, n3, n3>)
  XIO_TEST_CASE4(test_require<ex_mq_mr<n3, n3>, n3, n3>)

  XIO_TEST_CASE4(test_can_prefer<ex_mq_mr<n1, n1>, s, false>)
  XIO_TEST_CASE4(test_can_prefer<ex_mq_mr<n1, n1>, n1, true>)
  XIO_TEST_CASE4(test_can_prefer<ex_mq_mr<n1, n1>, n2, false>)
  XIO_TEST_CASE4(test_can_prefer<ex_mq_mr<n1, n1>, n3, true>)
  XIO_TEST_CASE4(test_can_prefer<ex_mq_mr<n1, n2>, s, false>)
  XIO_TEST_CASE4(test_can_prefer<ex_mq_mr<n1, n2>, n1, true>)
  XIO_TEST_CASE4(test_can_prefer<ex_mq_mr<n1, n2>, n2, false>)
  XIO_TEST_CASE4(test_can_prefer<ex_mq_mr<n1, n2>, n3, true>)
  XIO_TEST_CASE4(test_can_prefer<ex_mq_mr<n1, n3>, s, false>)
  XIO_TEST_CASE4(test_can_prefer<ex_mq_mr<n1, n3>, n1, true>)
  XIO_TEST_CASE4(test_can_prefer<ex_mq_mr<n1, n3>, n2, false>)
  XIO_TEST_CASE4(test_can_prefer<ex_mq_mr<n1, n3>, n3, true>)
  XIO_TEST_CASE4(test_can_prefer<ex_mq_mr<n2, n1>, s, false>)
  XIO_TEST_CASE4(test_can_prefer<ex_mq_mr<n2, n1>, n1, true>)
  XIO_TEST_CASE4(test_can_prefer<ex_mq_mr<n2, n1>, n2, false>)
  XIO_TEST_CASE4(test_can_prefer<ex_mq_mr<n2, n1>, n3, true>)
  XIO_TEST_CASE4(test_can_prefer<ex_mq_mr<n2, n2>, s, false>)
  XIO_TEST_CASE4(test_can_prefer<ex_mq_mr<n2, n2>, n1, true>)
  XIO_TEST_CASE4(test_can_prefer<ex_mq_mr<n2, n2>, n2, false>)
  XIO_TEST_CASE4(test_can_prefer<ex_mq_mr<n2, n2>, n3, true>)
  XIO_TEST_CASE4(test_can_prefer<ex_mq_mr<n2, n3>, s, false>)
  XIO_TEST_CASE4(test_can_prefer<ex_mq_mr<n2, n3>, n1, true>)
  XIO_TEST_CASE4(test_can_prefer<ex_mq_mr<n2, n3>, n2, false>)
  XIO_TEST_CASE4(test_can_prefer<ex_mq_mr<n2, n3>, n3, true>)
  XIO_TEST_CASE4(test_can_prefer<ex_mq_mr<n3, n1>, s, false>)
  XIO_TEST_CASE4(test_can_prefer<ex_mq_mr<n3, n1>, n1, true>)
  XIO_TEST_CASE4(test_can_prefer<ex_mq_mr<n3, n1>, n2, false>)
  XIO_TEST_CASE4(test_can_prefer<ex_mq_mr<n3, n1>, n3, true>)
  XIO_TEST_CASE4(test_can_prefer<ex_mq_mr<n3, n2>, s, false>)
  XIO_TEST_CASE4(test_can_prefer<ex_mq_mr<n3, n2>, n1, true>)
  XIO_TEST_CASE4(test_can_prefer<ex_mq_mr<n3, n2>, n2, false>)
  XIO_TEST_CASE4(test_can_prefer<ex_mq_mr<n3, n2>, n3, true>)
  XIO_TEST_CASE4(test_can_prefer<ex_mq_mr<n3, n3>, s, false>)
  XIO_TEST_CASE4(test_can_prefer<ex_mq_mr<n3, n3>, n1, true>)
  XIO_TEST_CASE4(test_can_prefer<ex_mq_mr<n3, n3>, n2, false>)
  XIO_TEST_CASE4(test_can_prefer<ex_mq_mr<n3, n3>, n3, true>)

  XIO_TEST_CASE4(test_prefer<ex_mq_mr<n1, n1>, n1, n1>)
  XIO_TEST_CASE4(test_prefer<ex_mq_mr<n1, n1>, n3, n1>)
  XIO_TEST_CASE4(test_prefer<ex_mq_mr<n1, n2>, n1, n1>)
  XIO_TEST_CASE4(test_prefer<ex_mq_mr<n1, n2>, n3, n1>)
  XIO_TEST_CASE4(test_prefer<ex_mq_mr<n1, n3>, n1, n1>)
  XIO_TEST_CASE4(test_prefer<ex_mq_mr<n1, n3>, n3, n3>)
  XIO_TEST_CASE4(test_prefer<ex_mq_mr<n2, n1>, n1, n1>)
  XIO_TEST_CASE4(test_prefer<ex_mq_mr<n2, n1>, n3, n2>)
  XIO_TEST_CASE4(test_prefer<ex_mq_mr<n2, n2>, n1, n2>)
  XIO_TEST_CASE4(test_prefer<ex_mq_mr<n2, n2>, n3, n2>)
  XIO_TEST_CASE4(test_prefer<ex_mq_mr<n2, n3>, n1, n2>)
  XIO_TEST_CASE4(test_prefer<ex_mq_mr<n2, n3>, n3, n3>)
  XIO_TEST_CASE4(test_prefer<ex_mq_mr<n3, n1>, n1, n1>)
  XIO_TEST_CASE4(test_prefer<ex_mq_mr<n3, n1>, n3, n3>)
  XIO_TEST_CASE4(test_prefer<ex_mq_mr<n3, n2>, n1, n3>)
  XIO_TEST_CASE4(test_prefer<ex_mq_mr<n3, n2>, n3, n3>)
  XIO_TEST_CASE4(test_prefer<ex_mq_mr<n3, n3>, n1, n3>)
  XIO_TEST_CASE4(test_prefer<ex_mq_mr<n3, n3>, n3, n3>)

  XIO_TEST_CASE4(test_can_query<ex_fq_fr<n1, n1>, s, true>)
  XIO_TEST_CASE4(test_can_query<ex_fq_fr<n1, n1>, n1, true>)
  XIO_TEST_CASE4(test_can_query<ex_fq_fr<n1, n1>, n2, false>)
  XIO_TEST_CASE4(test_can_query<ex_fq_fr<n1, n1>, n3, false>)
  XIO_TEST_CASE4(test_can_query<ex_fq_fr<n1, n2>, s, true>)
  XIO_TEST_CASE4(test_can_query<ex_fq_fr<n1, n2>, n1, true>)
  XIO_TEST_CASE4(test_can_query<ex_fq_fr<n1, n2>, n2, true>)
  XIO_TEST_CASE4(test_can_query<ex_fq_fr<n1, n2>, n3, false>)
  XIO_TEST_CASE4(test_can_query<ex_fq_fr<n1, n3>, s, true>)
  XIO_TEST_CASE4(test_can_query<ex_fq_fr<n1, n3>, n1, true>)
  XIO_TEST_CASE4(test_can_query<ex_fq_fr<n1, n3>, n2, false>)
  XIO_TEST_CASE4(test_can_query<ex_fq_fr<n1, n3>, n3, true>)
  XIO_TEST_CASE4(test_can_query<ex_fq_fr<n2, n1>, s, true>)
  XIO_TEST_CASE4(test_can_query<ex_fq_fr<n2, n1>, n1, true>)
  XIO_TEST_CASE4(test_can_query<ex_fq_fr<n2, n1>, n2, true>)
  XIO_TEST_CASE4(test_can_query<ex_fq_fr<n2, n1>, n3, false>)
  XIO_TEST_CASE4(test_can_query<ex_fq_fr<n2, n2>, s, true>)
  XIO_TEST_CASE4(test_can_query<ex_fq_fr<n2, n2>, n1, false>)
  XIO_TEST_CASE4(test_can_query<ex_fq_fr<n2, n2>, n2, true>)
  XIO_TEST_CASE4(test_can_query<ex_fq_fr<n2, n2>, n3, false>)
  XIO_TEST_CASE4(test_can_query<ex_fq_fr<n2, n3>, s, true>)
  XIO_TEST_CASE4(test_can_query<ex_fq_fr<n2, n3>, n1, false>)
  XIO_TEST_CASE4(test_can_query<ex_fq_fr<n2, n3>, n2, true>)
  XIO_TEST_CASE4(test_can_query<ex_fq_fr<n2, n3>, n3, true>)
  XIO_TEST_CASE4(test_can_query<ex_fq_fr<n3, n1>, s, true>)
  XIO_TEST_CASE4(test_can_query<ex_fq_fr<n3, n1>, n1, true>)
  XIO_TEST_CASE4(test_can_query<ex_fq_fr<n3, n1>, n2, false>)
  XIO_TEST_CASE4(test_can_query<ex_fq_fr<n3, n1>, n3, true>)
  XIO_TEST_CASE4(test_can_query<ex_fq_fr<n3, n2>, s, true>)
  XIO_TEST_CASE4(test_can_query<ex_fq_fr<n3, n2>, n1, false>)
  XIO_TEST_CASE4(test_can_query<ex_fq_fr<n3, n2>, n2, true>)
  XIO_TEST_CASE4(test_can_query<ex_fq_fr<n3, n2>, n3, true>)
  XIO_TEST_CASE4(test_can_query<ex_fq_fr<n3, n3>, s, true>)
  XIO_TEST_CASE4(test_can_query<ex_fq_fr<n3, n3>, n1, false>)
  XIO_TEST_CASE4(test_can_query<ex_fq_fr<n3, n3>, n2, false>)
  XIO_TEST_CASE4(test_can_query<ex_fq_fr<n3, n3>, n3, true>)

  XIO_TEST_CASE4(test_query<ex_fq_fr<n1, n1>, s, n1>)
  XIO_TEST_CASE4(test_query<ex_fq_fr<n1, n1>, n1, n1>)
  XIO_TEST_CASE4(test_query<ex_fq_fr<n1, n2>, s, n1>)
  XIO_TEST_CASE4(test_query<ex_fq_fr<n1, n2>, n1, n1>)
  XIO_TEST_CASE4(test_query<ex_fq_fr<n1, n3>, s, n1>)
  XIO_TEST_CASE4(test_query<ex_fq_fr<n1, n3>, n1, n1>)
  XIO_TEST_CASE4(test_query<ex_fq_fr<n2, n1>, s, n2>)
  XIO_TEST_CASE4(test_query<ex_fq_fr<n2, n1>, n2, n2>)
  XIO_TEST_CASE4(test_query<ex_fq_fr<n2, n2>, s, n2>)
  XIO_TEST_CASE4(test_query<ex_fq_fr<n2, n2>, n2, n2>)
  XIO_TEST_CASE4(test_query<ex_fq_fr<n2, n3>, s, n2>)
  XIO_TEST_CASE4(test_query<ex_fq_fr<n2, n3>, n2, n2>)
  XIO_TEST_CASE4(test_query<ex_fq_fr<n3, n1>, s, n3>)
  XIO_TEST_CASE4(test_query<ex_fq_fr<n3, n1>, n3, n3>)
  XIO_TEST_CASE4(test_query<ex_fq_fr<n3, n2>, s, n3>)
  XIO_TEST_CASE4(test_query<ex_fq_fr<n3, n2>, n3, n3>)
  XIO_TEST_CASE4(test_query<ex_fq_fr<n3, n3>, s, n3>)
  XIO_TEST_CASE4(test_query<ex_fq_fr<n3, n3>, n3, n3>)

  XIO_TEST_CASE4(test_can_require<ex_fq_fr<n1, n1>, s, false>)
  XIO_TEST_CASE4(test_can_require<ex_fq_fr<n1, n1>, n1, true>)
  XIO_TEST_CASE4(test_can_require<ex_fq_fr<n1, n1>, n2, false>)
  XIO_TEST_CASE4(test_can_require<ex_fq_fr<n1, n1>, n3, false>)
  XIO_TEST_CASE4(test_can_require<ex_fq_fr<n1, n2>, s, false>)
  XIO_TEST_CASE4(test_can_require<ex_fq_fr<n1, n2>, n1, true>)
  XIO_TEST_CASE4(test_can_require<ex_fq_fr<n1, n2>, n2, true>)
  XIO_TEST_CASE4(test_can_require<ex_fq_fr<n1, n2>, n3, false>)
  XIO_TEST_CASE4(test_can_require<ex_fq_fr<n1, n3>, s, false>)
  XIO_TEST_CASE4(test_can_require<ex_fq_fr<n1, n3>, n1, true>)
  XIO_TEST_CASE4(test_can_require<ex_fq_fr<n1, n3>, n2, false>)
  XIO_TEST_CASE4(test_can_require<ex_fq_fr<n1, n3>, n3, true>)
  XIO_TEST_CASE4(test_can_require<ex_fq_fr<n2, n1>, s, false>)
  XIO_TEST_CASE4(test_can_require<ex_fq_fr<n2, n1>, n1, true>)
  XIO_TEST_CASE4(test_can_require<ex_fq_fr<n2, n1>, n2, true>)
  XIO_TEST_CASE4(test_can_require<ex_fq_fr<n2, n1>, n3, false>)
  XIO_TEST_CASE4(test_can_require<ex_fq_fr<n2, n2>, s, false>)
  XIO_TEST_CASE4(test_can_require<ex_fq_fr<n2, n2>, n1, false>)
  XIO_TEST_CASE4(test_can_require<ex_fq_fr<n2, n2>, n2, true>)
  XIO_TEST_CASE4(test_can_require<ex_fq_fr<n2, n2>, n3, false>)
  XIO_TEST_CASE4(test_can_require<ex_fq_fr<n2, n3>, s, false>)
  XIO_TEST_CASE4(test_can_require<ex_fq_fr<n2, n3>, n1, false>)
  XIO_TEST_CASE4(test_can_require<ex_fq_fr<n2, n3>, n2, true>)
  XIO_TEST_CASE4(test_can_require<ex_fq_fr<n2, n3>, n3, true>)
  XIO_TEST_CASE4(test_can_require<ex_fq_fr<n3, n1>, s, false>)
  XIO_TEST_CASE4(test_can_require<ex_fq_fr<n3, n1>, n1, true>)
  XIO_TEST_CASE4(test_can_require<ex_fq_fr<n3, n1>, n2, false>)
  XIO_TEST_CASE4(test_can_require<ex_fq_fr<n3, n1>, n3, true>)
  XIO_TEST_CASE4(test_can_require<ex_fq_fr<n3, n2>, s, false>)
  XIO_TEST_CASE4(test_can_require<ex_fq_fr<n3, n2>, n1, false>)
  XIO_TEST_CASE4(test_can_require<ex_fq_fr<n3, n2>, n2, true>)
  XIO_TEST_CASE4(test_can_require<ex_fq_fr<n3, n2>, n3, true>)
  XIO_TEST_CASE4(test_can_require<ex_fq_fr<n3, n3>, s, false>)
  XIO_TEST_CASE4(test_can_require<ex_fq_fr<n3, n3>, n1, false>)
  XIO_TEST_CASE4(test_can_require<ex_fq_fr<n3, n3>, n2, false>)
  XIO_TEST_CASE4(test_can_require<ex_fq_fr<n3, n3>, n3, true>)

  XIO_TEST_CASE4(test_require<ex_fq_fr<n1, n1>, n1, n1>)
  XIO_TEST_CASE4(test_require<ex_fq_fr<n1, n2>, n1, n1>)
  XIO_TEST_CASE4(test_require<ex_fq_fr<n1, n2>, n2, n2>)
  XIO_TEST_CASE4(test_require<ex_fq_fr<n1, n3>, n1, n1>)
  XIO_TEST_CASE4(test_require<ex_fq_fr<n1, n3>, n3, n3>)
  XIO_TEST_CASE4(test_require<ex_fq_fr<n2, n1>, n1, n1>)
  XIO_TEST_CASE4(test_require<ex_fq_fr<n2, n1>, n2, n2>)
  XIO_TEST_CASE4(test_require<ex_fq_fr<n2, n2>, n2, n2>)
  XIO_TEST_CASE4(test_require<ex_fq_fr<n2, n3>, n2, n2>)
  XIO_TEST_CASE4(test_require<ex_fq_fr<n2, n3>, n3, n3>)
  XIO_TEST_CASE4(test_require<ex_fq_fr<n3, n1>, n1, n1>)
  XIO_TEST_CASE4(test_require<ex_fq_fr<n3, n1>, n3, n3>)
  XIO_TEST_CASE4(test_require<ex_fq_fr<n3, n2>, n2, n2>)
  XIO_TEST_CASE4(test_require<ex_fq_fr<n3, n2>, n3, n3>)
  XIO_TEST_CASE4(test_require<ex_fq_fr<n3, n3>, n3, n3>)

  XIO_TEST_CASE4(test_can_prefer<ex_fq_fr<n1, n1>, s, false>)
  XIO_TEST_CASE4(test_can_prefer<ex_fq_fr<n1, n1>, n1, true>)
  XIO_TEST_CASE4(test_can_prefer<ex_fq_fr<n1, n1>, n2, false>)
  XIO_TEST_CASE4(test_can_prefer<ex_fq_fr<n1, n1>, n3, true>)
  XIO_TEST_CASE4(test_can_prefer<ex_fq_fr<n1, n2>, s, false>)
  XIO_TEST_CASE4(test_can_prefer<ex_fq_fr<n1, n2>, n1, true>)
  XIO_TEST_CASE4(test_can_prefer<ex_fq_fr<n1, n2>, n2, false>)
  XIO_TEST_CASE4(test_can_prefer<ex_fq_fr<n1, n2>, n3, true>)
  XIO_TEST_CASE4(test_can_prefer<ex_fq_fr<n1, n3>, s, false>)
  XIO_TEST_CASE4(test_can_prefer<ex_fq_fr<n1, n3>, n1, true>)
  XIO_TEST_CASE4(test_can_prefer<ex_fq_fr<n1, n3>, n2, false>)
  XIO_TEST_CASE4(test_can_prefer<ex_fq_fr<n1, n3>, n3, true>)
  XIO_TEST_CASE4(test_can_prefer<ex_fq_fr<n2, n1>, s, false>)
  XIO_TEST_CASE4(test_can_prefer<ex_fq_fr<n2, n1>, n1, true>)
  XIO_TEST_CASE4(test_can_prefer<ex_fq_fr<n2, n1>, n2, false>)
  XIO_TEST_CASE4(test_can_prefer<ex_fq_fr<n2, n1>, n3, true>)
  XIO_TEST_CASE4(test_can_prefer<ex_fq_fr<n2, n2>, s, false>)
  XIO_TEST_CASE4(test_can_prefer<ex_fq_fr<n2, n2>, n1, true>)
  XIO_TEST_CASE4(test_can_prefer<ex_fq_fr<n2, n2>, n2, false>)
  XIO_TEST_CASE4(test_can_prefer<ex_fq_fr<n2, n2>, n3, true>)
  XIO_TEST_CASE4(test_can_prefer<ex_fq_fr<n2, n3>, s, false>)
  XIO_TEST_CASE4(test_can_prefer<ex_fq_fr<n2, n3>, n1, true>)
  XIO_TEST_CASE4(test_can_prefer<ex_fq_fr<n2, n3>, n2, false>)
  XIO_TEST_CASE4(test_can_prefer<ex_fq_fr<n2, n3>, n3, true>)
  XIO_TEST_CASE4(test_can_prefer<ex_fq_fr<n3, n1>, s, false>)
  XIO_TEST_CASE4(test_can_prefer<ex_fq_fr<n3, n1>, n1, true>)
  XIO_TEST_CASE4(test_can_prefer<ex_fq_fr<n3, n1>, n2, false>)
  XIO_TEST_CASE4(test_can_prefer<ex_fq_fr<n3, n1>, n3, true>)
  XIO_TEST_CASE4(test_can_prefer<ex_fq_fr<n3, n2>, s, false>)
  XIO_TEST_CASE4(test_can_prefer<ex_fq_fr<n3, n2>, n1, true>)
  XIO_TEST_CASE4(test_can_prefer<ex_fq_fr<n3, n2>, n2, false>)
  XIO_TEST_CASE4(test_can_prefer<ex_fq_fr<n3, n2>, n3, true>)
  XIO_TEST_CASE4(test_can_prefer<ex_fq_fr<n3, n3>, s, false>)
  XIO_TEST_CASE4(test_can_prefer<ex_fq_fr<n3, n3>, n1, true>)
  XIO_TEST_CASE4(test_can_prefer<ex_fq_fr<n3, n3>, n2, false>)
  XIO_TEST_CASE4(test_can_prefer<ex_fq_fr<n3, n3>, n3, true>)

  XIO_TEST_CASE4(test_prefer<ex_fq_fr<n1, n1>, n1, n1>)
  XIO_TEST_CASE4(test_prefer<ex_fq_fr<n1, n1>, n3, n1>)
  XIO_TEST_CASE4(test_prefer<ex_fq_fr<n1, n2>, n1, n1>)
  XIO_TEST_CASE4(test_prefer<ex_fq_fr<n1, n2>, n3, n1>)
  XIO_TEST_CASE4(test_prefer<ex_fq_fr<n1, n3>, n1, n1>)
  XIO_TEST_CASE4(test_prefer<ex_fq_fr<n1, n3>, n3, n3>)
  XIO_TEST_CASE4(test_prefer<ex_fq_fr<n2, n1>, n1, n1>)
  XIO_TEST_CASE4(test_prefer<ex_fq_fr<n2, n1>, n3, n2>)
  XIO_TEST_CASE4(test_prefer<ex_fq_fr<n2, n2>, n1, n2>)
  XIO_TEST_CASE4(test_prefer<ex_fq_fr<n2, n2>, n3, n2>)
  XIO_TEST_CASE4(test_prefer<ex_fq_fr<n2, n3>, n1, n2>)
  XIO_TEST_CASE4(test_prefer<ex_fq_fr<n2, n3>, n3, n3>)
  XIO_TEST_CASE4(test_prefer<ex_fq_fr<n3, n1>, n1, n1>)
  XIO_TEST_CASE4(test_prefer<ex_fq_fr<n3, n1>, n3, n3>)
  XIO_TEST_CASE4(test_prefer<ex_fq_fr<n3, n2>, n1, n3>)
  XIO_TEST_CASE4(test_prefer<ex_fq_fr<n3, n2>, n3, n3>)
  XIO_TEST_CASE4(test_prefer<ex_fq_fr<n3, n3>, n1, n3>)
  XIO_TEST_CASE4(test_prefer<ex_fq_fr<n3, n3>, n3, n3>)

  XIO_TEST_CASE(test_vars)
)
