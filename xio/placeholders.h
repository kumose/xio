//
// placeholders.hpp
// ~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef XIO_PLACEHOLDERS_HPP
#define XIO_PLACEHOLDERS_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>
#include <functional>
#include <xio/detail/type_traits.h>

#include <xio/detail/push_options.h>

namespace xio {


    namespace placeholders {

        inline constexpr std::decay_t<decltype(std::placeholders::_1)> error;
        inline constexpr std::decay_t<decltype(std::placeholders::_2)> bytes_transferred;
        inline constexpr std::decay_t<decltype(std::placeholders::_2)> iterator;
        inline constexpr std::decay_t<decltype(std::placeholders::_2)> results;
        inline constexpr std::decay_t<decltype(std::placeholders::_2)> endpoint;
        inline constexpr std::decay_t<decltype(std::placeholders::_2)> signal_number;

    } // namespace placeholders

} // namespace xio

#include <xio/detail/pop_options.h>

#endif // XIO_PLACEHOLDERS_HPP
