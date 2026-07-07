//
// detail/config.hpp
// ~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_DETAIL_CONFIG_HPP
#define ASIO_DETAIL_CONFIG_HPP

#include <xio/version.h>

// Make standard library feature macros available.
#if defined(__has_include)
# if __has_include(<version>)
#  include <version>
# else // __has_include(<version>)
#  include <cstddef>
# endif // __has_include(<version>)
#else // defined(__has_include)
# include <cstddef>
#endif // defined(__has_include)


#if !defined(ASIO_DECL)
#  if defined(_MSC_VER) || defined(__BORLANDC__) || defined(__CODEGEARC__)
// We need to import/export our code only if the user has specifically asked
// for it by defining ASIO_DYN_LINK.
#   if defined(ASIO_DYN_LINK)
// Export if this is our own source, otherwise import.
#    if defined(ASIO_SOURCE)
#     define ASIO_DECL __declspec(dllexport)
#    else // defined(ASIO_SOURCE)
#     define ASIO_DECL __declspec(dllimport)
#    endif // defined(ASIO_SOURCE)
#   endif // defined(ASIO_DYN_LINK)
#  endif // defined(_MSC_VER) || defined(__BORLANDC__) || defined(__CODEGEARC__)
// If ASIO_DECL isn't defined yet define it now.
# if !defined(ASIO_DECL)
#  define ASIO_DECL
# endif // !defined(ASIO_DECL)
#endif // !defined(ASIO_DECL)

// Helper macro for documentation.
#define ASIO_UNSPECIFIED(e) e

// Microsoft Visual C++ detection.
#if !defined(ASIO_MSVC)
# if defined(_MSC_VER) && (defined(__INTELLISENSE__) \
      || (!defined(__MWERKS__) && !defined(__EDG_VERSION__)))
#  define ASIO_MSVC _MSC_VER
# endif // defined(_MSC_VER) && (defined(__INTELLISENSE__)
#endif // !defined(ASIO_MSVC)

// Clang / libc++ detection.
#if defined(__clang__)
#  if __has_include(<__config>)
#   include <__config>
#   if defined(_LIBCPP_VERSION)
#    define ASIO_HAS_CLANG_LIBCXX 1
#   endif // defined(_LIBCPP_VERSION)
#  endif // __has_include(<__config>)
#endif // defined(__clang__)

// Android platform detection.
#if defined(__ANDROID__)
# include <android/api-level.h>
#endif // defined(__ANDROID__)

// Support for static constexpr with default initialisation.
#if !defined(ASIO_STATIC_CONSTEXPR_DEFAULT_INIT)
# if defined(__GNUC__)
#  if (__GNUC__ >= 8)
#   define ASIO_STATIC_CONSTEXPR_DEFAULT_INIT(type, name) \
     static constexpr const type name{}
#  else // (__GNUC__ >= 8)
#   define ASIO_STATIC_CONSTEXPR_DEFAULT_INIT(type, name) \
     static const type name
#  endif // (__GNUC__ >= 8)
# elif defined(ASIO_MSVC)
#  define ASIO_STATIC_CONSTEXPR_DEFAULT_INIT(type, name) \
    static const type name
# else // defined(ASIO_MSVC)
#  define ASIO_STATIC_CONSTEXPR_DEFAULT_INIT(type, name) \
    static constexpr const type name{}
# endif // defined(ASIO_MSVC)
#endif // !defined(ASIO_STATIC_CONSTEXPR_DEFAULT_INIT)

// Support noexcept on function types on compilers known to allow it.
#if !defined(ASIO_HAS_NOEXCEPT_FUNCTION_TYPE)
# if !defined(ASIO_DISABLE_NOEXCEPT_FUNCTION_TYPE)
#  if defined(__clang__)
#   if (__cplusplus >= 202002)
#    define ASIO_HAS_NOEXCEPT_FUNCTION_TYPE 1
#   endif // (__cplusplus >= 202002)
#  elif defined(__GNUC__)
#   if (__cplusplus >= 202002)
#    define ASIO_HAS_NOEXCEPT_FUNCTION_TYPE 1
#   endif // (__cplusplus >= 202002)
#  elif defined(ASIO_MSVC)
#   if (_MSC_VER >= 1900 && _MSVC_LANG >= 202002)
#    define ASIO_HAS_NOEXCEPT_FUNCTION_TYPE 1
#   endif // (_MSC_VER >= 1900 && _MSVC_LANG >= 202002)
#  endif // defined(ASIO_MSVC)
# endif // !defined(ASIO_DISABLE_NOEXCEPT_FUNCTION_TYPE)
#endif // !defined(ASIO_HAS_NOEXCEPT_FUNCTION_TYPE)

// Support concepts on compilers known to allow them.
#if !defined(ASIO_HAS_CONCEPTS)
# if !defined(ASIO_DISABLE_CONCEPTS)
#  if defined(__cpp_concepts)
#   define ASIO_HAS_CONCEPTS 1
#   if (__cpp_concepts >= 201707)
#    define ASIO_CONCEPT concept
#   else // (__cpp_concepts >= 201707)
#    define ASIO_CONCEPT concept bool
#   endif // (__cpp_concepts >= 201707)
#  endif // defined(__cpp_concepts)
# endif // !defined(ASIO_DISABLE_CONCEPTS)
#endif // !defined(ASIO_HAS_CONCEPTS)

// Support concepts on compilers known to allow them.
#if !defined(ASIO_HAS_STD_CONCEPTS)
# if !defined(ASIO_DISABLE_STD_CONCEPTS)
#  if defined(ASIO_HAS_CONCEPTS)
#   if (__cpp_lib_concepts >= 202002L)
#    define ASIO_HAS_STD_CONCEPTS 1
#   endif // (__cpp_concepts >= 202002L)
#  endif // defined(ASIO_HAS_CONCEPTS)
# endif // !defined(ASIO_DISABLE_STD_CONCEPTS)
#endif // !defined(ASIO_HAS_STD_CONCEPTS)

// Default alignment.
#if defined(__STDCPP_DEFAULT_NEW_ALIGNMENT__)
# define ASIO_DEFAULT_ALIGN __STDCPP_DEFAULT_NEW_ALIGNMENT__
#elif defined(__GNUC__)
# if ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 9)) || (__GNUC__ > 4)
#  define ASIO_DEFAULT_ALIGN alignof(std::max_align_t)
# else // ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 9)) || (__GNUC__ > 4)
#  define ASIO_DEFAULT_ALIGN alignof(max_align_t)
# endif // ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 9)) || (__GNUC__ > 4)
#else // defined(__GNUC__)
# define ASIO_DEFAULT_ALIGN alignof(std::max_align_t)
#endif // defined(__GNUC__)

// Standard library support for aligned allocation.
#if !defined(ASIO_HAS_STD_ALIGNED_ALLOC)
# if !defined(ASIO_DISABLE_STD_ALIGNED_ALLOC)
#  if (__cplusplus >= 201703)
#   if defined(__clang__)
#    if defined(ASIO_HAS_CLANG_LIBCXX)
#     if (_LIBCPP_STD_VER > 14)
#      if defined(__FreeBSD__) || defined(__Fuchsia__) || defined(__wasi__) \
         || defined(__NetBSD__) || defined(__OpenBSD__)
#       define ASIO_HAS_STD_ALIGNED_ALLOC 1
#      elif defined(__ANDROID__)
#       if (__ANDROID_API__ >= 28)
#         define ASIO_HAS_STD_ALIGNED_ALLOC 1
#       endif // (__ANDROID_API__ >= 28)
#      elif defined(__linux__)
#       if defined(_LIBCPP_HAS_MUSL_LIBC)
#        define ASIO_HAS_STD_ALIGNED_ALLOC 1
#       else // !defined(_LIBCPP_HAS_MUSL_LIBC)
#        if (__GLIBC__ > 2) || (__GLIBC__ == 2 && __GLIBC_MINOR__ >= 17)
#         define ASIO_HAS_STD_ALIGNED_ALLOC 1
#        endif // (__GLIBC__ > 2) || (__GLIBC__ == 2 && __GLIBC_MINOR__ >= 17)
#       endif // !defined(_LIBCPP_HAS_MUSL_LIBC)
#      elif defined(__APPLE__)
#       if (_LIBCPP_VERSION > 10000)
#        if defined(__MAC_OS_X_VERSION_MIN_REQUIRED)
#         if (__MAC_OS_X_VERSION_MIN_REQUIRED >= 101500)
#          define ASIO_HAS_STD_ALIGNED_ALLOC 1
#         endif // (__MAC_OS_X_VERSION_MIN_REQUIRED >= 101500)
#        elif defined(__IPHONE_OS_VERSION_MIN_REQUIRED)
#         if (__IPHONE_OS_VERSION_MIN_REQUIRED >= 130000)
#          define ASIO_HAS_STD_ALIGNED_ALLOC 1
#         endif // (__IPHONE_OS_VERSION_MIN_REQUIRED >= 130000)
#        elif defined(__TV_OS_VERSION_MIN_REQUIRED)
#         if (__TV_OS_VERSION_MIN_REQUIRED >= 130000)
#          define ASIO_HAS_STD_ALIGNED_ALLOC 1
#         endif // (__TV_OS_VERSION_MIN_REQUIRED >= 130000)
#        elif defined(__WATCH_OS_VERSION_MIN_REQUIRED)
#         if (__WATCH_OS_VERSION_MIN_REQUIRED >= 60000)
#          define ASIO_HAS_STD_ALIGNED_ALLOC 1
#         endif // (__WATCH_OS_VERSION_MIN_REQUIRED >= 60000)
#        endif // defined(__MAC_OS_X_VERSION_MIN_REQUIRED)
#       endif // (_LIBCPP_VERSION > 10000)
#      endif // defined(__APPLE__)
#     endif // (_LIBCPP_STD_VER > 14)
#    elif defined(_GLIBCXX_HAVE_ALIGNED_ALLOC)
#     define ASIO_HAS_STD_ALIGNED_ALLOC 1
#    endif // defined(_GLIBCXX_HAVE_ALIGNED_ALLOC)
#   elif defined(__GNUC__)
#    if ((__GNUC__ == 7) && (__GNUC_MINOR__ >= 4)) || (__GNUC__ > 7)
#     if defined(_GLIBCXX_HAVE_ALIGNED_ALLOC)
#      define ASIO_HAS_STD_ALIGNED_ALLOC 1
#     endif // defined(_GLIBCXX_HAVE_ALIGNED_ALLOC)
#    endif // ((__GNUC__ == 7) && (__GNUC_MINOR__ >= 4)) || (__GNUC__ > 7)
#   endif // defined(__GNUC__)
#  endif // (__cplusplus >= 201703)
# endif // !defined(ASIO_DISABLE_STD_ALIGNED_ALLOC)
#endif // !defined(ASIO_HAS_STD_ALIGNED_ALLOC)

// Standard library support for std::source_location.
#if !defined(ASIO_HAS_STD_SOURCE_LOCATION)
# if !defined(ASIO_DISABLE_STD_SOURCE_LOCATION)
#  if (__cpp_lib_source_location >= 201907)
#   define ASIO_HAS_STD_SOURCE_LOCATION 1
#  endif // (__cpp_lib_source_location >= 201907)
# endif // !defined(ASIO_DISABLE_STD_SOURCE_LOCATION)
#endif // !defined(ASIO_HAS_STD_SOURCE_LOCATION)

// Standard library support for std::experimental::source_location.
#if !defined(ASIO_HAS_STD_EXPERIMENTAL_SOURCE_LOCATION)
# if !defined(ASIO_DISABLE_STD_EXPERIMENTAL_SOURCE_LOCATION)
#  if defined(__GNUC__)
#   if (__cplusplus >= 201709)
#    if __has_include(<experimental/source_location>)
#     define ASIO_HAS_STD_EXPERIMENTAL_SOURCE_LOCATION 1
#    endif // __has_include(<experimental/source_location>)
#   endif // (__cplusplus >= 201709)
#  endif // defined(__GNUC__)
# endif // !defined(ASIO_DISABLE_STD_EXPERIMENTAL_SOURCE_LOCATION)
#endif // !defined(ASIO_HAS_STD_EXPERIMENTAL_SOURCE_LOCATION)

// Standard library has a source_location that we can use.
#if !defined(ASIO_HAS_SOURCE_LOCATION)
# if !defined(ASIO_DISABLE_SOURCE_LOCATION)
#  if defined(ASIO_HAS_STD_SOURCE_LOCATION)
#   define ASIO_HAS_SOURCE_LOCATION 1
#  elif defined(ASIO_HAS_STD_EXPERIMENTAL_SOURCE_LOCATION)
#   define ASIO_HAS_SOURCE_LOCATION 1
#  endif // defined(ASIO_HAS_STD_EXPERIMENTAL_SOURCE_LOCATION)
# endif // !defined(ASIO_DISABLE_SOURCE_LOCATION)
#endif // !defined(ASIO_HAS_SOURCE_LOCATION)

// Windows App target. Windows but with a limited API.
#if !defined(ASIO_WINDOWS_APP)
# if defined(_WIN32_WINNT) && (_WIN32_WINNT >= 0x0603)
#  include <winapifamily.h>
#  if (WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_APP) \
       || WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_TV_TITLE)) \
   && !WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
#   define ASIO_WINDOWS_APP 1
#  endif // WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_APP)
// && !WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
# endif // defined(_WIN32_WINNT) && (_WIN32_WINNT >= 0x0603)
#endif // !defined(ASIO_WINDOWS_APP)
#if defined(ASIO_WINDOWS_APP)
# define ASIO_VERSION_TAG_b b
#else // defined(ASIO_WINDOWS_APP)
# define ASIO_VERSION_TAG_b
#endif // defined(ASIO_WINDOWS_APP)

// Legacy WinRT target. Windows App is preferred.
#if !defined(ASIO_WINDOWS_RUNTIME)
# if !defined(ASIO_WINDOWS_APP)
#  if defined(__cplusplus_winrt)
#   include <winapifamily.h>
#   if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_APP) \
    && !WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
#    define ASIO_WINDOWS_RUNTIME 1
#   endif // WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_APP)
// && !WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
#  endif // defined(__cplusplus_winrt)
# endif // !defined(ASIO_WINDOWS_APP)
#endif // !defined(ASIO_WINDOWS_RUNTIME)

// Windows target. Excludes WinRT but includes Windows App targets.
#if !defined(ASIO_WINDOWS)
# if !defined(ASIO_WINDOWS_RUNTIME)
#  if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
#   define ASIO_WINDOWS 1
#  elif defined(ASIO_WINDOWS_APP)
#   define ASIO_WINDOWS 1
#  endif // defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
# endif // !defined(ASIO_WINDOWS_RUNTIME)
#endif // !defined(ASIO_WINDOWS)
#if defined(ASIO_WINDOWS)
# define ASIO_VERSION_TAG_c c
#else // defined(ASIO_WINDOWS)
# define ASIO_VERSION_TAG_c
#endif // defined(ASIO_WINDOWS)

// Cygwin target using Win32 sockets.
#if !defined(ASIO_CYGWIN_W32_SOCKETS)
# if defined(__CYGWIN__)
#  if defined(__USE_W32_SOCKETS)
#   define ASIO_CYGWIN_W32_SOCKETS 1
#  endif // defined(__USE_W32_SOCKETS)
# endif // defined(__CYGWIN__)
#endif // !defined(ASIO_CYGWIN_W32_SOCKETS)
#if defined(ASIO_CYGWIN_W32_SOCKETS)
# define ASIO_VERSION_TAG_d d
#else // defined(ASIO_CYGWIN_W32_SOCKETS)
# define ASIO_VERSION_TAG_d
#endif // defined(ASIO_CYGWIN_W32_SOCKETS)

// Windows: target OS version.
#if defined(ASIO_WINDOWS) || defined(ASIO_CYGWIN_W32_SOCKETS)
# if !defined(_WIN32_WINNT) && !defined(_WIN32_WINDOWS)
#  if defined(_MSC_VER) || (defined(__BORLANDC__) && !defined(__clang__))
#   pragma message( \
  "Please define _WIN32_WINNT or _WIN32_WINDOWS appropriately. For example:\n"\
  "- add -D_WIN32_WINNT=0x0601 to the compiler command line; or\n"\
  "- add _WIN32_WINNT=0x0601 to your project's Preprocessor Definitions.\n"\
  "Assuming _WIN32_WINNT=0x0601 (i.e. Windows 7 target).")
#  else // defined(_MSC_VER) || (defined(__BORLANDC__) && !defined(__clang__))
#   warning Please define _WIN32_WINNT or _WIN32_WINDOWS appropriately.
#   warning For example, add -D_WIN32_WINNT=0x0601 to the compiler command line.
#   warning Assuming _WIN32_WINNT=0x0601 (i.e. Windows 7 target).
#  endif // defined(_MSC_VER) || (defined(__BORLANDC__) && !defined(__clang__))
#  define _WIN32_WINNT 0x0601
# endif // !defined(_WIN32_WINNT) && !defined(_WIN32_WINDOWS)
# if defined(_MSC_VER)
#  if defined(_WIN32) && !defined(WIN32)
#   if !defined(_WINSOCK2API_)
#    define WIN32 // Needed for correct types in winsock2.h
#   else // !defined(_WINSOCK2API_)
#    error Please define the macro WIN32 in your compiler options
#   endif // !defined(_WINSOCK2API_)
#  endif // defined(_WIN32) && !defined(WIN32)
# endif // defined(_MSC_VER)
# if defined(__BORLANDC__)
#  if defined(__WIN32__) && !defined(WIN32)
#   if !defined(_WINSOCK2API_)
#    define WIN32 // Needed for correct types in winsock2.h
#   else // !defined(_WINSOCK2API_)
#    error Please define the macro WIN32 in your compiler options
#   endif // !defined(_WINSOCK2API_)
#  endif // defined(__WIN32__) && !defined(WIN32)
# endif // defined(__BORLANDC__)
# if defined(ASIO_CYGWIN_W32_SOCKETS)
#  if !defined(__USE_W32_SOCKETS)
#   error You must add -D__USE_W32_SOCKETS to your compiler options.
#  endif // !defined(__USE_W32_SOCKETS)
# endif // defined(ASIO_CYGWIN_W32_SOCKETS)
#endif // defined(ASIO_WINDOWS) || defined(ASIO_CYGWIN_W32_SOCKETS)

// Windows: minimise header inclusion.
#if defined(ASIO_WINDOWS) || defined(ASIO_CYGWIN_W32_SOCKETS)
# if !defined(ASIO_NO_WIN32_LEAN_AND_MEAN)
#  if !defined(WIN32_LEAN_AND_MEAN)
#   define WIN32_LEAN_AND_MEAN
#  endif // !defined(WIN32_LEAN_AND_MEAN)
# endif // !defined(ASIO_NO_WIN32_LEAN_AND_MEAN)
#endif // defined(ASIO_WINDOWS) || defined(ASIO_CYGWIN_W32_SOCKETS)

// Windows: suppress definition of "min" and "max" macros.
#if defined(ASIO_WINDOWS) || defined(ASIO_CYGWIN_W32_SOCKETS)
# if !defined(ASIO_NO_NOMINMAX)
#  if !defined(NOMINMAX)
#   define NOMINMAX 1
#  endif // !defined(NOMINMAX)
# endif // !defined(ASIO_NO_NOMINMAX)
#endif // defined(ASIO_WINDOWS) || defined(ASIO_CYGWIN_W32_SOCKETS)

// Windows: IO Completion Ports.
#if !defined(ASIO_HAS_IOCP)
# if defined(ASIO_WINDOWS) || defined(ASIO_CYGWIN_W32_SOCKETS)
#  if defined(_WIN32_WINNT) && (_WIN32_WINNT >= 0x0400)
#   if !defined(UNDER_CE) && !defined(ASIO_WINDOWS_APP)
#    if !defined(ASIO_DISABLE_IOCP)
#     define ASIO_HAS_IOCP 1
#    endif // !defined(ASIO_DISABLE_IOCP)
#   endif // !defined(UNDER_CE) && !defined(ASIO_WINDOWS_APP)
#  endif // defined(_WIN32_WINNT) && (_WIN32_WINNT >= 0x0400)
# endif // defined(ASIO_WINDOWS) || defined(ASIO_CYGWIN_W32_SOCKETS)
#endif // !defined(ASIO_HAS_IOCP)
#if defined(ASIO_HAS_IOCP)
# define ASIO_VERSION_TAG_e e
#else // defined(ASIO_HAS_IOCP)
# define ASIO_VERSION_TAG_e
#endif // defined(ASIO_HAS_IOCP)

// Windows: Slim Reader/Writer Locks.
// Requires Windows 7 or later for TryAcquireSRWLockExclusive support.
#if !defined(ASIO_HAS_WINDOWS_SRWLOCK)
# if !defined(ASIO_DISABLE_WINDOWS_SRWLOCK)
#  if defined(ASIO_WINDOWS)
#   if !defined(UNDER_CE)
#    if defined(_WIN32_WINNT) && (_WIN32_WINNT >= 0x0601)
#     define ASIO_HAS_WINDOWS_SRWLOCK 1
#    endif // defined(_WIN32_WINNT) && (_WIN32_WINNT >= 0x0601)
#   endif // !defined(UNDER_CE)
#  endif // defined(ASIO_WINDOWS)
# endif // !defined(ASIO_DISABLE_WINDOWS_SRWLOCK)
#endif // !defined(ASIO_HAS_WINDOWS_SRWLOCK)
#if defined(ASIO_HAS_WINDOWS_SRWLOCK)
# define ASIO_VERSION_TAG_f f
#else // defined(ASIO_HAS_WINDOWS_SRWLOCK)
# define ASIO_VERSION_TAG_f
#endif // defined(ASIO_HAS_WINDOWS_SRWLOCK)

// On POSIX (and POSIX-like) platforms we need to include unistd.h in order to
// get access to the various platform feature macros, e.g. to be able to test
// for threads support.
#if !defined(ASIO_HAS_UNISTD_H)
#  if defined(unix) \
   || defined(__unix) \
   || defined(_XOPEN_SOURCE) \
   || defined(_POSIX_SOURCE) \
   || (defined(__MACH__) && defined(__APPLE__)) \
   || defined(__FreeBSD__) \
   || defined(__NetBSD__) \
   || defined(__OpenBSD__) \
   || defined(__linux__) \
   || defined(__HAIKU__)
#   define ASIO_HAS_UNISTD_H 1
#  endif
#endif // !defined(ASIO_HAS_UNISTD_H)
#if defined(ASIO_HAS_UNISTD_H)
# include <unistd.h>
#endif // defined(ASIO_HAS_UNISTD_H)

// Linux: epoll, eventfd, timerfd and io_uring.
#if defined(__linux__)
# include <linux/version.h>
# if !defined(ASIO_HAS_EPOLL)
#  if !defined(ASIO_DISABLE_EPOLL)
#   if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 5, 45)
#    define ASIO_HAS_EPOLL 1
#   endif // LINUX_VERSION_CODE >= KERNEL_VERSION(2,5,45)
#  endif // !defined(ASIO_DISABLE_EPOLL)
# endif // !defined(ASIO_HAS_EPOLL)
# if !defined(ASIO_HAS_EVENTFD)
#  if !defined(ASIO_DISABLE_EVENTFD)
#   if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 22)
#    define ASIO_HAS_EVENTFD 1
#   endif // LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,22)
#  endif // !defined(ASIO_DISABLE_EVENTFD)
# endif // !defined(ASIO_HAS_EVENTFD)
# if !defined(ASIO_HAS_TIMERFD)
#  if !defined(ASIO_DISABLE_TIMERFD)
#   if defined(ASIO_HAS_EPOLL)
#    if (__GLIBC__ > 2) || (__GLIBC__ == 2 && __GLIBC_MINOR__ >= 8)
#     define ASIO_HAS_TIMERFD 1
#    endif // (__GLIBC__ > 2) || (__GLIBC__ == 2 && __GLIBC_MINOR__ >= 8)
#   endif // defined(ASIO_HAS_EPOLL)
#  endif // !defined(ASIO_DISABLE_TIMERFD)
# endif // !defined(ASIO_HAS_TIMERFD)
# if defined(ASIO_HAS_IO_URING)
#  if LINUX_VERSION_CODE < KERNEL_VERSION(5,10,0)
#   error Linux kernel 5.10 or later is required to support io_uring
#  endif // LINUX_VERSION_CODE < KERNEL_VERSION(5,10,0)
# endif // defined(ASIO_HAS_IO_URING)
#endif // defined(__linux__)
#if defined(ASIO_HAS_EPOLL)
# define ASIO_VERSION_TAG_g g
#else // defined(ASIO_HAS_EPOLL)
# define ASIO_VERSION_TAG_g
#endif // defined(ASIO_HAS_EPOLL)
#if defined(ASIO_HAS_EVENTFD)
# define ASIO_VERSION_TAG_h h
#else // defined(ASIO_HAS_EVENTFD)
# define ASIO_VERSION_TAG_h
#endif // defined(ASIO_HAS_EVENTFD)
#if defined(ASIO_HAS_TIMERFD)
# define ASIO_VERSION_TAG_i i
#else // defined(ASIO_HAS_TIMERFD)
# define ASIO_VERSION_TAG_i
#endif // defined(ASIO_HAS_TIMERFD)
#if defined(ASIO_HAS_IO_URING)
# define ASIO_VERSION_TAG_j j
#else // defined(ASIO_HAS_IO_URING)
# define ASIO_VERSION_TAG_j
#endif // defined(ASIO_HAS_IO_URING)

// Linux: io_uring is used instead of epoll.
#if !defined(ASIO_HAS_IO_URING_AS_DEFAULT)
# if !defined(ASIO_HAS_EPOLL) && defined(ASIO_HAS_IO_URING)
#  define ASIO_HAS_IO_URING_AS_DEFAULT 1
# endif // !defined(ASIO_HAS_EPOLL) && defined(ASIO_HAS_IO_URING)
#endif // !defined(ASIO_HAS_IO_URING_AS_DEFAULT)
#if defined(ASIO_HAS_IO_URING_AS_DEFAULT)
# define ASIO_VERSION_TAG_k k
#else // defined(ASIO_HAS_IO_URING_AS_DEFAULT)
# define ASIO_VERSION_TAG_k
#endif // defined(ASIO_HAS_IO_URING_AS_DEFAULT)

// Linux: futex.
#if !defined(ASIO_HAS_FUTEX)
# if !defined(ASIO_DISABLE_FUTEX)
#  if defined(__linux__)
#   if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 22)
#    define ASIO_HAS_FUTEX 1
#   endif // LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,22)
#  endif // defined(__linux__)
# endif // !defined(ASIO_DISABLE_FUTEX)
#endif // !defined(ASIO_HAS_FUTEX)
#if defined(ASIO_HAS_FUTEX)
# define ASIO_VERSION_TAG_l l
#else // defined(ASIO_HAS_FUTEX)
# define ASIO_VERSION_TAG_l
#endif // defined(ASIO_HAS_FUTEX)

// Mac OS X, FreeBSD, NetBSD, OpenBSD: kqueue.
#if (defined(__MACH__) && defined(__APPLE__)) \
  || defined(__FreeBSD__) \
  || defined(__NetBSD__) \
  || defined(__OpenBSD__)
# if !defined(ASIO_HAS_KQUEUE)
#  if !defined(ASIO_DISABLE_KQUEUE)
#   define ASIO_HAS_KQUEUE 1
#  endif // !defined(ASIO_DISABLE_KQUEUE)
# endif // !defined(ASIO_HAS_KQUEUE)
#endif // (defined(__MACH__) && defined(__APPLE__))
//   || defined(__FreeBSD__)
//   || defined(__NetBSD__)
//   || defined(__OpenBSD__)
#if defined(ASIO_HAS_KQUEUE)
# define ASIO_VERSION_TAG_m m
#else // defined(ASIO_HAS_KQUEUE)
# define ASIO_VERSION_TAG_m
#endif // defined(ASIO_HAS_KQUEUE)

// Solaris: /dev/poll.
#if defined(__sun)
# if !defined(ASIO_HAS_DEV_POLL)
#  if !defined(ASIO_DISABLE_DEV_POLL)
#   define ASIO_HAS_DEV_POLL 1
#  endif // !defined(ASIO_DISABLE_DEV_POLL)
# endif // !defined(ASIO_HAS_DEV_POLL)
#endif // defined(__sun)

// Serial ports.
#if !defined(ASIO_HAS_SERIAL_PORT)
# if defined(ASIO_HAS_IOCP) \
  || !defined(ASIO_WINDOWS) \
  && !defined(ASIO_WINDOWS_RUNTIME) \
  && !defined(ASIO_CYGWIN_W32_SOCKETS)
#  if !defined(__SYMBIAN32__)
#   if !defined(ASIO_DISABLE_SERIAL_PORT)
#    define ASIO_HAS_SERIAL_PORT 1
#   endif // !defined(ASIO_DISABLE_SERIAL_PORT)
#  endif // !defined(__SYMBIAN32__)
# endif // defined(ASIO_HAS_IOCP)
//   || !defined(ASIO_WINDOWS)
//   && !defined(ASIO_WINDOWS_RUNTIME)
//   && !defined(ASIO_CYGWIN_W32_SOCKETS)
#endif // !defined(ASIO_HAS_SERIAL_PORT)

// Windows: stream handles.
#if !defined(ASIO_HAS_WINDOWS_STREAM_HANDLE)
# if !defined(ASIO_DISABLE_WINDOWS_STREAM_HANDLE)
#  if defined(ASIO_HAS_IOCP)
#   define ASIO_HAS_WINDOWS_STREAM_HANDLE 1
#  endif // defined(ASIO_HAS_IOCP)
# endif // !defined(ASIO_DISABLE_WINDOWS_STREAM_HANDLE)
#endif // !defined(ASIO_HAS_WINDOWS_STREAM_HANDLE)

// Windows: random access handles.
#if !defined(ASIO_HAS_WINDOWS_RANDOM_ACCESS_HANDLE)
# if !defined(ASIO_DISABLE_WINDOWS_RANDOM_ACCESS_HANDLE)
#  if defined(ASIO_HAS_IOCP)
#   define ASIO_HAS_WINDOWS_RANDOM_ACCESS_HANDLE 1
#  endif // defined(ASIO_HAS_IOCP)
# endif // !defined(ASIO_DISABLE_WINDOWS_RANDOM_ACCESS_HANDLE)
#endif // !defined(ASIO_HAS_WINDOWS_RANDOM_ACCESS_HANDLE)

// Windows: object handles.
#if !defined(ASIO_HAS_WINDOWS_OBJECT_HANDLE)
# if !defined(ASIO_DISABLE_WINDOWS_OBJECT_HANDLE)
#  if defined(ASIO_WINDOWS) || defined(ASIO_CYGWIN_W32_SOCKETS)
#   if !defined(UNDER_CE) && !defined(ASIO_WINDOWS_APP)
#    define ASIO_HAS_WINDOWS_OBJECT_HANDLE 1
#   endif // !defined(UNDER_CE) && !defined(ASIO_WINDOWS_APP)
#  endif // defined(ASIO_WINDOWS)
//   || defined(ASIO_CYGWIN_W32_SOCKETS)
# endif // !defined(ASIO_DISABLE_WINDOWS_OBJECT_HANDLE)
#endif // !defined(ASIO_HAS_WINDOWS_OBJECT_HANDLE)

// Windows: OVERLAPPED wrapper.
#if !defined(ASIO_HAS_WINDOWS_OVERLAPPED_PTR)
# if !defined(ASIO_DISABLE_WINDOWS_OVERLAPPED_PTR)
#  if defined(ASIO_HAS_IOCP)
#   define ASIO_HAS_WINDOWS_OVERLAPPED_PTR 1
#  endif // defined(ASIO_HAS_IOCP)
# endif // !defined(ASIO_DISABLE_WINDOWS_OVERLAPPED_PTR)
#endif // !defined(ASIO_HAS_WINDOWS_OVERLAPPED_PTR)

// POSIX: stream-oriented file descriptors.
#if !defined(ASIO_HAS_POSIX_STREAM_DESCRIPTOR)
# if !defined(ASIO_DISABLE_POSIX_STREAM_DESCRIPTOR)
#  if !defined(ASIO_WINDOWS) \
  && !defined(ASIO_WINDOWS_RUNTIME) \
  && !defined(ASIO_CYGWIN_W32_SOCKETS)
#   define ASIO_HAS_POSIX_STREAM_DESCRIPTOR 1
#  endif // !defined(ASIO_WINDOWS)
//   && !defined(ASIO_WINDOWS_RUNTIME)
//   && !defined(ASIO_CYGWIN_W32_SOCKETS)
# endif // !defined(ASIO_DISABLE_POSIX_STREAM_DESCRIPTOR)
#endif // !defined(ASIO_HAS_POSIX_STREAM_DESCRIPTOR)

// UNIX domain sockets.
#if !defined(ASIO_HAS_LOCAL_SOCKETS)
# if !defined(ASIO_DISABLE_LOCAL_SOCKETS)
#  if !defined(ASIO_WINDOWS_RUNTIME)
#   define ASIO_HAS_LOCAL_SOCKETS 1
#  endif // !defined(ASIO_WINDOWS_RUNTIME)
# endif // !defined(ASIO_DISABLE_LOCAL_SOCKETS)
#endif // !defined(ASIO_HAS_LOCAL_SOCKETS)

// Files.
#if !defined(ASIO_HAS_FILE)
# if !defined(ASIO_DISABLE_FILE)
#  if defined(ASIO_HAS_WINDOWS_RANDOM_ACCESS_HANDLE)
#   define ASIO_HAS_FILE 1
#  elif defined(ASIO_HAS_IO_URING)
#   define ASIO_HAS_FILE 1
#  endif // defined(ASIO_HAS_IO_URING)
# endif // !defined(ASIO_DISABLE_FILE)
#endif // !defined(ASIO_HAS_FILE)

// Pipes.
#if !defined(ASIO_HAS_PIPE)
# if defined(ASIO_HAS_IOCP) \
  || !defined(ASIO_WINDOWS) \
  && !defined(ASIO_WINDOWS_RUNTIME) \
  && !defined(ASIO_CYGWIN_W32_SOCKETS)
#  if !defined(__SYMBIAN32__)
#   if !defined(ASIO_DISABLE_PIPE)
#    define ASIO_HAS_PIPE 1
#   endif // !defined(ASIO_DISABLE_PIPE)
#  endif // !defined(__SYMBIAN32__)
# endif // defined(ASIO_HAS_IOCP)
//   || !defined(ASIO_WINDOWS)
//   && !defined(ASIO_WINDOWS_RUNTIME)
//   && !defined(ASIO_CYGWIN_W32_SOCKETS)
#endif // !defined(ASIO_HAS_PIPE)

// Can use sigaction() instead of signal().
#if !defined(ASIO_HAS_SIGACTION)
# if !defined(ASIO_DISABLE_SIGACTION)
#  if !defined(ASIO_WINDOWS) \
  && !defined(ASIO_WINDOWS_RUNTIME) \
  && !defined(ASIO_CYGWIN_W32_SOCKETS)
#   define ASIO_HAS_SIGACTION 1
#  endif // !defined(ASIO_WINDOWS)
//   && !defined(ASIO_WINDOWS_RUNTIME)
//   && !defined(ASIO_CYGWIN_W32_SOCKETS)
# endif // !defined(ASIO_DISABLE_SIGACTION)
#endif // !defined(ASIO_HAS_SIGACTION)

// Can use signal().
#if !defined(ASIO_HAS_SIGNAL)
# if !defined(ASIO_DISABLE_SIGNAL)
#  if !defined(UNDER_CE)
#   define ASIO_HAS_SIGNAL 1
#  endif // !defined(UNDER_CE)
# endif // !defined(ASIO_DISABLE_SIGNAL)
#endif // !defined(ASIO_HAS_SIGNAL)

// Can use getaddrinfo() and getnameinfo().
#if !defined(ASIO_HAS_GETADDRINFO)
# if !defined(ASIO_DISABLE_GETADDRINFO)
#  if defined(ASIO_WINDOWS) || defined(ASIO_CYGWIN_W32_SOCKETS)
#   if defined(_WIN32_WINNT) && (_WIN32_WINNT >= 0x0501)
#    define ASIO_HAS_GETADDRINFO 1
#   elif defined(UNDER_CE)
#    define ASIO_HAS_GETADDRINFO 1
#   endif // defined(UNDER_CE)
#  elif defined(__MACH__) && defined(__APPLE__)
#   if defined(__MAC_OS_X_VERSION_MIN_REQUIRED)
#    if (__MAC_OS_X_VERSION_MIN_REQUIRED >= 1050)
#     define ASIO_HAS_GETADDRINFO 1
#    endif // (__MAC_OS_X_VERSION_MIN_REQUIRED >= 1050)
#   else // defined(__MAC_OS_X_VERSION_MIN_REQUIRED)
#    define ASIO_HAS_GETADDRINFO 1
#   endif // defined(__MAC_OS_X_VERSION_MIN_REQUIRED)
#  else // defined(__MACH__) && defined(__APPLE__)
#   define ASIO_HAS_GETADDRINFO 1
#  endif // defined(__MACH__) && defined(__APPLE__)
# endif // !defined(ASIO_DISABLE_GETADDRINFO)
#endif // !defined(ASIO_HAS_GETADDRINFO)

// Whether exception handling is disabled.
#if defined(ASIO_NO_EXCEPTIONS)
# define ASIO_VERSION_TAG_n n
#else // defined(ASIO_NO_EXCEPTIONS)
# define ASIO_VERSION_TAG_n
#endif // defined(ASIO_NO_EXCEPTIONS)

// Threads.
#if !defined(ASIO_HAS_THREADS)
# if !defined(ASIO_DISABLE_THREADS)
#  if defined(__GNUC__) && !defined(__MINGW32__) \
     && !defined(linux) && !defined(__linux) && !defined(__linux__)
#   define ASIO_HAS_THREADS 1
#  elif defined(_MT) || defined(__MT__)
#   define ASIO_HAS_THREADS 1
#  elif defined(_REENTRANT)
#   define ASIO_HAS_THREADS 1
#  elif defined(__APPLE__)
#   define ASIO_HAS_THREADS 1
#  elif defined(__HAIKU__)
#   define ASIO_HAS_THREADS 1
#  elif defined(_POSIX_THREADS) && (_POSIX_THREADS + 0 >= 0)
#   define ASIO_HAS_THREADS 1
#  elif defined(_PTHREADS)
#   define ASIO_HAS_THREADS 1
#  endif // defined(__GNUC__) && !defined(__MINGW32__)
# endif // !defined(ASIO_DISABLE_THREADS)
#endif // !defined(ASIO_HAS_THREADS)
#if defined(ASIO_HAS_THREADS)
# define ASIO_VERSION_TAG_o o
#else // defined(ASIO_HAS_THREADS)
# define ASIO_VERSION_TAG_o
#endif // defined(ASIO_HAS_THREADS)

// Thread sanitizer.
#if !defined(ASIO_HAS_THREAD_SANITIZER)
# if defined(__SANITIZE_THREAD__)
#  define ASIO_HAS_THREAD_SANITIZER 1
# elif defined(__has_feature)
#  if __has_feature(thread_sanitizer)
#   define ASIO_HAS_THREAD_SANITIZER 1
#  endif // __has_feature(thread_sanitizer)
# endif // defined(__SANITIZE_THREAD__)
#endif // !defined(ASIO_HAS_THREAD_SANITIZER)

// POSIX threads.
#if !defined(ASIO_HAS_PTHREADS)
# if defined(ASIO_HAS_THREADS)
#  if defined(_POSIX_THREADS) && (_POSIX_THREADS + 0 >= 0)
#   define ASIO_HAS_PTHREADS 1
#  elif defined(__HAIKU__)
#   define ASIO_HAS_PTHREADS 1
#  endif // defined(_POSIX_THREADS) && (_POSIX_THREADS + 0 >= 0)
# endif // defined(ASIO_HAS_THREADS)
#endif // !defined(ASIO_HAS_PTHREADS)
#if defined(ASIO_HAS_PTHREADS)
# define ASIO_VERSION_TAG_p p
#else // defined(ASIO_HAS_PTHREADS)
# define ASIO_VERSION_TAG_p
#endif // defined(ASIO_HAS_PTHREADS)

// Helper to prevent macro expansion.
#define ASIO_PREVENT_MACRO_SUBSTITUTION

// Helper to define in-class constants.
#if !defined(ASIO_STATIC_CONSTANT)
#  define ASIO_STATIC_CONSTANT(type, assignment) \
    static const type assignment
#endif // !defined(ASIO_STATIC_CONSTANT)

// Microsoft Visual C++'s secure C runtime library.
#if !defined(ASIO_HAS_SECURE_RTL)
# if !defined(ASIO_DISABLE_SECURE_RTL)
#  if defined(ASIO_MSVC) \
    && (ASIO_MSVC >= 1400) \
    && !defined(UNDER_CE)
#   define ASIO_HAS_SECURE_RTL 1
#  endif // defined(ASIO_MSVC)
// && (ASIO_MSVC >= 1400)
// && !defined(UNDER_CE)
# endif // !defined(ASIO_DISABLE_SECURE_RTL)
#endif // !defined(ASIO_HAS_SECURE_RTL)

// Handler hooking. Disabled for ancient Borland C++ and gcc compilers.
#if !defined(ASIO_HAS_HANDLER_HOOKS)
# if !defined(ASIO_DISABLE_HANDLER_HOOKS)
#  if defined(__GNUC__)
#   if (__GNUC__ >= 3)
#    define ASIO_HAS_HANDLER_HOOKS 1
#   endif // (__GNUC__ >= 3)
#  elif !defined(__BORLANDC__) || defined(__clang__)
#   define ASIO_HAS_HANDLER_HOOKS 1
#  endif // !defined(__BORLANDC__) || defined(__clang__)
# endif // !defined(ASIO_DISABLE_HANDLER_HOOKS)
#endif // !defined(ASIO_HAS_HANDLER_HOOKS)

// Support for the __thread keyword extension, or equivalent.
#if !defined(ASIO_DISABLE_THREAD_KEYWORD_EXTENSION)
# if defined(__linux__)
#  if defined(__GNUC__) && (defined(__i386__) || defined(__x86_64__))
#   if ((__GNUC__ == 3) && (__GNUC_MINOR__ >= 3)) || (__GNUC__ > 3)
#    if !defined(__INTEL_COMPILER) && !defined(__ICL) \
       && !(defined(__clang__) && defined(__ANDROID__))
#     define ASIO_HAS_THREAD_KEYWORD_EXTENSION 1
#     define ASIO_THREAD_KEYWORD __thread
#    elif defined(__INTEL_COMPILER) && (__INTEL_COMPILER >= 1100)
#     define ASIO_HAS_THREAD_KEYWORD_EXTENSION 1
#    endif // defined(__INTEL_COMPILER) && (__INTEL_COMPILER >= 1100)
// && !(defined(__clang__) && defined(__ANDROID__))
#   endif // ((__GNUC__ == 3) && (__GNUC_MINOR__ >= 3)) || (__GNUC__ > 3)
#  endif // defined(__GNUC__) && (defined(__i386__) || defined(__x86_64__))
# endif // defined(__linux__)
# if defined(ASIO_MSVC) && defined(ASIO_WINDOWS_RUNTIME)
#  if (_MSC_VER >= 1700)
#   define ASIO_HAS_THREAD_KEYWORD_EXTENSION 1
#   define ASIO_THREAD_KEYWORD __declspec(thread)
#  endif // (_MSC_VER >= 1700)
# endif // defined(ASIO_MSVC) && defined(ASIO_WINDOWS_RUNTIME)
# if defined(__APPLE__)
#  if defined(__clang__)
#   if defined(__apple_build_version__)
#    define ASIO_HAS_THREAD_KEYWORD_EXTENSION 1
#    define ASIO_THREAD_KEYWORD __thread
#   endif // defined(__apple_build_version__)
#  endif // defined(__clang__)
# endif // defined(__APPLE__)
#endif // !defined(ASIO_DISABLE_THREAD_KEYWORD_EXTENSION)
#if !defined(ASIO_THREAD_KEYWORD)
# define ASIO_THREAD_KEYWORD __thread
#endif // !defined(ASIO_THREAD_KEYWORD)

// Support for POSIX ssize_t typedef.
#if !defined(ASIO_DISABLE_SSIZE_T)
# if defined(__linux__) \
   || (defined(__MACH__) && defined(__APPLE__))
#  define ASIO_HAS_SSIZE_T 1
# endif // defined(__linux__)
//   || (defined(__MACH__) && defined(__APPLE__))
#endif // !defined(ASIO_DISABLE_SSIZE_T)

// Helper macros to manage transition away from error_code return values.
#if defined(ASIO_NO_DEPRECATED)
# define ASIO_SYNC_OP_VOID void
# define ASIO_SYNC_OP_VOID_RETURN(e) return
#else // defined(ASIO_NO_DEPRECATED)
# define ASIO_SYNC_OP_VOID xio::error_code
# define ASIO_SYNC_OP_VOID_RETURN(e) return e
#endif // defined(ASIO_NO_DEPRECATED)

// Newer gcc, clang need special treatment to suppress unused typedef warnings.
#if defined(__clang__)
# if defined(__apple_build_version__)
#  if (__clang_major__ >= 7)
#   define ASIO_UNUSED_TYPEDEF __attribute__((__unused__))
#  endif // (__clang_major__ >= 7)
# elif ((__clang_major__ == 3) && (__clang_minor__ >= 6)) \
    || (__clang_major__ > 3)
#  define ASIO_UNUSED_TYPEDEF __attribute__((__unused__))
# endif // ((__clang_major__ == 3) && (__clang_minor__ >= 6))
//   || (__clang_major__ > 3)
#elif defined(__GNUC__)
# if ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 8)) || (__GNUC__ > 4)
#  define ASIO_UNUSED_TYPEDEF __attribute__((__unused__))
# endif // ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 8)) || (__GNUC__ > 4)
#endif // defined(__GNUC__)
#if !defined(ASIO_UNUSED_TYPEDEF)
# define ASIO_UNUSED_TYPEDEF
#endif // !defined(ASIO_UNUSED_TYPEDEF)

// Some versions of gcc generate spurious warnings about unused variables.
#if defined(__GNUC__)
# if (__GNUC__ >= 4)
#  define ASIO_UNUSED_VARIABLE __attribute__((__unused__))
# endif // (__GNUC__ >= 4)
#endif // defined(__GNUC__)
#if !defined(ASIO_UNUSED_VARIABLE)
# define ASIO_UNUSED_VARIABLE
#endif // !defined(ASIO_UNUSED_VARIABLE)

// Helper macro to tell the optimiser what may be assumed to be true.
#if defined(ASIO_MSVC)
# define ASIO_ASSUME(expr) __assume(expr)
#elif defined(__clang__)
# if __has_builtin(__builtin_assume)
#  define ASIO_ASSUME(expr) __builtin_assume(expr)
# endif // __has_builtin(__builtin_assume)
#elif defined(__GNUC__)
# if ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 5)) || (__GNUC__ > 4)
#  define ASIO_ASSUME(expr) if (expr) {} else { __builtin_unreachable(); }
# endif // ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 5)) || (__GNUC__ > 4)
#endif // defined(__GNUC__)
#if !defined(ASIO_ASSUME)
# define ASIO_ASSUME(expr) (void)0
#endif // !defined(ASIO_ASSUME)

// Support the co_await keyword on compilers known to allow it.
#if !defined(ASIO_HAS_CO_AWAIT)
# if !defined(ASIO_DISABLE_CO_AWAIT)
#  if (__cplusplus >= 202002) \
     && (__cpp_impl_coroutine >= 201902) && (__cpp_lib_coroutine >= 201902)
#   define ASIO_HAS_CO_AWAIT 1
#  elif defined(ASIO_MSVC)
#   if (_MSC_VER >= 1928) && (_MSVC_LANG >= 201705) && !defined(__clang__)
#    define ASIO_HAS_CO_AWAIT 1
#   elif (_MSC_FULL_VER >= 190023506)
#    if defined(_RESUMABLE_FUNCTIONS_SUPPORTED)
#     define ASIO_HAS_CO_AWAIT 1
#    endif // defined(_RESUMABLE_FUNCTIONS_SUPPORTED)
#   endif // (_MSC_FULL_VER >= 190023506)
#  elif defined(__clang__)
#   if (__clang_major__ >= 14)
#    if (__cplusplus >= 202002) && (__cpp_impl_coroutine >= 201902)
#     if __has_include(<coroutine>)
#      define ASIO_HAS_CO_AWAIT 1
#     endif // __has_include(<coroutine>)
#    elif (__cplusplus >= 201703) && (__cpp_coroutines >= 201703)
#     if __has_include(<experimental/coroutine>)
#      define ASIO_HAS_CO_AWAIT 1
#     endif // __has_include(<experimental/coroutine>)
#    endif // (__cplusplus >= 201703) && (__cpp_coroutines >= 201703)
#   else // (__clang_major__ >= 14)
#    if (__cplusplus >= 201703) && (__cpp_coroutines >= 201703)
#     if __has_include(<experimental/coroutine>)
#      define ASIO_HAS_CO_AWAIT 1
#     endif // __has_include(<experimental/coroutine>)
#    endif // (__cplusplus >= 201703) && (__cpp_coroutines >= 201703)
#   endif // (__clang_major__ >= 14)
#  elif defined(__GNUC__)
#   if (__cplusplus >= 201709) && (__cpp_impl_coroutine >= 201902)
#    if __has_include(<coroutine>)
#     define ASIO_HAS_CO_AWAIT 1
#    endif // __has_include(<coroutine>)
#   endif // (__cplusplus >= 201709) && (__cpp_impl_coroutine >= 201902)
#  endif // defined(__GNUC__)
# endif // !defined(ASIO_DISABLE_CO_AWAIT)
#endif // !defined(ASIO_HAS_CO_AWAIT)

// Standard library support for coroutines.
#if !defined(ASIO_HAS_STD_COROUTINE)
# if !defined(ASIO_DISABLE_STD_COROUTINE)
#  if (__cplusplus >= 202002) \
     && (__cpp_impl_coroutine >= 201902) && (__cpp_lib_coroutine >= 201902)
#   define ASIO_HAS_STD_COROUTINE 1
#  elif defined(ASIO_MSVC)
#   if (_MSC_VER >= 1928) && (_MSVC_LANG >= 201705) && !defined(__clang__)
#    define ASIO_HAS_STD_COROUTINE 1
#   endif // (_MSC_VER >= 1928) && (_MSVC_LANG >= 201705) && !defined(__clang__)
#  elif defined(__clang__)
#   if (__clang_major__ >= 14)
#    if (__cplusplus >= 202002) && (__cpp_impl_coroutine >= 201902)
#     if __has_include(<coroutine>)
#      define ASIO_HAS_STD_COROUTINE 1
#     endif // __has_include(<coroutine>)
#    endif // (__cplusplus >= 202002) && (__cpp_impl_coroutine >= 201902)
#   endif // (__clang_major__ >= 14)
#  elif defined(__GNUC__)
#   if (__cplusplus >= 201709) && (__cpp_impl_coroutine >= 201902)
#    if __has_include(<coroutine>)
#     define ASIO_HAS_STD_COROUTINE 1
#    endif // __has_include(<coroutine>)
#   endif // (__cplusplus >= 201709) && (__cpp_impl_coroutine >= 201902)
#  endif // defined(__GNUC__)
# endif // !defined(ASIO_DISABLE_STD_COROUTINE)
#endif // !defined(ASIO_HAS_STD_COROUTINE)


// Compiler support for the the [[deprecated(msg)]] attribute.
#if !defined(ASIO_DEPRECATED_MSG)
# if !defined(ASIO_DISABLE_DEPRECATED_MSG)
#  if defined(ASIO_MSVC) && (ASIO_MSVC >= 1400)
#   define ASIO_DEPRECATED_MSG(msg) __declspec(deprecated(msg))
#  elif (__cplusplus >= 201402)
#   if defined(__has_cpp_attribute)
#    if __has_cpp_attribute(deprecated)
#     define ASIO_DEPRECATED_MSG(msg) [[deprecated(msg)]]
#    endif // __has_cpp_attribute(deprecated)
#   endif // defined(__has_cpp_attribute)
#  endif // __cplusplus >= 201402
# endif // !defined(ASIO_DISABLE_DEPRECATED_MSG)
#endif // !defined(ASIO_DEPRECATED_MSG)
#if !defined(ASIO_DEPRECATED_MSG)
# define ASIO_DEPRECATED_MSG(msg)
#endif // !defined(ASIO_DEPRECATED_MSG)

// Kernel support for MSG_NOSIGNAL.
#if !defined(ASIO_HAS_MSG_NOSIGNAL)
# if defined(__linux__) || defined(__NetBSD__)
#  define ASIO_HAS_MSG_NOSIGNAL 1
# elif defined(_POSIX_VERSION)
#  if (_POSIX_VERSION >= 200809L)
#   define ASIO_HAS_MSG_NOSIGNAL 1
#  endif // _POSIX_VERSION >= 200809L
# endif // defined(_POSIX_VERSION)
#endif // !defined(ASIO_HAS_MSG_NOSIGNAL)

// Standard library support for std::to_address.
#if !defined(ASIO_HAS_STD_TO_ADDRESS)
# if !defined(ASIO_DISABLE_STD_TO_ADDRESS)
#  if defined(__clang__)
#   if (__cplusplus >= 202002)
#    define ASIO_HAS_STD_TO_ADDRESS 1
#   endif // (__cplusplus >= 202002)
#  elif defined(__GNUC__)
#   if (__GNUC__ >= 8)
#    if (__cplusplus >= 202002)
#     define ASIO_HAS_STD_TO_ADDRESS 1
#    endif // (__cplusplus >= 202002)
#   endif // (__GNUC__ >= 8)
#  endif // defined(__GNUC__)
#  if defined(ASIO_MSVC)
#   if (_MSC_VER >= 1922) && (_MSVC_LANG >= 202002)
#    define ASIO_HAS_STD_TO_ADDRESS 1
#   endif // (_MSC_VER >= 1922) && (_MSVC_LANG >= 202002)
#  endif // defined(ASIO_MSVC)
# endif // !defined(ASIO_DISABLE_STD_TO_ADDRESS)
#endif // !defined(ASIO_HAS_STD_TO_ADDRESS)

// Standard library support for snprintf.
#if !defined(ASIO_HAS_SNPRINTF)
# if !defined(ASIO_DISABLE_SNPRINTF)
#  if defined(__APPLE__)
#   define ASIO_HAS_SNPRINTF 1
#  endif // defined(__APPLE__)
# endif // !defined(ASIO_DISABLE_SNPRINTF)
#endif // !defined(ASIO_HAS_SNPRINTF)

// Standard library support for std::atomic<T>::wait and notify functions.
// By default, this is only enabled on platforms where the standard library is
// known to implement them using efficient wait primitives (e.g. Linux futex,
// Windows WaitOnAddress, Apple ulock).
#if !defined(ASIO_HAS_STD_ATOMIC_WAIT)
# if !defined(ASIO_DISABLE_STD_ATOMIC_WAIT)
#   if defined(ASIO_MSVC)
#    if (_MSVC_LANG >= 202002) && (__cpp_lib_atomic_wait >= 201907L)
#     if defined(ASIO_WINDOWS)
#      if !defined(UNDER_CE)
#       if defined(_WIN32_WINNT) && (_WIN32_WINNT >= 0x0602)
#        define ASIO_HAS_STD_ATOMIC_WAIT 1
#       endif // defined(_WIN32_WINNT) && (_WIN32_WINNT >= 0x0602)
#      endif // !defined(UNDER_CE)
#     endif // defined(ASIO_WINDOWS)
#    endif // (_MSVC_LANG >= 202002) && (__cpp_lib_atomic_wait >= 201907L)
#   elif (__cplusplus >= 202002L) && (__cpp_lib_atomic_wait >= 201907L)
#    if defined(__linux__)
#     define ASIO_HAS_STD_ATOMIC_WAIT 1
#    elif defined(__APPLE__)
#     if defined(__MAC_OS_X_VERSION_MIN_REQUIRED) \
        && (__MAC_OS_X_VERSION_MIN_REQUIRED >= 140400)
#      define ASIO_HAS_STD_ATOMIC_WAIT 1
#     elif defined(__IPHONE_OS_VERSION_MIN_REQUIRED) \
        && (__IPHONE_OS_VERSION_MIN_REQUIRED >= 170400)
#      define ASIO_HAS_STD_ATOMIC_WAIT 1
#     endif // defined(__IPHONE_OS_VERSION_MIN_REQUIRED)
//   && (__IPHONE_OS_VERSION_MIN_REQUIRED >= 170400)
#    endif // defined(__APPLE__)
#   endif // (__cplusplus >= 202002L) && (__cpp_lib_atomic_wait >= 201907L)
# endif // !defined(ASIO_DISABLE_STD_ATOMIC_WAIT)
#endif // !defined(ASIO_HAS_STD_ATOMIC_WAIT)
#if defined(ASIO_HAS_STD_ATOMIC_WAIT)
# define ASIO_VERSION_TAG_q q
#else // defined(ASIO_HAS_STD_ATOMIC_WAIT)
# define ASIO_VERSION_TAG_q
#endif // defined(ASIO_HAS_STD_ATOMIC_WAIT)

// Token-pasting helper (two levels needed to allow macro arguments to expand).
#define ASIO_DETAIL_CAT_(a, b) a ## b
#define ASIO_DETAIL_CAT(a, b) ASIO_DETAIL_CAT_(a, b)

// Version tags for user-enabled features with no auto-detection in this file.
#if defined(ASIO_ENABLE_HANDLER_TRACKING)
# define ASIO_VERSION_TAG_r r
#else // defined(ASIO_ENABLE_HANDLER_TRACKING)
# define ASIO_VERSION_TAG_r
#endif // defined(ASIO_ENABLE_HANDLER_TRACKING)

# define XIO_VERSIONED_NAME(name) xio_ ## name


#endif // ASIO_DETAIL_CONFIG_HPP
