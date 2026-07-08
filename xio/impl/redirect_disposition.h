
// impl/redirect_disposition.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef XIO_IMPL_REDIRECT_DISPOSITION_HPP
#define XIO_IMPL_REDIRECT_DISPOSITION_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>
#include <xio/associated_executor.h>
#include <xio/associator.h>
#include <xio/async_result.h>
#include <xio/detail/handler_cont_helpers.h>
#include <xio/detail/initiation_base.h>
#include <xio/detail/type_traits.h>

#include <xio/detail/push_options.h>

namespace xio {


    namespace detail {
        // Class to adapt a redirect_disposition_t as a completion handler.
        template<typename Disposition, typename Handler>
        class redirect_disposition_handler {
        public:
            typedef void result_type;

            template<typename CompletionToken>
            redirect_disposition_handler(
                redirect_disposition_t<CompletionToken, Disposition> e)
                : d_(e.d_),
                  handler_(static_cast<CompletionToken &&>(e.token_)) {
            }

            template<typename RedirectedHandler>
            redirect_disposition_handler(Disposition &d, RedirectedHandler &&h)
                : d_(d),
                  handler_(static_cast<RedirectedHandler &&>(h)) {
            }

            void operator()() {
                static_cast<Handler &&>(handler_)();
            }

            template<typename Arg, typename... Args>
            std::enable_if_t<
                !std::is_same<std::decay_t<Arg>, Disposition>::value
            >
            operator()(Arg &&arg, Args &&... args) {
                static_cast<Handler &&>(handler_)(
                    static_cast<Arg &&>(arg),
                    static_cast<Args &&>(args)...);
            }

            template<typename... Args>
            void operator()(const Disposition &d, Args &&... args) {
                d_ = d;
                static_cast<Handler &&>(handler_)(static_cast<Args &&>(args)...);
            }

            //private:
            Disposition &d_;
            Handler handler_;
        };

        template<typename Handler>
        class redirect_disposition_handler<std::exception_ptr, Handler> {
        public:
            typedef void result_type;

            template<typename CompletionToken>
            redirect_disposition_handler(
                redirect_disposition_t<CompletionToken, std::exception_ptr> e)
                : d_(e.d_),
                  handler_(static_cast<CompletionToken &&>(e.token_)) {
            }

            template<typename RedirectedHandler>
            redirect_disposition_handler(std::exception_ptr &d, RedirectedHandler &&h)
                : d_(d),
                  handler_(static_cast<RedirectedHandler &&>(h)) {
            }

            void operator()() {
                static_cast<Handler &&>(handler_)();
            }

            template<typename Arg, typename... Args>
            std::enable_if_t<
                !is_disposition<std::decay_t<Arg> >::value
            >
            operator()(Arg &&arg, Args &&... args) {
                static_cast<Handler &&>(handler_)(
                    static_cast<Arg &&>(arg),
                    static_cast<Args &&>(args)...);
            }

            template<typename Disposition, typename... Args>
            std::enable_if_t<
                is_disposition<Disposition>::value
            >
            operator()(const Disposition &d, Args &&... args) {
                d_ = disposition_traits<Disposition>::to_exception_ptr(d);
                static_cast<Handler &&>(handler_)(static_cast<Args &&>(args)...);
            }

            //private:
            std::exception_ptr &d_;
            Handler handler_;
        };

        template<typename Disposition, typename Handler>
        inline bool xio_handler_is_continuation(
            redirect_disposition_handler<Disposition, Handler> *this_handler) {
            return XIO_VERSIONED_NAME(handler_cont_helpers)
            ::is_continuation(
                this_handler->handler_);
        }

        template<typename Disposition, typename Signature, typename = void>
        struct redirect_disposition_signature {
            typedef Signature type;
        };

        template<typename Disposition, typename R, typename... Args>
        struct redirect_disposition_signature<
                    Disposition, R(Disposition, Args...)> {
            typedef R type(Args...);
        };

        template<typename Disposition, typename R, typename... Args>
        struct redirect_disposition_signature<
                    Disposition, R(const Disposition &, Args...)> {
            typedef R type(Args...);
        };

        template<typename Disposition, typename R, typename... Args>
        struct redirect_disposition_signature<
                    Disposition, R(Disposition, Args...) &> {
            typedef R type(Args...) &;
        };

        template<typename Disposition, typename R, typename... Args>
        struct redirect_disposition_signature<
                    Disposition, R(const Disposition &, Args...) &> {
            typedef R type(Args...) &;
        };

        template<typename Disposition, typename R, typename... Args>
        struct redirect_disposition_signature<
                    Disposition, R(Disposition, Args...) &&> {
            typedef R type(Args...) &&;
        };

        template<typename Disposition, typename R, typename... Args>
        struct redirect_disposition_signature<
                    Disposition, R(const Disposition &, Args...) &&> {
            typedef R type(Args...) &&;
        };

#if defined(XIO_HAS_NOEXCEPT_FUNCTION_TYPE)

        template<typename Disposition, typename R, typename... Args>
        struct redirect_disposition_signature<
                    Disposition, R(Disposition, Args...) noexcept> {
            typedef R type(Args...) & noexcept;
        };

        template<typename Disposition, typename R, typename... Args>
        struct redirect_disposition_signature<
                    Disposition, R(const Disposition &, Args...) noexcept> {
            typedef R type(Args...) & noexcept;
        };

        template<typename Disposition, typename R, typename... Args>
        struct redirect_disposition_signature<
                    Disposition, R(Disposition, Args...) & noexcept> {
            typedef R type(Args...) & noexcept;
        };

        template<typename Disposition, typename R, typename... Args>
        struct redirect_disposition_signature<
                    Disposition, R(const Disposition &, Args...) & noexcept> {
            typedef R type(Args...) & noexcept;
        };

        template<typename Disposition, typename R, typename... Args>
        struct redirect_disposition_signature<
                    Disposition, R(Disposition, Args...) && noexcept> {
            typedef R type(Args...) && noexcept;
        };

        template<typename Disposition, typename R, typename... Args>
        struct redirect_disposition_signature<
                    Disposition, R(const Disposition &, Args...) && noexcept> {
            typedef R type(Args...) && noexcept;
        };

#endif // defined(XIO_HAS_NOEXCEPT_FUNCTION_TYPE)

        template<typename Disposition, typename R, typename... Args>
        struct redirect_disposition_signature<
                    std::exception_ptr, R(Disposition, Args...),
                    std::enable_if_t<is_disposition<Disposition>::value> > {
            typedef R type(Args...);
        };

        template<typename Disposition, typename R, typename... Args>
        struct redirect_disposition_signature<
                    std::exception_ptr, R(const Disposition &, Args...),
                    std::enable_if_t<is_disposition<Disposition>::value> > {
            typedef R type(Args...);
        };

        template<typename Disposition, typename R, typename... Args>
        struct redirect_disposition_signature<
                    std::exception_ptr, R(Disposition, Args...) &,
                    std::enable_if_t<is_disposition<Disposition>::value> > {
            typedef R type(Args...) &;
        };

        template<typename Disposition, typename R, typename... Args>
        struct redirect_disposition_signature<
                    std::exception_ptr, R(const Disposition &, Args...) &,
                    std::enable_if_t<is_disposition<Disposition>::value> > {
            typedef R type(Args...) &;
        };

        template<typename Disposition, typename R, typename... Args>
        struct redirect_disposition_signature<
                    std::exception_ptr, R(Disposition, Args...) &&,
                    std::enable_if_t<is_disposition<Disposition>::value> > {
            typedef R type(Args...) &&;
        };

        template<typename Disposition, typename R, typename... Args>
        struct redirect_disposition_signature<
                    std::exception_ptr, R(const Disposition &, Args...) &&,
                    std::enable_if_t<is_disposition<Disposition>::value> > {
            typedef R type(Args...) &&;
        };

#if defined(XIO_HAS_NOEXCEPT_FUNCTION_TYPE)

        template<typename Disposition, typename R, typename... Args>
        struct redirect_disposition_signature<
                    std::exception_ptr, R(Disposition, Args...) noexcept,
                    std::enable_if_t<is_disposition<Disposition>::value> > {
            typedef R type(Args...) & noexcept;
        };

        template<typename Disposition, typename R, typename... Args>
        struct redirect_disposition_signature<
                    std::exception_ptr, R(const Disposition &, Args...) noexcept,
                    std::enable_if_t<is_disposition<Disposition>::value> > {
            typedef R type(Args...) & noexcept;
        };

        template<typename Disposition, typename R, typename... Args>
        struct redirect_disposition_signature<
                    std::exception_ptr, R(Disposition, Args...) & noexcept,
                    std::enable_if_t<is_disposition<Disposition>::value> > {
            typedef R type(Args...) & noexcept;
        };

        template<typename Disposition, typename R, typename... Args>
        struct redirect_disposition_signature<
                    std::exception_ptr, R(const Disposition &, Args...) & noexcept,
                    std::enable_if_t<is_disposition<Disposition>::value> > {
            typedef R type(Args...) & noexcept;
        };

        template<typename Disposition, typename R, typename... Args>
        struct redirect_disposition_signature<
                    std::exception_ptr, R(Disposition, Args...) && noexcept,
                    std::enable_if_t<is_disposition<Disposition>::value> > {
            typedef R type(Args...) && noexcept;
        };

        template<typename Disposition, typename R, typename... Args>
        struct redirect_disposition_signature<
                    std::exception_ptr, R(const Disposition &, Args...) && noexcept,
                    std::enable_if_t<is_disposition<Disposition>::value> > {
            typedef R type(Args...) && noexcept;
        };

#endif // defined(XIO_HAS_NOEXCEPT_FUNCTION_TYPE)
    } // namespace detail


    template<typename CompletionToken, typename Disposition, typename Signature>
    struct async_result<
                redirect_disposition_t<CompletionToken, Disposition>, Signature>
            : async_result<CompletionToken,
                typename detail::redirect_disposition_signature<
                    Disposition, Signature>::type> {
        template<typename Initiation>
        struct init_wrapper : detail::initiation_base<Initiation> {
            using detail::initiation_base<Initiation>::initiation_base;

            template<typename Handler, typename... Args>
            void operator()(Handler &&handler,
                            Disposition *d, Args &&... args) && {
                static_cast<Initiation &&>(*this)(
                    detail::redirect_disposition_handler<Disposition, std::decay_t<Handler> >(
                        *d, static_cast<Handler &&>(handler)),
                    static_cast<Args &&>(args)...);
            }

            template<typename Handler, typename... Args>
            void operator()(Handler &&handler,
                            Disposition *d, Args &&... args) const & {
                static_cast<const Initiation &>(*this)(
                    detail::redirect_disposition_handler<Disposition, std::decay_t<Handler> >(
                        *d, static_cast<Handler &&>(handler)),
                    static_cast<Args &&>(args)...);
            }
        };

        template<typename Initiation, typename RawCompletionToken, typename... Args>
        static auto initiate(Initiation &&initiation,
                             RawCompletionToken &&token, Args &&... args)
            -> decltype(
                async_initiate<
                    std::conditional_t<
                        std::is_const<std::remove_reference_t<RawCompletionToken> >::value,
                        const CompletionToken, CompletionToken>,
                    typename detail::redirect_disposition_signature<
                        Disposition, Signature>::type>(
                    std::declval<init_wrapper<std::decay_t<Initiation> > >(),
                    token.token_, &token.d_, static_cast<Args &&>(args)...)) {
            return async_initiate<
                std::conditional_t<
                    std::is_const<std::remove_reference_t<RawCompletionToken> >::value,
                    const CompletionToken, CompletionToken>,
                typename detail::redirect_disposition_signature<
                    Disposition, Signature>::type>(
                init_wrapper<std::decay_t<Initiation> >(
                    static_cast<Initiation &&>(initiation)),
                token.token_, &token.d_, static_cast<Args &&>(args)...);
        }
    };

    template<template <typename, typename> class Associator,
        typename Disposition, typename Handler, typename DefaultCandidate>
    struct associator<Associator,
                detail::redirect_disposition_handler<
                    Disposition, Handler>, DefaultCandidate>
            : Associator<Handler, DefaultCandidate> {
        static typename Associator<Handler, DefaultCandidate>::type get(
            const detail::redirect_disposition_handler<Disposition, Handler> &h)
            noexcept {
            return Associator<Handler, DefaultCandidate>::get(h.handler_);
        }

        static auto get(
            const detail::redirect_disposition_handler<Disposition, Handler> &h,
            const DefaultCandidate &c) noexcept
            -> decltype(Associator<Handler, DefaultCandidate>::get(h.handler_, c)) {
            return Associator<Handler, DefaultCandidate>::get(h.handler_, c);
        }
    };

    template<typename Disposition, typename... Signatures>
    struct async_result<partial_redirect_disposition<Disposition>, Signatures...> {
        template<typename Initiation, typename RawCompletionToken, typename... Args>
        static auto initiate(Initiation &&initiation,
                             RawCompletionToken &&token, Args &&... args)
            -> decltype(
                async_initiate < Signatures

        ...
        >
        (


        static_cast
        <
        Initiation &&

        >
        (initiation),
                redirect_disposition_t<
                    default_completion_token_t<associated_executor_t<Initiation> >,
                    Disposition>(
                    default_completion_token_t<associated_executor_t<Initiation> >  {
        }

        ,
        token
        .
        d_
        )
        ,
        static_cast
        <
        Args &&

        >
        (args)
        ...
        )
        )
  {
    return async_initiate<Signatures...>(
        static_cast<Initiation&&>(initiation),
        redirect_disposition_t<
          default_completion_token_t<associated_executor_t<Initiation>>,
          Disposition>(
            default_completion_token_t<associated_executor_t<Initiation>>{},
            token.d_),
        static_cast<Args&&>(args)...);
  }
    };



} // namespace xio

#include <xio/detail/pop_options.h>

#endif // XIO_IMPL_REDIRECT_DISPOSITION_HPP
