//
// executor_work_guard.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef XIO_EXECUTOR_WORK_GUARD_HPP
#define XIO_EXECUTOR_WORK_GUARD_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>

#include <xio/associated_executor.h>
#include <xio/detail/type_traits.h>
#include <xio/execution.h>
#include <xio/is_executor.h>

#include <xio/detail/push_options.h>

namespace xio {


#if !defined(XIO_EXECUTOR_WORK_GUARD_DECL)
#define XIO_EXECUTOR_WORK_GUARD_DECL



    template<typename Executor, typename = void, typename = void>
    class executor_work_guard;

#endif // !defined(XIO_EXECUTOR_WORK_GUARD_DECL)


#if !defined(XIO_NO_TS_EXECUTORS)

    template<typename Executor>
    class executor_work_guard<Executor,
                std::enable_if_t<
                    is_executor<Executor>::value
                > > {
    public:
        typedef Executor executor_type;

        explicit executor_work_guard(const executor_type &e) noexcept
            : executor_(e),
              owns_(true) {
            executor_.on_work_started();
        }

        executor_work_guard(const executor_work_guard &other) noexcept
            : executor_(other.executor_),
              owns_(other.owns_) {
            if (owns_)
                executor_.on_work_started();
        }

        executor_work_guard(executor_work_guard &&other) noexcept
            : executor_(static_cast<Executor &&>(other.executor_)),
              owns_(other.owns_) {
            other.owns_ = false;
        }

        ~executor_work_guard() {
            if (owns_)
                executor_.on_work_finished();
        }

        executor_type get_executor() const noexcept {
            return executor_;
        }

        bool owns_work() const noexcept {
            return owns_;
        }

        void reset() noexcept {
            if (owns_) {
                executor_.on_work_finished();
                owns_ = false;
            }
        }

    private:
        // Disallow assignment.
        executor_work_guard &operator=(const executor_work_guard &);

        executor_type executor_;
        bool owns_;
    };

#endif // !defined(XIO_NO_TS_EXECUTORS)

    template<typename Executor>
    class executor_work_guard<Executor,
        std::enable_if_t <
        !is_executor<Executor>::value
    >
    ,
    std::enable_if_t<
        execution::is_executor<Executor>::value
    >
    >
{
public:
  typedef Executor executor_type;

  explicit executor_work_guard(const executor_type& e) noexcept
    : executor_(e),
      owns_(true)
  {
    new (&work_) work_type(xio::prefer(executor_,
          execution::outstanding_work.tracked));
  }

  executor_work_guard(const executor_work_guard& other) noexcept
    : executor_(other.executor_),
      owns_(other.owns_)
  {
    if (owns_)
    {
      new (&work_) work_type(xio::prefer(executor_,
            execution::outstanding_work.tracked));
    }
  }

  executor_work_guard(executor_work_guard&& other) noexcept
    : executor_(static_cast<Executor&&>(other.executor_)),
      owns_(other.owns_)
  {
    if (owns_)
    {
      new (&work_) work_type(
          static_cast<work_type&&>(
            *static_cast<work_type*>(
              static_cast<void*>(&other.work_))));
      other.owns_ = false;
    }
  }

  ~executor_work_guard()
  {
    if (owns_)
      static_cast<work_type*>(static_cast<void*>(&work_))->~work_type();
  }

  executor_type get_executor() const noexcept
  {
    return executor_;
  }

  bool owns_work() const noexcept
  {
    return owns_;
  }

  void reset() noexcept
  {
    if (owns_)
    {
      static_cast<work_type*>(static_cast<void*>(&work_))->~work_type();
      owns_ = false;
    }
  }

private:
  // Disallow assignment.
  executor_work_guard& operator=(const executor_work_guard&);

  typedef std::decay_t<
      prefer_result_t<
        const executor_type&,
        execution::outstanding_work_t::tracked_t
      >
    > work_type;

  executor_type executor_;
  aligned_storage_t<sizeof(work_type), std::alignment_of<work_type>::value> work_;
  bool owns_;
};

    /// Create an @ref executor_work_guard object.
    /**
 * @param ex An executor.
 *
 * @returns A work guard constructed with the specified executor.
 */
    template<typename Executor>
    [[nodiscard]] inline executor_work_guard<Executor>

    make_work_guard(const Executor &ex,
                    constraint_t<
                        is_executor<Executor>::value || execution::is_executor<Executor>::value
                    > = 0) {
        return executor_work_guard<Executor>(ex);
    }

    /// Create an @ref executor_work_guard object.
    /**
 * @param ctx An execution context, from which an executor will be obtained.
 *
 * @returns A work guard constructed with the execution context's executor,
 * obtained by performing <tt>ctx.get_executor()</tt>.
 */
    template<typename ExecutionContext>
    [[nodiscard]] inline
    executor_work_guard<typename ExecutionContext::executor_type>

    make_work_guard(ExecutionContext & ctx,
                    constraint_t<
                        std::is_convertible<ExecutionContext &, execution_context &>::value
                    > = 0) {
        return executor_work_guard<typename ExecutionContext::executor_type>(
            ctx.get_executor());
    }

    /// Create an @ref executor_work_guard object.
    /**
 * @param t An arbitrary object, such as a completion handler, for which the
 * associated executor will be obtained.
 *
 * @returns A work guard constructed with the associated executor of the object
 * @c t, which is obtained as if by calling <tt>get_associated_executor(t)</tt>.
 */
    template<typename T>
    [[nodiscard]] inline
    executor_work_guard<
        typename constraint_t <
        !is_executor<T>::value
        && !execution::is_executor<T>::value
        && !std::is_convertible<T &, execution_context &>::value,
        associated_executor<T>
    >::type
    >
    make_work_guard(const T &t) {
        return executor_work_guard<associated_executor_t<T> >(
            associated_executor<T>::get(t));
    }

    /// Create an @ref executor_work_guard object.
    /**
 * @param t An arbitrary object, such as a completion handler, for which the
 * associated executor will be obtained.
 *
 * @param ex An executor to be used as the candidate object when determining the
 * associated executor.
 *
 * @returns A work guard constructed with the associated executor of the object
 * @c t, which is obtained as if by calling <tt>get_associated_executor(t,
 * ex)</tt>.
 */
    template<typename T, typename Executor>
    [[nodiscard]] inline
    executor_work_guard<associated_executor_t<T, Executor> >

    make_work_guard(const T &t, const Executor &ex,
                    constraint_t<
                        is_executor<Executor>::value || execution::is_executor<Executor>::value
                    > = 0) {
        return executor_work_guard<associated_executor_t<T, Executor> >(
            associated_executor<T, Executor>::get(t, ex));
    }

    /// Create an @ref executor_work_guard object.
    /**
 * @param t An arbitrary object, such as a completion handler, for which the
 * associated executor will be obtained.
 *
 * @param ctx An execution context, from which an executor is obtained to use as
 * the candidate object for determining the associated executor.
 *
 * @returns A work guard constructed with the associated executor of the object
 * @c t, which is obtained as if by calling <tt>get_associated_executor(t,
 * ctx.get_executor())</tt>.
 */
    template<typename T, typename ExecutionContext>
    [[nodiscard]] inline executor_work_guard<
        associated_executor_t<T, typename ExecutionContext::executor_type> >

    make_work_guard(const T &t, ExecutionContext &ctx,
                    constraint_t<
                        !is_executor<T>::value
                    > = 0,
                    constraint_t<
                        !execution::is_executor<T>::value
                    > = 0,
                    constraint_t<
                        !std::is_convertible<T &, execution_context &>::value
                    > = 0,
                    constraint_t<
                        std::is_convertible<ExecutionContext &, execution_context &>::value
                    > = 0) {
        return executor_work_guard<
            associated_executor_t<T, typename ExecutionContext::executor_type> >(
            associated_executor<T, typename ExecutionContext::executor_type>::get(
                t, ctx.get_executor()));
    }


} // namespace xio

#include <xio/detail/pop_options.h>

#endif // XIO_EXECUTOR_WORK_GUARD_HPP
