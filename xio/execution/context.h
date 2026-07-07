//
// execution/context.hpp
// ~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_EXECUTION_CONTEXT2_HPP
#define ASIO_EXECUTION_CONTEXT2_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>
#include <xio/detail/type_traits.h>
#include <xio/execution/executor.h>
#include <xio/is_applicable_property.h>
#include <xio/traits/query_static_constexpr_member.h>
#include <xio/traits/static_query.h>
#include <any>

#include <xio/detail/push_options.h>

namespace xio {


#if defined(GENERATING_DOCUMENTATION)

    namespace execution {
        /// A property that is used to obtain the execution context that is associated
/// with an executor.
        struct context_t {
            /// The context_t property applies to executors.
            template<typename T>
            static constexpr bool is_applicable_property_v = is_executor_v<T>;

            /// The context_t property cannot be required.
            static constexpr bool is_requirable = false;

            /// The context_t property cannot be preferred.
            static constexpr bool is_preferable = false;

            /// The type returned by queries against an @c any_executor.
            typedef std::any polymorphic_query_result_type;
        };

        /// A special value used for accessing the context_t property.
        constexpr context_t context;
    } // namespace execution

#else // defined(GENERATING_DOCUMENTATION)




    namespace execution {
        namespace detail {
            template<int I = 0>
            struct context_t {
                template<typename T>
                static constexpr bool is_applicable_property_v = is_executor<T>::value;

                static constexpr bool is_requirable = false;
                static constexpr bool is_preferable = false;

                typedef std::any polymorphic_query_result_type;

                constexpr context_t() {
                }

                template<typename T>
                struct static_proxy {
                    struct type {
                        template<typename P>
                        static constexpr auto query(P &&p)
                            noexcept(
                                noexcept(
                                    conditional_t < true, T, P > ::query(static_cast<P &&>(p))
                                )
                            )
                            -> decltype(
                                conditional_t < true, T, P > ::query(static_cast<P &&>(p))
                            ) {
                            return T::query(static_cast<P &&>(p));
                        }
                    };
                };

                template<typename T>
                struct query_static_constexpr_member :
                        traits::query_static_constexpr_member<
                            typename static_proxy<T>::type, context_t> {
                };

                template<typename T>
                static constexpr typename query_static_constexpr_member<T>::result_type
                static_query()
                    noexcept(query_static_constexpr_member<T>::is_noexcept) {
                    return query_static_constexpr_member<T>::value();
                }

                template<typename E, typename T = decltype(context_t::static_query<E>())>
                static constexpr const T static_query_v = context_t::static_query<E>();

            };


            template<int I>
            template<typename E, typename T>
            const T context_t<I>::static_query_v;

        } // namespace detail

        typedef detail::context_t<> context_t;

inline constexpr context_t context;
    } // namespace execution


#endif // defined(GENERATING_DOCUMENTATION)


} // namespace xio

#include <xio/detail/pop_options.h>

#endif // ASIO_EXECUTION_CONTEXT2_HPP
