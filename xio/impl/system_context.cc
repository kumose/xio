//
// impl/system_context.ipp
// ~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef XIO_IMPL_SYSTEM_CONTEXT_IPP
#define XIO_IMPL_SYSTEM_CONTEXT_IPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>
#include <xio/system_context.h>

#include <xio/detail/push_options.h>

namespace xio {


    struct system_context::thread_function {
        detail::scheduler *scheduler_;

        void operator()() {
#if !defined(XIO_NO_EXCEPTIONS)
            try {
#endif// !defined(XIO_NO_EXCEPTIONS)
                xio::error_code ec;
                scheduler_->run(ec);
#if !defined(XIO_NO_EXCEPTIONS)
            } catch (...) {
                std::terminate();
            }
#endif// !defined(XIO_NO_EXCEPTIONS)
        }
    };

    system_context::system_context()
        : scheduler_(add_scheduler(new detail::scheduler(*this, false))),
          threads_(std::allocator<void>()) {
        scheduler_.work_started();

        thread_function f = {&scheduler_};
        num_threads_ = detail::thread::hardware_concurrency() * 2;
        num_threads_ = num_threads_ ? num_threads_ : 2;
        threads_.create_threads(f, num_threads_);
    }

    system_context::~system_context() {
        scheduler_.work_finished();
        scheduler_.stop();
        threads_.join();
    }

    void system_context::stop() {
        scheduler_.stop();
    }

    bool system_context::stopped() const noexcept {
        return scheduler_.stopped();
    }

    void system_context::join() {
        scheduler_.work_finished();
        threads_.join();
    }

    detail::scheduler &system_context::add_scheduler(detail::scheduler *s) {
        detail::scoped_ptr<detail::scheduler> scoped_impl(s);
        xio::add_service<detail::scheduler>(*this, scoped_impl.get());
        return *scoped_impl.release();
    }


} // namespace xio

#include <xio/detail/pop_options.h>

#endif // XIO_IMPL_SYSTEM_CONTEXT_IPP
