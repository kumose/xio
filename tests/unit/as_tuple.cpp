//
// as_tuple.cpp
// ~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//


// Test that header file is self-contained.
#include <xio/as_tuple.h>

#include <xio/bind_executor.h>
#include <xio/deferred.h>
#include <xio/io_context.h>
#include <xio/post.h>
#include <xio/system_timer.h>
#include <xio/use_future.h>
#include "unit_test.hpp"

void as_tuple_test() {
    xio::io_context io1;
    xio::io_context io2;
    xio::system_timer timer1(io1);
    int count = 0;

    timer1.expires_after(std::chrono::seconds(0));
    timer1.async_wait(
        xio::as_tuple(
            xio::bind_executor(io2.get_executor(),
                               [&count](std::tuple<xio::error_code>) {
                                   ++count;
                               })));

    XIO_CHECK(count == 0);

    io1.run();

    XIO_CHECK(count == 0);

    io2.run();

    XIO_CHECK(count == 1);

    timer1.async_wait(
        xio::as_tuple(
            xio::bind_executor(io2.get_executor(),
                               xio::deferred)))(
        [&count](std::tuple<xio::error_code>) {
            ++count;
        });

    XIO_CHECK(count == 1);

    io1.restart();
    io1.run();

    XIO_CHECK(count == 1);

    io2.restart();
    io2.run();

    XIO_CHECK(count == 2);

    std::future<std::tuple<xio::error_code> > f = timer1.async_wait(
        xio::as_tuple(
            xio::bind_executor(io2.get_executor(),
                               xio::use_future)));

    XIO_CHECK(f.wait_for(std::chrono::seconds(0))
        == std::future_status::timeout);

    io1.restart();
    io1.run();

    XIO_CHECK(f.wait_for(std::chrono::seconds(0))
        == std::future_status::timeout);

    io2.restart();
    io2.run();

    XIO_CHECK(f.wait_for(std::chrono::seconds(0))
        == std::future_status::ready);
}

void as_tuple_constness_test() {
    xio::io_context io1;
    xio::system_timer timer1(io1);

    auto tok1 = xio::as_tuple(xio::use_future);
    (void) timer1.async_wait(tok1);
    (void) timer1.async_wait(std::move(tok1));

    const auto tok2 = xio::as_tuple(xio::use_future);
    (void) timer1.async_wait(tok2);
    (void) timer1.async_wait(std::move(tok2));

    constexpr auto tok3 = xio::as_tuple(xio::use_future);
    (void) timer1.async_wait(tok3);
    (void) timer1.async_wait(std::move(tok3));

}

void partial_as_tuple_test() {
    xio::io_context io1;
    xio::io_context io2;
    xio::system_timer timer1(io1);
    int count = 0;

    timer1.expires_after(std::chrono::seconds(0));
    timer1.async_wait(xio::as_tuple)(
        xio::bind_executor(io2.get_executor(),
                           [&count](std::tuple<xio::error_code>) {
                               ++count;
                           }));

    XIO_CHECK(count == 0);

    io1.run();

    XIO_CHECK(count == 0);

    io2.run();

    XIO_CHECK(count == 1);

    timer1.async_wait(xio::as_tuple)(
        xio::bind_executor(io2.get_executor(),
                           xio::deferred))(
        [&count](std::tuple<xio::error_code>) {
            ++count;
        });

    XIO_CHECK(count == 1);

    io1.restart();
    io1.run();

    XIO_CHECK(count == 1);

    io2.restart();
    io2.run();

    XIO_CHECK(count == 2);


    std::future<std::tuple<xio::error_code> > f
            = timer1.async_wait(xio::as_tuple)(
                xio::bind_executor(io2.get_executor(),
                                   xio::use_future));

    XIO_CHECK(f.wait_for(std::chrono::seconds(0))
        == std::future_status::timeout);

    io1.restart();
    io1.run();

    XIO_CHECK(f.wait_for(std::chrono::seconds(0))
        == std::future_status::timeout);

    io2.restart();
    io2.run();

    XIO_CHECK(f.wait_for(std::chrono::seconds(0))
        == std::future_status::ready);

}

XIO_TEST_SUITE
(
    "as_tuple",
    XIO_TEST_CASE(as_tuple_test)
    XIO_COMPILE_TEST_CASE(as_tuple_constness_test)
    XIO_TEST_CASE(partial_as_tuple_test)
)
