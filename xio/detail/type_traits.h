//
// detail/type_traits.hpp
// ~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_DETAIL_TYPE_TRAITS_HPP
#define ASIO_DETAIL_TYPE_TRAITS_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>
#include <type_traits>

namespace xio {


    template<std::size_t N, std::size_t A>
    struct aligned_storage {
        struct type {
            alignas(A) unsigned char data[N];
        };
    };

    template<std::size_t N, std::size_t A>
    using aligned_storage_t = typename aligned_storage<N, A>::type;


    template<typename T>
    struct remove_cvref :
            std::remove_cv<typename std::remove_reference<T>::type> {
    };

    template<typename T>
    using remove_cvref_t = typename remove_cvref<T>::type;


    template<typename>
    struct result_of;

    template<typename F, typename... Args>
    struct result_of<F(Args...)> : std::invoke_result<F, Args...> {
    };

    template<typename T>
    using result_of_t = typename result_of<T>::type;



    template<typename>
    struct void_type {
        typedef void type;
    };

    template<typename T>
    using void_t = typename void_type<T>::type;

    template<typename...>
    struct conjunction : std::true_type {
    };

    template<typename T>
    struct conjunction<T> : T {
    };

    template<typename Head, typename... Tail>
    struct conjunction<Head, Tail...> :
            std::conditional_t<Head::value, conjunction<Tail...>, Head> {
    };

    struct defaulted_constraint {
        constexpr defaulted_constraint() {
        }
    };

    template<bool Condition, typename Type = int>
    struct constraint : std::enable_if<Condition, Type> {
    };

    template<bool Condition, typename Type = int>
    using constraint_t = typename constraint<Condition, Type>::type;

    template<typename T>
    struct type_identity {
        typedef T type;
    };

    template<typename T>
    using type_identity_t = typename type_identity<T>::type;


} // namespace xio

#endif // ASIO_DETAIL_TYPE_TRAITS_HPP
