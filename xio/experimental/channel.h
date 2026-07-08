//
// experimental/channel.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef XIO_EXPERIMENTAL_CHANNEL_HPP
#define XIO_EXPERIMENTAL_CHANNEL_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>
#include <xio/any_io_executor.h>
#include <xio/detail/type_traits.h>
#include <xio/execution/executor.h>
#include <xio/is_executor.h>
#include <xio/experimental/basic_channel.h>
#include <xio/experimental/channel_traits.h>

#include <xio/detail/push_options.h>

namespace xio {


    namespace experimental {
        namespace detail {
            template<typename ExecutorOrSignature, typename = void>
            struct channel_type {
                template<typename... Signatures>
                struct inner {
                    typedef basic_channel<any_io_executor, channel_traits<>,
                        ExecutorOrSignature, Signatures...> type;
                };
            };

            template<typename ExecutorOrSignature>
            struct channel_type<ExecutorOrSignature,
                std::enable_if_t <
                is_executor<ExecutorOrSignature>::value
                || execution::is_executor<ExecutorOrSignature>::value
            >
            >
{
  template <typename... Signatures>
  struct inner
  {
    typedef basic_channel<ExecutorOrSignature,
        channel_traits<>, Signatures...> type;
  };
};
        } // namespace detail

        /// Template type alias for common use of channel.

        template<typename ExecutorOrSignature, typename... Signatures>
        using channel = typename detail::channel_type<
            ExecutorOrSignature>::template inner<Signatures...>::type;
    } // namespace experimental

} // namespace xio

#include <xio/detail/pop_options.h>

#endif // XIO_EXPERIMENTAL_CHANNEL_HPP
