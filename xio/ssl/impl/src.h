//
// impl/ssl/src.hpp
// ~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_SSL_IMPL_SRC_HPP
#define ASIO_SSL_IMPL_SRC_HPP

#define ASIO_SOURCE

#include <xio/detail/config.h>

#if defined(ASIO_HEADER_ONLY)
# error Do not compile Asio library source with ASIO_HEADER_ONLY defined
#endif

#include "xio/ssl/impl/context.ipp"
#include "xio/ssl/impl/error.ipp"
#include "xio/ssl/detail/impl/engine.ipp"
#include "xio/ssl/detail/impl/openssl_init.ipp"
#include "xio/ssl/impl/host_name_verification.ipp"

#endif // ASIO_SSL_IMPL_SRC_HPP
