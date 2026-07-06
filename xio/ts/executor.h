//
// ts/executor.hpp
// ~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_TS_EXECUTOR_HPP
#define ASIO_TS_EXECUTOR_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/async_result.h>
#include <xio/associated_allocator.h>
#include <xio/execution_context.h>
#include <xio/is_executor.h>
#include <xio/associated_executor.h>
#include <xio/bind_executor.h>
#include <xio/executor_work_guard.h>
#include <xio/system_executor.h>
#include <xio/executor.h>
#include <xio/any_io_executor.h>
#include <xio/dispatch.h>
#include <xio/post.h>
#include <xio/defer.h>
#include <xio/strand.h>
#include <xio/packaged_task.h>
#include <xio/use_future.h>

#endif // ASIO_TS_EXECUTOR_HPP
