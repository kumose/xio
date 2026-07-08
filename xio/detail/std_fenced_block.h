//
// detail/std_fenced_block.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef XIO_DETAIL_STD_FENCED_BLOCK_HPP
#define XIO_DETAIL_STD_FENCED_BLOCK_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>
#include <atomic>
#include <xio/detail/noncopyable.h>

#include <xio/detail/push_options.h>

namespace xio {


    namespace detail {
        class std_fenced_block
                : private noncopyable {
        public:
            enum half_t { half };

            enum full_t { full };

            // Constructor for a half fenced block.
            explicit std_fenced_block(half_t) {
            }

            // Constructor for a full fenced block.
            explicit std_fenced_block(full_t) {
                std::atomic_thread_fence(std::memory_order_acquire);
            }

            // Destructor.
            ~std_fenced_block() {
                std::atomic_thread_fence(std::memory_order_release);
            }
        };
    } // namespace detail

} // namespace xio

#include <xio/detail/pop_options.h>

#endif // XIO_DETAIL_STD_FENCED_BLOCK_HPP
