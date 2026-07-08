//
// detail/resolve_op.hpp
// ~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef XIO_DETAIL_RESOLVE_OP_HPP
#define XIO_DETAIL_RESOLVE_OP_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>
#include <xio/error.h>
#include <xio/detail/operation.h>

#include <xio/detail/push_options.h>

namespace xio {


    namespace detail {
        class resolve_op : public operation {
        public:
            // The error code to be passed to the completion handler.
            xio::error_code ec_;

        protected:
            resolve_op(func_type complete_func)
                : operation(complete_func) {
            }
        };
    } // namespace detail

} // namespace xio

#include <xio/detail/pop_options.h>

#endif // XIO_DETAIL_RESOLVE_OP_HPP
