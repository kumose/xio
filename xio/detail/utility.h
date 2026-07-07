//
// detail/utility.hpp
// ~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_DETAIL_UTILITY_HPP
#define ASIO_DETAIL_UTILITY_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>
#include <utility>

namespace xio {


    namespace detail {
        using std::index_sequence;
        using std::index_sequence_for;
        using std::make_index_sequence;

    } // namespace detail

} // namespace xio

#endif // ASIO_DETAIL_UTILITY_HPP
