//
// associator.hpp
// ~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_ASSOCIATOR_HPP
#define ASIO_ASSOCIATOR_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>

#include <xio/detail/push_options.h>

namespace xio {


    /// Used to generically specialise associators for a type.




    template<template <typename, typename> class Associator,
        typename T, typename DefaultCandidate, typename _ = void>
    struct associator {
    };


} // namespace xio

#include <xio/detail/pop_options.h>

#endif // ASIO_ASSOCIATOR_HPP
