//
// impl/buffered_read_stream.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef XIO_IMPL_BUFFERED_READ_STREAM_HPP
#define XIO_IMPL_BUFFERED_READ_STREAM_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/associator.h>
#include <xio/detail/handler_cont_helpers.h>
#include <xio/detail/handler_type_requirements.h>
#include <xio/detail/non_const_lvalue.h>
#include <xio/detail/type_traits.h>

#include <xio/detail/push_options.h>

namespace xio {


    template<typename Stream>
    std::size_t buffered_read_stream<Stream>::fill() {
        detail::buffer_resize_guard<detail::buffered_stream_storage>
                resize_guard(storage_);
        std::size_t previous_size = storage_.size();
        storage_.resize(storage_.capacity());
        storage_.resize(previous_size + next_layer_.read_some(buffer(
                            storage_.data() + previous_size,
                            storage_.size() - previous_size)));
        resize_guard.commit();
        return storage_.size() - previous_size;
    }

    template<typename Stream>
    std::size_t buffered_read_stream<Stream>::fill(xio::error_code &ec) {
        detail::buffer_resize_guard<detail::buffered_stream_storage>
                resize_guard(storage_);
        std::size_t previous_size = storage_.size();
        storage_.resize(storage_.capacity());
        storage_.resize(previous_size + next_layer_.read_some(buffer(
                                                                  storage_.data() + previous_size,
                                                                  storage_.size() - previous_size),
                                                              ec));
        resize_guard.commit();
        return storage_.size() - previous_size;
    }

    namespace detail {
        template<typename ReadHandler>
        class buffered_fill_handler {
        public:
            buffered_fill_handler(detail::buffered_stream_storage &storage,
                                  std::size_t previous_size, ReadHandler &handler)
                : storage_(storage),
                  previous_size_(previous_size),
                  handler_(static_cast<ReadHandler &&>(handler)) {
            }

            buffered_fill_handler(const buffered_fill_handler &other)
                : storage_(other.storage_),
                  previous_size_(other.previous_size_),
                  handler_(other.handler_) {
            }

            buffered_fill_handler(buffered_fill_handler &&other)
                : storage_(other.storage_),
                  previous_size_(other.previous_size_),
                  handler_(static_cast<ReadHandler &&>(other.handler_)) {
            }

            void operator()(const xio::error_code &ec,
                            const std::size_t bytes_transferred) {
                storage_.resize(previous_size_ + bytes_transferred);
                static_cast<ReadHandler &&>(handler_)(ec, bytes_transferred);
            }

            //private:
            detail::buffered_stream_storage &storage_;
            std::size_t previous_size_;
            ReadHandler handler_;
        };

        template<typename ReadHandler>
        inline bool xio_handler_is_continuation(
            buffered_fill_handler<ReadHandler> *this_handler) {
            return XIO_VERSIONED_NAME(handler_cont_helpers)
            ::is_continuation(
                this_handler->handler_);
        }

        template<typename Stream>
        class initiate_async_buffered_fill {
        public:
            typedef typename std::remove_reference_t<
                Stream>::lowest_layer_type::executor_type executor_type;

            explicit initiate_async_buffered_fill(
                std::remove_reference_t<Stream> &next_layer)
                : next_layer_(next_layer) {
            }

            executor_type get_executor() const noexcept {
                return next_layer_.lowest_layer().get_executor();
            }

            template<typename ReadHandler>
            void operator()(ReadHandler &&handler,
                            buffered_stream_storage *storage) const {
                // If you get an error on the following line it means that your handler
                // does not meet the documented type requirements for a ReadHandler.
                XIO_READ_HANDLER_CHECK(ReadHandler, handler)
                type_check;

                non_const_lvalue<ReadHandler> handler2(handler);
                std::size_t previous_size = storage->size();
                storage->resize(storage->capacity());
                next_layer_.async_read_some(
                    buffer(
                        storage->data() + previous_size,
                        storage->size() - previous_size),
                    buffered_fill_handler<std::decay_t<ReadHandler> >(
                        *storage, previous_size, handler2.value));
            }

        private:
            std::remove_reference_t<Stream> &next_layer_;
        };
    } // namespace detail

    template<template <typename, typename> class Associator,
        typename ReadHandler, typename DefaultCandidate>
    struct associator<Associator,
                detail::buffered_fill_handler<ReadHandler>,
                DefaultCandidate>
            : Associator<ReadHandler, DefaultCandidate> {
        static typename Associator<ReadHandler, DefaultCandidate>::type get(
            const detail::buffered_fill_handler<ReadHandler> &h) noexcept {
            return Associator<ReadHandler, DefaultCandidate>::get(h.handler_);
        }

        static auto get(const detail::buffered_fill_handler<ReadHandler> &h,
                        const DefaultCandidate &c) noexcept
            -> decltype(Associator<ReadHandler, DefaultCandidate>::get(h.handler_, c)) {
            return Associator<ReadHandler, DefaultCandidate>::get(h.handler_, c);
        }
    };

    template<typename Stream>
    template<
        XIO_COMPLETION_TOKEN_FOR(void (xio::error_code,
        std::size_t)) ReadHandler>
    inline auto buffered_read_stream<Stream>::async_fill(ReadHandler &&handler)
        -> decltype(
            async_initiate<ReadHandler,
                void(xio::error_code, std::size_t)>(
                std::declval<detail::initiate_async_buffered_fill<Stream> >(),
                handler, std::declval<detail::buffered_stream_storage *>())) {
        return async_initiate<ReadHandler,
            void(xio::error_code, std::size_t)>(
            detail::initiate_async_buffered_fill<Stream>(next_layer_),
            handler, &storage_);
    }

    template<typename Stream>
    template<typename MutableBufferSequence>
    std::size_t buffered_read_stream<Stream>::read_some(
        const MutableBufferSequence &buffers) {
        using xio::buffer_size;
        if (buffer_size(buffers) == 0)
            return 0;

        if (storage_.empty())
            this->fill();

        return this->copy(buffers);
    }

    template<typename Stream>
    template<typename MutableBufferSequence>
    std::size_t buffered_read_stream<Stream>::read_some(
        const MutableBufferSequence &buffers, xio::error_code &ec) {
        ec = xio::error_code();

        using xio::buffer_size;
        if (buffer_size(buffers) == 0)
            return 0;

        if (storage_.empty() && !this->fill(ec))
            return 0;

        return this->copy(buffers);
    }

    namespace detail {
        template<typename MutableBufferSequence, typename ReadHandler>
        class buffered_read_some_handler {
        public:
            buffered_read_some_handler(detail::buffered_stream_storage &storage,
                                       const MutableBufferSequence &buffers, ReadHandler &handler)
                : storage_(storage),
                  buffers_(buffers),
                  handler_(static_cast<ReadHandler &&>(handler)) {
            }

            buffered_read_some_handler(const buffered_read_some_handler &other)
                : storage_(other.storage_),
                  buffers_(other.buffers_),
                  handler_(other.handler_) {
            }

            buffered_read_some_handler(buffered_read_some_handler &&other)
                : storage_(other.storage_),
                  buffers_(other.buffers_),
                  handler_(static_cast<ReadHandler &&>(other.handler_)) {
            }

            void operator()(const xio::error_code &ec, std::size_t) {
                if (ec || storage_.empty()) {
                    const std::size_t length = 0;
                    static_cast<ReadHandler &&>(handler_)(ec, length);
                } else {
                    const std::size_t bytes_copied = xio::buffer_copy(
                        buffers_, storage_.data(), storage_.size());
                    storage_.consume(bytes_copied);
                    static_cast<ReadHandler &&>(handler_)(ec, bytes_copied);
                }
            }

            //private:
            detail::buffered_stream_storage &storage_;
            MutableBufferSequence buffers_;
            ReadHandler handler_;
        };

        template<typename MutableBufferSequence, typename ReadHandler>
        inline bool xio_handler_is_continuation(
            buffered_read_some_handler<
                MutableBufferSequence, ReadHandler> *this_handler) {
            return XIO_VERSIONED_NAME(handler_cont_helpers)
            ::is_continuation(
                this_handler->handler_);
        }

        template<typename Stream>
        class initiate_async_buffered_read_some {
        public:
            typedef typename std::remove_reference_t<
                Stream>::lowest_layer_type::executor_type executor_type;

            explicit initiate_async_buffered_read_some(
                std::remove_reference_t<Stream> &next_layer)
                : next_layer_(next_layer) {
            }

            executor_type get_executor() const noexcept {
                return next_layer_.lowest_layer().get_executor();
            }

            template<typename ReadHandler, typename MutableBufferSequence>
            void operator()(ReadHandler &&handler,
                            buffered_stream_storage *storage,
                            const MutableBufferSequence &buffers) const {
                // If you get an error on the following line it means that your handler
                // does not meet the documented type requirements for a ReadHandler.
                XIO_READ_HANDLER_CHECK(ReadHandler, handler)
                type_check;

                using xio::buffer_size;
                non_const_lvalue<ReadHandler> handler2(handler);
                if (buffer_size(buffers) == 0 || !storage->empty()) {
                    next_layer_.async_read_some(mutable_buffer(0, 0),
                                                buffered_read_some_handler<MutableBufferSequence,
                                                    std::decay_t<ReadHandler> >(
                                                    *storage, buffers, handler2.value));
                } else {
                    initiate_async_buffered_fill<Stream>(this->next_layer_)(
                        buffered_read_some_handler<MutableBufferSequence,
                            std::decay_t<ReadHandler> >(
                            *storage, buffers, handler2.value),
                        storage);
                }
            }

        private:
            std::remove_reference_t<Stream> &next_layer_;
        };
    } // namespace detail


    template<template <typename, typename> class Associator,
        typename MutableBufferSequence, typename ReadHandler,
        typename DefaultCandidate>
    struct associator<Associator,
                detail::buffered_read_some_handler<MutableBufferSequence, ReadHandler>,
                DefaultCandidate>
            : Associator<ReadHandler, DefaultCandidate> {
        static typename Associator<ReadHandler, DefaultCandidate>::type get(
            const detail::buffered_read_some_handler<
                MutableBufferSequence, ReadHandler> &h) noexcept {
            return Associator<ReadHandler, DefaultCandidate>::get(h.handler_);
        }

        static auto get(
            const detail::buffered_read_some_handler<
                MutableBufferSequence, ReadHandler> &h,
            const DefaultCandidate &c) noexcept
            -> decltype(Associator<ReadHandler, DefaultCandidate>::get(h.handler_, c)) {
            return Associator<ReadHandler, DefaultCandidate>::get(h.handler_, c);
        }
    };


    template<typename Stream>
    template<typename MutableBufferSequence,
        XIO_COMPLETION_TOKEN_FOR(void (xio::error_code,
        std::size_t)) ReadHandler>
    inline auto buffered_read_stream<Stream>::async_read_some(
        const MutableBufferSequence &buffers, ReadHandler &&handler)
        -> decltype(
            async_initiate<ReadHandler,
                void(xio::error_code, std::size_t)>(
                std::declval<detail::initiate_async_buffered_read_some<Stream> >(),
                handler, std::declval<detail::buffered_stream_storage *>(), buffers)) {
        return async_initiate<ReadHandler,
            void(xio::error_code, std::size_t)>(
            detail::initiate_async_buffered_read_some<Stream>(next_layer_),
            handler, &storage_, buffers);
    }

    template<typename Stream>
    template<typename MutableBufferSequence>
    std::size_t buffered_read_stream<Stream>::peek(
        const MutableBufferSequence &buffers) {
        if (storage_.empty())
            this->fill();
        return this->peek_copy(buffers);
    }

    template<typename Stream>
    template<typename MutableBufferSequence>
    std::size_t buffered_read_stream<Stream>::peek(
        const MutableBufferSequence &buffers, xio::error_code &ec) {
        ec = xio::error_code();
        if (storage_.empty() && !this->fill(ec))
            return 0;
        return this->peek_copy(buffers);
    }


} // namespace xio

#include <xio/detail/pop_options.h>

#endif // XIO_IMPL_BUFFERED_READ_STREAM_HPP
