//
// thread.hpp
// ~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef XIO_THREAD_HPP
#define XIO_THREAD_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>
#include <xio/detail/noncopyable.h>
#include <xio/detail/thread.h>

#include <xio/detail/push_options.h>

namespace xio {


    /// A simple abstraction for starting threads.
    /**
 * The xio::thread class implements the smallest possible subset of the
 * functionality of boost::thread. It is intended to be used only for starting
 * a thread and waiting for it to exit. If more extensive threading
 * capabilities are required, you are strongly advised to use something else.
 *
 * @par Thread Safety
 * @e Distinct @e objects: Safe.@n
 * @e Shared @e objects: Unsafe.
 *
 * @par Example
 * A typical use of xio::thread would be to launch a thread to run an
 * io_context's event processing loop:
 *
 * @par
 * @code xio::io_context io_context;
 * // ...
 * xio::thread t(boost::bind(&xio::io_context::run, &io_context));
 * // ...
 * t.join(); @endcode
 */
    class thread
            : private noncopyable {
    public:
        /// Start a new thread that executes the supplied function.
        /**
   * This constructor creates a new thread that will execute the given function
   * or function object.
   *
   * @param f The function or function object to be run in the thread. The
   * function signature must be: @code void f(); @endcode
   */
        template<typename Function>
        explicit thread(Function f)
            : impl_(f) {
        }

        /// Destructor.
        ~thread() {
        }

        /// Wait for the thread to exit.
        /**
   * This function will block until the thread has exited.
   *
   * If this function is not called before the thread object is destroyed, the
   * thread itself will continue to run until completion. You will, however,
   * no longer have the ability to wait for it to exit.
   */
        void join() {
            impl_.join();
        }

    private:
        detail::thread impl_;
    };


} // namespace xio

#include <xio/detail/pop_options.h>

#endif // XIO_THREAD_HPP
