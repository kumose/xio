//
// traits/static_query.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef XIO_TRAITS_STATIC_QUERY_HPP
#define XIO_TRAITS_STATIC_QUERY_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>
#include <xio/detail/type_traits.h>
#include <xio/detail/push_options.h>

namespace xio {


    namespace traits {
        template<typename T, typename Property, typename = void>
        struct static_query_default;

        template<typename T, typename Property, typename = void>
        struct static_query;
    } // namespace traits
    namespace detail {
        struct no_static_query {
            static constexpr bool is_valid = false;
            static constexpr bool is_noexcept = false;
        };

        template<typename T, typename Property, typename = void>
        struct static_query_trait :
                std::conditional_t<
                    std::is_same<T, std::decay_t<T> >::value
                    && std::is_same<Property, std::decay_t<Property> >::value,
                    no_static_query,
                    traits::static_query<
                        std::decay_t<T>,
                        std::decay_t<Property> >
                > {
        };


        template<typename T, typename Property>
        struct static_query_trait<T, Property,
                    void_t<
                        decltype(std::decay_t<Property>::template static_query_v<T>)
                    > > {
            static constexpr bool is_valid = true;

            using result_type = decltype(
                std::decay_t<Property>::template static_query_v<T>);

            static constexpr bool is_noexcept =
                    noexcept(std::decay_t<Property>::template static_query_v<T>);

            static constexpr result_type value() noexcept(is_noexcept) {
                return std::decay_t<Property>::template static_query_v<T>;
            }
        };

    } // namespace detail
    namespace traits {
        template<typename T, typename Property, typename>
        struct static_query_default : detail::static_query_trait<T, Property> {
        };

        template<typename T, typename Property, typename>
        struct static_query : static_query_default<T, Property> {
        };
    } // namespace traits

} // namespace xio

#include <xio/detail/pop_options.h>

#endif // XIO_TRAITS_STATIC_QUERY_HPP
