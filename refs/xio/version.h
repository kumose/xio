// Copyright (C) 2026 Kumo inc. and its affiliates. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#pragma once

// project name version
#define XIO_VERSION_MAJOR 0
#define XIO_VERSION_MINOR 0
#define XIO_VERSION_PATCH 5
#define XIO_VERSION ((PROJECT_VERSION_MAJOR * 1000) + PROJECT_VERSION_MINOR) * 1000 + PROJECT_VERSION_PATCH

#define XIO_VERSION_STRING "0.0.5"

// build system
#define XIO_BUILD_SYSTEM "ubuntu 20.04.6 lts"

// build system version
#define XIO_BUILD_SYSTEM_VERSION "20.04"


// feature flags
/* #undef XIO_ENABLE_URING */

// compiler gnu or clang
#define XIO_CXX_COMPILER_ID "GNU"

// compiler version
#define XIO_CXX_COMPILER_VERSION "11.5.0"

// cmake cxx compiler flags
#define XIO_CMAKE_CXX_COMPILER_FLAGS ""

// user defined cxx compiler flags
#define XIO_CXX_COMPILER_FLAGS "-Wall;-Wextra;-Wno-cast-qual;-Wconversion-null;-Wformat-security;-Woverlength-strings;-Wpointer-arith;-Wno-undef;-Wunused-local-typedefs;-Wunused-result;-Wvarargs;-Wno-attributes;-Wno-implicit-fallthrough;-Wno-unused-parameter;-Wno-unused-function;-Wwrite-strings;-Wclass-memaccess;-Wno-sign-compare;-DNOMINMAX;-msse;-msse2;-msse3;-mssse3;-msse4.1;-msse4;-msse4.2;-mavx;-mavx2;-mbmi;-mbmi2;-mfma;-mmovbe;-maes"

// cxx standard
#define XIO_CXX_STANDARD "17"

// build type
#define XIO_BUILD_TYPE_STRING "RELEASE"

// build type
#define XIO_BUILD_RELEASE

// build type
#if defined(XIO_BUILD_DEBUG)
    #define IS_XIO_BUILD_TYPE_DEBUG 1
#else
    #define IS_XIO_BUILD_TYPE_DEBUG 0
#endif

////////////////////////////////////////////////////////////////////////////////
/// simd region

#define KMCMAKE_SIMD_LEVEL_NONE 0
#define KMCMAKE_SIMD_LEVEL_SSE 1
#define KMCMAKE_SIMD_LEVEL_AVX 1
#define KMCMAKE_SIMD_LEVEL_AVX2 1
#define KMCMAKE_SIMD_LEVEL_BMI 1
#define KMCMAKE_SIMD_LEVEL_BMI2 1
#define KMCMAKE_SIMD_LEVEL_FMA 1
#define KMCMAKE_SIMD_LEVEL_MOVBE 1
#define KMCMAKE_RUNTIME_SIMD_LEVEL "AVX2"

///////////////////////////////////////////////////////////////////////////////////
/// git and version region

////////////////////////////////////////////////////////////////////////////////
/// Git Version Information
////////////////////////////////////////////////////////////////////////////////
// Full Git commit hash (e.g., "a1b2c3d4e5f67890abcdef1234567890abcdef12")
#define XIO_GIT_COMMIT_HASH "1344b33fe772c4f8a955cda883ef7583efa93543"
// Short Git commit hash (e.g., "a1b2c3d")
#define XIO_GIT_COMMIT_SHORT_HASH "1344b33"
// Git dirty flag (0 = clean working tree, 1 = uncommitted changes)
#define XIO_GIT_IS_DIRTY 1
// Combined Git version string (e.g., "0.6.0-a1b2c3d" or "0.6.0-a1b2c3d-dirty")
#define XIO_GIT_VERSION_STRING "-1344b33-dirty"
