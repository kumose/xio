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
// AI: This file is a human/AI-readable summary of xio's public API and
// AI: design principles. AI agents should read this instead of scanning all
// AI: source files. For dependency libraries, read their skills.h similarly.
//
// AI: Style: triple-slash Doxygen comments for all entries.

#pragma once

/// @defgroup project_summary Project Summary
/// @brief C++ asynchronous I/O and networking library, derived from Boost.Asio.
///
/// xio provides the familiar async model (io_context, completion tokens,
/// coroutines, SSL, timers) in the xio namespace, with integrated Raft
/// consensus support via NuRaft on top of the xio transport layer.
/// @{

/// @brief Project name
/// AI: xio

/// @brief Build system
/// AI: Uses kmcmake (https://github.com/kumose/kmcmake) as the CMake framework.
/// AI: Framework modules live under kmcmake/ — DO NOT MODIFY.
/// AI: User configuration lives under cmake/ — MODIFY FREELY.

/// @brief Key design goals
/// AI: - Asio-compatible async model (io_context, completion tokens, strands)
/// AI: - C++20 coroutine support via co_spawn / awaitable
/// AI: - Integrated Raft consensus (leader election, log replication, snapshots)
/// AI: - Optional Linux io_uring backend (XIO_ENABLE_URING)
/// AI: - Automatic SIMD detection and level control via kmcmake/arch/
/// AI: - Versioned framework (kmcmake/) that can be replaced on upgrade

/// @brief Directory layout
/// AI: .
/// AI: ├── CMakeLists.txt              # Entry point
/// AI: ├── cmake/                      # User configuration (modifiable)
/// AI: │   ├── xio_user_option.cmake   # User overrides
/// AI: │   ├── xio_deps.cmake          # Dependencies (xlog, OpenSSL, uring)
/// AI: │   ├── xio_cxx_config.cmake    # C++ flags aggregation
/// AI: │   ├── xio_cpack_config.cmake  # Packaging config
/// AI: │   └── xio_config.cmake.in     # CMake export template
/// AI: ├── kmcmake/                    # Framework (update-safe)
/// AI: │   ├── kmcmake_module.cmake
/// AI: │   ├── kmcmake_option.cmake
/// AI: │   ├── arch/                   # Per-CPU SIMD detection + level
/// AI: │   └── tools/                  # Build functions
/// AI: ├── xio/                        # Source code
/// AI: │   ├── CMakeLists.txt
/// AI: │   ├── *.h / *.cc
/// AI: │   ├── version.h (generated)
/// AI: │   ├── skills.h (this file)
/// AI: │   └── raft/                   # Raft consensus module
/// AI: ├── tests/
/// AI: ├── benchmark/
/// AI: └── examples/

/// @brief Build flow
/// AI: 1. project() sets name + version
/// AI: 2. include(kmcmake_module) loads all framework modules
/// AI: 3. include(xio_user_option OPTIONAL) — user overrides
/// AI: 4. include(xio_deps) — find_package xlog OpenSSL Threads uring
/// AI: 5. include(xio_cxx_config) — sets KMCMAKE_CXX_OPTIONS
/// AI: 6. configure_file(xio/version.h.in) — generates version.h
/// AI: 7. add_subdirectory(xio) — builds main library
/// AI: 8. add_subdirectory(tests) / benchmark / examples — optional
/// @}

/// @defgroup deps Dependencies
/// AI: Runtime: xlog (logging), OpenSSL (TLS/SSL), Threads.
/// AI: Optional: liburing (Linux, XIO_ENABLE_URING).
/// AI: Build-only: GoogleTest, benchmark.
/// @}

/// @defgroup core_api Core API (namespace xio)
/// @brief xio derives from Boost.Asio. The public API follows Asio's
///        async model. Key types and headers:
/// @{

/// @brief io_context              — event loop (xio/io_context.h)
/// @brief executor / executor_work — execution context (xio/executor*.h)
/// @brief awaitable               — C++20 coroutine return type (xio/awaitable.h)
/// @brief co_spawn                — launch a coroutine (xio/co_spawn.h)
/// @brief steady_timer            — deadline timer (xio/steady_timer.h)
/// @brief system_timer            — wall-clock timer (xio/system_timer.h)
/// @brief ip::tcp::socket         — TCP socket (xio/ip/tcp.h)
/// @brief ip::udp::socket         — UDP socket (xio/ip/udp.h)
/// @brief ip::tcp::acceptor       — TCP acceptor (xio/ip/tcp.h)
/// @brief ip::tcp::resolver       — DNS resolver (xio/ip/tcp.h)
/// @brief local::stream_protocol  — Unix domain sockets (xio/local/)
/// @brief ssl::stream             — SSL/TLS stream (xio/ssl/stream.h)
/// @brief signal_set              — POSIX signal handling (xio/signal_set.h)
/// @brief posix::stream_descriptor — arbitrary fd I/O (xio/posix/stream_descriptor.h)
/// @brief streambuf               — buffer for async read/write (xio/streambuf.h)
/// @brief mutable_buffer / const_buffer — buffer abstractions (xio/buffer*.h)
/// @brief serial_port             — serial port I/O (xio/serial_port.h)
/// @brief readable_pipe / writable_pipe — pipe I/O (xio/readable_pipe.h)
/// @brief static_thread_pool      — fixed-size thread pool (xio/static_thread_pool.h)
/// @brief post / dispatch / defer  — executor free functions (xio/post.h etc.)
/// @brief bind_executor / bind_cancellation_slot — token adaptors
/// @brief any_completion_handler  — type-erased completion handler (xio/any_completion_handler.h)
/// @brief as_tuple                — completion token adaptor (xio/as_tuple.h)
/// @brief compose                 — composed operation helper (xio/compose.h)
/// @brief cancellable / cancellation_signal / cancellation_slot — cancellation (xio/cancellation_*.h)
/// @}

/// @defgroup raft_api Raft Consensus (namespace xio::raft)
/// @brief Raft is built on NuRaft integrated with xio transport.
/// @{
/// @brief raft::server            — Raft server node
/// @brief raft::client            — Raft client for submitting commands
/// @brief raft::state_machine     — user-defined state machine interface
/// @brief raft::logger            — Raft log storage interface
/// @brief raft::snapshot           — snapshot support
/// @brief raft::rpc               — RPC transport (built on xio sockets)
/// @brief raft::config            — cluster configuration
/// @}

/// @defgroup config_vars Configuration Variables
/// AI: Output variables available after include(xio_cxx_config)
/// @{
/// @brief KMCMAKE_BASE_CXX_FLAGS  — compiler flags determined by compiler ID
/// @brief KMCMAKE_SIMD_CXX_FLAGS  — SIMD flags from arch detection
/// @brief KMCMAKE_RANDEN_FLAGS    — AES/hardware random flags
/// @brief KMCMAKE_CXX_OPTIONS     — aggregated flags (BASE + SIMD + RANDEN)
/// @brief KMCMAKE_ARCH_ENABLE_*   — per-feature SIMD enable as 0/1 integers
/// @}

/// @defgroup version_header Generated version.h Macros
/// @{
/// @brief XIO_VERSION_MAJOR / _MINOR / _PATCH  — version components
/// @brief XIO_VERSION_STRING    — e.g. "0.0.5"
/// @brief XIO_SIMD_LEVEL        — target SIMD level string
/// @brief XIO_SIMD_ENABLE_SSE..AVX512F  — 0 or 1 per feature
/// @brief XIO_CXX_COMPILER_ID     — compiler name
/// @brief XIO_CXX_COMPILER_VERSION — compiler version
/// @brief XIO_BUILD_TYPE_STRING     — Debug/Release/etc
/// @brief XIO_BUILD_SYSTEM        — OS distro name
/// @brief XIO_GIT_COMMIT_HASH     — full git commit
/// @brief XIO_GIT_VERSION_STRING  — "tag-hash[-dirty]"
/// @brief XIO_ENABLE_URING         — defined if io_uring enabled
/// @}

// AI: End of skills.h — AI assistants should prioritize this file and docs/AI.md
// AI: over reading all source files. For third-party dependencies (xlog, etc.),
// AI: look for their skills.h at <dep>/include/<dep>/skills.h or similar location.
