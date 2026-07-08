//
// use_future.cpp
// ~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//


// Test that header file is self-contained.
#include <xio/use_future.h>

#include <string>
#include "unit_test.hpp"


#include "archetypes/async_ops.hpp"

void use_future_0_test() {
    using xio::use_future;
    using namespace archetypes;

    std::future<void> f;

    f = async_op_0(use_future);
    try {
        f.get();
    } catch (...) {
        XIO_CHECK(false);
    }

    f = async_op_ec_0(true, use_future);
    try {
        f.get();
    } catch (...) {
        XIO_CHECK(false);
    }

    f = async_op_ec_0(false, use_future);
    try {
        f.get();
        XIO_CHECK(false);
    } catch (std::system_error &e) {
        XIO_CHECK(e.code() == xio::error::operation_aborted);
    } catch (...) {
        XIO_CHECK(false);
    }

    f = async_op_ex_0(true, use_future);
    try {
        f.get();
    } catch (...) {
        XIO_CHECK(false);
    }

    f = async_op_ex_0(false, use_future);
    try {
        f.get();
        XIO_CHECK(false);
    } catch (std::exception &e) {
        XIO_CHECK(e.what() == std::string("blah"));
    } catch (...) {
        XIO_CHECK(false);
    }
}

void use_future_1_test() {
    using xio::use_future;
    using namespace archetypes;

    std::future<int> f;

    f = async_op_1(use_future);
    try {
        int i = f.get();
        XIO_CHECK(i == 42);
    } catch (...) {
        XIO_CHECK(false);
    }

    f = async_op_ec_1(true, use_future);
    try {
        int i = f.get();
        XIO_CHECK(i == 42);
    } catch (...) {
        XIO_CHECK(false);
    }

    f = async_op_ec_1(false, use_future);
    try {
        int i = f.get();
        XIO_CHECK(false);
        (void) i;
    } catch (std::system_error &e) {
        XIO_CHECK(e.code() == xio::error::operation_aborted);
    } catch (...) {
        XIO_CHECK(false);
    }

    f = async_op_ex_1(true, use_future);
    try {
        int i = f.get();
        XIO_CHECK(i == 42);
    } catch (...) {
        XIO_CHECK(false);
    }

    f = async_op_ex_1(false, use_future);
    try {
        int i = f.get();
        XIO_CHECK(false);
        (void) i;
    } catch (std::exception &e) {
        XIO_CHECK(e.what() == std::string("blah"));
    } catch (...) {
        XIO_CHECK(false);
    }
}

void use_future_2_test() {
    using xio::use_future;
    using namespace archetypes;

    std::future<std::tuple<int, double> > f;

    f = async_op_2(use_future);
    try {
        int i;
        double d;
        std::tie(i, d) = f.get();
        XIO_CHECK(i == 42);
        XIO_CHECK(d == 2.0);
    } catch (...) {
        XIO_CHECK(false);
    }

    f = async_op_ec_2(true, use_future);
    try {
        int i;
        double d;
        std::tie(i, d) = f.get();
        XIO_CHECK(i == 42);
        XIO_CHECK(d == 2.0);
    } catch (...) {
        XIO_CHECK(false);
    }

    f = async_op_ec_2(false, use_future);
    try {
        std::tuple<int, double> t = f.get();
        XIO_CHECK(false);
        (void) t;
    } catch (std::system_error &e) {
        XIO_CHECK(e.code() == xio::error::operation_aborted);
    } catch (...) {
        XIO_CHECK(false);
    }

    f = async_op_ex_2(true, use_future);
    try {
        int i;
        double d;
        std::tie(i, d) = f.get();
        XIO_CHECK(i == 42);
        XIO_CHECK(d == 2.0);
    } catch (...) {
        XIO_CHECK(false);
    }

    f = async_op_ex_2(false, use_future);
    try {
        std::tuple<int, double> t = f.get();
        XIO_CHECK(false);
        (void) t;
    } catch (std::exception &e) {
        XIO_CHECK(e.what() == std::string("blah"));
    } catch (...) {
        XIO_CHECK(false);
    }
}

void use_future_3_test() {
    using xio::use_future;
    using namespace archetypes;

    std::future<std::tuple<int, double, char> > f;

    f = async_op_3(use_future);
    try {
        int i;
        double d;
        char c;
        std::tie(i, d, c) = f.get();
        XIO_CHECK(i == 42);
        XIO_CHECK(d == 2.0);
        XIO_CHECK(c == 'a');
    } catch (...) {
        XIO_CHECK(false);
    }

    f = async_op_ec_3(true, use_future);
    try {
        int i;
        double d;
        char c;
        std::tie(i, d, c) = f.get();
        XIO_CHECK(i == 42);
        XIO_CHECK(d == 2.0);
        XIO_CHECK(c == 'a');
    } catch (...) {
        XIO_CHECK(false);
    }

    f = async_op_ec_3(false, use_future);
    try {
        std::tuple<int, double, char> t = f.get();
        XIO_CHECK(false);
        (void) t;
    } catch (std::system_error &e) {
        XIO_CHECK(e.code() == xio::error::operation_aborted);
    } catch (...) {
        XIO_CHECK(false);
    }

    f = async_op_ex_3(true, use_future);
    try {
        int i;
        double d;
        char c;
        std::tie(i, d, c) = f.get();
        XIO_CHECK(i == 42);
        XIO_CHECK(d == 2.0);
        XIO_CHECK(c == 'a');
    } catch (...) {
        XIO_CHECK(false);
    }

    f = async_op_ex_3(false, use_future);
    try {
        std::tuple<int, double, char> t = f.get();
        XIO_CHECK(false);
        (void) t;
    } catch (std::exception &e) {
        XIO_CHECK(e.what() == std::string("blah"));
    } catch (...) {
        XIO_CHECK(false);
    }
}

int package_0() {
    return 42;
}

int package_ec_0(xio::error_code ec) {
    return ec ? 0 : 42;
}

int package_ex_0(std::exception_ptr ex) {
    return ex ? 0 : 42;
}

void use_future_package_0_test() {
    using xio::use_future;
    using namespace archetypes;

    std::future<int> f;

    f = async_op_0(use_future(package_0));
    try {
        int i = f.get();
        XIO_CHECK(i == 42);
    } catch (...) {
        XIO_CHECK(false);
    }

    f = async_op_ec_0(true, use_future(&package_ec_0));
    try {
        int i = f.get();
        XIO_CHECK(i == 42);
    } catch (...) {
        XIO_CHECK(false);
    }

    f = async_op_ec_0(false, use_future(package_ec_0));
    try {
        int i = f.get();
        XIO_CHECK(i == 0);
    } catch (...) {
        XIO_CHECK(false);
    }

    f = async_op_ex_0(true, use_future(package_ex_0));
    try {
        int i = f.get();
        XIO_CHECK(i == 42);
    } catch (...) {
        XIO_CHECK(false);
    }

    f = async_op_ex_0(false, use_future(package_ex_0));
    try {
        int i = f.get();
        XIO_CHECK(i == 0);
    } catch (...) {
        XIO_CHECK(false);
    }
}

int package_1(int i) {
    return i;
}

int package_ec_1(xio::error_code ec, int i) {
    return ec ? 0 : i;
}

int package_ex_1(std::exception_ptr ex, int i) {
    return ex ? 0 : i;
}

void use_future_package_1_test() {
    using xio::use_future;
    using namespace archetypes;

    std::future<int> f;

    f = async_op_1(use_future(package_1));
    try {
        int i = f.get();
        XIO_CHECK(i == 42);
    } catch (...) {
        XIO_CHECK(false);
    }

    f = async_op_ec_1(true, use_future(package_ec_1));
    try {
        int i = f.get();
        XIO_CHECK(i == 42);
    } catch (...) {
        XIO_CHECK(false);
    }

    f = async_op_ec_1(false, use_future(package_ec_1));
    try {
        int i = f.get();
        XIO_CHECK(i == 0);
    } catch (...) {
        XIO_CHECK(false);
    }

    f = async_op_ex_1(true, use_future(package_ex_1));
    try {
        int i = f.get();
        XIO_CHECK(i == 42);
    } catch (...) {
        XIO_CHECK(false);
    }

    f = async_op_ex_1(false, use_future(package_ex_1));
    try {
        int i = f.get();
        XIO_CHECK(i == 0);
    } catch (...) {
        XIO_CHECK(false);
    }
}

int package_2(int i, double) {
    return i;
}

int package_ec_2(xio::error_code ec, int i, double) {
    return ec ? 0 : i;
}

int package_ex_2(std::exception_ptr ex, int i, double) {
    return ex ? 0 : i;
}

void use_future_package_2_test() {
    using xio::use_future;
    using namespace archetypes;

    std::future<int> f;

    f = async_op_2(use_future(package_2));
    try {
        int i = f.get();
        XIO_CHECK(i == 42);
    } catch (...) {
        XIO_CHECK(false);
    }

    f = async_op_ec_2(true, use_future(package_ec_2));
    try {
        int i = f.get();
        XIO_CHECK(i == 42);
    } catch (...) {
        XIO_CHECK(false);
    }

    f = async_op_ec_2(false, use_future(package_ec_2));
    try {
        int i = f.get();
        XIO_CHECK(i == 0);
    } catch (...) {
        XIO_CHECK(false);
    }

    f = async_op_ex_2(true, use_future(package_ex_2));
    try {
        int i = f.get();
        XIO_CHECK(i == 42);
    } catch (...) {
        XIO_CHECK(false);
    }

    f = async_op_ex_2(false, use_future(package_ex_2));
    try {
        int i = f.get();
        XIO_CHECK(i == 0);
    } catch (...) {
        XIO_CHECK(false);
    }
}

int package_3(int i, double, char) {
    return i;
}

int package_ec_3(xio::error_code ec, int i, double, char) {
    return ec ? 0 : i;
}

int package_ex_3(std::exception_ptr ex, int i, double, char) {
    return ex ? 0 : i;
}

void use_future_package_3_test() {
    using xio::use_future;
    using namespace archetypes;

    std::future<int> f;

    f = async_op_3(use_future(package_3));
    try {
        int i = f.get();
        XIO_CHECK(i == 42);
    } catch (...) {
        XIO_CHECK(false);
    }

    f = async_op_ec_3(true, use_future(package_ec_3));
    try {
        int i = f.get();
        XIO_CHECK(i == 42);
    } catch (...) {
        XIO_CHECK(false);
    }

    f = async_op_ec_3(false, use_future(package_ec_3));
    try {
        int i = f.get();
        XIO_CHECK(i == 0);
    } catch (...) {
        XIO_CHECK(false);
    }

    f = async_op_ex_3(true, use_future(package_ex_3));
    try {
        int i = f.get();
        XIO_CHECK(i == 42);
    } catch (...) {
        XIO_CHECK(false);
    }

    f = async_op_ex_3(false, use_future(package_ex_3));
    try {
        int i = f.get();
        XIO_CHECK(i == 0);
    } catch (...) {
        XIO_CHECK(false);
    }
}

XIO_TEST_SUITE
(
    "use_future",
    XIO_TEST_CASE(use_future_0_test)
    XIO_TEST_CASE(use_future_1_test)
    XIO_TEST_CASE(use_future_2_test)
    XIO_TEST_CASE(use_future_3_test)
    XIO_TEST_CASE(use_future_package_0_test)
    XIO_TEST_CASE(use_future_package_1_test)
    XIO_TEST_CASE(use_future_package_2_test)
    XIO_TEST_CASE(use_future_package_3_test)
)
