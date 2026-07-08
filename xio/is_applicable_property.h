//
// is_applicable_property.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef XIO_IS_APPLICABLE_PROPERTY_HPP
#define XIO_IS_APPLICABLE_PROPERTY_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>
#include <xio/detail/type_traits.h>

namespace xio {


    namespace detail {
        template<typename T, typename Property, typename = void>
        struct is_applicable_property_trait : std::false_type {
        };


        template<typename T, typename Property>
        struct is_applicable_property_trait<T, Property,
            void_t <
            std::enable_if_t <
            !!Property::template is_applicable_property_v<T>
        >
  >> : std::true_type {
        };

    } // namespace detail

    template<typename T, typename Property, typename = void>
    struct is_applicable_property :
            detail::is_applicable_property_trait<T, Property> {
    };


    template<typename T, typename Property>
    constexpr const bool is_applicable_property_v
            = is_applicable_property<T, Property>::value;


} // namespace xio

#endif // XIO_IS_APPLICABLE_PROPERTY_HPP
