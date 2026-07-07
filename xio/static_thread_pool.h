//
// static_thread_pool.hpp
// ~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_STATIC_THREAD_POOL_HPP
#define ASIO_STATIC_THREAD_POOL_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>
#include <xio/thread_pool.h>

#include <xio/detail/push_options.h>

namespace xio {


    typedef thread_pool static_thread_pool;


} // namespace xio

#include <xio/detail/pop_options.h>

#endif // ASIO_STATIC_THREAD_POOL_HPP
