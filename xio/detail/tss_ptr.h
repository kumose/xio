//
// detail/tss_ptr.hpp
// ~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_DETAIL_TSS_PTR_HPP
#define ASIO_DETAIL_TSS_PTR_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>

#if !defined(ASIO_HAS_THREADS)
#include <xio/detail/null_tss_ptr.h>
#elif defined(ASIO_HAS_THREAD_KEYWORD_EXTENSION)
#include <xio/detail/keyword_tss_ptr.h>
#elif defined(ASIO_WINDOWS)
#include <xio/detail/win_tss_ptr.h>
#elif defined(ASIO_HAS_PTHREADS)
#include <xio/detail/posix_tss_ptr.h>
#else
# error Only Windows and POSIX are supported!
#endif

#include <xio/detail/push_options.h>

namespace xio {
    ASIO_INLINE_NAMESPACE_BEGIN

    namespace detail {
        template<typename T>
        class tss_ptr
#if !defined(ASIO_HAS_THREADS)
                : public null_tss_ptr<T>
#elif defined(ASIO_HAS_THREAD_KEYWORD_EXTENSION)
                : public keyword_tss_ptr<T>
#elif defined(ASIO_WINDOWS)
                : public win_tss_ptr<T>
#elif defined(ASIO_HAS_PTHREADS)
                : public posix_tss_ptr<T>
#endif
        {
        public:
            void operator=(T *value) {
#if !defined(ASIO_HAS_THREADS)
                null_tss_ptr<T>::operator=(value);
#elif defined(ASIO_HAS_THREAD_KEYWORD_EXTENSION)
                keyword_tss_ptr<T>::operator=(value);
#elif defined(ASIO_WINDOWS)
                win_tss_ptr<T>::operator=(value);
#elif defined(ASIO_HAS_PTHREADS)
                posix_tss_ptr<T>::operator=(value);
#endif
            }
        };
    } // namespace detail
    ASIO_INLINE_NAMESPACE_END
} // namespace xio

#include <xio/detail/pop_options.h>

#endif // ASIO_DETAIL_TSS_PTR_HPP
