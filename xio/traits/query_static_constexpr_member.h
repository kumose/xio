//
// traits/query_static_constexpr_member.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef XIO_TRAITS_QUERY_STATIC_CONSTEXPR_MEMBER_HPP
#define XIO_TRAITS_QUERY_STATIC_CONSTEXPR_MEMBER_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>
#include <xio/detail/type_traits.h>
#include <xio/detail/push_options.h>

namespace xio {


    namespace traits {
        template<typename T, typename Property, typename = void>
        struct query_static_constexpr_member_default;

        template<typename T, typename Property, typename = void>
        struct query_static_constexpr_member;
    } // namespace traits
    namespace detail {
        struct no_query_static_constexpr_member {
            static constexpr bool is_valid = false;
        };

        template<typename T, typename Property, typename = void>
        struct query_static_constexpr_member_trait :
                std::conditional_t<
                    std::is_same<T, std::decay_t<T> >::value
                    && std::is_same<Property, std::decay_t<Property> >::value,
                    no_query_static_constexpr_member,
                    traits::query_static_constexpr_member<
                        std::decay_t<T>,
                        std::decay_t<Property> >
                > {
        };


        template<typename T, typename Property>
        struct query_static_constexpr_member_trait<T, Property,
                    std::enable_if_t<
                        (static_cast<void>(T::query(Property{})), true)
                    > > {
            static constexpr bool is_valid = true;

            using result_type = decltype(T::query(Property{}));

            static constexpr bool is_noexcept = noexcept(T::query(Property{}));

            static constexpr result_type value() noexcept(is_noexcept) {
                return T::query(Property{});
            }
        };

    } // namespace detail
    namespace traits {
        template<typename T, typename Property, typename>
        struct query_static_constexpr_member_default :
                detail::query_static_constexpr_member_trait<T, Property> {
        };

        template<typename T, typename Property, typename>
        struct query_static_constexpr_member :
                query_static_constexpr_member_default<T, Property> {
        };
    } // namespace traits

} // namespace xio

#include <xio/detail/pop_options.h>

#endif // XIO_TRAITS_QUERY_STATIC_CONSTEXPR_MEMBER_HPP
