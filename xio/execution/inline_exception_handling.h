//
// execution/inline_exception_handling.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef XIO_EXECUTION_INLINE_EXCEPTION_HANDLING_HPP
#define XIO_EXECUTION_INLINE_EXCEPTION_HANDLING_HPP

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


    namespace execution {
        namespace detail {
            namespace inline_exception_handling {
                template<int I>
                struct propagate_t;
                template<int I>
                struct capture_t;
                template<int I>
                struct terminate_t;
            } // namespace inline_exception_handling

            template<int I = 0>
            struct inline_exception_handling_t {
                template<typename T>
                static constexpr bool is_applicable_property_v = is_executor<T>::value;

                static constexpr bool is_requirable = false;
                static constexpr bool is_preferable = false;
                typedef inline_exception_handling_t polymorphic_query_result_type;

                typedef detail::inline_exception_handling::propagate_t<I> propagate_t;
                typedef detail::inline_exception_handling::capture_t<I> capture_t;
                typedef detail::inline_exception_handling::terminate_t<I> terminate_t;

                constexpr inline_exception_handling_t()
                    : value_(-1) {
                }

                constexpr inline_exception_handling_t(propagate_t)
                    : value_(0) {
                }

                constexpr inline_exception_handling_t(capture_t)
                    : value_(1) {
                }

                constexpr inline_exception_handling_t(terminate_t)
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
                        traits::query_member<typename proxy<T>::type,
                            inline_exception_handling_t> {
                };

                template<typename T>
                struct query_static_constexpr_member :
                        traits::query_static_constexpr_member<
                            typename static_proxy<T>::type, inline_exception_handling_t> {
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
                typename traits::static_query<T, propagate_t>::result_type
                static_query(
                    std::enable_if_t<
                        !query_static_constexpr_member<T>::is_valid
                    > * = 0,
                    std::enable_if_t<
                        !query_member<T>::is_valid
                    > * = 0,
                    std::enable_if_t<
                        traits::static_query<T, propagate_t>::is_valid
                    > * = 0) noexcept {
                    return traits::static_query<T, propagate_t>::value();
                }

                template<typename T>
                static constexpr
                typename traits::static_query<T, capture_t>::result_type
                static_query(
                    std::enable_if_t<
                        !query_static_constexpr_member<T>::is_valid
                    > * = 0,
                    std::enable_if_t<
                        !query_member<T>::is_valid
                    > * = 0,
                    std::enable_if_t<
                        !traits::static_query<T, propagate_t>::is_valid
                    > * = 0,
                    std::enable_if_t<
                        traits::static_query<T, capture_t>::is_valid
                    > * = 0) noexcept {
                    return traits::static_query<T, capture_t>::value();
                }

                template<typename T>
                static constexpr
                typename traits::static_query<T, terminate_t>::result_type
                static_query(
                    std::enable_if_t<
                        !query_static_constexpr_member<T>::is_valid
                    > * = 0,
                    std::enable_if_t<
                        !query_member<T>::is_valid
                    > * = 0,
                    std::enable_if_t<
                        !traits::static_query<T, propagate_t>::is_valid
                    > * = 0,
                    std::enable_if_t<
                        !traits::static_query<T, capture_t>::is_valid
                    > * = 0,
                    std::enable_if_t<
                        traits::static_query<T, terminate_t>::is_valid
                    > * = 0) noexcept {
                    return traits::static_query<T, terminate_t>::value();
                }

                template<typename E,
                    typename T = decltype(inline_exception_handling_t::static_query<E>())>
                static constexpr const T static_query_v
                        = inline_exception_handling_t::static_query<E>();


                friend constexpr bool operator==(const inline_exception_handling_t &a,
                                                 const inline_exception_handling_t &b) {
                    return a.value_ == b.value_;
                }

                friend constexpr bool operator!=(const inline_exception_handling_t &a,
                                                 const inline_exception_handling_t &b) {
                    return a.value_ != b.value_;
                }

                struct convertible_from_inline_exception_handling_t {
                    constexpr convertible_from_inline_exception_handling_t(
                        inline_exception_handling_t) {
                    }
                };

                template<typename Executor>
                friend constexpr inline_exception_handling_t query(
                    const Executor &ex, convertible_from_inline_exception_handling_t,
                    std::enable_if_t<
                        can_query<const Executor &, propagate_t>::value
                    > * = 0)
#if !defined(__clang__) // Clang crashes if noexcept is used here.
#if defined(XIO_MSVC) // Visual C++ wants the type to be qualified.
                noexcept(is_nothrow_query<const Executor &,
                    inline_exception_handling_t<>::propagate_t>::value)
#else // defined(XIO_MSVC)
                    noexcept(is_nothrow_query<const Executor &, propagate_t>::value)
#endif // defined(XIO_MSVC)
#endif // !defined(__clang__)
                {
                    return xio::query(ex, propagate_t());
                }

                template<typename Executor>
                friend constexpr inline_exception_handling_t query(
                    const Executor &ex, convertible_from_inline_exception_handling_t,
                    std::enable_if_t<
                        !can_query<const Executor &, propagate_t>::value
                    > * = 0,
                    std::enable_if_t<
                        can_query<const Executor &, capture_t>::value
                    > * = 0)
#if !defined(__clang__) // Clang crashes if noexcept is used here.
#if defined(XIO_MSVC) // Visual C++ wants the type to be qualified.
                noexcept(is_nothrow_query<const Executor &,
                    inline_exception_handling_t<>::capture_t>::value)
#else // defined(XIO_MSVC)
                    noexcept(is_nothrow_query<const Executor &, capture_t>::value)
#endif // defined(XIO_MSVC)
#endif // !defined(__clang__)
                {
                    return xio::query(ex, capture_t());
                }

                template<typename Executor>
                friend constexpr inline_exception_handling_t query(
                    const Executor &ex, convertible_from_inline_exception_handling_t,
                    std::enable_if_t<
                        !can_query<const Executor &, propagate_t>::value
                    > * = 0,
                    std::enable_if_t<
                        !can_query<const Executor &, capture_t>::value
                    > * = 0,
                    std::enable_if_t<
                        can_query<const Executor &, terminate_t>::value
                    > * = 0)
#if !defined(__clang__) // Clang crashes if noexcept is used here.
#if defined(XIO_MSVC) // Visual C++ wants the type to be qualified.
                noexcept(is_nothrow_query<const Executor &,
                    inline_exception_handling_t<>::terminate_t>::value)
#else // defined(XIO_MSVC)
                    noexcept(is_nothrow_query<const Executor &, terminate_t>::value)
#endif // defined(XIO_MSVC)
#endif // !defined(__clang__)
                {
                    return xio::query(ex, terminate_t());
                }

                XIO_STATIC_CONSTEXPR_DEFAULT_INIT(propagate_t, propagate);

                XIO_STATIC_CONSTEXPR_DEFAULT_INIT(capture_t, capture);

                XIO_STATIC_CONSTEXPR_DEFAULT_INIT(terminate_t, terminate);

            private:
                int value_;
            };


            template<int I>
            template<typename E, typename T>
            const T inline_exception_handling_t<I>::static_query_v;


            template<int I>
            const typename inline_exception_handling_t<I>::propagate_t
            inline_exception_handling_t<I>::propagate;

            template<int I>
            const typename inline_exception_handling_t<I>::capture_t
            inline_exception_handling_t<I>::capture;

            template<int I>
            const typename inline_exception_handling_t<I>::terminate_t
            inline_exception_handling_t<I>::terminate;

            namespace inline_exception_handling {
                template<int I = 0>
                struct propagate_t {
                    template<typename T>
                    static constexpr bool is_applicable_property_v = is_executor<T>::value;

                    static constexpr bool is_requirable = true;
                    static constexpr bool is_preferable = true;
                    typedef inline_exception_handling_t<I> polymorphic_query_result_type;

                    constexpr propagate_t() {
                    }

                    template<typename T>
                    struct query_member :
                            traits::query_member<
                                typename inline_exception_handling_t<I>::template
                                proxy<T>::type, propagate_t> {
                    };

                    template<typename T>
                    struct query_static_constexpr_member :
                            traits::query_static_constexpr_member<
                                typename inline_exception_handling_t<I>::template
                                static_proxy<T>::type, propagate_t> {
                    };


                    template<typename T>
                    static constexpr
                    typename query_static_constexpr_member<T>::result_type
                    static_query()
                        noexcept(query_static_constexpr_member<T>::is_noexcept) {
                        return query_static_constexpr_member<T>::value();
                    }

                    template<typename T>
                    static constexpr propagate_t static_query(
                        std::enable_if_t<
                            !query_static_constexpr_member<T>::is_valid
                        > * = 0,
                        std::enable_if_t<
                            !query_member<T>::is_valid
                        > * = 0,
                        std::enable_if_t<
                            !traits::query_free<T, propagate_t>::is_valid
                        > * = 0,
                        std::enable_if_t<
                            !can_query<T, capture_t<I> >::value
                        > * = 0,
                        std::enable_if_t<
                            !can_query<T, terminate_t<I> >::value
                        > * = 0) noexcept {
                        return propagate_t();
                    }

                    template<typename E, typename T = decltype(propagate_t::static_query<E>())>
                    static constexpr const T static_query_v
                            = propagate_t::static_query<E>();


                    static constexpr inline_exception_handling_t<I> value() {
                        return propagate_t();
                    }

                    friend constexpr bool operator==(
                        const propagate_t &, const propagate_t &) {
                        return true;
                    }

                    friend constexpr bool operator!=(
                        const propagate_t &, const propagate_t &) {
                        return false;
                    }

                    friend constexpr bool operator==(
                        const propagate_t &, const capture_t<I> &) {
                        return false;
                    }

                    friend constexpr bool operator!=(
                        const propagate_t &, const capture_t<I> &) {
                        return true;
                    }

                    friend constexpr bool operator==(
                        const propagate_t &, const terminate_t<I> &) {
                        return false;
                    }

                    friend constexpr bool operator!=(
                        const propagate_t &, const terminate_t<I> &) {
                        return true;
                    }
                };

                template<int I>
                template<typename E, typename T>
                const T propagate_t<I>::static_query_v;


                template<int I = 0>
                struct capture_t {
                    template<typename T>
                    static constexpr bool is_applicable_property_v = is_executor<T>::value;

                    static constexpr bool is_requirable = true;
                    static constexpr bool is_preferable = false;
                    typedef inline_exception_handling_t<I> polymorphic_query_result_type;

                    constexpr capture_t() {
                    }

                    template<typename T>
                    struct query_member :
                            traits::query_member<
                                typename inline_exception_handling_t<I>::template
                                proxy<T>::type, capture_t> {
                    };

                    template<typename T>
                    struct query_static_constexpr_member :
                            traits::query_static_constexpr_member<
                                typename inline_exception_handling_t<I>::template
                                static_proxy<T>::type, capture_t> {
                    };

                    template<typename T>
                    static constexpr typename query_static_constexpr_member<T>::result_type
                    static_query()
                        noexcept(query_static_constexpr_member<T>::is_noexcept) {
                        return query_static_constexpr_member<T>::value();
                    }

                    template<typename E, typename T = decltype(capture_t::static_query<E>())>
                    static constexpr const T static_query_v = capture_t::static_query<E>();


                    static constexpr inline_exception_handling_t<I> value() {
                        return capture_t();
                    }

                    friend constexpr bool operator==(
                        const capture_t &, const capture_t &) {
                        return true;
                    }

                    friend constexpr bool operator!=(
                        const capture_t &, const capture_t &) {
                        return false;
                    }

                    friend constexpr bool operator==(
                        const capture_t &, const propagate_t<I> &) {
                        return false;
                    }

                    friend constexpr bool operator!=(
                        const capture_t &, const propagate_t<I> &) {
                        return true;
                    }

                    friend constexpr bool operator==(
                        const capture_t &, const terminate_t<I> &) {
                        return false;
                    }

                    friend constexpr bool operator!=(
                        const capture_t &, const terminate_t<I> &) {
                        return true;
                    }
                };

                template<int I>
                template<typename E, typename T>
                const T capture_t<I>::static_query_v;


                template<int I>
                struct terminate_t {
                    template<typename T>
                    static constexpr bool is_applicable_property_v = is_executor<T>::value;

                    static constexpr bool is_requirable = true;
                    static constexpr bool is_preferable = true;
                    typedef inline_exception_handling_t<I> polymorphic_query_result_type;

                    constexpr terminate_t() {
                    }

                    template<typename T>
                    struct query_member :
                            traits::query_member<
                                typename inline_exception_handling_t<I>::template
                                proxy<T>::type, terminate_t> {
                    };

                    template<typename T>
                    struct query_static_constexpr_member :
                            traits::query_static_constexpr_member<
                                typename inline_exception_handling_t<I>::template
                                static_proxy<T>::type, terminate_t> {
                    };

                    template<typename T>
                    static constexpr
                    typename query_static_constexpr_member<T>::result_type
                    static_query()
                        noexcept(query_static_constexpr_member<T>::is_noexcept) {
                        return query_static_constexpr_member<T>::value();
                    }

                    template<typename E, typename T = decltype(terminate_t::static_query<E>())>
                    static constexpr const T static_query_v
                            = terminate_t::static_query<E>();


                    static constexpr inline_exception_handling_t<I> value() {
                        return terminate_t();
                    }

                    friend constexpr bool operator==(const terminate_t &, const terminate_t &) {
                        return true;
                    }

                    friend constexpr bool operator!=(const terminate_t &, const terminate_t &) {
                        return false;
                    }

                    friend constexpr bool operator==(const terminate_t &, const propagate_t<I> &) {
                        return false;
                    }

                    friend constexpr bool operator!=(const terminate_t &, const propagate_t<I> &) {
                        return true;
                    }

                    friend constexpr bool operator==(const terminate_t &, const capture_t<I> &) {
                        return false;
                    }

                    friend constexpr bool operator!=(const terminate_t &, const capture_t<I> &) {
                        return true;
                    }
                };

                template<int I>
                template<typename E, typename T>
                const T terminate_t<I>::static_query_v;
            } // namespace inline_exception_handling
        } // namespace detail

        typedef detail::inline_exception_handling_t<> inline_exception_handling_t;

        inline
        constexpr
        inline_exception_handling_t inline_exception_handling;
    } // namespace execution


} // namespace xio

#include <xio/detail/pop_options.h>

#endif // XIO_EXECUTION_INLINE_EXCEPTION_HANDLING_HPP
