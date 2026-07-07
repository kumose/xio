//
// experimental/impl/channel_error.ipp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_EXPERIMENTAL_IMPL_CHANNEL_ERROR_IPP
#define ASIO_EXPERIMENTAL_IMPL_CHANNEL_ERROR_IPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>
#include <xio/experimental/channel_error.h>

#include <xio/detail/push_options.h>

namespace xio {


    namespace experimental {
        namespace error {
            namespace detail {
                class channel_category : public xio::error_category {
                public:
                    const char *name() const noexcept {
                        return "xio.channel";
                    }

                    std::string message(int value) const {
                        switch (value) {
                            case channel_closed: return "Channel closed";
                            case channel_cancelled: return "Channel cancelled";
                            default: return "xio.channel error";
                        }
                    }
                };
            } // namespace detail

            const xio::error_category &get_channel_category() {
                static detail::channel_category instance;
                return instance;
            }
        } // namespace error
    } // namespace experimental

} // namespace xio

#include <xio/detail/pop_options.h>

#endif // ASIO_EXPERIMENTAL_IMPL_CHANNEL_ERROR_IPP
