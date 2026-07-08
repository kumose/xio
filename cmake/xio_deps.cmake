# Copyright (C) Kumo inc. and its affiliates.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
############################################################
# system pthread and rt, dl
############################################################
set(KMCMAKE_CONFIG_PRIVATE_FIND_SNIPPETS "")

# Record a private find_package(...) call for generated <Project>Config.cmake.
# Usage:
#   kmcmake_private_find_package(ZLIB REQUIRED)
function(kmcmake_private_find_package)
    string(JOIN " " _KMCMAKE_PRIVATE_FIND_PACKAGE_ARGS ${ARGV})
    string(APPEND KMCMAKE_CONFIG_PRIVATE_FIND_SNIPPETS
            "find_dependency(${_KMCMAKE_PRIVATE_FIND_PACKAGE_ARGS})\n")
    set(KMCMAKE_CONFIG_PRIVATE_FIND_SNIPPETS "${KMCMAKE_CONFIG_PRIVATE_FIND_SNIPPETS}" PARENT_SCOPE)
endfunction()

# Record a private find_library(...) call for generated <Project>Config.cmake.
# Usage:
#   kmcmake_private_find_library(MYLIB NAMES mylib PATHS /opt/mylib/lib)
function(kmcmake_private_find_library)
    string(JOIN " " _KMCMAKE_PRIVATE_FIND_LIBRARY_ARGS ${ARGV})
    string(APPEND KMCMAKE_CONFIG_PRIVATE_FIND_SNIPPETS
            "find_library(${_KMCMAKE_PRIVATE_FIND_LIBRARY_ARGS})\n")
    set(KMCMAKE_CONFIG_PRIVATE_FIND_SNIPPETS "${KMCMAKE_CONFIG_PRIVATE_FIND_SNIPPETS}" PARENT_SCOPE)
endfunction()

set(KMCMAKE_SYSTEM_DYLINK)
if (APPLE)
    find_library(CoreFoundation CoreFoundation)
    list(APPEND KMCMAKE_SYSTEM_DYLINK ${CoreFoundation})
elseif (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    list(APPEND KMCMAKE_SYSTEM_DYLINK rt dl)
endif ()

if (KMCMAKE_BUILD_TEST)
    enable_testing()
endif (KMCMAKE_BUILD_TEST)

if (KMCMAKE_BUILD_BENCHMARK)
endif ()

find_package(Threads REQUIRED)
kmcmake_private_find_package(Threads REQUIRED)
list(APPEND KMCMAKE_SYSTEM_DYLINK Threads::Threads)

find_package(xlog REQUIRED)
find_package(ZLIB REQUIRED)
find_package(lz4 CONFIG REQUIRED)
find_package(Snappy CONFIG REQUIRED)
find_package(zstd CONFIG REQUIRED)
find_package(OpenSSL REQUIRED CONFIG)
kmcmake_private_find_package(OpenSSL REQUIRED CONFIG)

find_path(XLOG_DIR xlog/logging.h)
include_directories(${XLOG_DIR})
if (XIO_ENABLE_URING)
    if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
        find_path(URING_INCLUDE_DIR NAMES liburing.h)
        find_library(URING_LIBRARY NAMES uring)
        if (NOT URING_INCLUDE_DIR OR NOT URING_LIBRARY)
            message(FATAL_ERROR "XIO_ENABLE_URING is ON but liburing was not found. "
                    "Install liburing-dev (or similar) and ensure liburing.h and liburing.so are in standard paths.")
        endif ()
    else ()
        message(FATAL_ERROR "XIO_ENABLE_URING is ON but io_uring is only supported on Linux.")
    endif ()
else ()
    set(URING_LIBRARY)
endif ()

############################################################
#
# add you libs to the KMCMAKE_DEPS_LINK variable eg as turbo
# so you can and system pthread and rt, dl already add to
# KMCMAKE_SYSTEM_DYLINK, using it for fun.
##########################################################
set(KMCMAKE_DEPS_LINK
        OpenSSL::SSL
        OpenSSL::Crypto
        ${URING_LIBRARY}
        xlog::xlog_static
        ZLIB::ZLIB
        lz4::lz4
        Snappy::snappy
        zstd::libzstd
        ${KMCMAKE_SYSTEM_DYLINK}
        )
list(REMOVE_DUPLICATES KMCMAKE_DEPS_LINK)
kmcmake_print_list_label("Denpendcies:" KMCMAKE_DEPS_LINK)

############################################################
# for binary
############################################################
set(KMCMAKE_STATIC_BIN_OPTION)
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    list(APPEND KMCMAKE_STATIC_BIN_OPTION -static-libgcc -static-libstdc++)
endif ()
