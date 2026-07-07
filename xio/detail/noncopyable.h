//
// detail/noncopyable.hpp
// ~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_DETAIL_NONCOPYABLE_HPP
#define ASIO_DETAIL_NONCOPYABLE_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>

#include <xio/detail/push_options.h>

namespace xio {


    namespace detail {
        class noncopyable {
        protected:
            noncopyable() {
            }

            ~noncopyable() {
            }

        private:
            noncopyable(const noncopyable &);

            const noncopyable &operator=(const noncopyable &);
        };
    } // namespace detail

    using xio::detail::noncopyable;


} // namespace xio

#include <xio/detail/pop_options.h>

#endif // ASIO_DETAIL_NONCOPYABLE_HPP
