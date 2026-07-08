//
// traits/static_require_concept.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef XIO_TRAITS_STATIC_REQUIRE_CONCEPT_HPP
#define XIO_TRAITS_STATIC_REQUIRE_CONCEPT_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>
#include <xio/detail/type_traits.h>
#include <xio/traits/static_query.h>

#define XIO_HAS_DEDUCED_STATIC_REQUIRE_CONCEPT_TRAIT 1

#include <xio/detail/push_options.h>

namespace xio {


    namespace traits {
        template<typename T, typename Property, typename = void>
        struct static_require_concept_default;

        template<typename T, typename Property, typename = void>
        struct static_require_concept;
    } // namespace traits
    namespace detail {
        struct no_static_require_concept {
            static constexpr bool is_valid = false;
        };

        template<typename T, typename Property, typename = void>
        struct static_require_concept_trait :
                std::conditional_t<
                    std::is_same<T, std::decay_t<T> >::value
                    && std::is_same<Property, std::decay_t<Property> >::value,
                    no_static_require_concept,
                    traits::static_require_concept<
                        std::decay_t<T>,
                        std::decay_t<Property> >
                > {
        };


        template<typename T, typename Property>
        struct static_require_concept_trait<T, Property,
            std::enable_if_t <
            std::decay_t<Property>::value() == traits::static_query<T, Property>::value()
        >>
{
  static constexpr bool is_valid = true;
};


    } // namespace detail
    namespace traits {
        template<typename T, typename Property, typename>
        struct static_require_concept_default :
                detail::static_require_concept_trait<T, Property> {
        };

        template<typename T, typename Property, typename>
        struct static_require_concept : static_require_concept_default<T, Property> {
        };
    } // namespace traits

} // namespace xio

#include <xio/detail/pop_options.h>

#endif // XIO_TRAITS_STATIC_REQUIRE_CONCEPT_HPP
