//
// async_result.hpp
// ~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef XIO_ASYNC_RESULT_HPP
#define XIO_ASYNC_RESULT_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>
#include <xio/detail/type_traits.h>

#include <xio/detail/push_options.h>

namespace xio {


    namespace detail {
        template<typename T>
        struct is_completion_signature : std::false_type {
        };

        template<typename R, typename... Args>
        struct is_completion_signature<R(Args...)> : std::true_type {
        };

        template<typename R, typename... Args>
        struct is_completion_signature<R(Args...) &> : std::true_type {
        };

        template<typename R, typename... Args>
        struct is_completion_signature<R(Args...) &&> : std::true_type {
        };

# if defined(XIO_HAS_NOEXCEPT_FUNCTION_TYPE)

        template<typename R, typename... Args>
        struct is_completion_signature<R(Args...) noexcept> : std::true_type {
        };

        template<typename R, typename... Args>
        struct is_completion_signature<R(Args...) & noexcept> : std::true_type {
        };

        template<typename R, typename... Args>
        struct is_completion_signature<R(Args...) && noexcept> : std::true_type {
        };

# endif // defined(XIO_HAS_NOEXCEPT_FUNCTION_TYPE)

        template<typename... T>
        struct are_completion_signatures : std::false_type {
        };

        template<>
        struct are_completion_signatures<>
                : std::true_type {
        };

        template<typename T0>
        struct are_completion_signatures<T0>
                : is_completion_signature<T0> {
        };

        template<typename T0, typename... TN>
        struct are_completion_signatures<T0, TN...>
                : std::integral_constant<bool, (
                    is_completion_signature<T0>::value
                    && are_completion_signatures<TN...>::value)> {
        };
    } // namespace detail

#if defined(XIO_HAS_CONCEPTS)

    namespace detail {
        template<typename T, typename... Args>
        XIO_CONCEPT callable_with = requires(T &&t, Args &&... args)
        {
            static_cast<T &&>(t)(static_cast<Args &&>(args)...);
        };

        template<typename T, typename... Signatures>
        struct is_completion_handler_for : std::false_type {
        };

        template<typename T, typename R, typename... Args>
        struct is_completion_handler_for<T, R(Args...)>
                : std::integral_constant<bool, (callable_with<std::decay_t<T>, Args...>)> {
        };

        template<typename T, typename R, typename... Args>
        struct is_completion_handler_for<T, R(Args...) &>
                : std::integral_constant<bool, (callable_with<std::decay_t<T> &, Args...>)> {
        };

        template<typename T, typename R, typename... Args>
        struct is_completion_handler_for<T, R(Args...) &&>
                : std::integral_constant<bool, (callable_with<std::decay_t<T> &&, Args...>)> {
        };

# if defined(XIO_HAS_NOEXCEPT_FUNCTION_TYPE)

    template<typename T, typename R, typename... Args>
    struct is_completion_handler_for<T, R(Args...) noexcept>
            : std::integral_constant<bool, (callable_with<std::decay_t<T>, Args...>)> {
    };

    template<typename T, typename R, typename... Args>
    struct is_completion_handler_for<T, R(Args...) & noexcept>
            : std::integral_constant<bool, (callable_with<std::decay_t<T> &, Args...>)> {
    };

    template<typename T, typename R, typename... Args>
    struct is_completion_handler_for<T, R(Args...) && noexcept>
            : std::integral_constant<bool, (callable_with<std::decay_t<T> &&, Args...>)> {
    };

# endif // defined(XIO_HAS_NOEXCEPT_FUNCTION_TYPE)

    template<typename T, typename Signature0, typename... SignatureN>
    struct is_completion_handler_for<T, Signature0, SignatureN...>
            : std::integral_constant<bool, (
                is_completion_handler_for<T, Signature0>::value
                && is_completion_handler_for<T, SignatureN...>::value)> {
    };

} // namespace detail

    template<typename T>
    XIO_CONCEPT completion_signature =
            detail::is_completion_signature<T>::value;

#define XIO_COMPLETION_SIGNATURE \
  ::xio::completion_signature

    template<typename T, typename... Signatures>
    XIO_CONCEPT completion_handler_for =
            detail::are_completion_signatures < Signatures...>::value
    &&detail::is_completion_handler_for<T, Signatures...>::value;

#define XIO_COMPLETION_HANDLER_FOR(sig) \
  ::xio::completion_handler_for<sig>
#define XIO_COMPLETION_HANDLER_FOR2(sig0, sig1) \
  ::xio::completion_handler_for<sig0, sig1>
#define XIO_COMPLETION_HANDLER_FOR3(sig0, sig1, sig2) \
  ::xio::completion_handler_for<sig0, sig1, sig2>

#else // defined(XIO_HAS_CONCEPTS)

#define XIO_COMPLETION_SIGNATURE typename
#define XIO_COMPLETION_HANDLER_FOR(sig) typename
#define XIO_COMPLETION_HANDLER_FOR2(sig0, sig1) typename
#define XIO_COMPLETION_HANDLER_FOR3(sig0, sig1, sig2) typename

#endif // defined(XIO_HAS_CONCEPTS)

    namespace detail {
        template<typename T>
        struct is_lvalue_completion_signature : std::false_type {
        };

        template<typename R, typename... Args>
        struct is_lvalue_completion_signature<R(Args...) &> : std::true_type {
        };

# if defined(XIO_HAS_NOEXCEPT_FUNCTION_TYPE)

        template<typename R, typename... Args>
        struct is_lvalue_completion_signature<R(Args...) & noexcept> : std::true_type {
        };

# endif // defined(XIO_HAS_NOEXCEPT_FUNCTION_TYPE)

        template<typename... Signatures>
        struct are_any_lvalue_completion_signatures : std::false_type {
        };

        template<typename Sig0>
        struct are_any_lvalue_completion_signatures<Sig0>
                : is_lvalue_completion_signature<Sig0> {
        };

        template<typename Sig0, typename... SigN>
        struct are_any_lvalue_completion_signatures<Sig0, SigN...>
                : std::integral_constant<bool, (
                    is_lvalue_completion_signature<Sig0>::value
                    || are_any_lvalue_completion_signatures<SigN...>::value)> {
        };

        template<typename T>
        struct is_rvalue_completion_signature : std::false_type {
        };

        template<typename R, typename... Args>
        struct is_rvalue_completion_signature<R(Args...) &&> : std::true_type {
        };

# if defined(XIO_HAS_NOEXCEPT_FUNCTION_TYPE)

        template<typename R, typename... Args>
        struct is_rvalue_completion_signature<R(Args...) && noexcept> : std::true_type {
        };

# endif // defined(XIO_HAS_NOEXCEPT_FUNCTION_TYPE)

        template<typename... Signatures>
        struct are_any_rvalue_completion_signatures : std::false_type {
        };

        template<typename Sig0>
        struct are_any_rvalue_completion_signatures<Sig0>
                : is_rvalue_completion_signature<Sig0> {
        };

        template<typename Sig0, typename... SigN>
        struct are_any_rvalue_completion_signatures<Sig0, SigN...>
                : std::integral_constant<bool, (
                    is_rvalue_completion_signature<Sig0>::value
                    || are_any_rvalue_completion_signatures<SigN...>::value)> {
        };

        template<typename T>
        struct simple_completion_signature;

        template<typename R, typename... Args>
        struct simple_completion_signature<R(Args...)> {
            typedef R type(Args...);
        };

        template<typename R, typename... Args>
        struct simple_completion_signature<R(Args...) &> {
            typedef R type(Args...);
        };

        template<typename R, typename... Args>
        struct simple_completion_signature<R(Args...) &&> {
            typedef R type(Args...);
        };

# if defined(XIO_HAS_NOEXCEPT_FUNCTION_TYPE)

        template<typename R, typename... Args>
        struct simple_completion_signature<R(Args...) noexcept> {
            typedef R type(Args...);
        };

        template<typename R, typename... Args>
        struct simple_completion_signature<R(Args...) & noexcept> {
            typedef R type(Args...);
        };

        template<typename R, typename... Args>
        struct simple_completion_signature<R(Args...) && noexcept> {
            typedef R type(Args...);
        };

# endif // defined(XIO_HAS_NOEXCEPT_FUNCTION_TYPE)

        template<typename CompletionToken,
            XIO_COMPLETION_SIGNATURE... Signatures>
        class completion_handler_async_result {
        public:
            typedef CompletionToken completion_handler_type;
            typedef void return_type;

            explicit completion_handler_async_result(completion_handler_type &) {
            }

            return_type get() {
            }

            template<typename Initiation,
                XIO_COMPLETION_HANDLER_FOR(Signatures...) RawCompletionToken,
                typename... Args>
            static return_type initiate(Initiation &&initiation,
                                        RawCompletionToken &&token, Args &&... args) {
                static_cast<Initiation &&>(initiation)(
                    static_cast<RawCompletionToken &&>(token),
                    static_cast<Args &&>(args)...);
            }

        private:
            completion_handler_async_result(
                const completion_handler_async_result &) = delete;

            completion_handler_async_result &operator=(
                const completion_handler_async_result &) = delete;
        };
    } // namespace detail

    template<typename CompletionToken,
        XIO_COMPLETION_SIGNATURE... Signatures>
    class async_result :
            public std::conditional_t<
                detail::are_any_lvalue_completion_signatures<Signatures...>::value
                || !detail::are_any_rvalue_completion_signatures<Signatures...>::value,
                detail::completion_handler_async_result<CompletionToken, Signatures...>,
                async_result<CompletionToken,
                    typename detail::simple_completion_signature<Signatures>::type...>
            > {
    public:
        typedef std::conditional_t<
            detail::are_any_lvalue_completion_signatures<Signatures...>::value
            || !detail::are_any_rvalue_completion_signatures<Signatures...>::value,
            detail::completion_handler_async_result<CompletionToken, Signatures...>,
            async_result<CompletionToken,
                typename detail::simple_completion_signature<Signatures>::type...>
        > base_type;

        using base_type::base_type;

    private:
        async_result(const async_result &) = delete;

        async_result &operator=(const async_result &) = delete;
    };

    template<XIO_COMPLETION_SIGNATURE... Signatures>
    class async_result<void, Signatures...> {
        // Empty.
    };


    /// (Legacy.) Helper template to deduce the handler type from a CompletionToken,
/// capture a local copy of the handler, and then create an async_result for the
/// handler.
    template<typename CompletionToken,
        XIO_COMPLETION_SIGNATURE... Signatures>
    struct async_completion {
        /// The real handler type to be used for the asynchronous operation.
        typedef typename xio::async_result<
            std::decay_t<CompletionToken>, Signatures...>::completion_handler_type
        completion_handler_type;

        /// Constructor.
        /**
   * The constructor creates the concrete completion handler and makes the link
   * between the handler and the asynchronous result.
   */
        explicit async_completion(CompletionToken &token)
            : completion_handler(static_cast<std::conditional_t<
                  std::is_same<CompletionToken, completion_handler_type>::value,
                  completion_handler_type &, CompletionToken &&>>(token)),
              result(completion_handler) {
        }

        /// A copy of, or reference to, a real handler object.
        std::conditional_t<
            std::is_same<CompletionToken, completion_handler_type>::value,
            completion_handler_type &, completion_handler_type> completion_handler;

        /// The result of the asynchronous operation's initiating function.
        async_result<std::decay_t<CompletionToken>, Signatures...> result;
    };

    namespace detail {
        struct async_result_memfns_base {
            void initiate();
        };

        template<typename T>
        struct async_result_memfns_derived
                : T, async_result_memfns_base {
        };

        template<typename T, T>
        struct async_result_memfns_check {
        };

        template<typename>
        char (&async_result_initiate_memfn_helper(...))[2];

        template<typename T>
        char async_result_initiate_memfn_helper(
            async_result_memfns_check<
                void (async_result_memfns_base::*)(),
                &async_result_memfns_derived<T>::initiate> *);

        template<typename CompletionToken,
            XIO_COMPLETION_SIGNATURE... Signatures>
        struct async_result_has_initiate_memfn
                : std::integral_constant<bool, sizeof(async_result_initiate_memfn_helper<
                                              async_result<std::decay_t<CompletionToken>, Signatures...>
                                          >(0)) != 1> {
        };
    } // namespace detail

# define XIO_INITFN_RESULT_TYPE(ct, sig) \
  typename ::xio::async_result< \
    typename ::std::decay<ct>::type, sig>::return_type
# define XIO_INITFN_RESULT_TYPE2(ct, sig0, sig1) \
  typename ::xio::async_result< \
    typename ::std::decay<ct>::type, sig0, sig1>::return_type
# define XIO_INITFN_RESULT_TYPE3(ct, sig0, sig1, sig2) \
  typename ::xio::async_result< \
    typename ::std::decay<ct>::type, sig0, sig1, sig2>::return_type
#define XIO_HANDLER_TYPE(ct, sig) \
  typename ::xio::async_result< \
    typename ::std::decay<ct>::type, sig>::completion_handler_type
#define XIO_HANDLER_TYPE2(ct, sig0, sig1) \
  typename ::xio::async_result< \
    typename ::std::decay<ct>::type, \
      sig0, sig1>::completion_handler_type
#define XIO_HANDLER_TYPE3(ct, sig0, sig1, sig2) \
  typename ::xio::async_result< \
    typename ::std::decay<ct>::type, \
      sig0, sig1, sig2>::completion_handler_type

# define XIO_INITFN_AUTO_RESULT_TYPE(ct, sig) \
  auto
# define XIO_INITFN_AUTO_RESULT_TYPE2(ct, sig0, sig1) \
  auto
# define XIO_INITFN_AUTO_RESULT_TYPE3(ct, sig0, sig1, sig2) \
  auto

# define XIO_INITFN_AUTO_RESULT_TYPE_PREFIX(ct, sig) \
  auto
# define XIO_INITFN_AUTO_RESULT_TYPE_PREFIX2(ct, sig0, sig1) \
  auto
# define XIO_INITFN_AUTO_RESULT_TYPE_PREFIX3(ct, sig0, sig1, sig2) \
  auto
# define XIO_INITFN_AUTO_RESULT_TYPE_SUFFIX(expr)


# define XIO_INITFN_DEDUCED_RESULT_TYPE(ct, sig, expr) \
  decltype expr
# define XIO_INITFN_DEDUCED_RESULT_TYPE2(ct, sig0, sig1, expr) \
  decltype expr
# define XIO_INITFN_DEDUCED_RESULT_TYPE3(ct, sig0, sig1, sig2, expr) \
  decltype expr


    template<typename CompletionToken,
        XIO_COMPLETION_SIGNATURE... Signatures,
        typename Initiation, typename... Args>
    inline auto async_initiate(Initiation &&initiation,
                               type_identity_t<CompletionToken> &token, Args &&... args)
        -> decltype(std::enable_if_t <
                    std::enable_if_t <
                    detail::are_completion_signatures<Signatures...>::value,
                    detail::async_result_has_initiate_memfn<
                        CompletionToken, Signatures...> > ::value,
                    async_result<std::decay_t<CompletionToken>, Signatures...> > ::initiate(
                        static_cast<Initiation &&>(initiation),
                        static_cast<CompletionToken &&>(token),
                        static_cast<Args &&>(args)...)) {
        return async_result<std::decay_t<CompletionToken>, Signatures...>::initiate(
            static_cast<Initiation &&>(initiation),
            static_cast<CompletionToken &&>(token),
            static_cast<Args &&>(args)...);
    }

    template<
        XIO_COMPLETION_SIGNATURE... Signatures,
        typename CompletionToken, typename Initiation, typename... Args>
    inline auto async_initiate(Initiation &&initiation,
                               CompletionToken &&token, Args &&... args)
        -> decltype(std::enable_if_t <
                    std::enable_if_t <
                    detail::are_completion_signatures<Signatures...>::value,
                    detail::async_result_has_initiate_memfn<
                        CompletionToken, Signatures...> > ::value,
                    async_result<std::decay_t<CompletionToken>, Signatures...> > ::initiate(
                        static_cast<Initiation &&>(initiation),
                        static_cast<CompletionToken &&>(token),
                        static_cast<Args &&>(args)...)) {
        return async_result<std::decay_t<CompletionToken>, Signatures...>::initiate(
            static_cast<Initiation &&>(initiation),
            static_cast<CompletionToken &&>(token),
            static_cast<Args &&>(args)...);
    }

    template<typename CompletionToken,
        XIO_COMPLETION_SIGNATURE... Signatures,
        typename Initiation, typename... Args>
    inline typename std::enable_if_t<
        !std::enable_if_t <
        detail::are_completion_signatures<Signatures...>::value,
        detail::async_result_has_initiate_memfn<
            CompletionToken, Signatures...> >::value
    ,
    async_result<std::decay_t<CompletionToken>, Signatures...>
    >
    ::return_type
    async_initiate(Initiation &&initiation,
                   type_identity_t<CompletionToken> &token, Args &&... args) {
        async_completion<CompletionToken, Signatures...> completion(token);

        static_cast<Initiation &&>(initiation)(
            static_cast<
                typename async_result<std::decay_t<CompletionToken>,
                    Signatures...>::completion_handler_type &&>(
                completion.completion_handler),
            static_cast<Args &&>(args)...);

        return completion.result.get();
    }

    template<XIO_COMPLETION_SIGNATURE... Signatures,
        typename CompletionToken, typename Initiation, typename... Args>
    inline typename std::enable_if_t<
        !std::enable_if_t <
        detail::are_completion_signatures<Signatures...>::value,
        detail::async_result_has_initiate_memfn<
            CompletionToken, Signatures...> >::value
    ,
    async_result<std::decay_t<CompletionToken>, Signatures...>
    >
    ::return_type
    async_initiate(Initiation &&initiation, CompletionToken &&token, Args &&... args) {
        async_completion<CompletionToken, Signatures...> completion(token);

        static_cast<Initiation &&>(initiation)(
            static_cast<
                typename async_result<std::decay_t<CompletionToken>,
                    Signatures...>::completion_handler_type &&>(
                completion.completion_handler),
            static_cast<Args &&>(args)...);

        return completion.result.get();
    }


#if defined(XIO_HAS_CONCEPTS)

    namespace detail {
        template<typename... Signatures>
        struct initiation_archetype {
            template<completion_handler_for<Signatures...> CompletionHandler>
            void operator()(CompletionHandler &&) const {
            }
        };
    } // namespace detail

    template<typename T, typename... Signatures>
    XIO_CONCEPT completion_token_for =
            detail::are_completion_signatures < Signatures...>::value
    &&
    requires(T &&t) {
        async_initiate<T, Signatures...>(
            detail::initiation_archetype<Signatures...>{}, t);
    };

#define XIO_COMPLETION_TOKEN_FOR(sig) \
  ::xio::completion_token_for<sig>
#define XIO_COMPLETION_TOKEN_FOR2(sig0, sig1) \
  ::xio::completion_token_for<sig0, sig1>
#define XIO_COMPLETION_TOKEN_FOR3(sig0, sig1, sig2) \
  ::xio::completion_token_for<sig0, sig1, sig2>

#else // defined(XIO_HAS_CONCEPTS)

#define XIO_COMPLETION_TOKEN_FOR(sig) typename
#define XIO_COMPLETION_TOKEN_FOR2(sig0, sig1) typename
#define XIO_COMPLETION_TOKEN_FOR3(sig0, sig1, sig2) typename

#endif // defined(XIO_HAS_CONCEPTS)

    namespace detail {
        struct async_operation_probe {
        };

        struct async_operation_probe_result {
        };

        template<typename Call, typename = void>
        struct is_async_operation_call : std::false_type {
        };

        template<typename Call>
        struct is_async_operation_call<Call,
                    void_t<
                        std::enable_if_t<
                            std::is_same<
                                result_of_t<Call>,
                                async_operation_probe_result
                            >::value
                        >
                    >
                > : std::true_type {
        };
    } // namespace detail

    template<typename... Signatures>
    class async_result<detail::async_operation_probe, Signatures...> {
    public:
        typedef detail::async_operation_probe_result return_type;

        template<typename Initiation, typename... InitArgs>
        static return_type initiate(Initiation &&,
                                    detail::async_operation_probe, InitArgs &&...) {
            return return_type();
        }
    };



    template<typename T, typename... Args>
    struct is_async_operation :
            detail::is_async_operation_call<
                T(Args..., detail::async_operation_probe)> {
    };



#if defined(XIO_HAS_CONCEPTS)

    template<typename T, typename... Args>
    XIO_CONCEPT async_operation = is_async_operation<T, Args...>::value;

#define XIO_ASYNC_OPERATION \
  ::xio::async_operation
#define XIO_ASYNC_OPERATION1(a0) \
  ::xio::async_operation<a0>
#define XIO_ASYNC_OPERATION2(a0, a1) \
  ::xio::async_operation<a0, a1>
#define XIO_ASYNC_OPERATION3(a0, a1, a2) \
  ::xio::async_operation<a0, a1, a2>

#else // defined(XIO_HAS_CONCEPTS)

#define XIO_ASYNC_OPERATION typename
#define XIO_ASYNC_OPERATION1(a0) typename
#define XIO_ASYNC_OPERATION2(a0, a1) typename
#define XIO_ASYNC_OPERATION3(a0, a1, a2) typename

#endif // defined(XIO_HAS_CONCEPTS)

    namespace detail {
        struct completion_signature_probe {
        };

        template<typename... T>
        struct completion_signature_probe_result {
            template<template <typename...> class Op>
            struct apply {
                typedef Op<T...> type;
            };
        };

        template<typename T>
        struct completion_signature_probe_result<T> {
            typedef T type;

            template<template <typename...> class Op>
            struct apply {
                typedef Op<T> type;
            };
        };

        template<>
        struct completion_signature_probe_result<void> {
            template<template <typename...> class Op>
            struct apply {
                typedef Op<> type;
            };
        };
    } // namespace detail

    template<typename... Signatures>
    class async_result<detail::completion_signature_probe, Signatures...> {
    public:
        typedef detail::completion_signature_probe_result<Signatures...> return_type;

        template<typename Initiation, typename... InitArgs>
        static return_type initiate(Initiation &&,
                                    detail::completion_signature_probe, InitArgs &&...) {
            return return_type();
        }
    };

    template<typename Signature>
    class async_result<detail::completion_signature_probe, Signature> {
    public:
        typedef detail::completion_signature_probe_result<Signature> return_type;

        template<typename Initiation, typename... InitArgs>
        static return_type initiate(Initiation &&,
                                    detail::completion_signature_probe, InitArgs &&...) {
            return return_type();
        }
    };



    template<typename T, typename... Args>
    struct completion_signature_of :
            result_of_t<T(Args..., detail::completion_signature_probe)> {
    };


    template<typename T, typename... Args>
    using completion_signature_of_t =
    typename completion_signature_of<T, Args...>::type;


} // namespace xio

#include <xio/detail/pop_options.h>

#include <xio/default_completion_token.h>

#endif // XIO_ASYNC_RESULT_HPP
