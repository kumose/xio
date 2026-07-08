//
// serial_port_base.hpp
// ~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
// Copyright (c) 2008 Rep Invariant Systems, Inc. (info@repinvariant.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef XIO_SERIAL_PORT_BASE_HPP
#define XIO_SERIAL_PORT_BASE_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>

#if defined(XIO_HAS_SERIAL_PORT)

#if !defined(XIO_WINDOWS) \
  && !defined(XIO_CYGWIN_W32_SOCKETS)
# include <termios.h>
#endif // !defined(XIO_WINDOWS)
//   && !defined(XIO_CYGWIN_W32_SOCKETS)

#include <xio/detail/socket_types.h>
#include <xio/error_code.h>

# define XIO_OPTION_STORAGE termios

#include <xio/detail/push_options.h>

namespace xio {


    /// The serial_port_base class is used as a base for the basic_serial_port class
/// template so that we have a common place to define the serial port options.
    class serial_port_base {
    public:
        /// Serial port option to permit changing the baud rate.
        /**
   * Implements changing the baud rate for a given serial port.
   */
        class baud_rate {
        public:
            explicit baud_rate(unsigned int rate = 0);

            unsigned int value() const;

    XIO_DECL XIO_SYNC_OP_VOID store(
                XIO_OPTION_STORAGE &storage,
                xio::error_code &ec) const;

    XIO_DECL XIO_SYNC_OP_VOID load(
                const XIO_OPTION_STORAGE &storage,
                xio::error_code &ec);

        private:
            unsigned int value_;
        };

        /// Serial port option to permit changing the flow control.
        /**
   * Implements changing the flow control for a given serial port.
   */
        class flow_control {
        public:
            enum type { none, software, hardware };

            XIO_DECL explicit flow_control(type t = none);

            type value() const;

    XIO_DECL XIO_SYNC_OP_VOID store(
                XIO_OPTION_STORAGE &storage,
                xio::error_code &ec) const;

    XIO_DECL XIO_SYNC_OP_VOID load(
                const XIO_OPTION_STORAGE &storage,
                xio::error_code &ec);

        private:
            type value_;
        };

        /// Serial port option to permit changing the parity.
        /**
   * Implements changing the parity for a given serial port.
   */
        class parity {
        public:
            enum type { none, odd, even };

            XIO_DECL explicit parity(type t = none);

            type value() const;

    XIO_DECL XIO_SYNC_OP_VOID store(
                XIO_OPTION_STORAGE &storage,
                xio::error_code &ec) const;

    XIO_DECL XIO_SYNC_OP_VOID load(
                const XIO_OPTION_STORAGE &storage,
                xio::error_code &ec);

        private:
            type value_;
        };

        /// Serial port option to permit changing the number of stop bits.
        /**
   * Implements changing the number of stop bits for a given serial port.
   */
        class stop_bits {
        public:
            enum type { one, onepointfive, two };

            XIO_DECL explicit stop_bits(type t = one);

            type value() const;

    XIO_DECL XIO_SYNC_OP_VOID store(
                XIO_OPTION_STORAGE &storage,
                xio::error_code &ec) const;

    XIO_DECL XIO_SYNC_OP_VOID load(
                const XIO_OPTION_STORAGE &storage,
                xio::error_code &ec);

        private:
            type value_;
        };

        /// Serial port option to permit changing the character size.
        /**
   * Implements changing the character size for a given serial port.
   */
        class character_size {
        public:
            XIO_DECL explicit character_size(unsigned int t = 8);

            unsigned int value() const;

    XIO_DECL XIO_SYNC_OP_VOID store(
                XIO_OPTION_STORAGE &storage,
                xio::error_code &ec) const;

    XIO_DECL XIO_SYNC_OP_VOID load(
                const XIO_OPTION_STORAGE &storage,
                xio::error_code &ec);

        private:
            unsigned int value_;
        };

    protected:
        /// Protected destructor to prevent deletion through this type.
        ~serial_port_base() {
        }
    };


} // namespace xio

#include <xio/detail/pop_options.h>

#undef XIO_OPTION_STORAGE

#include <xio/impl/serial_port_base.h>


#endif // defined(XIO_HAS_SERIAL_PORT)

#endif // XIO_SERIAL_PORT_BASE_HPP
