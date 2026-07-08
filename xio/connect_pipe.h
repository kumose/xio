//
// connect_pipe.hpp
// ~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef XIO_CONNECT_PIPE_HPP
#define XIO_CONNECT_PIPE_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>

#if defined(XIO_HAS_PIPE)

#include <xio/basic_readable_pipe.h>
#include <xio/basic_writable_pipe.h>
#include <xio/error.h>

#include <xio/detail/push_options.h>

namespace xio {


    namespace detail {
#if defined(XIO_HAS_IOCP)
        typedef HANDLE native_pipe_handle;
#else // defined(XIO_HAS_IOCP)
        typedef int native_pipe_handle;
#endif // defined(XIO_HAS_IOCP)

        XIO_DECL void create_pipe(native_pipe_handle p[2],
                                   xio::error_code &ec);

        XIO_DECL void close_pipe(native_pipe_handle p);
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
    XIO_SYNC_OP_VOID connect_pipe(basic_readable_pipe<Executor1> &read_end,
                                   basic_writable_pipe<Executor2> &write_end, xio::error_code &ec);

    } // namespace xio

#include <xio/detail/pop_options.h>

#include <xio/impl/connect_pipe.h>


#endif // defined(XIO_HAS_PIPE)

#endif // XIO_CONNECT_PIPE_HPP
