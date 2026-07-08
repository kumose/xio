//
// detail/posix_tss_ptr.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef XIO_DETAIL_POSIX_TSS_PTR_HPP
#define XIO_DETAIL_POSIX_TSS_PTR_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>

#if defined(XIO_HAS_PTHREADS)

#include <pthread.h>
#include <xio/detail/noncopyable.h>

#include <xio/detail/push_options.h>

namespace xio {


    namespace detail {
        // Helper function to create thread-specific storage.
        XIO_DECL void posix_tss_ptr_create(pthread_key_t &key);

        template<typename T>
        class posix_tss_ptr
                : private noncopyable {
        public:
            // Constructor.
            posix_tss_ptr() {
                posix_tss_ptr_create(tss_key_);
            }

            // Destructor.
            ~posix_tss_ptr() {
                ::pthread_key_delete(tss_key_);
            }

            // Get the value.
            operator T *() const {
                return static_cast<T *>(::pthread_getspecific(tss_key_));
            }

            // Set the value.
            void operator=(T *value) {
                ::pthread_setspecific(tss_key_, value);
            }

        private:
            // Thread-specific storage to allow unlocked access to determine whether a
            // thread is a member of the pool.
            pthread_key_t tss_key_;
        };
    } // namespace detail

} // namespace xio

#include <xio/detail/pop_options.h>


#endif // defined(XIO_HAS_PTHREADS)

#endif // XIO_DETAIL_POSIX_TSS_PTR_HPP
