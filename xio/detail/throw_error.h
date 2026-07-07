//
// detail/throw_error.hpp
// ~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_DETAIL_THROW_ERROR_HPP
#define ASIO_DETAIL_THROW_ERROR_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>
#include <xio/detail/throw_exception.h>
#include <xio/error_code.h>

#include <xio/detail/push_options.h>

namespace xio {


    namespace detail {
ASIO_DECL void do_throw_error(
            const xio::error_code &err
            ASIO_SOURCE_LOCATION_PARAM);

ASIO_DECL void do_throw_error(
            const xio::error_code &err,
            const char *location
            ASIO_SOURCE_LOCATION_PARAM);

        inline void throw_error(
            const xio::error_code &err
            ASIO_SOURCE_LOCATION_DEFAULTED_PARAM) {
            if (err)
                do_throw_error(err ASIO_SOURCE_LOCATION_ARG);
        }

        inline void throw_error(
            const xio::error_code &err,
            const char *location
            ASIO_SOURCE_LOCATION_DEFAULTED_PARAM) {
            if (err)
                do_throw_error(err, location ASIO_SOURCE_LOCATION_ARG);
        }
    } // namespace detail

} // namespace xio

#include <xio/detail/pop_options.h>


#endif // ASIO_DETAIL_THROW_ERROR_HPP
