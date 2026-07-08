//
// detail/fenced_block.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef XIO_DETAIL_FENCED_BLOCK_HPP
#define XIO_DETAIL_FENCED_BLOCK_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>

#if !defined(XIO_HAS_THREADS) \
  || defined(XIO_DISABLE_FENCED_BLOCK) \
  || defined(XIO_HAS_THREAD_SANITIZER)
#include <xio/detail/null_fenced_block.h>
#else
#include <xio/detail/std_fenced_block.h>
#endif

namespace xio {


    namespace detail {
#if !defined(XIO_HAS_THREADS) \
  || defined(XIO_DISABLE_FENCED_BLOCK) \
  || defined(XIO_HAS_THREAD_SANITIZER)
        typedef null_fenced_block fenced_block;
#else
        typedef std_fenced_block fenced_block;
#endif
    } // namespace detail

} // namespace xio

#endif // XIO_DETAIL_FENCED_BLOCK_HPP
