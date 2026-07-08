//
// traits/execute_member.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef XIO_TRAITS_EXECUTE_MEMBER_HPP
#define XIO_TRAITS_EXECUTE_MEMBER_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>
#include <xio/detail/type_traits.h>
#include <xio/detail/push_options.h>

namespace xio {


    namespace traits {
        template<typename T, typename F, typename = void>
        struct execute_member_default;

        template<typename T, typename F, typename = void>
        struct execute_member;
    } // namespace traits
    namespace detail {
        struct no_execute_member {
            static constexpr bool is_valid = false;
            static constexpr bool is_noexcept = false;
        };


        template<typename T, typename F, typename = void>
        struct execute_member_trait : no_execute_member {
        };

        template<typename T, typename F>
        struct execute_member_trait<T, F,
                    void_t<
                        decltype(std::declval<T>().execute(std::declval<F>()))
                    > > {
            static constexpr bool is_valid = true;

            using result_type = decltype(
                std::declval<T>().execute(std::declval<F>()));

            static constexpr bool is_noexcept =
                    noexcept(std::declval<T>().execute(std::declval<F>()));
        };

    } // namespace detail
    namespace traits {
        template<typename T, typename F, typename>
        struct execute_member_default :
                detail::execute_member_trait<T, F> {
        };

        template<typename T, typename F, typename>
        struct execute_member :
                execute_member_default<T, F> {
        };
    } // namespace traits

} // namespace xio

#include <xio/detail/pop_options.h>

#endif // XIO_TRAITS_EXECUTE_MEMBER_HPP
