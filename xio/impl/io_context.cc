//
// impl/io_context.ipp
// ~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_IMPL_IO_CONTEXT_IPP
#define ASIO_IMPL_IO_CONTEXT_IPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>
#include <xio/config.h>
#include <xio/io_context.h>
#include <xio/detail/concurrency_hint.h>
#include <limits>
#include <xio/detail/service_registry.h>
#include <xio/detail/throw_error.h>

#if defined(ASIO_HAS_IOCP)
#include <xio/detail/win_iocp_io_context.h>
#else
#include <xio/detail/scheduler.h>
#endif

#include <xio/detail/push_options.h>

namespace xio {


    io_context::io_context()
        : execution_context(config_from_concurrency_hint()),
          impl_(xio::make_service<impl_type>(*this, false)) {
    }

    io_context::io_context(int concurrency_hint)
        : execution_context(config_from_concurrency_hint(concurrency_hint)),
          impl_(xio::make_service<impl_type>(*this, false)) {
    }

    io_context::io_context(const execution_context::service_maker &initial_services)
        : execution_context(initial_services),
          impl_(xio::make_service<impl_type>(*this, false)) {
    }

    io_context::~io_context() {
        shutdown();
    }

    io_context::count_type io_context::run() {
        xio::error_code ec;
        count_type s = impl_.run(ec);
        xio::detail::throw_error(ec);
        return s;
    }

    io_context::count_type io_context::run_one() {
        xio::error_code ec;
        count_type s = impl_.run_one(ec);
        xio::detail::throw_error(ec);
        return s;
    }

    io_context::count_type io_context::poll() {
        xio::error_code ec;
        count_type s = impl_.poll(ec);
        xio::detail::throw_error(ec);
        return s;
    }

    io_context::count_type io_context::poll_one() {
        xio::error_code ec;
        count_type s = impl_.poll_one(ec);
        xio::detail::throw_error(ec);
        return s;
    }

    void io_context::stop() {
        impl_.stop();
    }

    bool io_context::stopped() const {
        return impl_.stopped();
    }

    void io_context::restart() {
        impl_.restart();
    }

    io_context::service::service(xio::io_context &owner)
        : execution_context::service(owner) {
    }

    io_context::service::~service() {
    }

    void io_context::service::shutdown() {
    }

    void io_context::service::notify_fork(io_context::fork_event) {
    }


} // namespace xio

#include <xio/detail/pop_options.h>

#endif // ASIO_IMPL_IO_CONTEXT_IPP
