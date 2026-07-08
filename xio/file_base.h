//
// file_base.hpp
// ~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef XIO_FILE_BASE_HPP
#define XIO_FILE_BASE_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>

#if defined(XIO_HAS_FILE)

#if !defined(XIO_WINDOWS)
# include <fcntl.h>
#endif // !defined(XIO_WINDOWS)

#include <xio/detail/push_options.h>

namespace xio {


    /// The file_base class is used as a base for the basic_stream_file and
/// basic_random_access_file class templates so that we have a common place to
/// define flags.
    class file_base {
    public:

enum flags {
#if defined(XIO_WINDOWS)
read_only=1,
write_only=2,
read_write=4,
append=8,
create=16,
exclusive=32,
truncate=64,
sync_all_on_write=128
#else // defined(XIO_WINDOWS)
read_only= O_RDONLY,
write_only= O_WRONLY,
read_write= O_RDWR,
append= O_APPEND,
create= O_CREAT,
exclusive= O_EXCL,
truncate= O_TRUNC,
sync_all_on_write= O_SYNC
#endif // defined(XIO_WINDOWS)
};

// Implement bitmask operations as shown in C++ Std [lib.bitmask.types].

friend flags operator&(flags x, flags y) {
    return static_cast<flags>(
        static_cast<unsigned int>(x) & static_cast<unsigned int>(y));
}

friend flags operator|(flags x, flags y) {
    return static_cast<flags>(
        static_cast<unsigned int>(x) | static_cast<unsigned int>(y));
}

friend flags operator^(flags x, flags y) {
    return static_cast<flags>(
        static_cast<unsigned int>(x) ^ static_cast<unsigned int>(y));
}

friend flags operator~(flags x) {
    return static_cast<flags>(~static_cast<unsigned int>(x));
}

friend flags &operator&=(flags &x, flags y) {
    x = x & y;
    return x;
}

friend flags &operator|=(flags &x, flags y) {
    x = x | y;
    return x;
}

friend flags &operator^=(flags &x, flags y) {
    x = x ^ y;
    return x;
}

/// Basis for seeking in a file.
enum seek_basis {

seek_set= SEEK_SET,
seek_cur= SEEK_CUR,
seek_end= SEEK_END

};

protected:
/// Protected destructor to prevent deletion through this type.
~file_base() {
}
};

} // namespace xio

#include <xio/detail/pop_options.h>

#endif // defined(XIO_HAS_FILE)

#endif // XIO_FILE_BASE_HPP
