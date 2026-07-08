//
// execution/mapping.hpp
// ~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef XIO_EXECUTION_MAPPING_HPP
#define XIO_EXECUTION_MAPPING_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>
#include <xio/detail/type_traits.h>
#include <xio/execution/executor.h>
#include <xio/is_applicable_property.h>
#include <xio/query.h>
#include <xio/traits/query_free.h>
#include <xio/traits/query_member.h>
#include <xio/traits/query_static_constexpr_member.h>
#include <xio/traits/static_query.h>
#include <xio/traits/static_require.h>

#include <xio/detail/push_options.h>

namespace xio {
    namespace execution {
        namespace detail {
            namespace mapping {
                template<int I>
                struct thread_t;
                template<int I>
                struct new_thread_t;
                template<int I>
                struct other_t;
            } // namespace mapping

            template<int I = 0>
            struct mapping_t {
                template<typename T>
                static constexpr bool is_applicable_property_v = is_executor<T>::value;

                static constexpr bool is_requirable = false;
                static constexpr bool is_preferable = false;
                typedef mapping_t polymorphic_query_result_type;

                typedef detail::mapping::thread_t<I> thread_t;
                typedef detail::mapping::new_thread_t<I> new_thread_t;
                typedef detail::mapping::other_t<I> other_t;

                constexpr mapping_t()
                    : value_(-1) {
                }

                constexpr mapping_t(thread_t)
                    : value_(0) {
                }

                constexpr mapping_t(new_thread_t)
                    : value_(1) {
                }

                constexpr mapping_t(other_t)
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
                                    std::conditional_t<true, T, P>::query(static_cast<P &&>(p))
                                )
                            )
                            -> decltype(
                                std::conditional_t<true, T, P>::query(static_cast<P &&>(p))
                            ) {
                            return T::query(static_cast<P &&>(p));
                        }
                    };
                };

                template<typename T>
                struct query_member :
                        traits::query_member<typename proxy<T>::type, mapping_t> {
                };

                template<typename T>
                struct query_static_constexpr_member :
                        traits::query_static_constexpr_member<
                            typename static_proxy<T>::type, mapping_t> {
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
                typename traits::static_query<T, thread_t>::result_type
                static_query(
                    std::enable_if_t<
                        !query_static_constexpr_member<T>::is_valid
                    > * = 0,
                    std::enable_if_t<
                        !query_member<T>::is_valid
                    > * = 0,
                    std::enable_if_t<
                        traits::static_query<T, thread_t>::is_valid
                    > * = 0) noexcept {
                    return traits::static_query<T, thread_t>::value();
                }

                template<typename T>
                static constexpr
                typename traits::static_query<T, new_thread_t>::result_type
                static_query(
                    std::enable_if_t<
                        !query_static_constexpr_member<T>::is_valid
                    > * = 0,
                    std::enable_if_t<
                        !query_member<T>::is_valid
                    > * = 0,
                    std::enable_if_t<
                        !traits::static_query<T, thread_t>::is_valid
                    > * = 0,
                    std::enable_if_t<
                        traits::static_query<T, new_thread_t>::is_valid
                    > * = 0) noexcept {
                    return traits::static_query<T, new_thread_t>::value();
                }

                template<typename T>
                static constexpr
                typename traits::static_query<T, other_t>::result_type
                static_query(
                    std::enable_if_t<
                        !query_static_constexpr_member<T>::is_valid
                    > * = 0,
                    std::enable_if_t<
                        !query_member<T>::is_valid
                    > * = 0,
                    std::enable_if_t<
                        !traits::static_query<T, thread_t>::is_valid
                    > * = 0,
                    std::enable_if_t<
                        !traits::static_query<T, new_thread_t>::is_valid
                    > * = 0,
                    std::enable_if_t<
                        traits::static_query<T, other_t>::is_valid
                    > * = 0) noexcept {
                    return traits::static_query<T, other_t>::value();
                }

                template<typename E, typename T = decltype(mapping_t::static_query<E>())>
                static constexpr const T static_query_v
                        = mapping_t::static_query<E>();


                friend constexpr bool operator==(
                    const mapping_t &a, const mapping_t &b) {
                    return a.value_ == b.value_;
                }

                friend constexpr bool operator!=(
                    const mapping_t &a, const mapping_t &b) {
                    return a.value_ != b.value_;
                }

                struct convertible_from_mapping_t {
                    constexpr convertible_from_mapping_t(mapping_t) {
                    }
                };

                template<typename Executor>
                friend constexpr mapping_t query(
                    const Executor &ex, convertible_from_mapping_t,
                    std::enable_if_t<
                        can_query<const Executor &, thread_t>::value
                    > * = 0)
#if !defined(__clang__) // Clang crashes if noexcept is used here.
#if defined(XIO_MSVC) // Visual C++ wants the type to be qualified.
                noexcept(is_nothrow_query<const Executor &, mapping_t<>::thread_t>::value)
#else // defined(XIO_MSVC)
                    noexcept(is_nothrow_query<const Executor &, thread_t>::value)
#endif // defined(XIO_MSVC)
#endif // !defined(__clang__)
                {
                    return xio::query(ex, thread_t());
                }

                template<typename Executor>
                friend constexpr mapping_t query(
                    const Executor &ex, convertible_from_mapping_t,
                    std::enable_if_t<
                        !can_query<const Executor &, thread_t>::value
                    > * = 0,
                    std::enable_if_t<
                        can_query<const Executor &, new_thread_t>::value
                    > * = 0)
#if !defined(__clang__) // Clang crashes if noexcept is used here.
#if defined(XIO_MSVC) // Visual C++ wants the type to be qualified.
                noexcept(
                    is_nothrow_query<const Executor &, mapping_t<>::new_thread_t>::value)
#else // defined(XIO_MSVC)
                    noexcept(is_nothrow_query<const Executor &, new_thread_t>::value)
#endif // defined(XIO_MSVC)
#endif // !defined(__clang__)
                {
                    return xio::query(ex, new_thread_t());
                }

                template<typename Executor>
                friend constexpr mapping_t query(
                    const Executor &ex, convertible_from_mapping_t,
                    std::enable_if_t<
                        !can_query<const Executor &, thread_t>::value
                    > * = 0,
                    std::enable_if_t<
                        !can_query<const Executor &, new_thread_t>::value
                    > * = 0,
                    std::enable_if_t<
                        can_query<const Executor &, other_t>::value
                    > * = 0)
#if !defined(__clang__) // Clang crashes if noexcept is used here.
#if defined(XIO_MSVC) // Visual C++ wants the type to be qualified.
                noexcept(is_nothrow_query<const Executor &, mapping_t<>::other_t>::value)
#else // defined(XIO_MSVC)
                    noexcept(is_nothrow_query<const Executor &, other_t>::value)
#endif // defined(XIO_MSVC)
#endif // !defined(__clang__)
                {
                    return xio::query(ex, other_t());
                }

                XIO_STATIC_CONSTEXPR_DEFAULT_INIT(thread_t, thread);

                XIO_STATIC_CONSTEXPR_DEFAULT_INIT(new_thread_t, new_thread);

                XIO_STATIC_CONSTEXPR_DEFAULT_INIT(other_t, other);

            private:
                int value_;
            };

            template<int I>
            template<typename E, typename T>
            const T mapping_t<I>::static_query_v;


            template<int I>
            const typename mapping_t<I>::thread_t mapping_t<I>::thread;

            template<int I>
            const typename mapping_t<I>::new_thread_t mapping_t<I>::new_thread;

            template<int I>
            const typename mapping_t<I>::other_t mapping_t<I>::other;

            namespace mapping {
                template<int I = 0>
                struct thread_t {
                    template<typename T>
                    static constexpr bool is_applicable_property_v = is_executor<T>::value;

                    static constexpr bool is_requirable = true;
                    static constexpr bool is_preferable = true;
                    typedef mapping_t<I> polymorphic_query_result_type;

                    constexpr thread_t() {
                    }

                    template<typename T>
                    struct query_member :
                            traits::query_member<
                                typename mapping_t<I>::template proxy<T>::type, thread_t> {
                    };

                    template<typename T>
                    struct query_static_constexpr_member :
                            traits::query_static_constexpr_member<
                                typename mapping_t<I>::template static_proxy<T>::type, thread_t> {
                    };


                    template<typename T>
                    static constexpr
                    typename query_static_constexpr_member<T>::result_type
                    static_query()
                        noexcept(query_static_constexpr_member<T>::is_noexcept) {
                        return query_static_constexpr_member<T>::value();
                    }

                    template<typename T>
                    static constexpr thread_t static_query(
                        std::enable_if_t<
                            !query_static_constexpr_member<T>::is_valid
                        > * = 0,
                        std::enable_if_t<
                            !query_member<T>::is_valid
                        > * = 0,
                        std::enable_if_t<
                            !traits::query_free<T, thread_t>::is_valid
                        > * = 0,
                        std::enable_if_t<
                            !can_query<T, new_thread_t<I> >::value
                        > * = 0,
                        std::enable_if_t<
                            !can_query<T, other_t<I> >::value
                        > * = 0) noexcept {
                        return thread_t();
                    }

                    template<typename E, typename T = decltype(thread_t::static_query<E>())>
                    static constexpr const T static_query_v
                            = thread_t::static_query<E>();


                    static constexpr mapping_t<I> value() {
                        return thread_t();
                    }

                    friend constexpr bool operator==(const thread_t &, const thread_t &) {
                        return true;
                    }

                    friend constexpr bool operator!=(const thread_t &, const thread_t &) {
                        return false;
                    }

                    friend constexpr bool operator==(const thread_t &, const new_thread_t<I> &) {
                        return false;
                    }

                    friend constexpr bool operator!=(const thread_t &, const new_thread_t<I> &) {
                        return true;
                    }

                    friend constexpr bool operator==(const thread_t &, const other_t<I> &) {
                        return false;
                    }

                    friend constexpr bool operator!=(const thread_t &, const other_t<I> &) {
                        return true;
                    }
                };


                template<int I>
                template<typename E, typename T>
                const T thread_t<I>::static_query_v;

                template<int I = 0>
                struct new_thread_t {
                    template<typename T>
                    static constexpr bool is_applicable_property_v = is_executor<T>::value;

                    static constexpr bool is_requirable = true;
                    static constexpr bool is_preferable = true;
                    typedef mapping_t<I> polymorphic_query_result_type;

                    constexpr new_thread_t() {
                    }

                    template<typename T>
                    struct query_member :
                            traits::query_member<
                                typename mapping_t<I>::template proxy<T>::type, new_thread_t> {
                    };

                    template<typename T>
                    struct query_static_constexpr_member :
                            traits::query_static_constexpr_member<
                                typename mapping_t<I>::template static_proxy<T>::type, new_thread_t> {
                    };

                    template<typename T>
                    static constexpr typename query_static_constexpr_member<T>::result_type
                    static_query()
                        noexcept(query_static_constexpr_member<T>::is_noexcept) {
                        return query_static_constexpr_member<T>::value();
                    }

                    template<typename E, typename T = decltype(new_thread_t::static_query<E>())>
                    static constexpr const T static_query_v = new_thread_t::static_query<E>();


                    static constexpr mapping_t<I> value() {
                        return new_thread_t();
                    }

                    friend constexpr bool operator==(const new_thread_t &, const new_thread_t &) {
                        return true;
                    }

                    friend constexpr bool operator!=(const new_thread_t &, const new_thread_t &) {
                        return false;
                    }

                    friend constexpr bool operator==(const new_thread_t &, const thread_t<I> &) {
                        return false;
                    }

                    friend constexpr bool operator!=(const new_thread_t &, const thread_t<I> &) {
                        return true;
                    }

                    friend constexpr bool operator==(const new_thread_t &, const other_t<I> &) {
                        return false;
                    }

                    friend constexpr bool operator!=(const new_thread_t &, const other_t<I> &) {
                        return true;
                    }
                };

                template<int I>
                template<typename E, typename T>
                const T new_thread_t<I>::static_query_v;

                template<int I>
                struct other_t {
                    template<typename T>
                    static constexpr bool is_applicable_property_v = is_executor<T>::value;

                    static constexpr bool is_requirable = true;
                    static constexpr bool is_preferable = true;
                    typedef mapping_t<I> polymorphic_query_result_type;

                    constexpr other_t() {
                    }

                    template<typename T>
                    struct query_member :
                            traits::query_member<
                                typename mapping_t<I>::template proxy<T>::type, other_t> {
                    };

                    template<typename T>
                    struct query_static_constexpr_member :
                            traits::query_static_constexpr_member<
                                typename mapping_t<I>::template static_proxy<T>::type, other_t> {
                    };

                    template<typename T>
                    static constexpr
                    typename query_static_constexpr_member<T>::result_type
                    static_query()
                        noexcept(query_static_constexpr_member<T>::is_noexcept) {
                        return query_static_constexpr_member<T>::value();
                    }

                    template<typename E, typename T = decltype(other_t::static_query<E>())>
                    static constexpr const T static_query_v = other_t::static_query<E>();


                    static constexpr mapping_t<I> value() {
                        return other_t();
                    }

                    friend constexpr bool operator==(const other_t &, const other_t &) {
                        return true;
                    }

                    friend constexpr bool operator!=(const other_t &, const other_t &) {
                        return false;
                    }

                    friend constexpr bool operator==(const other_t &, const thread_t<I> &) {
                        return false;
                    }

                    friend constexpr bool operator!=(const other_t &, const thread_t<I> &) {
                        return true;
                    }

                    friend constexpr bool operator==(const other_t &, const new_thread_t<I> &) {
                        return false;
                    }

                    friend constexpr bool operator!=(const other_t &, const new_thread_t<I> &) {
                        return true;
                    }
                };

                template<int I>
                template<typename E, typename T>
                const T other_t<I>::static_query_v;
            } // namespace mapping
        } // namespace detail

        typedef detail::mapping_t<> mapping_t;

        inline constexpr mapping_t mapping;
    } // namespace execution
} // namespace xio

#include <xio/detail/pop_options.h>

#endif // XIO_EXECUTION_MAPPING_HPP
