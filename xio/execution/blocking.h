//
// execution/blocking.hpp
// ~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_EXECUTION_BLOCKING_HPP
#define ASIO_EXECUTION_BLOCKING_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>
#include <xio/detail/type_traits.h>
#include <xio/execution/executor.h>
#include <xio/is_applicable_property.h>
#include <xio/prefer.h>
#include <xio/query.h>
#include <xio/require.h>
#include <xio/traits/execute_member.h>
#include <xio/traits/query_free.h>
#include <xio/traits/query_member.h>
#include <xio/traits/query_static_constexpr_member.h>
#include <xio/traits/static_query.h>
#include <xio/traits/static_require.h>

#include <xio/detail/push_options.h>

namespace xio {


#if defined(GENERATING_DOCUMENTATION)

    namespace execution {
        /// A property to describe what guarantees an executor makes about the blocking
/// behaviour of their execution functions.
        struct blocking_t {
            /// The blocking_t property applies to executors.
            template<typename T>
            static constexpr bool is_applicable_property_v = is_executor_v<T>;

            /// The top-level blocking_t property cannot be required.
            static constexpr bool is_requirable = false;

            /// The top-level blocking_t property cannot be preferred.
            static constexpr bool is_preferable = false;

            /// The type returned by queries against an @c any_executor.
            typedef blocking_t polymorphic_query_result_type;

            /// A sub-property that indicates that invocation of an executor's execution
  /// function may block pending completion of one or more invocations of the
  /// submitted function object.
            struct possibly_t {
                /// The blocking_t::possibly_t property applies to executors.
                template<typename T>
                static constexpr bool is_applicable_property_v = is_executor_v<T>;

                /// The blocking_t::possibly_t property can be required.
                static constexpr bool is_requirable = true;

                /// The blocking_t::possibly_t property can be preferred.
                static constexpr bool is_preferable = true;

                /// The type returned by queries against an @c any_executor.
                typedef blocking_t polymorphic_query_result_type;

                /// Default constructor.
                constexpr possibly_t();

                /// Get the value associated with a property object.
                /**
     * @returns possibly_t();
     */
                static constexpr blocking_t value();
            };

            /// A sub-property that indicates that invocation of an executor's execution
  /// function shall block until completion of all invocations of the submitted
  /// function object.
            struct always_t {
                /// The blocking_t::always_t property applies to executors.
                template<typename T>
                static constexpr bool is_applicable_property_v = is_executor_v<T>;

                /// The blocking_t::always_t property can be required.
                static constexpr bool is_requirable = true;

                /// The blocking_t::always_t property can be preferred.
                static constexpr bool is_preferable = false;

                /// The type returned by queries against an @c any_executor.
                typedef blocking_t polymorphic_query_result_type;

                /// Default constructor.
                constexpr always_t();

                /// Get the value associated with a property object.
                /**
     * @returns always_t();
     */
                static constexpr blocking_t value();
            };

            /// A sub-property that indicates that invocation of an executor's execution
  /// function shall not block pending completion of the invocations of the
  /// submitted function object.
            struct never_t {
                /// The blocking_t::never_t property applies to executors.
                template<typename T>
                static constexpr bool is_applicable_property_v = is_executor_v<T>;

                /// The blocking_t::never_t property can be required.
                static constexpr bool is_requirable = true;

                /// The blocking_t::never_t property can be preferred.
                static constexpr bool is_preferable = true;

                /// The type returned by queries against an @c any_executor.
                typedef blocking_t polymorphic_query_result_type;

                /// Default constructor.
                constexpr never_t();

                /// Get the value associated with a property object.
                /**
     * @returns never_t();
     */
                static constexpr blocking_t value();
            };

            /// A special value used for accessing the blocking_t::possibly_t property.
            static constexpr possibly_t possibly;

            /// A special value used for accessing the blocking_t::always_t property.
            static constexpr always_t always;

            /// A special value used for accessing the blocking_t::never_t property.
            static constexpr never_t never;

            /// Default constructor.
            constexpr blocking_t();

            /// Construct from a sub-property value.
            constexpr blocking_t(possibly_t);

            /// Construct from a sub-property value.
            constexpr blocking_t(always_t);

            /// Construct from a sub-property value.
            constexpr blocking_t(never_t);

            /// Compare property values for equality.
            friend constexpr bool operator==(
                const blocking_t &a, const blocking_t &b) noexcept;

            /// Compare property values for inequality.
            friend constexpr bool operator!=(
                const blocking_t &a, const blocking_t &b) noexcept;
        };

        /// A special value used for accessing the blocking_t property.
        constexpr blocking_t blocking;
    } // namespace execution

#else // defined(GENERATING_DOCUMENTATION)




    namespace execution {
        namespace detail {
            namespace blocking {
                template<int I>
                struct possibly_t;
                template<int I>
                struct always_t;
                template<int I>
                struct never_t;
            } // namespace blocking
            namespace blocking_adaptation {
                template<int I>
                struct allowed_t;

                template<typename Executor, typename Function>
                void blocking_execute(
                    Executor &&ex,
                    Function &&func);
            } // namespace blocking_adaptation

            template<int I = 0>
            struct blocking_t {
                template<typename T>
                static constexpr bool is_applicable_property_v = is_executor<T>::value;

                static constexpr bool is_requirable = false;
                static constexpr bool is_preferable = false;
                typedef blocking_t polymorphic_query_result_type;

                typedef detail::blocking::possibly_t<I> possibly_t;
                typedef detail::blocking::always_t<I> always_t;
                typedef detail::blocking::never_t<I> never_t;

                constexpr blocking_t()
                    : value_(-1) {
                }

                constexpr blocking_t(possibly_t)
                    : value_(0) {
                }

                constexpr blocking_t(always_t)
                    : value_(1) {
                }

                constexpr blocking_t(never_t)
                    : value_(2) {
                }

                template<typename T>
                struct proxy {
                    struct type {
                        template<typename P>
                        auto query(P &&p) const
                            noexcept(
                                noexcept(
                                    std::declval<std::conditional_t<true, T, P> >().query(static_cast<P &&>(p))
                                )
                            )
                            -> decltype(
                                std::declval<std::conditional_t<true, T, P> >().query(static_cast<P &&>(p))
                            );
                    };

                };

                template<typename T>
                struct static_proxy {
                    struct type {
                        template<typename P>
                        static constexpr auto query(P &&p)
                            noexcept(
                                noexcept(
                                    std::conditional_t < true, T, P > ::query(static_cast<P &&>(p))
                                )
                            )
                            -> decltype(
                                std::conditional_t < true, T, P > ::query(static_cast<P &&>(p))
                            ) {
                            return T::query(static_cast<P &&>(p));
                        }
                    };

                };

                template<typename T>
                struct query_member :
                        traits::query_member<typename proxy<T>::type, blocking_t> {
                };

                template<typename T>
                struct query_static_constexpr_member :
                        traits::query_static_constexpr_member<
                            typename static_proxy<T>::type, blocking_t> {
                };

                template<typename T>
                static constexpr
                typename query_static_constexpr_member<T>::result_type
                static_query()
                    noexcept(query_static_constexpr_member<T>::is_noexcept) {
                    return query_static_constexpr_member<T>::value();
                }

                template<typename T>
                static constexpr
                typename traits::static_query<T, possibly_t>::result_type
                static_query(
                    std::enable_if_t<
                        !query_static_constexpr_member<T>::is_valid
                    > * = 0,
                    std::enable_if_t<
                        !query_member<T>::is_valid
                    > * = 0,
                    std::enable_if_t<
                        traits::static_query<T, possibly_t>::is_valid
                    > * = 0) noexcept {
                    return traits::static_query<T, possibly_t>::value();
                }

                template<typename T>
                static constexpr
                typename traits::static_query<T, always_t>::result_type
                static_query(
                    std::enable_if_t<
                        !query_static_constexpr_member<T>::is_valid
                    > * = 0,
                    std::enable_if_t<
                        !query_member<T>::is_valid
                    > * = 0,
                    std::enable_if_t<
                        !traits::static_query<T, possibly_t>::is_valid
                    > * = 0,
                    std::enable_if_t<
                        traits::static_query<T, always_t>::is_valid
                    > * = 0) noexcept {
                    return traits::static_query<T, always_t>::value();
                }

                template<typename T>
                static constexpr
                typename traits::static_query<T, never_t>::result_type
                static_query(
                    std::enable_if_t<
                        !query_static_constexpr_member<T>::is_valid
                    > * = 0,
                    std::enable_if_t<
                        !query_member<T>::is_valid
                    > * = 0,
                    std::enable_if_t<
                        !traits::static_query<T, possibly_t>::is_valid
                    > * = 0,
                    std::enable_if_t<
                        !traits::static_query<T, always_t>::is_valid
                    > * = 0,
                    std::enable_if_t<
                        traits::static_query<T, never_t>::is_valid
                    > * = 0) noexcept {
                    return traits::static_query<T, never_t>::value();
                }

                template<typename E, typename T = decltype(blocking_t::static_query<E>())>
                static constexpr const T static_query_v
                        = blocking_t::static_query<E>();


                friend constexpr bool operator==(
                    const blocking_t &a, const blocking_t &b) {
                    return a.value_ == b.value_;
                }

                friend constexpr bool operator!=(
                    const blocking_t &a, const blocking_t &b) {
                    return a.value_ != b.value_;
                }

                struct convertible_from_blocking_t {
                    constexpr convertible_from_blocking_t(blocking_t) {
                    }
                };

                template<typename Executor>
                friend constexpr blocking_t query(
                    const Executor &ex, convertible_from_blocking_t,
                    std::enable_if_t<
                        can_query<const Executor &, possibly_t>::value
                    > * = 0)
#if !defined(__clang__) // Clang crashes if noexcept is used here.
#if defined(ASIO_MSVC) // Visual C++ wants the type to be qualified.
                noexcept(is_nothrow_query<const Executor &, blocking_t<>::possibly_t>::value)
#else // defined(ASIO_MSVC)
                    noexcept(is_nothrow_query<const Executor &, possibly_t>::value)
#endif // defined(ASIO_MSVC)
#endif // !defined(__clang__)
                {
                    return xio::query(ex, possibly_t());
                }

                template<typename Executor>
                friend constexpr blocking_t query(
                    const Executor &ex, convertible_from_blocking_t,
                    std::enable_if_t<
                        !can_query<const Executor &, possibly_t>::value
                    > * = 0,
                    std::enable_if_t<
                        can_query<const Executor &, always_t>::value
                    > * = 0)
#if !defined(__clang__) // Clang crashes if noexcept is used here.
#if defined(ASIO_MSVC) // Visual C++ wants the type to be qualified.
                noexcept(is_nothrow_query<const Executor &, blocking_t<>::always_t>::value)
#else // defined(ASIO_MSVC)
                    noexcept(is_nothrow_query<const Executor &, always_t>::value)
#endif // defined(ASIO_MSVC)
#endif // !defined(__clang__)
                {
                    return xio::query(ex, always_t());
                }

                template<typename Executor>
                friend constexpr blocking_t query(
                    const Executor &ex, convertible_from_blocking_t,
                    std::enable_if_t<
                        !can_query<const Executor &, possibly_t>::value
                    > * = 0,
                    std::enable_if_t<
                        !can_query<const Executor &, always_t>::value
                    > * = 0,
                    std::enable_if_t<
                        can_query<const Executor &, never_t>::value
                    > * = 0)
#if !defined(__clang__) // Clang crashes if noexcept is used here.
#if defined(ASIO_MSVC) // Visual C++ wants the type to be qualified.
                noexcept(is_nothrow_query<const Executor &, blocking_t<>::never_t>::value)
#else // defined(ASIO_MSVC)
                    noexcept(is_nothrow_query<const Executor &, never_t>::value)
#endif // defined(ASIO_MSVC)
#endif // !defined(__clang__)
                {
                    return xio::query(ex, never_t());
                }

                ASIO_STATIC_CONSTEXPR_DEFAULT_INIT(possibly_t, possibly);

                ASIO_STATIC_CONSTEXPR_DEFAULT_INIT(always_t, always);

                ASIO_STATIC_CONSTEXPR_DEFAULT_INIT(never_t, never);

            private:
                int value_;
            };

            template<int I>
            template<typename E, typename T>
            const T blocking_t<I>::static_query_v;


            template<int I>
            const typename blocking_t<I>::possibly_t blocking_t<I>::possibly;

            template<int I>
            const typename blocking_t<I>::always_t blocking_t<I>::always;

            template<int I>
            const typename blocking_t<I>::never_t blocking_t<I>::never;

            namespace blocking {
                template<int I = 0>
                struct possibly_t {
                    template<typename T>
                    static constexpr bool is_applicable_property_v = is_executor<T>::value;

                    static constexpr bool is_requirable = true;
                    static constexpr bool is_preferable = true;
                    typedef blocking_t<I> polymorphic_query_result_type;

                    constexpr possibly_t() {
                    }

                    template<typename T>
                    struct query_member :
                            traits::query_member<
                                typename blocking_t<I>::template proxy<T>::type, possibly_t> {
                    };

                    template<typename T>
                    struct query_static_constexpr_member :
                            traits::query_static_constexpr_member<
                                typename blocking_t<I>::template static_proxy<T>::type, possibly_t> {
                    };


                    template<typename T>
                    static constexpr
                    typename query_static_constexpr_member<T>::result_type
                    static_query()
                        noexcept(query_static_constexpr_member<T>::is_noexcept) {
                        return query_static_constexpr_member<T>::value();
                    }

                    template<typename T>
                    static constexpr possibly_t static_query(
                        std::enable_if_t<
                            !query_static_constexpr_member<T>::is_valid
                        > * = 0,
                        std::enable_if_t<
                            !query_member<T>::is_valid
                        > * = 0,
                        std::enable_if_t<
                            !traits::query_free<T, possibly_t>::is_valid
                        > * = 0,
                        std::enable_if_t<
                            !can_query<T, always_t<I> >::value
                        > * = 0,
                        std::enable_if_t<
                            !can_query<T, never_t<I> >::value
                        > * = 0) noexcept {
                        return possibly_t();
                    }

                    template<typename E, typename T = decltype(possibly_t::static_query<E>())>
                    static constexpr const T static_query_v
                            = possibly_t::static_query<E>();


                    static constexpr blocking_t<I> value() {
                        return possibly_t();
                    }

                    friend constexpr bool operator==(
                        const possibly_t &, const possibly_t &) {
                        return true;
                    }

                    friend constexpr bool operator!=(
                        const possibly_t &, const possibly_t &) {
                        return false;
                    }

                    friend constexpr bool operator==(
                        const possibly_t &, const always_t<I> &) {
                        return false;
                    }

                    friend constexpr bool operator!=(
                        const possibly_t &, const always_t<I> &) {
                        return true;
                    }

                    friend constexpr bool operator==(
                        const possibly_t &, const never_t<I> &) {
                        return false;
                    }

                    friend constexpr bool operator!=(
                        const possibly_t &, const never_t<I> &) {
                        return true;
                    }
                };


                template<int I>
                template<typename E, typename T>
                const T possibly_t<I>::static_query_v;


                template<typename Executor>
                class adapter {
                public:
                    adapter(int, const Executor &e) noexcept
                        : executor_(e) {
                    }

                    adapter(const adapter &other) noexcept
                        : executor_(other.executor_) {
                    }

                    adapter(adapter &&other) noexcept
                        : executor_(static_cast<Executor &&>(other.executor_)) {
                    }

                    template<int I>
                    static constexpr always_t<I> query(blocking_t<I>) noexcept {
                        return always_t<I>();
                    }

                    template<int I>
                    static constexpr always_t<I> query(possibly_t<I>) noexcept {
                        return always_t<I>();
                    }

                    template<int I>
                    static constexpr always_t<I> query(always_t<I>) noexcept {
                        return always_t<I>();
                    }

                    template<int I>
                    static constexpr always_t<I> query(never_t<I>) noexcept {
                        return always_t<I>();
                    }

                    template<typename Property>
                    std::enable_if_t<
                        can_query<const Executor &, Property>::value,
                        query_result_t<const Executor &, Property>
                    > query(const Property &p) const
                        noexcept(is_nothrow_query<const Executor &, Property>::value) {
                        return xio::query(executor_, p);
                    }

                    template<int I>
                    std::enable_if_t<
                        can_require<const Executor &, possibly_t<I> >::value,
                        require_result_t<const Executor &, possibly_t<I> >
                    > require(possibly_t<I>) const noexcept {
                        return xio::require(executor_, possibly_t<I>());
                    }

                    template<int I>
                    std::enable_if_t<
                        can_require<const Executor &, never_t<I> >::value,
                        require_result_t<const Executor &, never_t<I> >
                    > require(never_t<I>) const noexcept {
                        return xio::require(executor_, never_t<I>());
                    }

                    template<typename Property>
                    std::enable_if_t<
                        can_require<const Executor &, Property>::value,
                        adapter<std::decay_t<require_result_t<const Executor &, Property> > >
                    > require(const Property &p) const
                        noexcept(is_nothrow_require<const Executor &, Property>::value) {
                        return adapter<std::decay_t<require_result_t<const Executor &, Property> > >(
                            0, xio::require(executor_, p));
                    }

                    template<typename Property>
                    std::enable_if_t<
                        can_prefer<const Executor &, Property>::value,
                        adapter<std::decay_t<prefer_result_t<const Executor &, Property> > >
                    > prefer(const Property &p) const
                        noexcept(is_nothrow_prefer<const Executor &, Property>::value) {
                        return adapter<std::decay_t<prefer_result_t<const Executor &, Property> > >(
                            0, xio::prefer(executor_, p));
                    }

                    template<typename Function>
                    std::enable_if_t<
                        traits::execute_member<const Executor &, Function>::is_valid
                    > execute(Function &&f) const {
                        blocking_adaptation::blocking_execute(
                            executor_, static_cast<Function &&>(f));
                    }

                    friend bool operator==(const adapter &a, const adapter &b) noexcept {
                        return a.executor_ == b.executor_;
                    }

                    friend bool operator!=(const adapter &a, const adapter &b) noexcept {
                        return a.executor_ != b.executor_;
                    }

                private:
                    Executor executor_;
                };

                template<int I = 0>
                struct always_t {
                    template<typename T>
                    static constexpr bool is_applicable_property_v = is_executor<T>::value;

                    static constexpr bool is_requirable = true;
                    static constexpr bool is_preferable = false;
                    typedef blocking_t<I> polymorphic_query_result_type;

                    constexpr always_t() {
                    }

                    template<typename T>
                    struct query_member :
                            traits::query_member<
                                typename blocking_t<I>::template proxy<T>::type, always_t> {
                    };

                    template<typename T>
                    struct query_static_constexpr_member :
                            traits::query_static_constexpr_member<
                                typename blocking_t<I>::template static_proxy<T>::type, always_t> {
                    };

                    template<typename T>
                    static constexpr typename query_static_constexpr_member<T>::result_type
                    static_query()
                        noexcept(query_static_constexpr_member<T>::is_noexcept) {
                        return query_static_constexpr_member<T>::value();
                    }

                    template<typename E, typename T = decltype(always_t::static_query<E>())>
                    static constexpr const T static_query_v = always_t::static_query<E>();


                    static constexpr blocking_t<I> value() {
                        return always_t();
                    }

                    friend constexpr bool operator==(
                        const always_t &, const always_t &) {
                        return true;
                    }

                    friend constexpr bool operator!=(
                        const always_t &, const always_t &) {
                        return false;
                    }

                    friend constexpr bool operator==(
                        const always_t &, const possibly_t<I> &) {
                        return false;
                    }

                    friend constexpr bool operator!=(
                        const always_t &, const possibly_t<I> &) {
                        return true;
                    }

                    friend constexpr bool operator==(
                        const always_t &, const never_t<I> &) {
                        return false;
                    }

                    friend constexpr bool operator!=(
                        const always_t &, const never_t<I> &) {
                        return true;
                    }

                    template<typename Executor>
                    friend adapter<Executor> require(
                        const Executor &e, const always_t &,
                        std::enable_if_t<
                            is_executor<Executor>::value
                        > * = 0,
                        std::enable_if_t<
                            traits::static_require<
                                const Executor &,
                                blocking_adaptation::allowed_t<0>
                            >::is_valid
                        > * = 0) {
                        return adapter<Executor>(0, e);
                    }
                };


                template<int I>
                template<typename E, typename T>
                const T always_t<I>::static_query_v;


                template<int I>
                struct never_t {
                    template<typename T>
                    static constexpr bool is_applicable_property_v = is_executor<T>::value;

                    static constexpr bool is_requirable = true;
                    static constexpr bool is_preferable = true;
                    typedef blocking_t<I> polymorphic_query_result_type;

                    constexpr never_t() {
                    }

                    template<typename T>
                    struct query_member :
                            traits::query_member<
                                typename blocking_t<I>::template proxy<T>::type, never_t> {
                    };

                    template<typename T>
                    struct query_static_constexpr_member :
                            traits::query_static_constexpr_member<
                                typename blocking_t<I>::template static_proxy<T>::type, never_t> {
                    };

                    template<typename T>
                    static constexpr
                    typename query_static_constexpr_member<T>::result_type
                    static_query()
                        noexcept(query_static_constexpr_member<T>::is_noexcept) {
                        return query_static_constexpr_member<T>::value();
                    }

                    template<typename E, typename T = decltype(never_t::static_query<E>())>
                    static constexpr const T static_query_v
                            = never_t::static_query<E>();

                    static constexpr blocking_t<I> value() {
                        return never_t();
                    }

                    friend constexpr bool operator==(const never_t &, const never_t &) {
                        return true;
                    }

                    friend constexpr bool operator!=(const never_t &, const never_t &) {
                        return false;
                    }

                    friend constexpr bool operator==(const never_t &, const possibly_t<I> &) {
                        return false;
                    }

                    friend constexpr bool operator!=(const never_t &, const possibly_t<I> &) {
                        return true;
                    }

                    friend constexpr bool operator==(const never_t &, const always_t<I> &) {
                        return false;
                    }

                    friend constexpr bool operator!=(const never_t &, const always_t<I> &) {
                        return true;
                    }
                };


                template<int I>
                template<typename E, typename T>
                const T never_t<I>::static_query_v;
            } // namespace blocking
        } // namespace detail

        typedef detail::blocking_t<> blocking_t;

inline constexpr blocking_t blocking;
    } // namespace execution


#endif // defined(GENERATING_DOCUMENTATION)


} // namespace xio

#include <xio/detail/pop_options.h>

#endif // ASIO_EXECUTION_BLOCKING_HPP
