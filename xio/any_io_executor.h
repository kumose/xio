//
// any_io_executor.hpp
// ~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef XIO_ANY_IO_EXECUTOR_HPP
#define XIO_ANY_IO_EXECUTOR_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>
#if defined(XIO_USE_TS_EXECUTOR_AS_DEFAULT)
#include <xio/executor.h>
#else // defined(XIO_USE_TS_EXECUTOR_AS_DEFAULT)
#include <xio/execution.h>
#include <xio/execution_context.h>
#endif // defined(XIO_USE_TS_EXECUTOR_AS_DEFAULT)

#include <xio/detail/push_options.h>

namespace xio {


#if defined(XIO_USE_TS_EXECUTOR_AS_DEFAULT)

    typedef executor any_io_executor;

#else // defined(XIO_USE_TS_EXECUTOR_AS_DEFAULT)

    /// Polymorphic executor type for use with I/O objects.
    /**
 * The @c any_io_executor type is a polymorphic executor that supports the set
 * of properties required by I/O objects. It is defined as the
 * execution::any_executor class template parameterised as follows:
 * @code execution::any_executor<
 *   execution::context_as_t<execution_context&>,
 *   execution::blocking_t::never_t,
 *   execution::prefer_only<execution::blocking_t::possibly_t>,
 *   execution::prefer_only<execution::outstanding_work_t::tracked_t>,
 *   execution::prefer_only<execution::outstanding_work_t::untracked_t>,
 *   execution::prefer_only<execution::relationship_t::fork_t>,
 *   execution::prefer_only<execution::relationship_t::continuation_t>
 * > @endcode
 */
    class any_io_executor :

            public execution::any_executor<
                execution::context_as_t<execution_context &>,
                execution::blocking_t::never_t,
                execution::prefer_only<execution::blocking_t::possibly_t>,
                execution::prefer_only<execution::outstanding_work_t::tracked_t>,
                execution::prefer_only<execution::outstanding_work_t::untracked_t>,
                execution::prefer_only<execution::relationship_t::fork_t>,
                execution::prefer_only<execution::relationship_t::continuation_t>
            >
    {
    public:
        typedef execution::any_executor<
            execution::context_as_t<execution_context &>,
            execution::blocking_t::never_t,
            execution::prefer_only<execution::blocking_t::possibly_t>,
            execution::prefer_only<execution::outstanding_work_t::tracked_t>,
            execution::prefer_only<execution::outstanding_work_t::untracked_t>,
            execution::prefer_only<execution::relationship_t::fork_t>,
            execution::prefer_only<execution::relationship_t::continuation_t>
        > base_type;

        typedef void supportable_properties_type(
            execution::context_as_t<execution_context &>,
            execution::blocking_t::never_t,
            execution::prefer_only<execution::blocking_t::possibly_t>,
            execution::prefer_only<execution::outstanding_work_t::tracked_t>,
            execution::prefer_only<execution::outstanding_work_t::untracked_t>,
            execution::prefer_only<execution::relationship_t::fork_t>,
            execution::prefer_only<execution::relationship_t::continuation_t>);


        /// Default constructor.
        XIO_DECL any_io_executor() noexcept;

        /// Construct in an empty state. Equivalent effects to default constructor.
        XIO_DECL any_io_executor(nullptr_t) noexcept;

        /// Copy constructor.
        XIO_DECL any_io_executor(const any_io_executor &e) noexcept;

        /// Move constructor.
        XIO_DECL any_io_executor(any_io_executor &&e) noexcept;

        /// Construct to point to the same target as another any_executor.

        template<typename OtherAnyExecutor>
        any_io_executor(OtherAnyExecutor e,
                        constraint_t<
                            std::conditional_t<
                                !std::is_same<OtherAnyExecutor, any_io_executor>::value
                                && std::is_base_of<execution::detail::any_executor_base,
                                    OtherAnyExecutor>::value,
                                typename execution::detail::supportable_properties<
                                    0, supportable_properties_type>::template
                                is_valid_target<OtherAnyExecutor>,
                                std::false_type
                            >::value
                        > = 0)
            : base_type(static_cast<OtherAnyExecutor &&>(e)) {
        }

        /// Construct to point to the same target as another any_executor.
        template<typename OtherAnyExecutor>
        any_io_executor(std::nothrow_t, OtherAnyExecutor e,
                        constraint_t<
                            std::conditional_t<
                                !std::is_same<OtherAnyExecutor, any_io_executor>::value
                                && std::is_base_of<execution::detail::any_executor_base,
                                    OtherAnyExecutor>::value,
                                typename execution::detail::supportable_properties<
                                    0, supportable_properties_type>::template
                                is_valid_target<OtherAnyExecutor>,
                                std::false_type
                            >::value
                        > = 0) noexcept
            : base_type(std::nothrow, static_cast<OtherAnyExecutor &&>(e)) {
        }

        /// Construct to point to the same target as another any_executor.
        XIO_DECL any_io_executor(std::nothrow_t,
                                  const any_io_executor &e) noexcept;

        /// Construct to point to the same target as another any_executor.
        XIO_DECL any_io_executor(std::nothrow_t, any_io_executor &&e) noexcept;

        /// Construct a polymorphic wrapper for the specified executor.

        template<XIO_EXECUTION_EXECUTOR Executor>
        any_io_executor(Executor e,
                        constraint_t<
                            std::conditional_t<
                                !std::is_same<Executor, any_io_executor>::value
                                && !std::is_base_of<execution::detail::any_executor_base,
                                    Executor>::value,
                                execution::detail::is_valid_target_executor<
                                    Executor, supportable_properties_type>,
                                std::false_type
                            >::value
                        > = 0)
            : base_type(static_cast<Executor &&>(e)) {
        }


        /// Construct a polymorphic wrapper for the specified executor.

        template<XIO_EXECUTION_EXECUTOR Executor>
        any_io_executor(std::nothrow_t, Executor e,
                        constraint_t<
                            std::conditional_t<
                                !std::is_same<Executor, any_io_executor>::value
                                && !std::is_base_of<execution::detail::any_executor_base,
                                    Executor>::value,
                                execution::detail::is_valid_target_executor<
                                    Executor, supportable_properties_type>,
                                std::false_type
                            >::value
                        > = 0) noexcept
            : base_type(std::nothrow, static_cast<Executor &&>(e)) {
        }

        /// Assignment operator.
        XIO_DECL any_io_executor &operator=(
            const any_io_executor &e) noexcept;

        /// Move assignment operator.
        XIO_DECL any_io_executor &operator=(any_io_executor &&e) noexcept;

        /// Assignment operator that sets the polymorphic wrapper to the empty state.
        XIO_DECL any_io_executor &operator=(nullptr_t);

        /// Destructor.
        XIO_DECL ~any_io_executor();

        /// Swap targets with another polymorphic wrapper.
        XIO_DECL void swap(any_io_executor &other) noexcept;

        /// Obtain a polymorphic wrapper with the specified property.
        /**
   * Do not call this function directly. It is intended for use with the
   * xio::require and xio::prefer customisation points.
   *
   * For example:
   * @code any_io_executor ex = ...;
   * auto ex2 = xio::require(ex, execution::blocking.possibly); @endcode
   */
        template<typename Property>
        any_io_executor require(const Property &p,
                                constraint_t<
                                    traits::require_member<const base_type &, const Property &>::is_valid
                                > = 0) const {
            return static_cast<const base_type &>(*this).require(p);
        }

        /// Obtain a polymorphic wrapper with the specified property.
        /**
   * Do not call this function directly. It is intended for use with the
   * xio::prefer customisation point.
   *
   * For example:
   * @code any_io_executor ex = ...;
   * auto ex2 = xio::prefer(ex, execution::blocking.possibly); @endcode
   */
        template<typename Property>
        any_io_executor prefer(const Property &p,
                               constraint_t<
                                   traits::prefer_member<const base_type &, const Property &>::is_valid
                               > = 0) const {
            return static_cast<const base_type &>(*this).prefer(p);
        }
    };

    template<>
    XIO_DECL any_io_executor any_io_executor::require(
        const execution::blocking_t::never_t &, int) const;

    template<>
    XIO_DECL any_io_executor any_io_executor::prefer(
        const execution::blocking_t::possibly_t &, int) const;

    template<>
    XIO_DECL any_io_executor any_io_executor::prefer(
        const execution::outstanding_work_t::tracked_t &, int) const;

    template<>
    XIO_DECL any_io_executor any_io_executor::prefer(
        const execution::outstanding_work_t::untracked_t &, int) const;

    template<>
    XIO_DECL any_io_executor any_io_executor::prefer(
        const execution::relationship_t::fork_t &, int) const;

    template<>
    XIO_DECL any_io_executor any_io_executor::prefer(
        const execution::relationship_t::continuation_t &, int) const;

#endif // defined(XIO_USE_TS_EXECUTOR_AS_DEFAULT)


} // namespace xio

#include <xio/detail/pop_options.h>


#endif // XIO_ANY_IO_EXECUTOR_HPP
