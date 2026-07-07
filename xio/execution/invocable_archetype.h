//
// execution/invocable_archetype.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_EXECUTION_INVOCABLE_ARCHETYPE_HPP
#define ASIO_EXECUTION_INVOCABLE_ARCHETYPE_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>
#include <xio/detail/type_traits.h>

#include <xio/detail/push_options.h>

namespace xio {


    namespace execution {
        /// An archetypal function object used for determining adherence to the
/// execution::executor concept.
        struct invocable_archetype {
            /// Function call operator.
            template<typename... Args>
            void operator()(Args &&...) {
            }
        };
    } // namespace execution

} // namespace xio

#include <xio/detail/pop_options.h>

#endif // ASIO_EXECUTION_INVOCABLE_ARCHETYPE_HPP

