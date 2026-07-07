//
// detail/completion_payload.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_DETAIL_COMPLETION_PAYLOAD_HPP
#define ASIO_DETAIL_COMPLETION_PAYLOAD_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>
#include <xio/detail/type_traits.h>
#include <xio/error_code.h>
#include <xio/detail/completion_message.h>
#include <variant>


#include <xio/detail/push_options.h>

namespace xio {


    namespace detail {
        template<typename... Signatures>
        class completion_payload;

        template<typename R>
        class completion_payload<R()> {
        public:
            explicit completion_payload (completion_message<R()>)
            {
            }

            template<typename Handler>
            void receive(Handler &handler) {
                static_cast<Handler &&>(handler)();
            }
        };

        template<typename Signature>
        class completion_payload<Signature> {
        public:
            completion_payload(completion_message<Signature> &&m)
                : message_(static_cast<completion_message<Signature> &&>(m)) {
            }

            template<typename Handler>
            void receive(Handler &handler) {
                message_.receive(handler);
            }

        private:
            completion_message<Signature> message_;
        };

        template<typename... Signatures>
        class completion_payload {
        public:
            template<typename Signature>
            completion_payload(completion_message<Signature> &&m)
                : message_(static_cast<completion_message<Signature> &&>(m)) {
            }

            template<typename Handler>
            void receive(Handler &handler) {
                std::visit(
                    [&](auto &message) {
                        message.receive(handler);
                    }, message_);
            }

        private:
            std::variant<completion_message<Signatures>...> message_;
        };

    } // namespace detail

} // namespace xio

#include <xio/detail/pop_options.h>

#endif // ASIO_DETAIL_COMPLETION_PAYLOAD_HPP
