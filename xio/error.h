//
// error.hpp
// ~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef XIO_ERROR_HPP
#define XIO_ERROR_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>
#include <xio/error_code.h>
#include <system_error>
#if defined(XIO_WINDOWS) \
  || defined(XIO_CYGWIN_W32_SOCKETS) \
  || defined(XIO_WINDOWS_RUNTIME)
# include <winerror.h>
#else
# include <cerrno>
# include <netdb.h>
#endif

#if defined(XIO_CYGWIN_W32_SOCKETS)
#include <xio/detail/socket_types.h>
#endif // defined(XIO_CYGWIN_W32_SOCKETS)

#if defined(XIO_WINDOWS_RUNTIME)
# define XIO_NATIVE_ERROR(e) __HRESULT_FROM_WIN32(e)
# define XIO_SOCKET_ERROR(e) __HRESULT_FROM_WIN32(WSA ## e)
# define XIO_NETDB_ERROR(e) __HRESULT_FROM_WIN32(WSA ## e)
# define XIO_GETADDRINFO_ERROR(e) __HRESULT_FROM_WIN32(WSA ## e)
# define XIO_WIN_OR_POSIX(e_win, e_posix) e_win
#elif defined(XIO_WINDOWS) || defined(XIO_CYGWIN_W32_SOCKETS)
# define XIO_NATIVE_ERROR(e) e
# define XIO_SOCKET_ERROR(e) WSA ## e
# define XIO_NETDB_ERROR(e) WSA ## e
# define XIO_GETADDRINFO_ERROR(e) WSA ## e
# define XIO_WIN_OR_POSIX(e_win, e_posix) e_win
#else
# define XIO_NATIVE_ERROR(e) e
# define XIO_SOCKET_ERROR(e) e
# define XIO_NETDB_ERROR(e) e
# define XIO_GETADDRINFO_ERROR(e) e
# define XIO_WIN_OR_POSIX(e_win, e_posix) e_posix
#endif

#include <xio/detail/push_options.h>

namespace xio {
    namespace error {
        enum basic_errors {
            /// Permission denied.
            access_denied = XIO_SOCKET_ERROR(EACCES),

            /// Address family not supported by protocol.
            address_family_not_supported = XIO_SOCKET_ERROR(EAFNOSUPPORT),

            /// Address already in use.
            address_in_use = XIO_SOCKET_ERROR(EADDRINUSE),

            /// Transport endpoint is already connected.
            already_connected = XIO_SOCKET_ERROR(EISCONN),

            /// Operation already in progress.
            already_started = XIO_SOCKET_ERROR(EALREADY),

            /// Broken pipe.
            broken_pipe = XIO_WIN_OR_POSIX(
                XIO_NATIVE_ERROR(ERROR_BROKEN_PIPE),
                XIO_NATIVE_ERROR(EPIPE)),

            /// A connection has been aborted.
            connection_aborted = XIO_SOCKET_ERROR(ECONNABORTED),

            /// Connection refused.
            connection_refused = XIO_SOCKET_ERROR(ECONNREFUSED),

            /// Connection reset by peer.
            connection_reset = XIO_SOCKET_ERROR(ECONNRESET),

            /// Bad file descriptor.
            bad_descriptor = XIO_SOCKET_ERROR(EBADF),

            /// Bad address.
            fault = XIO_SOCKET_ERROR(EFAULT),

            /// No route to host.
            host_unreachable = XIO_SOCKET_ERROR(EHOSTUNREACH),

            /// Operation now in progress.
            in_progress = XIO_SOCKET_ERROR(EINPROGRESS),

            /// Interrupted system call.
            interrupted = XIO_SOCKET_ERROR(EINTR),

            /// Invalid argument.
            invalid_argument = XIO_SOCKET_ERROR(EINVAL),

            /// Message too long.
            message_size = XIO_SOCKET_ERROR(EMSGSIZE),

            /// The name was too long.
            name_too_long = XIO_SOCKET_ERROR(ENAMETOOLONG),

            /// Network is down.
            network_down = XIO_SOCKET_ERROR(ENETDOWN),

            /// Network dropped connection on reset.
            network_reset = XIO_SOCKET_ERROR(ENETRESET),

            /// Network is unreachable.
            network_unreachable = XIO_SOCKET_ERROR(ENETUNREACH),

            /// Too many open files.
            no_descriptors = XIO_SOCKET_ERROR(EMFILE),

            /// No buffer space available.
            no_buffer_space = XIO_SOCKET_ERROR(ENOBUFS),

            /// Cannot allocate memory.
            no_memory = XIO_WIN_OR_POSIX(
                XIO_NATIVE_ERROR(ERROR_OUTOFMEMORY),
                XIO_NATIVE_ERROR(ENOMEM)),

            /// Operation not permitted.
            no_permission = XIO_WIN_OR_POSIX(
                XIO_NATIVE_ERROR(ERROR_ACCESS_DENIED),
                XIO_NATIVE_ERROR(EPERM)),

            /// Protocol not available.
            no_protocol_option = XIO_SOCKET_ERROR(ENOPROTOOPT),

            /// No such device.
            no_such_device = XIO_WIN_OR_POSIX(
                XIO_NATIVE_ERROR(ERROR_BAD_UNIT),
                XIO_NATIVE_ERROR(ENODEV)),

            /// Transport endpoint is not connected.
            not_connected = XIO_SOCKET_ERROR(ENOTCONN),

            /// Socket operation on non-socket.
            not_socket = XIO_SOCKET_ERROR(ENOTSOCK),

            /// Operation cancelled.
            operation_aborted = XIO_WIN_OR_POSIX(
                XIO_NATIVE_ERROR(ERROR_OPERATION_ABORTED),
                XIO_NATIVE_ERROR(ECANCELED)),

            /// Operation not supported.
            operation_not_supported = XIO_SOCKET_ERROR(EOPNOTSUPP),

            /// Cannot send after transport endpoint shutdown.
            shut_down = XIO_SOCKET_ERROR(ESHUTDOWN),

            /// Connection timed out.
            timed_out = XIO_SOCKET_ERROR(ETIMEDOUT),

            /// Resource temporarily unavailable.
            try_again = XIO_WIN_OR_POSIX(
                XIO_NATIVE_ERROR(ERROR_RETRY),
                XIO_NATIVE_ERROR(EAGAIN)),

            /// The socket is marked non-blocking and the requested operation would block.
            would_block = XIO_SOCKET_ERROR(EWOULDBLOCK)
        };

        enum netdb_errors {
            /// Host not found (authoritative).
            host_not_found = XIO_NETDB_ERROR(HOST_NOT_FOUND),

            /// Host not found (non-authoritative).
            host_not_found_try_again = XIO_NETDB_ERROR(TRY_AGAIN),

            /// The query is valid but does not have associated address data.
            no_data = XIO_NETDB_ERROR(NO_DATA),

            /// A non-recoverable error occurred.
            no_recovery = XIO_NETDB_ERROR(NO_RECOVERY)
        };

        enum addrinfo_errors {
            /// The service is not supported for the given socket type.
            service_not_found = XIO_WIN_OR_POSIX(
                XIO_NATIVE_ERROR(WSATYPE_NOT_FOUND),
                XIO_GETADDRINFO_ERROR(EAI_SERVICE)),

            /// The socket type is not supported.
            socket_type_not_supported = XIO_WIN_OR_POSIX(
                XIO_NATIVE_ERROR(WSAESOCKTNOSUPPORT),
                XIO_GETADDRINFO_ERROR(EAI_SOCKTYPE))
        };

        enum misc_errors {
            /// Already open.
            already_open = 1,

            /// End of file or stream.
            eof,

            /// Element not found.
            not_found,

            /// The descriptor cannot fit into the select system call's fd_set.
            fd_set_failure
        };

#if !defined(XIO_ERROR_LOCATION)
# define XIO_ERROR_LOCATION(e) (void)0
#endif // !defined(XIO_ERROR_LOCATION)


        inline void clear(xio::error_code &ec) {
            ec.assign(0, ec.category());
        }

        inline const xio::error_category &get_system_category() {
            return xio::system_category();
        }

#if !defined(XIO_WINDOWS) \
  && !defined(XIO_CYGWIN_W32_SOCKETS)

        extern

        XIO_DECL
        const xio::error_category &get_netdb_category();

        extern

        XIO_DECL
        const xio::error_category &get_addrinfo_category();

#else // !defined(XIO_WINDOWS)
        //   && !defined(XIO_CYGWIN_W32_SOCKETS)

        inline const xio::error_category &get_netdb_category() {
            return get_system_category();
        }

        inline const xio::error_category &get_addrinfo_category() {
            return get_system_category();
        }

#endif // !defined(XIO_WINDOWS)
        //   && !defined(XIO_CYGWIN_W32_SOCKETS)

        extern

        XIO_DECL
        const xio::error_category &get_misc_category();

        inline const xio::error_category &
                system_category XIO_UNUSED_VARIABLE
                = xio::error::get_system_category();
        inline const xio::error_category &
                netdb_category XIO_UNUSED_VARIABLE
                = xio::error::get_netdb_category();
        inline const xio::error_category &
                addrinfo_category XIO_UNUSED_VARIABLE
                = xio::error::get_addrinfo_category();
        inline const xio::error_category &
                misc_category XIO_UNUSED_VARIABLE
                = xio::error::get_misc_category();
    } // namespace error
} // namespace xio

namespace std {
    template<>
    struct is_error_code_enum<xio::error::basic_errors> {
        static const bool value = true;
    };

    template<>
    struct is_error_code_enum<xio::error::netdb_errors> {
        static const bool value = true;
    };

    template<>
    struct is_error_code_enum<xio::error::addrinfo_errors> {
        static const bool value = true;
    };

    template<>
    struct is_error_code_enum<xio::error::misc_errors> {
        static const bool value = true;
    };
} // namespace std

namespace xio {
    namespace error {
        inline xio::error_code make_error_code(basic_errors e) {
            return xio::error_code(
                static_cast<int>(e), get_system_category());
        }

        inline xio::error_code make_error_code(netdb_errors e) {
            return xio::error_code(
                static_cast<int>(e), get_netdb_category());
        }

        inline xio::error_code make_error_code(addrinfo_errors e) {
            return xio::error_code(
                static_cast<int>(e), get_addrinfo_category());
        }

        inline xio::error_code make_error_code(misc_errors e) {
            return xio::error_code(
                static_cast<int>(e), get_misc_category());
        }

        // boostify: non-boost code starts here
        namespace detail {
            XIO_DECL std::error_condition error_number_to_condition(int ev);
        } // namespace detail
        // boostify: non-boost code ends here
    } // namespace error
    namespace stream_errc {
        // Simulates the proposed stream_errc scoped enum.
        using error::eof;
        using error::not_found;
    } // namespace stream_errc
    namespace socket_errc {
        // Simulates the proposed socket_errc scoped enum.
        using error::already_open;
        using error::not_found;
    } // namespace socket_errc
    namespace resolver_errc {
        // Simulates the proposed resolver_errc scoped enum.
        using error::host_not_found;
        const error::netdb_errors try_again = error::host_not_found_try_again;
        using error::service_not_found;
    } // namespace resolver_errc
} // namespace xio

#include <xio/detail/pop_options.h>

#undef XIO_NATIVE_ERROR
#undef XIO_SOCKET_ERROR
#undef XIO_NETDB_ERROR
#undef XIO_GETADDRINFO_ERROR
#undef XIO_WIN_OR_POSIX

#endif // XIO_ERROR_HPP
