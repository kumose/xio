//
// detail/non_const_lvalue.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef XIO_DETAIL_NON_CONST_LVALUE_HPP
#define XIO_DETAIL_NON_CONST_LVALUE_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>
#include <xio/detail/type_traits.h>

#include <xio/detail/push_options.h>

namespace xio {


    namespace detail {
        template<typename T>
        struct non_const_lvalue {
            explicit non_const_lvalue(T &t)
                : value(static_cast<std::conditional_t<
                    std::is_same<T, std::decay_t<T> >::value, std::decay_t<T> &, T &&>>(t)) {
            }

            std::conditional_t<std::is_same<T, std::decay_t<T> >::value, std::decay_t<T> &, std::decay_t<T> > value;
        };
    } // namespace detail

} // namespace xio

#include <xio/detail/pop_options.h>

#endif // XIO_DETAIL_NON_CONST_LVALUE_HPP
