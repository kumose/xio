//
// error_code.hpp
// ~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_ERROR_CODE_HPP
#define ASIO_ERROR_CODE_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>
#include <system_error>

#include <xio/detail/push_options.h>

namespace xio {


    typedef std::error_category error_category;
    typedef std::error_code error_code;

    /// Returns the error category used for the system errors produced by xio.
    extern ASIO_DECL const error_category &system_category();


} // namespace xio

#include <xio/detail/pop_options.h>


#endif // ASIO_ERROR_CODE_HPP
