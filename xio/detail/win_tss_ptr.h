//
// detail/win_tss_ptr.hpp
// ~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_DETAIL_WIN_TSS_PTR_HPP
#define ASIO_DETAIL_WIN_TSS_PTR_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>

#if defined(ASIO_WINDOWS)

#include <xio/detail/noncopyable.h>
#include <xio/detail/socket_types.h>

#include <xio/detail/push_options.h>

namespace xio {


    namespace detail {
        // Helper function to create thread-specific storage.
ASIO_DECL DWORD win_tss_ptr_create();

        template<typename T>
        class win_tss_ptr
                : private noncopyable {
        public:
            // Constructor.
            win_tss_ptr()
                : tss_key_(win_tss_ptr_create()) {
            }

            // Destructor.
            ~win_tss_ptr() {
                ::TlsFree(tss_key_);
            }

            // Get the value.
            operator T *() const {
                return static_cast<T *>(::TlsGetValue(tss_key_));
            }

            // Set the value.
            void operator=(T *value) {
                ::TlsSetValue(tss_key_, value);
            }

        private:
            // Thread-specific storage to allow unlocked access to determine whether a
            // thread is a member of the pool.
            DWORD tss_key_;
        };
    } // namespace detail

} // namespace xio

#include <xio/detail/pop_options.h>


#endif // defined(ASIO_WINDOWS)

#endif // ASIO_DETAIL_WIN_TSS_PTR_HPP
