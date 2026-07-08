//
// append.hpp
// ~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef XIO_APPEND_HPP
#define XIO_APPEND_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>
#include <tuple>
#include <xio/detail/type_traits.h>

#include <xio/detail/push_options.h>

namespace xio {


    /// Completion token type used to specify that the completion handler
/// arguments should be passed additional values after the results of the
/// operation.


    template<typename CompletionToken, typename... Values>
    class append_t {
    public:
        /// Constructor.
        template<typename T, typename... V>
        constexpr explicit append_t(T &&completion_token, V &&... values)
            : token_(static_cast<T &&>(completion_token)),
              values_(static_cast<V &&>(values)...) {
        }

        //private:
        CompletionToken token_;
        std::tuple<Values...> values_;
    };

    /// Completion token type used to specify that the completion handler
/// arguments should be passed additional values after the results of the
/// operation.
    template<typename CompletionToken, typename... Values>
    [[nodiscard]] inline constexpr
    append_t<std::decay_t<CompletionToken>, std::decay_t<Values>...>

    append(CompletionToken &&completion_token, Values &&... values) {
        return append_t<std::decay_t<CompletionToken>, std::decay_t<Values>...>(
            static_cast<CompletionToken &&>(completion_token),
            static_cast<Values &&>(values)...);
    }


} // namespace xio

#include <xio/detail/pop_options.h>

#include <xio/impl/append.h>

#endif // XIO_APPEND_HPP
