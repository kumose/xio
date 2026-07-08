//
// impl/connect_pipe.hpp
// ~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef XIO_IMPL_CONNECT_PIPE_HPP
#define XIO_IMPL_CONNECT_PIPE_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>

#if defined(XIO_HAS_PIPE)

#include <xio/connect_pipe.h>
#include <xio/detail/throw_error.h>

#include <xio/detail/push_options.h>

namespace xio {


    template<typename Executor1, typename Executor2>
    void connect_pipe(basic_readable_pipe<Executor1> &read_end,
                      basic_writable_pipe<Executor2> &write_end) {
        xio::error_code ec;
        xio::connect_pipe(read_end, write_end, ec);
        xio::detail::throw_error(ec, "connect_pipe");
    }

    template<typename Executor1, typename Executor2>
    XIO_SYNC_OP_VOID connect_pipe(basic_readable_pipe<Executor1> &read_end,
                                   basic_writable_pipe<Executor2> &write_end, xio::error_code &ec) {
        detail::native_pipe_handle p[2];
        detail::create_pipe(p, ec);
        if (ec)
            XIO_SYNC_OP_VOID_RETURN(ec);

        read_end.assign(p[0], ec);
        if (ec) {
            detail::close_pipe(p[0]);
            detail::close_pipe(p[1]);
            XIO_SYNC_OP_VOID_RETURN(ec);
        }

        write_end.assign(p[1], ec);
        if (ec) {
            xio::error_code temp_ec;
            read_end.close(temp_ec);
            detail::close_pipe(p[1]);
            XIO_SYNC_OP_VOID_RETURN(ec);
        }

        XIO_SYNC_OP_VOID_RETURN(ec);
    }


} // namespace xio

#include <xio/detail/pop_options.h>

#endif // defined(XIO_HAS_PIPE)

#endif // XIO_IMPL_CONNECT_PIPE_HPP
