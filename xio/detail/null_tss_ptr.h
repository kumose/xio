//
// detail/null_tss_ptr.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef XIO_DETAIL_NULL_TSS_PTR_HPP
#define XIO_DETAIL_NULL_TSS_PTR_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>

#if !defined(XIO_HAS_THREADS)

#include <xio/detail/noncopyable.h>

#include <xio/detail/push_options.h>

namespace xio {


    namespace detail {
        template<typename T>
        class null_tss_ptr
                : private noncopyable {
        public:
            // Constructor.
            null_tss_ptr()
                : value_(0) {
            }

            // Destructor.
            ~null_tss_ptr() {
            }

            // Get the value.
            operator T *() const {
                return value_;
            }

            // Set the value.
            void operator=(T *value) {
                value_ = value;
            }

        private:
            T *value_;
        };
    } // namespace detail

} // namespace xio

#include <xio/detail/pop_options.h>

#endif // !defined(XIO_HAS_THREADS)

#endif // XIO_DETAIL_NULL_TSS_PTR_HPP
