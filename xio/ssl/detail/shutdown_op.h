//
// ssl/detail/shutdown_op.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef XIO_SSL_DETAIL_SHUTDOWN_OP_HPP
#define XIO_SSL_DETAIL_SHUTDOWN_OP_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>

#include <xio/ssl/detail/engine.h>

#include <xio/detail/push_options.h>

namespace xio {


    namespace ssl {
        namespace detail {
            class shutdown_op {
            public:
                static constexpr const char *tracking_name() {
                    return "ssl::stream<>::async_shutdown";
                }

                engine::want operator()(engine &eng,
                                        xio::error_code &ec,
                                        std::size_t &bytes_transferred) const {
                    bytes_transferred = 0;
                    return eng.shutdown(ec);
                }

                void complete_sync(xio::error_code &ec) const {
                    if (ec == xio::error::eof) {
                        // The engine only generates an eof when the shutdown notification has
                        // been received from the peer. This indicates that the shutdown has
                        // completed successfully, and thus need not be returned to the caller.
                        ec = xio::error_code();
                    }
                }

                template<typename Handler>
                void call_handler(Handler &handler,
                                  const xio::error_code &ec,
                                  const std::size_t &) const {
                    if (ec == xio::error::eof) {
                        // The engine only generates an eof when the shutdown notification has
                        // been received from the peer. This indicates that the shutdown has
                        // completed successfully, and thus need not be passed on to the handler.
                        static_cast<Handler &&>(handler)(xio::error_code());
                    } else {
                        static_cast<Handler &&>(handler)(ec);
                    }
                }
            };
        } // namespace detail
    } // namespace ssl

} // namespace xio

#include <xio/detail/pop_options.h>

#endif // XIO_SSL_DETAIL_SHUTDOWN_OP_HPP
