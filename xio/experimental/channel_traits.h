//
// experimental/channel_traits.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef XIO_EXPERIMENTAL_CHANNEL_TRAITS_HPP
#define XIO_EXPERIMENTAL_CHANNEL_TRAITS_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>
#include <deque>
#include <exception>
#include <xio/detail/type_traits.h>
#include <xio/error.h>
#include <xio/experimental/channel_error.h>

#include <xio/detail/push_options.h>

namespace xio {


    namespace experimental {


        /// Traits used for customising channel behaviour.
        template<typename... Signatures>
        struct channel_traits {
            template<typename... NewSignatures>
            struct rebind {
                typedef channel_traits<NewSignatures...> other;
            };
        };

        template<typename R>
        struct channel_traits<R(xio::error_code)> {
            template<typename... NewSignatures>
            struct rebind {
                typedef channel_traits<NewSignatures...> other;
            };

            template<typename Element>
            struct container {
                typedef std::deque<Element> type;
            };

            typedef R receive_cancelled_signature(xio::error_code);

            template<typename F>
            static void invoke_receive_cancelled(F f) {
                const xio::error_code e = error::channel_cancelled;
                static_cast<F &&>(f)(e);
            }

            typedef R receive_closed_signature(xio::error_code);

            template<typename F>
            static void invoke_receive_closed(F f) {
                const xio::error_code e = error::channel_closed;
                static_cast<F &&>(f)(e);
            }
        };

        template<typename R, typename... Args, typename... Signatures>
        struct channel_traits<R(xio::error_code, Args...), Signatures...> {
            template<typename... NewSignatures>
            struct rebind {
                typedef channel_traits<NewSignatures...> other;
            };

            template<typename Element>
            struct container {
                typedef std::deque<Element> type;
            };

            typedef R receive_cancelled_signature(xio::error_code, Args...);

            template<typename F>
            static void invoke_receive_cancelled(F f) {
                const xio::error_code e = error::channel_cancelled;
                static_cast<F &&>(f)(e, std::decay_t<Args>()...);
            }

            typedef R receive_closed_signature(xio::error_code, Args...);

            template<typename F>
            static void invoke_receive_closed(F f) {
                const xio::error_code e = error::channel_closed;
                static_cast<F &&>(f)(e, std::decay_t<Args>()...);
            }
        };

        template<typename R>
        struct channel_traits<R(std::exception_ptr)> {
            template<typename... NewSignatures>
            struct rebind {
                typedef channel_traits<NewSignatures...> other;
            };

            template<typename Element>
            struct container {
                typedef std::deque<Element> type;
            };

            typedef R receive_cancelled_signature(std::exception_ptr);

            template<typename F>
            static void invoke_receive_cancelled(F f) {
                const xio::error_code e = error::channel_cancelled;
                static_cast<F &&>(f)(
                    std::make_exception_ptr(std::system_error(e)));
            }

            typedef R receive_closed_signature(std::exception_ptr);

            template<typename F>
            static void invoke_receive_closed(F f) {
                const xio::error_code e = error::channel_closed;
                static_cast<F &&>(f)(
                    std::make_exception_ptr(std::system_error(e)));
            }
        };

        template<typename R, typename... Args, typename... Signatures>
        struct channel_traits<R(std::exception_ptr, Args...), Signatures...> {
            template<typename... NewSignatures>
            struct rebind {
                typedef channel_traits<NewSignatures...> other;
            };

            template<typename Element>
            struct container {
                typedef std::deque<Element> type;
            };

            typedef R receive_cancelled_signature(std::exception_ptr, Args...);

            template<typename F>
            static void invoke_receive_cancelled(F f) {
                const xio::error_code e = error::channel_cancelled;
                static_cast<F &&>(f)(
                    std::make_exception_ptr(std::system_error(e)),
                    std::decay_t<Args>()...);
            }

            typedef R receive_closed_signature(std::exception_ptr, Args...);

            template<typename F>
            static void invoke_receive_closed(F f) {
                const xio::error_code e = error::channel_closed;
                static_cast<F &&>(f)(
                    std::make_exception_ptr(std::system_error(e)),
                    std::decay_t<Args>()...);
            }
        };

        template<typename R>
        struct channel_traits<R()> {
            template<typename... NewSignatures>
            struct rebind {
                typedef channel_traits<NewSignatures...> other;
            };

            template<typename Element>
            struct container {
                typedef std::deque<Element> type;
            };

            typedef R receive_cancelled_signature(xio::error_code);

            template<typename F>
            static void invoke_receive_cancelled(F f) {
                const xio::error_code e = error::channel_cancelled;
                static_cast<F &&>(f)(e);
            }

            typedef R receive_closed_signature(xio::error_code);

            template<typename F>
            static void invoke_receive_closed(F f) {
                const xio::error_code e = error::channel_closed;
                static_cast<F &&>(f)(e);
            }
        };

        template<typename R, typename T>
        struct channel_traits<R(T)> {
            template<typename... NewSignatures>
            struct rebind {
                typedef channel_traits<NewSignatures...> other;
            };

            template<typename Element>
            struct container {
                typedef std::deque<Element> type;
            };

            typedef R receive_cancelled_signature(xio::error_code);

            template<typename F>
            static void invoke_receive_cancelled(F f) {
                const xio::error_code e = error::channel_cancelled;
                static_cast<F &&>(f)(e);
            }

            typedef R receive_closed_signature(xio::error_code);

            template<typename F>
            static void invoke_receive_closed(F f) {
                const xio::error_code e = error::channel_closed;
                static_cast<F &&>(f)(e);
            }
        };

    } // namespace experimental

} // namespace xio

#include <xio/detail/pop_options.h>

#endif // XIO_EXPERIMENTAL_CHANNEL_TRAITS_HPP
