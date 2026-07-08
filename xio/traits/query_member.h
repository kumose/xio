//
// traits/query_member.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef XIO_TRAITS_QUERY_MEMBER_HPP
#define XIO_TRAITS_QUERY_MEMBER_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>
#include <xio/detail/type_traits.h>
#include <xio/detail/push_options.h>

namespace xio {


    namespace traits {
        template<typename T, typename Property, typename = void>
        struct query_member_default;

        template<typename T, typename Property, typename = void>
        struct query_member;
    } // namespace traits
    namespace detail {
        struct no_query_member {
            static constexpr bool is_valid = false;
            static constexpr bool is_noexcept = false;
        };


        template<typename T, typename Property, typename = void>
        struct query_member_trait : no_query_member {
        };

        template<typename T, typename Property>
        struct query_member_trait<T, Property,
                    void_t<
                        decltype(std::declval<T>().query(std::declval<Property>()))
                    > > {
            static constexpr bool is_valid = true;

            using result_type = decltype(
                std::declval<T>().query(std::declval<Property>()));

            static constexpr bool is_noexcept =
                    noexcept(std::declval<T>().query(std::declval<Property>()));
        };

    } // namespace detail
    namespace traits {
        template<typename T, typename Property, typename>
        struct query_member_default :
                detail::query_member_trait<T, Property> {
        };

        template<typename T, typename Property, typename>
        struct query_member :
                query_member_default<T, Property> {
        };
    } // namespace traits

} // namespace xio

#include <xio/detail/pop_options.h>

#endif // XIO_TRAITS_QUERY_MEMBER_HPP
