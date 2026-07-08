//
// yield.hpp
// ~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include <xio/coroutine.h>

#ifndef reenter
# define reenter(c) XIO_CORO_REENTER(c)
#endif

#ifndef yield
# define yield XIO_CORO_YIELD
#endif

#ifndef fork
# define fork XIO_CORO_FORK
#endif
