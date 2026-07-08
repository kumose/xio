//
// detail/pipe_select_interrupter.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef XIO_DETAIL_PIPE_SELECT_INTERRUPTER_HPP
#define XIO_DETAIL_PIPE_SELECT_INTERRUPTER_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>

#if !defined(XIO_WINDOWS)
#if !defined(XIO_WINDOWS_RUNTIME)
#if !defined(XIO_CYGWIN_W32_SOCKETS)
#if !defined(__SYMBIAN32__)
#if !defined(XIO_HAS_EVENTFD)

#include <xio/detail/push_options.h>

namespace xio {


    namespace detail {
        class pipe_select_interrupter {
        public:
            // Constructor.
            XIO_DECL explicit pipe_select_interrupter(bool = true);

            // Destructor.
            XIO_DECL ~pipe_select_interrupter();

            // Recreate the interrupter's descriptors. Used after a fork.
  XIO_DECL void recreate();

            // Interrupt the select call.
  XIO_DECL void interrupt();

            // Reset the select interrupter. Returns true if the reset was successful.
  XIO_DECL bool reset();

            // Get the read descriptor to be passed to select.
            int read_descriptor() const {
                return read_descriptor_;
            }

        private:
            // Open the descriptors. Throws on error.
  XIO_DECL void open_descriptors();

            // Close the descriptors.
  XIO_DECL void close_descriptors();

            // The read end of a connection used to interrupt the select call. This file
            // descriptor is passed to select such that when it is time to stop, a single
            // byte will be written on the other end of the connection and this
            // descriptor will become readable.
            int read_descriptor_;

            // The write end of a connection used to interrupt the select call. A single
            // byte may be written to this to wake up the select which is waiting for the
            // other end to become readable.
            int write_descriptor_;
        };
    } // namespace detail

} // namespace xio

#include <xio/detail/pop_options.h>

#endif // !defined(XIO_HAS_EVENTFD)
#endif // !defined(__SYMBIAN32__)
#endif // !defined(XIO_CYGWIN_W32_SOCKETS)
#endif // !defined(XIO_WINDOWS_RUNTIME)
#endif // !defined(XIO_WINDOWS)

#endif // XIO_DETAIL_PIPE_SELECT_INTERRUPTER_HPP
