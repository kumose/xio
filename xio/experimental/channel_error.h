//
// experimental/channel_error.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef XIO_EXPERIMENTAL_CHANNEL_ERROR_HPP
#define XIO_EXPERIMENTAL_CHANNEL_ERROR_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>
#include <xio/error_code.h>

#include <xio/detail/push_options.h>

namespace xio {


    namespace experimental {
        namespace error {
            enum channel_errors {
                /// The channel was closed.
                channel_closed = 1,

                /// The channel was cancelled.
                channel_cancelled = 2
            };

            extern

            XIO_DECL
            const xio::error_category &get_channel_category();

            inline const xio::error_category &
  channel_category XIO_UNUSED_VARIABLE
                    = xio::experimental::error::get_channel_category();
        } // namespace error
        namespace channel_errc {
            // Simulates a scoped enum.
            using error::channel_closed;
            using error::channel_cancelled;
        } // namespace channel_errc
    } // namespace experimental

} // namespace xio

namespace std {
    template<>
    struct is_error_code_enum<
                xio::experimental::error::channel_errors> {
        static const bool value = true;
    };
} // namespace std

namespace xio {


    namespace experimental {
        namespace error {
            inline xio::error_code make_error_code(channel_errors e) {
                return xio::error_code(
                    static_cast<int>(e), get_channel_category());
            }
        } // namespace error
    } // namespace experimental

} // namespace xio

#include <xio/detail/pop_options.h>


#endif // XIO_EXPERIMENTAL_CHANNEL_ERROR_HPP
