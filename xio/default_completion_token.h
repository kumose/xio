//
// default_completion_token.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef XIO_DEFAULT_COMPLETION_TOKEN_HPP
#define XIO_DEFAULT_COMPLETION_TOKEN_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>
#include <xio/detail/type_traits.h>

#include <xio/detail/push_options.h>

namespace xio {


    class deferred_t;

    namespace detail {
        template<typename T, typename = void>
        struct default_completion_token_impl {
            typedef deferred_t type;
        };

        template<typename T>
        struct default_completion_token_impl<T,
                    void_t<typename T::default_completion_token_type>
                > {
            typedef typename T::default_completion_token_type type;
        };
    } // namespace detail

    template<typename T>
    struct default_completion_token
            : detail::default_completion_token_impl<T> {
    };

    template<typename T>
    using default_completion_token_t = typename default_completion_token<T>::type;

#define XIO_DEFAULT_COMPLETION_TOKEN_TYPE(e) \
  = typename ::xio::default_completion_token<e>::type
#define XIO_DEFAULT_COMPLETION_TOKEN(e) \
  = typename ::xio::default_completion_token<e>::type()


} // namespace xio

#include <xio/detail/pop_options.h>

#include <xio/deferred.h>

#endif // XIO_DEFAULT_COMPLETION_TOKEN_HPP
