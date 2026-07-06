//
// experimental/promise.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2021-2023 Klemens D. Morgenstern
//                         (klemens dot morgenstern at gmx dot net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_EXPERIMENTAL_PROMISE_HPP
#define ASIO_EXPERIMENTAL_PROMISE_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>
#include <xio/detail/type_traits.h>
#include <xio/any_io_executor.h>
#include <xio/associated_cancellation_slot.h>
#include <xio/associated_executor.h>
#include <xio/bind_executor.h>
#include <xio/cancellation_signal.h>
#include <xio/dispatch.h>
#include <xio/experimental/impl/promise.h>
#include <xio/post.h>

#include <algorithm>

#include <xio/detail/push_options.h>

namespace xio {
    ASIO_INLINE_NAMESPACE_BEGIN

    namespace experimental {
        template<typename T>
        struct is_promise : std::false_type {
        };

        template<typename... Ts>
        struct is_promise<promise<Ts...> > : std::true_type {
        };

        template<typename T>
        constexpr bool is_promise_v = is_promise<T>::value;

        template<typename... Ts>
        struct promise_value_type {
            using type = std::tuple<Ts...>;
        };

        template<typename T>
        struct promise_value_type<T> {
            using type = T;
        };

        template<>
        struct promise_value_type<> {
            using type = std::tuple<>;
        };

#if defined(GENERATING_DOCUMENTATION)
        /// A disposable handle for an eager operation.
        /**
 * @tparam Signature The signature of the operation.
 *
 * @tparam Executor The executor to be used by the promise (taken from the
 * operation).
 *
 * @tparam Allocator The allocator used for the promise. Can be set through
 * use_allocator.
 *
 * A promise can be used to initiate an asynchronous option that can be
 * completed later. If the promise gets destroyed before completion, the
 * operation gets a cancel signal and the result is ignored.
 *
 * A promise fulfills the requirements of async_operation.
 *
 * @par Examples
 * Reading and writing from one coroutine.
 * @code
 * awaitable<void> read_write_some(xio::ip::tcp::socket & sock,
 *     xio::mutable_buffer read_buf, xio::const_buffer to_write)
 * {
 *   auto p = xio::async_read(read_buf,
 *       xio::experimental::use_promise);
 *   co_await xio::async_write_some(to_write);
 *   co_await p;
 * }
 * @endcode
 */
        template<typename Signature = void(),
            typename Executor = xio::any_io_executor,
            typename Allocator = std::allocator<void> >
        struct promise
#else
        template<typename... Ts, typename Executor, typename Allocator>
        struct promise<void(Ts...), Executor, Allocator>
#endif // defined(GENERATING_DOCUMENTATION)
        {
            /// The value that's returned by the promise.
            using value_type = typename promise_value_type<Ts...>::type;

            /// Cancel the promise. Usually done through the destructor.
            void cancel(cancellation_type level = cancellation_type::all) {
                if (impl_ && !impl_->done) {
                    xio::dispatch(impl_->executor,
                                   [level, impl = impl_] { impl->cancel.emit(level); });
                }
            }

            /// Check if the promise is completed already.
            bool completed() const noexcept {
                return impl_ && impl_->done;
            }

            /// Wait for the promise to become ready.
            template<ASIO_COMPLETION_TOKEN_FOR(void (Ts...)) CompletionToken>
            inline ASIO_INITFN_AUTO_RESULT_TYPE(CompletionToken, void (Ts...))

            operator()(CompletionToken &&token) {
                assert(impl_);

                return async_initiate<CompletionToken, void(Ts...)>(
                    initiate_async_wait{impl_}, token);
            }

            promise() = delete;

            promise(const promise &) = delete;

            promise(promise &&) noexcept = default;

            /// Destruct the promise and cancel the operation.
            /**
   * It is safe to destruct a promise of a promise that didn't complete.
   */
            ~promise() { cancel(); }

        private:
#if !defined(GENERATING_DOCUMENTATION)
            template<typename, typename, typename>
            friend struct promise;
            friend struct detail::promise_handler<void(Ts...), Executor, Allocator>;
#endif // !defined(GENERATING_DOCUMENTATION)

            std::shared_ptr<detail::promise_impl<
                void(Ts...), Executor, Allocator> > impl_;

            promise(
                std::shared_ptr<detail::promise_impl<
                    void(Ts...), Executor, Allocator> > impl)
                : impl_(impl) {
            }

            struct initiate_async_wait {
                std::shared_ptr<detail::promise_impl<
                    void(Ts...), Executor, Allocator> > self_;

                template<typename WaitHandler>
                void operator()(WaitHandler &&handler) const {
                    const auto alloc = get_associated_allocator(
                        handler, self_->get_allocator());

                    auto cancel = get_associated_cancellation_slot(handler);

                    if (self_->done) {
                        auto exec = xio::get_associated_executor(
                            handler, self_->get_executor());

                        xio::post(exec,
                                   [self = std::move(self_),
                                       handler = std::forward<WaitHandler>(handler)]() mutable {
                                       self->apply(std::move(handler));
                                   });
                    } else {
                        if (cancel.is_connected()) {
                            struct cancel_handler {
                                std::weak_ptr<detail::promise_impl<
                                    void(Ts...), Executor, Allocator> > self;

                                cancel_handler(
                                    std::weak_ptr<detail::promise_impl<
                                        void(Ts...), Executor, Allocator> > self)
                                    : self(std::move(self)) {
                                }

                                void operator()(cancellation_type level) const {
                                    if (auto p = self.lock()) {
                                        p->cancel.emit(level);
                                        p->cancel_();
                                    }
                                }
                            };
                            cancel.template emplace<cancel_handler>(self_);
                        }

                        self_->set_completion(alloc, std::forward<WaitHandler>(handler));
                    }
                }
            };
        };
    } // namespace experimental

    ASIO_INLINE_NAMESPACE_END
} // namespace xio

#include <xio/detail/pop_options.h>

#endif // ASIO_EXPERIMENTAL_PROMISE_HPP
