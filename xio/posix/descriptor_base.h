//
// posix/descriptor_base.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef XIO_POSIX_DESCRIPTOR_BASE_HPP
#define XIO_POSIX_DESCRIPTOR_BASE_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>

#if defined(XIO_HAS_POSIX_STREAM_DESCRIPTOR)

#include <xio/detail/io_control.h>
#include <xio/detail/socket_option.h>

#include <xio/detail/push_options.h>

namespace xio {


    namespace posix {
        /// The descriptor_base class is used as a base for the descriptor class as a
/// place to define the associated IO control commands.
        class descriptor_base {
        public:
            /// Wait types.
            /**
   * For use with descriptor::wait() and descriptor::async_wait().
   */
            enum wait_type {
                /// Wait for a descriptor to become ready to read.
                wait_read,

                /// Wait for a descriptor to become ready to write.
                wait_write,

                /// Wait for a descriptor to have error conditions pending.
                wait_error
            };

/// IO control command to get the amount of data that can be read without
  /// blocking.
/**
   * Implements the FIONREAD IO control command.
   *
   * @par Example
   * @code
   * xio::posix::stream_descriptor descriptor(my_context);
   * ...
   * xio::descriptor_base::bytes_readable command(true);
   * descriptor.io_control(command);
   * std::size_t bytes_readable = command.get();
   * @endcode
   *
   * @par Concepts:
   * IoControlCommand.
   */

typedef xio::detail::io_control::bytes_readable bytes_readable;

protected:
/// Protected destructor to prevent deletion through this type.
~descriptor_base() {
}
};

} // namespace posix
} // namespace xio

#include <xio/detail/pop_options.h>

#endif // defined(XIO_HAS_POSIX_STREAM_DESCRIPTOR)

#endif // XIO_POSIX_DESCRIPTOR_BASE_HPP
