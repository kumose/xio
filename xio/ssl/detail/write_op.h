//
// ssl/detail/write_op.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_SSL_DETAIL_WRITE_OP_HPP
#define ASIO_SSL_DETAIL_WRITE_OP_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>

#include <xio/detail/buffer_sequence_adapter.h>
#include <xio/ssl/detail/engine.h>

#include <xio/detail/push_options.h>

namespace xio {


    namespace ssl {
        namespace detail {
            template<typename ConstBufferSequence>
            class write_op {
            public:
                static constexpr const char *tracking_name() {
                    return "ssl::stream<>::async_write_some";
                }

                write_op(const ConstBufferSequence &buffers)
                    : buffers_(buffers) {
                }

                engine::want operator()(engine &eng,
                                        xio::error_code &ec,
                                        std::size_t &bytes_transferred) const {
                    unsigned char storage[
                        xio::detail::buffer_sequence_adapter<xio::const_buffer,
                            ConstBufferSequence>::linearisation_storage_size];

                    xio::const_buffer buffer =
                            xio::detail::buffer_sequence_adapter<xio::const_buffer,
                                ConstBufferSequence>::linearise(buffers_, xio::buffer(storage));

                    return eng.write(buffer, ec, bytes_transferred);
                }

                void complete_sync(xio::error_code &) const {
                }

                template<typename Handler>
                void call_handler(Handler &handler,
                                  const xio::error_code &ec,
                                  const std::size_t &bytes_transferred) const {
                    static_cast<Handler &&>(handler)(ec, bytes_transferred);
                }

            private:
                ConstBufferSequence buffers_;
            };
        } // namespace detail
    } // namespace ssl

} // namespace xio

#include <xio/detail/pop_options.h>

#endif // ASIO_SSL_DETAIL_WRITE_OP_HPP
