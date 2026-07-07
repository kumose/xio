//
// detail/impl/winsock_init.ipp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_DETAIL_IMPL_WINSOCK_INIT_IPP
#define ASIO_DETAIL_IMPL_WINSOCK_INIT_IPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>

#if defined(ASIO_WINDOWS) || defined(ASIO_CYGWIN_W32_SOCKETS)

#include <xio/detail/socket_types.h>
#include <xio/detail/winsock_init.h>
#include <xio/detail/throw_error.h>
#include <xio/error.h>

#include <xio/detail/push_options.h>

namespace xio {


    namespace detail {
        void winsock_init_base::startup(data &d,
                                        unsigned char major, unsigned char minor) {
            if (::InterlockedIncrement(&d.init_count_) == 1) {
                WSADATA wsa_data;
                long result = ::WSAStartup(MAKEWORD(major, minor), &wsa_data);
                ::InterlockedExchange(&d.result_, result);
            }
        }

        void winsock_init_base::manual_startup(data &d) {
            if (::InterlockedIncrement(&d.init_count_) == 1) {
                ::InterlockedExchange(&d.result_, 0);
            }
        }

        void winsock_init_base::cleanup(data &d) {
            if (::InterlockedDecrement(&d.init_count_) == 0) {
                ::WSACleanup();
            }
        }

        void winsock_init_base::manual_cleanup(data &d) {
            ::InterlockedDecrement(&d.init_count_);
        }

        void winsock_init_base::throw_on_error(data &d) {
            long result = ::InterlockedExchangeAdd(&d.result_, 0);
            if (result != 0) {
                xio::error_code ec(result,
                                   xio::error::get_system_category());
                xio::detail::throw_error(ec, "winsock");
            }
        }
    } // namespace detail

} // namespace xio

#include <xio/detail/pop_options.h>

#endif // defined(ASIO_WINDOWS) || defined(ASIO_CYGWIN_W32_SOCKETS)

#endif // ASIO_DETAIL_IMPL_WINSOCK_INIT_IPP
