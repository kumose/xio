//
// experimental/concurrent_channel.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_EXPERIMENTAL_CONCURRENT_CHANNEL_HPP
#define ASIO_EXPERIMENTAL_CONCURRENT_CHANNEL_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>
#include <xio/any_io_executor.h>
#include <xio/detail/type_traits.h>
#include <xio/execution/executor.h>
#include <xio/is_executor.h>
#include <xio/experimental/basic_concurrent_channel.h>
#include <xio/experimental/channel_traits.h>

#include <xio/detail/push_options.h>

namespace xio {
    ASIO_INLINE_NAMESPACE_BEGIN

    namespace experimental {
        namespace detail {
            template<typename ExecutorOrSignature, typename = void>
            struct concurrent_channel_type {
                template<typename... Signatures>
                struct inner {
                    typedef basic_concurrent_channel<any_io_executor, channel_traits<>,
                        ExecutorOrSignature, Signatures...> type;
                };
            };

            template<typename ExecutorOrSignature>
            struct concurrent_channel_type<ExecutorOrSignature,
                enable_if_t <
                is_executor<ExecutorOrSignature>::value
                || execution::is_executor<ExecutorOrSignature>::value
            >
            >
{
  template <typename... Signatures>
  struct inner
  {
    typedef basic_concurrent_channel<ExecutorOrSignature,
        channel_traits<>, Signatures...> type;
  };
};
        } // namespace detail

        /// Template type alias for common use of channel.
#if defined(GENERATING_DOCUMENTATION)
        template<typename ExecutorOrSignature, typename... Signatures>
        using concurrent_channel = basic_concurrent_channel<
            specified_executor_or_any_io_executor, channel_traits<>, signatures...>;
#else // defined(GENERATING_DOCUMENTATION)
        template<typename ExecutorOrSignature, typename... Signatures>
        using concurrent_channel = typename detail::concurrent_channel_type<
            ExecutorOrSignature>::template inner<Signatures...>::type;
#endif // defined(GENERATING_DOCUMENTATION)
    } // namespace experimental
    ASIO_INLINE_NAMESPACE_END
} // namespace xio

#include <xio/detail/pop_options.h>

#endif // ASIO_EXPERIMENTAL_CONCURRENT_CHANNEL_HPP
