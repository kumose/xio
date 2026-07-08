//
// is_contiguous_iterator.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef XIO_IS_CONTIGUOUS_ITERATOR_HPP
#define XIO_IS_CONTIGUOUS_ITERATOR_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>
#include <iterator>
#include <xio/detail/type_traits.h>

#include <xio/detail/push_options.h>

namespace xio {


    /// The is_contiguous_iterator class is a traits class that may be used to
/// determine whether a type is a contiguous iterator.




    template<typename T>
    struct is_contiguous_iterator :
#if defined(XIO_HAS_STD_CONCEPTS)
            std::integral_constant<bool, std::contiguous_iterator<T> >
#else // defined(XIO_HAS_STD_CONCEPTS)
            std::is_pointer<T>
#endif // defined(XIO_HAS_STD_CONCEPTS)
    {
    };


} // namespace xio

#include <xio/detail/pop_options.h>

#endif // XIO_IS_CONTIGUOUS_ITERATOR_HPP
