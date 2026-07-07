//
// connect_pipe.hpp
// ~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_CONNECT_PIPE_HPP
#define ASIO_CONNECT_PIPE_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>

#if defined(ASIO_HAS_PIPE) \
  || defined(GENERATING_DOCUMENTATION)

#include <xio/basic_readable_pipe.h>
#include <xio/basic_writable_pipe.h>
#include <xio/error.h>

#include <xio/detail/push_options.h>

namespace xio {


    namespace detail {
#if defined(ASIO_HAS_IOCP)
        typedef HANDLE native_pipe_handle;
#else // defined(ASIO_HAS_IOCP)
        typedef int native_pipe_handle;
#endif // defined(ASIO_HAS_IOCP)

        ASIO_DECL void create_pipe(native_pipe_handle p[2],
                                   xio::error_code &ec);

        ASIO_DECL void close_pipe(native_pipe_handle p);
    } // namespace detail

    /// Connect two pipe ends using an anonymous pipe.
    /**
     * @param read_end The read end of the pipe.
     *
     * @param write_end The write end of the pipe.
     *
     * @throws std::system_error Thrown on failure.
     */
    template<typename Executor1, typename Executor2>
    void connect_pipe(basic_readable_pipe<Executor1> &read_end,
                      basic_writable_pipe<Executor2> &write_end);

    /// Connect two pipe ends using an anonymous pipe.
    /**
     * @param read_end The read end of the pipe.
     *
     * @param write_end The write end of the pipe.
     *
     * @throws std::system_error Thrown on failure.
     *
     * @param ec Set to indicate what error occurred, if any.
     */
    template<typename Executor1, typename Executor2>
    ASIO_SYNC_OP_VOID connect_pipe(basic_readable_pipe<Executor1> &read_end,
                                   basic_writable_pipe<Executor2> &write_end, xio::error_code &ec);

    } // namespace xio

#include <xio/detail/pop_options.h>

#include <xio/impl/connect_pipe.h>


#endif // defined(ASIO_HAS_PIPE)
//   || defined(GENERATING_DOCUMENTATION)

#endif // ASIO_CONNECT_PIPE_HPP
