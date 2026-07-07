//
// inline_or_executor.hpp
// ~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_INLINE_OR_EXECUTOR_HPP
#define ASIO_INLINE_OR_EXECUTOR_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>
#include <exception>
#include <xio/detail/non_const_lvalue.h>
#include <xio/detail/type_traits.h>
#include <xio/execution/blocking.h>
#include <xio/execution/executor.h>
#include <xio/execution/inline_exception_handling.h>
#include <xio/execution_context.h>
#include <xio/is_executor.h>

#include <xio/detail/push_options.h>

namespace xio {


    /// Adapts an executor to add inline invocation of the submitted function.
    /**
 * The @c inline_or_executor class template adapts an existing executor such
 * that:
 *
 * @li posted function objects (or when the @c blocking property is set to
 * @c blocking.never) are submitted to the wrapped executor; and
 *
 * @li dispatched function objects (or when @c blocking is @c blocking.always or
 * @c blocking.possibly) are executed inline.
 */




    template<typename Executor,
        typename Blocking = execution::blocking_t::always_t,
        typename InlineExceptionHandling
        = execution::inline_exception_handling_t::propagate_t>
    class inline_or_executor {
    public:
        /// The type of the underlying executor.
        typedef Executor inner_executor_type;

        /// Default constructor.
        /**
   * This constructor is only valid if the underlying executor type is default
   * constructible.
   */
        inline_or_executor()
            : executor_() {
        }

        /// Construct an inline_or_executor for the specified executor.
        template<typename Executor1>
        explicit inline_or_executor(const Executor1 &e,
                                    constraint_t<
                                        std::conditional_t <
                                        !std::is_same<Executor1, inline_or_executor>::value,
                                        std::is_convertible<Executor1, Executor>,
                                        std::false_type
                                    >::value
      > = 0)
            : executor_(e) {
        }

        /// Copy constructor.
        inline_or_executor(const inline_or_executor &other) noexcept
            : executor_(other.executor_) {
        }

        /// Converting constructor.
        /**
   * This constructor is only valid if the @c OtherExecutor type is convertible
   * to @c Executor.
   */
        template<typename OtherExecutor>
        inline_or_executor(
            const inline_or_executor<OtherExecutor> &other) noexcept
            : executor_(other.executor_) {
        }

        /// Assignment operator.
        inline_or_executor &operator=(const inline_or_executor &other) noexcept {
            executor_ = other.executor_;
            return *this;
        }

        /// Converting assignment operator.
        /**
   * This assignment operator is only valid if the @c OtherExecutor type is
   * convertible to @c Executor.
   */
        template<typename OtherExecutor>
        inline_or_executor &operator=(
            const inline_or_executor<OtherExecutor> &other) noexcept {
            executor_ = other.executor_;
            return *this;
        }

        /// Move constructor.
        inline_or_executor(inline_or_executor &&other) noexcept
            : executor_(static_cast<Executor &&>(other.executor_)) {
        }

        /// Converting move constructor.
        /**
   * This constructor is only valid if the @c OtherExecutor type is convertible
   * to @c Executor.
   */
        template<typename OtherExecutor>
        inline_or_executor(inline_or_executor<OtherExecutor> &&other) noexcept
            : executor_(static_cast<OtherExecutor &&>(other.executor_)) {
        }

        /// Move assignment operator.
        inline_or_executor &operator=(inline_or_executor &&other) noexcept {
            executor_ = static_cast<Executor &&>(other.executor_);
            return *this;
        }

        /// Converting move assignment operator.
        /**
   * This assignment operator is only valid if the @c OtherExecutor type is
   * convertible to @c Executor.
   */
        template<typename OtherExecutor>
        inline_or_executor &operator=(
            inline_or_executor<OtherExecutor> &&other) noexcept {
            executor_ = static_cast<OtherExecutor &&>(other.executor_);
            return *this;
        }

        /// Destructor.
        ~inline_or_executor() noexcept {
        }

        /// Obtain the underlying executor.
        inner_executor_type get_inner_executor() const noexcept {
            return executor_;
        }

#if !defined(ASIO_NO_TS_EXECUTORS)
        /// Obtain the underlying execution context.
        execution_context &context() const noexcept {
            return executor_.context();
        }

        /// Inform the inline_or_executor that it has some outstanding work to do.
        /**
   * The inline_or_executor delegates this call to its underlying executor.
   */
        void on_work_started() const noexcept {
            executor_.on_work_started();
        }

        /// Inform the inline_or_executor that some work is no longer outstanding.
        /**
   * The inline_or_executor delegates this call to its underlying executor.
   */
        void on_work_finished() const noexcept {
            executor_.on_work_finished();
        }
#endif // !defined(ASIO_NO_TS_EXECUTORS)

#if !defined(GENERATING_DOCUMENTATION)

    private:
        friend struct XIO_VERSIONED_NAME (require_fn)::impl;
        friend struct XIO_VERSIONED_NAME (prefer_fn)::impl;
#endif // !defined(GENERATING_DOCUMENTATION)

        /// Obtain an executor with the @c blocking.possibly property.
        /**
   * Do not call this function directly. It is intended for use with the
   * xio::require customisation point.
   *
   * For example:
   * @code xio::inline_or_executor<my_executor_type> ex = ...;
   * auto ex2 = xio::require(ex1,
   *     xio::execution::blocking.possibly); @endcode
   */
        inline_or_executor<Executor, execution::blocking_t::possibly_t,
            InlineExceptionHandling>
        require(const execution::blocking_t::possibly_t &) const noexcept {
            return inline_or_executor<Executor, execution::blocking_t::possibly_t,
                InlineExceptionHandling>(executor_);
        }

        /// Obtain an executor with the @c blocking.always property.
        /**
   * Do not call this function directly. It is intended for use with the
   * xio::require customisation point.
   *
   * For example:
   * @code xio::inline_or_executor<my_executor_type> ex = ...;
   * auto ex2 = xio::require(ex1,
   *     xio::execution::blocking.always); @endcode
   */
        inline_or_executor<Executor, execution::blocking_t::always_t,
            InlineExceptionHandling>
        require(const execution::blocking_t::always_t &) const noexcept {
            return inline_or_executor<Executor, execution::blocking_t::always_t,
                InlineExceptionHandling>(executor_);
        }

        /// Obtain an executor with the @c blocking.never property.
        /**
   * Do not call this function directly. It is intended for use with the
   * xio::require customisation point.
   *
   * For example:
   * @code xio::inline_or_executor<my_executor_type> ex = ...;
   * auto ex2 = xio::require(ex1,
   *     xio::execution::blocking.never); @endcode
   */
        inline_or_executor<Executor, execution::blocking_t::never_t,
            InlineExceptionHandling>
        require(const execution::blocking_t::never_t &) const noexcept {
            return inline_or_executor<Executor, execution::blocking_t::never_t,
                InlineExceptionHandling>(executor_);
        }

        /// Obtain an executor with the @c inline_exception_handling.propagate
  /// property.
        /**
   * Do not call this function directly. It is intended for use with the
   * xio::require customisation point.
   *
   * For example:
   * @code xio::inline_or_executor<my_executor_type> ex = ...;
   * auto ex2 = xio::require(ex1,
   *     xio::execution::inline_exception_handling.propagate); @endcode
   */
        inline_or_executor<Executor, Blocking,
            execution::inline_exception_handling_t::propagate_t>
        require(const execution::inline_exception_handling_t::propagate_t &)
        const noexcept {
            return inline_or_executor<Executor, Blocking,
                execution::inline_exception_handling_t::propagate_t>(executor_);
        }

        /// Obtain an executor with the @c inline_exception_handling.terminate
  /// property.
        /**
   * Do not call this function directly. It is intended for use with the
   * xio::require customisation point.
   *
   * For example:
   * @code xio::inline_or_executor<my_executor_type> ex = ...;
   * auto ex2 = xio::require(ex1,
   *     xio::execution::inline_exception_handling.terminate); @endcode
   */
        inline_or_executor<Executor, Blocking,
            execution::inline_exception_handling_t::terminate_t>
        require(const execution::inline_exception_handling_t::terminate_t &)
        const noexcept {
            return inline_or_executor<Executor, Blocking,
                execution::inline_exception_handling_t::terminate_t>(executor_);
        }

        /// Forward a requirement to the underlying executor.
        /**
   * Do not call this function directly. It is intended for use with the
   * xio::require customisation point.
   *
   * For example:
   * @code xio::inline_or_executor<my_executor_type> ex1 = ...;
   * auto ex2 = xio::require(ex1,
   *     xio::execution::relationship.continuation); @endcode
   */
        template<typename Property>
        inline_or_executor<std::decay_t<require_result_t<const Executor &, Property> >,
            Blocking, InlineExceptionHandling>
        require(const Property &p,
                constraint_t<
                    can_require<const Executor &, Property>::value
                > = 0,
                constraint_t<
                    !std::is_convertible<Property, execution::blocking_t>::value
                > = 0,
                constraint_t<
                    !std::is_convertible<Property, execution::inline_exception_handling_t>::value
                > = 0) const
            noexcept(is_nothrow_require<const Executor &, Property>::value) {
            return inline_or_executor<
                std::decay_t<require_result_t<const Executor &, Property> >,
                Blocking, InlineExceptionHandling>(xio::require(executor_, p));
        }

        /// Forward a preference to the underlying executor.
        /**
   * Do not call this function directly. It is intended for use with the
   * xio::prefer customisation point.
   *
   * For example:
   * @code xio::inline_or_executor<my_executor_type> ex1 = ...;
   * auto ex2 = xio::prefer(ex1,
   *     xio::execution::relationship.continuation); @endcode
   */
        template<typename Property>
        inline_or_executor<std::decay_t<prefer_result_t<const Executor &, Property> >,
            Blocking, InlineExceptionHandling>
        prefer(const Property &p,
               constraint_t<
                   can_prefer<const Executor &, Property>::value
               > = 0,
               constraint_t<
                   !std::is_convertible<Property, execution::blocking_t>::value
               > = 0,
               constraint_t<
                   !std::is_convertible<Property, execution::inline_exception_handling_t>::value
               > = 0) const
            noexcept(is_nothrow_prefer<const Executor &, Property>::value) {
            return inline_or_executor<
                std::decay_t<prefer_result_t<const Executor &, Property> >,
                Blocking, InlineExceptionHandling>(xio::prefer(executor_, p));
        }

#if !defined(GENERATING_DOCUMENTATION)

    private:
        friend struct XIO_VERSIONED_NAME (query_fn)::impl;
        friend struct xio::execution::detail::blocking_t<0>;
        friend struct xio::execution::detail::inline_exception_handling_t<0>;
#endif // !defined(GENERATING_DOCUMENTATION)

        /// Query the current value of the @c blocking property.
        /**
   * Do not call this function directly. It is intended for use with the
   * xio::query customisation point.
   *
   * For example:
   * @code xio::inline_or_executor<my_executor_type> ex = ...;
   * if (xio::query(ex, xio::execution::blocking)
   *     == xio::execution::blocking.possibly)
   *   ... @endcode
   */
        static constexpr execution::blocking_t query(execution::blocking_t) noexcept {
            return Blocking();
        }

        /// Query the current value of the @c inline_exception_handling property.
        /**
   * Do not call this function directly. It is intended for use with the
   * xio::query customisation point.
   *
   * For example:
   * @code xio::inline_or_executor<my_executor_type> ex = ...;
   * if (xio::query(ex,
   *       xio::execution::inline_exception_handling)
   *     == xio::execution::inline_exception_handling.propagate)
   *   ... @endcode
   */
        static constexpr execution::inline_exception_handling_t query(
            execution::inline_exception_handling_t) noexcept {
            return InlineExceptionHandling();
        }

        /// Forward a query to the underlying executor.
        /**
   * Do not call this function directly. It is intended for use with the
   * xio::query customisation point.
   *
   * For example:
   * @code xio::inline_or_executor<my_executor_type> ex = ...;
   * if (xio::query(ex, xio::execution::blocking)
   *       == xio::execution::blocking.never)
   *   ... @endcode
   */
        template<typename Property>
        query_result_t<const Executor &, Property> query(const Property &p,
                                                         constraint_t<
                                                             can_query<const Executor &, Property>::value
                                                         > = 0,
                                                         constraint_t<
                                                             !std::is_convertible<Property, execution::blocking_t>::value
                                                         > = 0,
                                                         constraint_t<
                                                             !std::is_convertible<Property,
                                                                 execution::inline_exception_handling_t>::value
                                                         > = 0) const
            noexcept(is_nothrow_query<const Executor &, Property>::value) {
            return xio::query(executor_, p);
        }

    public:
        /// Request the inline_or_executor to invoke the given function object.
        /**
   * This function is used to ask the inline_or_executor to execute the given
   * function object. The function object will be executed inline or according
   * to the properties of the underlying executor.
   *
   * @param f The function object to be called. The executor will make
   * a copy of the handler object as required. The function signature of the
   * function object must be: @code void function(); @endcode
   */
        template<typename Function>
        constraint_t<
            traits::execute_member<const Executor &, Function>::is_valid,
            void> execute(Function &&f) const {
            this->execute_helper(static_cast<Function &&>(f), Blocking{});
        }

#if !defined(ASIO_NO_TS_EXECUTORS)
        /// Request the inline_or_executor to invoke the given function object.
        /**
   * This function is used to ask the inline_or_executor to execute the given
   * function object. The function object will be executed inside this function.
   *
   * @param f The function object to be called. The executor will make
   * a copy of the handler object as required. The function signature of the
   * function object must be: @code void function(); @endcode
   *
   * @param a An allocator that may be used by the executor to allocate the
   * internal storage needed for function invocation.
   */
        template<typename Function, typename Allocator>
        void dispatch(Function &&f, const Allocator &a) const {
            (void) a;
            detail::non_const_lvalue<Function> f2(f);
            static_cast<std::decay_t<Function> &&>(f2.value)();
        }

        /// Request the inline_or_executor to invoke the given function object.
        /**
   * This function is used to ask the executor to execute the given function
   * object. The function object will never be executed inside this function.
   * Instead, it will be scheduled by the underlying executor's post function.
   *
   * @param f The function object to be called. The executor will make
   * a copy of the handler object as required. The function signature of the
   * function object must be: @code void function(); @endcode
   *
   * @param a An allocator that may be used by the executor to allocate the
   * internal storage needed for function invocation.
   */
        template<typename Function, typename Allocator>
        void post(Function &&f, const Allocator &a) const {
            executor_.post(static_cast<Function &&>(f), a);
        }

        /// Request the inline_or_executor to invoke the given function object.
        /**
   * This function is used to ask the executor to execute the given function
   * object. The function object will never be executed inside this function.
   * Instead, it will be scheduled by the underlying executor's defer function.
   *
   * @param f The function object to be called. The executor will make
   * a copy of the handler object as required. The function signature of the
   * function object must be: @code void function(); @endcode
   *
   * @param a An allocator that may be used by the executor to allocate the
   * internal storage needed for function invocation.
   */
        template<typename Function, typename Allocator>
        void defer(Function &&f, const Allocator &a) const {
            executor_.defer(static_cast<Function &&>(f), a);
        }
#endif // !defined(ASIO_NO_TS_EXECUTORS)

        /// Compare two inline_or_executors for equality.
        /**
   * Two inline_or_executors are equal if their underlying executors are equal.
   */
        friend bool operator==(const inline_or_executor &a,
                               const inline_or_executor &b) noexcept {
            return a.executor_ == b.executor_;
        }

        /// Compare two inline_or_executors for inequality.
        /**
   * Two inline_or_executors are equal if their underlying executors are equal.
   */
        friend bool operator!=(const inline_or_executor &a,
                               const inline_or_executor &b) noexcept {
            return a.executor_ != b.executor_;
        }

#if defined(GENERATING_DOCUMENTATION)
    private:
#endif // defined(GENERATING_DOCUMENTATION)
        template<typename Function>
        void execute_helper(Function &&f, execution::blocking_t::possibly_t) const {
#if !defined(ASIO_NO_EXCEPTIONS)
            try
#endif // !defined(ASIO_NO_EXCEPTIONS)
            {
                detail::non_const_lvalue<Function> f2(f);
                static_cast<std::decay_t<Function> &&>(f2.value)();
            }
#if !defined(ASIO_NO_EXCEPTIONS)
            catch (...) {
                if (std::is_same<InlineExceptionHandling,
                    execution::inline_exception_handling_t::terminate_t>::value) {
                    std::terminate();
                } else {
                    throw;
                }
            }
#endif // !defined(ASIO_NO_EXCEPTIONS)
        }

        template<typename Function>
        void execute_helper(Function &&f, execution::blocking_t::always_t) const {
            this->execute_helper(static_cast<Function &&>(f),
                                 execution::blocking.possibly);
        }

        template<typename Function>
        void execute_helper(Function &&f, execution::blocking_t::never_t) const {
            xio::require(executor_, execution::blocking.never).execute(
                static_cast<Function &&>(f));
        }

        Executor executor_;
    };

    /** @defgroup inline_or xio::inline_or
 *
 * @brief The xio::inline_or function creates an @ref inline_or_executor
 * object for an executor or execution context.
 */
    /*@{*/

    /// Create an @ref inline_or_executor object for an executor.
    /**
 * @param ex An executor.
 *
 * @returns An inline_or_executor constructed with the specified executor.
 */
    template<typename Executor>
    inline inline_or_executor<Executor> inline_or(const Executor &ex,
                                                  constraint_t<
                                                      is_executor<Executor>::value || execution::is_executor<
                                                          Executor>::value
                                                  > = 0) {
        return inline_or_executor<Executor>(ex);
    }

    /// Create an @ref inline_or_executor object for an execution context.
    /**
 * @param ctx An execution context, from which an executor will be obtained.
 *
 * @returns An inline_or_executor constructed with the execution context's
 * executor, obtained by performing <tt>ctx.get_executor()</tt>.
 */
    template<typename ExecutionContext>
    inline inline_or_executor<typename ExecutionContext::executor_type>
    inline_or(ExecutionContext & ctx,
              constraint_t<
                  std::is_convertible<ExecutionContext &, execution_context &>::value
              > = 0) {
        return inline_or_executor<typename ExecutionContext::executor_type>(
            ctx.get_executor());
    }

    /*@}*/


} // namespace xio

#include <xio/detail/pop_options.h>

#endif // ASIO_INLINE_OR_EXECUTOR_HPP
