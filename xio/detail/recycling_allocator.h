//
// detail/recycling_allocator.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_DETAIL_RECYCLING_ALLOCATOR_HPP
#define ASIO_DETAIL_RECYCLING_ALLOCATOR_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>
#include <xio/detail/memory.h>
#include <xio/detail/thread_context.h>
#include <xio/detail/thread_info_base.h>

#include <xio/detail/push_options.h>

namespace xio {


    namespace detail {
        template<typename T, typename Purpose = thread_info_base::default_tag>
        class recycling_allocator {
        public:
            typedef T value_type;

            template<typename U>
            struct rebind {
                typedef recycling_allocator<U, Purpose> other;
            };

            recycling_allocator() {
            }

            template<typename U>
            recycling_allocator(const recycling_allocator<U, Purpose> &) {
            }

            T *allocate(std::size_t n) {
#if !defined(ASIO_DISABLE_SMALL_BLOCK_RECYCLING)
                void *p = thread_info_base::allocate(Purpose(),
                                                     thread_context::top_of_thread_call_stack(),
                                                     sizeof(T) * n, alignof(T));
#else // !defined(ASIO_DISABLE_SMALL_BLOCK_RECYCLING)
                void *p = xio::aligned_new(alignof(T), sizeof(T) * n);
#endif // !defined(ASIO_DISABLE_SMALL_BLOCK_RECYCLING)
                return static_cast<T *>(p);
            }

            void deallocate(T *p, std::size_t n) {
#if !defined(ASIO_DISABLE_SMALL_BLOCK_RECYCLING)
                thread_info_base::deallocate(Purpose(),
                                             thread_context::top_of_thread_call_stack(), p, sizeof(T) * n);
#else // !defined(ASIO_DISABLE_SMALL_BLOCK_RECYCLING)
                (void) n;
                xio::aligned_delete(p);
#endif // !defined(ASIO_DISABLE_SMALL_BLOCK_RECYCLING)
            }
        };

        template<typename Purpose>
        class recycling_allocator<void, Purpose> {
        public:
            typedef void value_type;

            template<typename U>
            struct rebind {
                typedef recycling_allocator<U, Purpose> other;
            };

            recycling_allocator() {
            }

            template<typename U>
            recycling_allocator(const recycling_allocator<U, Purpose> &) {
            }
        };

        template<typename Allocator, typename Purpose>
        struct get_recycling_allocator {
            typedef Allocator type;
            static type get(const Allocator &a) { return a; }
        };

        template<typename T, typename Purpose>
        struct get_recycling_allocator<std::allocator<T>, Purpose> {
            typedef recycling_allocator<T, Purpose> type;
            static type get(const std::allocator<T> &) { return type(); }
        };
    } // namespace detail

} // namespace xio

#include <xio/detail/pop_options.h>

#endif // ASIO_DETAIL_RECYCLING_ALLOCATOR_HPP
