//
// impl/execution_context.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef XIO_IMPL_EXECUTION_CONTEXT_HPP
#define XIO_IMPL_EXECUTION_CONTEXT_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <cstring>
#include <xio/detail/memory.h>
#include <xio/detail/throw_exception.h>

#include <xio/detail/push_options.h>

namespace xio {


    template<typename Allocator>
    execution_context::execution_context(allocator_arg_t, const Allocator &a)
        : execution_context(detail::allocate_object<allocator_impl<Allocator> >(a, a)) {
    }

    template<typename Allocator>
    execution_context::execution_context(allocator_arg_t, const Allocator &a,
                                         const service_maker &initial_services)
        : execution_context(detail::allocate_object<allocator_impl<Allocator> >(a, a),
                            initial_services) {
    }

    inline execution_context::auto_allocator_ptr::~auto_allocator_ptr() {
        ptr_->destroy();
    }

    template<typename Allocator>
    void execution_context::allocator_impl<Allocator>::destroy() {
        detail::deallocate_object(allocator_, this);
    }

    template<typename Allocator>
    void *execution_context::allocator_impl<Allocator>::allocate(
        std::size_t size, std::size_t align) {
        typename std::allocator_traits<Allocator>::template
                rebind_alloc<unsigned char> alloc(allocator_);

        std::size_t space = size + align - 1;
        unsigned char *base = std::allocator_traits<decltype(alloc)>::allocate(
            alloc, space + sizeof(std::ptrdiff_t));

        void *p = base;
        if (detail::align(align, size, p, space)) {
            std::ptrdiff_t off = static_cast<unsigned char *>(p) - base;
            std::memcpy(static_cast<unsigned char *>(p) + size, &off, sizeof(off));
            return p;
        }

        std::bad_alloc ex;
        xio::detail::throw_exception(ex);
        return 0;
    }

    template<typename Allocator>
    void execution_context::allocator_impl<Allocator>::deallocate(
        void *ptr, std::size_t size, std::size_t align) {
        if (ptr) {
            typename std::allocator_traits<Allocator>::template
                    rebind_alloc<unsigned char> alloc(allocator_);

            std::ptrdiff_t off;
            std::memcpy(&off, static_cast<unsigned char *>(ptr) + size, sizeof(off));
            unsigned char *base = static_cast<unsigned char *>(ptr) - off;

            std::allocator_traits<decltype(alloc)>::deallocate(
                alloc, base, size + align - 1 + sizeof(std::ptrdiff_t));
        }
    }


    inline execution_context &execution_context::service::context() {
        return owner_;
    }


} // namespace xio

#include <xio/detail/pop_options.h>

#endif // XIO_IMPL_EXECUTION_CONTEXT_HPP
