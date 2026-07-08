//
// detail/local_free_on_block_exit.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef XIO_DETAIL_LOCAL_FREE_ON_BLOCK_EXIT_HPP
#define XIO_DETAIL_LOCAL_FREE_ON_BLOCK_EXIT_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>

#if defined(XIO_WINDOWS) || defined(XIO_CYGWIN_W32_SOCKETS)
#if !defined(XIO_WINDOWS_APP)

#include <xio/detail/noncopyable.h>
#include <xio/detail/socket_types.h>

#include <xio/detail/push_options.h>

namespace xio {


    namespace detail {
        class local_free_on_block_exit
                : private noncopyable {
        public:
            // Constructor blocks all signals for the calling thread.
            explicit local_free_on_block_exit(void *p)
                : p_(p) {
            }

            // Destructor restores the previous signal mask.
            ~local_free_on_block_exit() {
                ::LocalFree(p_);
            }

        private:
            void *p_;
        };
    } // namespace detail

} // namespace xio

#include <xio/detail/pop_options.h>

#endif // !defined(XIO_WINDOWS_APP)
#endif // defined(XIO_WINDOWS) || defined(XIO_CYGWIN_W32_SOCKETS)

#endif // XIO_DETAIL_LOCAL_FREE_ON_BLOCK_EXIT_HPP
