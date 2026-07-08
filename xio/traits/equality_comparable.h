//
// traits/equality_comparable.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef XIO_TRAITS_EQUALITY_COMPARABLE_HPP
#define XIO_TRAITS_EQUALITY_COMPARABLE_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>
#include <xio/detail/type_traits.h>

namespace xio {


    namespace traits {
        template<typename T, typename = void>
        struct equality_comparable_default;

        template<typename T, typename = void>
        struct equality_comparable;
    } // namespace traits
    namespace detail {
        struct no_equality_comparable {
            static constexpr bool is_valid = false;
            static constexpr bool is_noexcept = false;
        };


        template<typename T, typename = void>
        struct equality_comparable_trait : no_equality_comparable {
        };

        template<typename T>
        struct equality_comparable_trait<T,
                    void_t<
                        decltype(
                            static_cast<void>(
                                static_cast<bool>(std::declval<const T>() == std::declval<const T>())
                            ),
                            static_cast<void>(
                                static_cast<bool>(std::declval<const T>() != std::declval<const T>())
                            )
                        )
                    > > {
            static constexpr bool is_valid = true;

            static constexpr bool is_noexcept =
                    noexcept(std::declval<const T>() == std::declval<const T>())
                    && noexcept(std::declval<const T>() != std::declval<const T>());
        };

    } // namespace detail
    namespace traits {
        template<typename T, typename>
        struct equality_comparable_default : detail::equality_comparable_trait<T> {
        };

        template<typename T, typename>
        struct equality_comparable : equality_comparable_default<T> {
        };
    } // namespace traits

} // namespace xio

#endif // XIO_TRAITS_EQUALITY_COMPARABLE_HPP
