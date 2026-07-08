//
// ssl/detail/engine.hpp
// ~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef XIO_SSL_DETAIL_ENGINE_HPP
#define XIO_SSL_DETAIL_ENGINE_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>

#include <xio/buffer.h>
#include <xio/detail/static_mutex.h>
#include <xio/ssl/detail/openssl_types.h>
#include <xio/ssl/detail/verify_callback.h>
#include <xio/ssl/stream_base.h>
#include <xio/ssl/verify_mode.h>

#include <xio/detail/push_options.h>

namespace xio {


    namespace ssl {
        namespace detail {
            class engine {
            public:
                enum want {
                    // Returned by functions to indicate that the engine wants input. The input
                    // buffer should be updated to point to the data. The engine then needs to
                    // be called again to retry the operation.
                    want_input_and_retry = -2,

                    // Returned by functions to indicate that the engine wants to write output.
                    // The output buffer points to the data to be written. The engine then
                    // needs to be called again to retry the operation.
                    want_output_and_retry = -1,

                    // Returned by functions to indicate that the engine doesn't need input or
                    // output.
                    want_nothing = 0,

                    // Returned by functions to indicate that the engine wants to write output.
                    // The output buffer points to the data to be written. After that the
                    // operation is complete, and the engine does not need to be called again.
                    want_output = 1
                };

                // Construct a new engine for the specified context.
                XIO_DECL engine(SSL_CTX *context,
                                 std::size_t output_buffer_size, std::size_t input_buffer_size);

                // Construct a new engine for an existing native SSL implementation.
                XIO_DECL engine(SSL *ssl_impl,
                                 std::size_t output_buffer_size, std::size_t input_buffer_size);

                // Move construct from another engine.
                XIO_DECL engine(engine &&other) noexcept;

                // Destructor.
                XIO_DECL ~engine();

                // Move assign from another engine.
  XIO_DECL engine &operator=(engine &&other) noexcept;

                // Get the underlying implementation in the native type.
  XIO_DECL SSL *native_handle();

                // Set the peer verification mode.
                XIO_DECL xio::error_code set_verify_mode(
                    verify_mode v, xio::error_code &ec);

                // Set the peer verification depth.
                XIO_DECL xio::error_code set_verify_depth(
                    int depth, xio::error_code &ec);

                // Set a peer certificate verification callback.
                XIO_DECL xio::error_code set_verify_callback(
                    verify_callback_base *callback, xio::error_code &ec);

                // Perform an SSL handshake using either SSL_connect (client-side) or
                // SSL_accept (server-side).
  XIO_DECL want handshake(
                    stream_base::handshake_type type, xio::error_code &ec);

                // Perform a graceful shutdown of the SSL session.
  XIO_DECL want shutdown(xio::error_code &ec);

                // Write bytes to the SSL session.
  XIO_DECL want write(const xio::const_buffer &data,
                       xio::error_code &ec, std::size_t &bytes_transferred);

                // Read bytes from the SSL session.
  XIO_DECL want read(const xio::mutable_buffer &data,
                      xio::error_code &ec, std::size_t &bytes_transferred);

                // Get output data to be written to the transport.
                XIO_DECL xio::mutable_buffer get_output(
                    const xio::mutable_buffer &data);

                // Put input data that was read from the transport.
                XIO_DECL xio::const_buffer put_input(
                    const xio::const_buffer &data);

                // Map an error::eof code returned by the underlying transport according to
                // the type and state of the SSL session. Returns a const reference to the
                // error code object, suitable for passing to a completion handler.
                XIO_DECL const xio::error_code &map_error_code(
                    xio::error_code &ec) const;

            private:
                // Disallow copying and assignment.
                engine(const engine &);

                engine &operator=(const engine &);

                // Helper to complete construction of the engine.
  XIO_DECL void init(std::size_t output_buffer_size,
                      std::size_t input_buffer_size);

                // Callback used when the SSL implementation wants to verify a certificate.
  XIO_DECL static int verify_callback_function(
                    int preverified, X509_STORE_CTX *ctx);

#if (OPENSSL_VERSION_NUMBER < 0x10000000L)
                // The SSL_accept function may not be thread safe. This mutex is used to
                // protect all calls to the SSL_accept function.
                XIO_DECL static xio::detail::static_mutex &accept_mutex();
#endif // (OPENSSL_VERSION_NUMBER < 0x10000000L)

                // Perform one operation. Returns >= 0 on success or error, want_read if the
                // operation needs more input, or want_write if it needs to write some output
                // before the operation can complete.
  XIO_DECL want perform(int (engine::*op)(void *, std::size_t),
                         void *data, std::size_t length, xio::error_code &ec,
                         std::size_t *bytes_transferred);

                // Adapt the SSL_accept function to the signature needed for perform().
  XIO_DECL int do_accept(void *, std::size_t);

                // Adapt the SSL_connect function to the signature needed for perform().
  XIO_DECL int do_connect(void *, std::size_t);

                // Adapt the SSL_shutdown function to the signature needed for perform().
  XIO_DECL int do_shutdown(void *, std::size_t);

                // Adapt the SSL_read function to the signature needed for perform().
  XIO_DECL int do_read(void *data, std::size_t length);

                // Adapt the SSL_write function to the signature needed for perform().
  XIO_DECL int do_write(void *data, std::size_t length);

                SSL *ssl_;
                BIO *ext_bio_;
            };
        } // namespace detail
    } // namespace ssl

} // namespace xio

#include <xio/detail/pop_options.h>


#endif // XIO_SSL_DETAIL_ENGINE_HPP
