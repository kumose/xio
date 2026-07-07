//
// redirect_error.cpp
// ~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//


// Test that header file is self-contained.
#include <xio/redirect_error.h>

#include <xio/bind_executor.h>
#include <xio/deferred.h>
#include <xio/io_context.h>
#include <xio/post.h>
#include <xio/system_timer.h>
#include <xio/use_future.h>
#include "unit_test.hpp"

struct redirect_error_handler {
    int *count_;

    explicit redirect_error_handler(int *c)
        : count_(c) {
    }

    void operator()() {
        ++(*count_);
    }
};

void redirect_error_test() {
    xio::io_context io1;
    xio::io_context io2;
    xio::system_timer timer1(io1);
    xio::error_code ec = xio::error::would_block;
    int count = 0;

    timer1.expires_after(xio::chrono::seconds(0));
    timer1.async_wait(
        xio::redirect_error(
            xio::bind_executor(io2.get_executor(),
                               redirect_error_handler(&count)), ec));

    ASIO_CHECK(ec == xio::error::would_block);
    ASIO_CHECK(count == 0);

    io1.run();

    ASIO_CHECK(ec == xio::error::would_block);
    ASIO_CHECK(count == 0);

    io2.run();

    ASIO_CHECK(!ec);
    ASIO_CHECK(count == 1);

    ec = xio::error::would_block;
    timer1.async_wait(
        xio::redirect_error(
            xio::bind_executor(io2.get_executor(),
                               xio::deferred), ec))(redirect_error_handler(&count));

    ASIO_CHECK(ec == xio::error::would_block);
    ASIO_CHECK(count == 1);

    io1.restart();
    io1.run();

    ASIO_CHECK(ec == xio::error::would_block);
    ASIO_CHECK(count == 1);

    io2.restart();
    io2.run();

    ASIO_CHECK(!ec);
    ASIO_CHECK(count == 2);

    ec = xio::error::would_block;
    std::future<void> f = timer1.async_wait(
        xio::redirect_error(
            xio::bind_executor(io2.get_executor(),
                               xio::use_future), ec));

    ASIO_CHECK(ec == xio::error::would_block);
    ASIO_CHECK(f.wait_for(std::chrono::seconds(0))
        == std::future_status::timeout);

    io1.restart();
    io1.run();

    ASIO_CHECK(ec == xio::error::would_block);
    ASIO_CHECK(f.wait_for(std::chrono::seconds(0))
        == std::future_status::timeout);

    io2.restart();
    io2.run();

    ASIO_CHECK(!ec);
    ASIO_CHECK(f.wait_for(std::chrono::seconds(0))
        == std::future_status::ready);
}

void partial_redirect_error_test() {
    xio::io_context io1;
    xio::io_context io2;
    xio::system_timer timer1(io1);
    xio::error_code ec = xio::error::would_block;
    int count = 0;

    timer1.expires_after(xio::chrono::seconds(0));
    timer1.async_wait(xio::redirect_error(ec))(
        xio::bind_executor(io2.get_executor(),
                           redirect_error_handler(&count)));

    ASIO_CHECK(ec == xio::error::would_block);
    ASIO_CHECK(count == 0);

    io1.run();

    ASIO_CHECK(ec == xio::error::would_block);
    ASIO_CHECK(count == 0);

    io2.run();

    ASIO_CHECK(!ec);
    ASIO_CHECK(count == 1);

    ec = xio::error::would_block;
    timer1.async_wait(xio::redirect_error(ec))(
        xio::bind_executor(io2.get_executor(),
                           xio::deferred))(redirect_error_handler(&count));

    ASIO_CHECK(ec == xio::error::would_block);
    ASIO_CHECK(count == 1);

    io1.restart();
    io1.run();

    ASIO_CHECK(ec == xio::error::would_block);
    ASIO_CHECK(count == 1);

    io2.restart();
    io2.run();

    ASIO_CHECK(!ec);
    ASIO_CHECK(count == 2);

    ec = xio::error::would_block;
    timer1.async_wait()(xio::redirect_error(ec))(
        xio::bind_executor(io2.get_executor(),
                           xio::deferred))(redirect_error_handler(&count));

    ASIO_CHECK(ec == xio::error::would_block);
    ASIO_CHECK(count == 2);

    io1.restart();
    io1.run();

    ASIO_CHECK(ec == xio::error::would_block);
    ASIO_CHECK(count == 2);

    io2.restart();
    io2.run();

    ASIO_CHECK(!ec);
    ASIO_CHECK(count == 3);

    ec = xio::error::would_block;
    std::future<void> f = timer1.async_wait(xio::redirect_error(ec))(
        xio::bind_executor(io2.get_executor(), xio::use_future));

    ASIO_CHECK(ec == xio::error::would_block);
    ASIO_CHECK(f.wait_for(std::chrono::seconds(0))
        == std::future_status::timeout);

    io1.restart();
    io1.run();

    ASIO_CHECK(ec == xio::error::would_block);
    ASIO_CHECK(f.wait_for(std::chrono::seconds(0))
        == std::future_status::timeout);

    io2.restart();
    io2.run();

    ASIO_CHECK(!ec);
    ASIO_CHECK(f.wait_for(std::chrono::seconds(0))
        == std::future_status::ready);
}

ASIO_TEST_SUITE
(
    "redirect_error",
    ASIO_TEST_CASE(redirect_error_test)
    ASIO_TEST_CASE(partial_redirect_error_test)
)
