//
// detail/signal_init.hpp
// ~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_DETAIL_SIGNAL_INIT_HPP
#define ASIO_DETAIL_SIGNAL_INIT_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>

#if !defined(ASIO_WINDOWS) \
  && !defined(ASIO_CYGWIN_W32_SOCKETS)

#include <csignal>

#include <xio/detail/push_options.h>

namespace xio {
    ASIO_INLINE_NAMESPACE_BEGIN

    namespace detail {
        template<int Signal = SIGPIPE>
        class signal_init {
        public:
            // Constructor.
            signal_init() {
                std::signal(Signal, SIG_IGN);
            }
        };
    } // namespace detail
    ASIO_INLINE_NAMESPACE_END
} // namespace xio

#include <xio/detail/pop_options.h>

#endif // !defined(ASIO_WINDOWS)
//   && !defined(ASIO_CYGWIN_W32_SOCKETS)

#endif // ASIO_DETAIL_SIGNAL_INIT_HPP
