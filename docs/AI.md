# xio AI Context

This file describes the xio project structure and conventions for AI assistants.

## AI Constraints

**Before modifying any file:**
1. Read the file's full contents first
2. Propose the exact change (what + why) in text
3. Wait for user approval — do not edit until user says "go ahead" / "do it" / "改" / "干"

**Before running any git mutating command** (commit, push, rebase, merge, reset, etc.):
1. State the exact command you intend to run
2. Wait for user approval

**Other restrictions:**
- Do not create new files unless the task explicitly requires it; prefer editing existing files
- Do not list directory trees (`ls`, `tree`) or glob files unless necessary for the task at hand
- Do not run `cmake --build` or `ctest` unless explicitly asked to verify a change
- Do not read files you already have context for — reuse what you've been given. Only re-read when the content has likely changed (e.g. after an edit you just made).
- Do not suggest adding new dependencies, libraries, or external tools without asking first
- Do not change `kmcmake/` framework files unless the task specifically targets them
- Do not reformat, restyle, or rename variables/functions as side work — stick to the task
- Batch independent operations (reads, searches, parallel commands) to reduce round-trips
- **Distinguish questions from tasks.** If the user asks "Is X appropriate?" / "合适吗" / "应该...吧", they are seeking an opinion, not requesting action. Answer concisely and stop. Do not propose changes, do not offer to implement, do not touch files — unless the user explicitly follows up with "do it" / "改" / "干".

## AI Workflow

When the user gives a task, follow this sequence:

1. **Diagnose** — explain the problem and propose a solution
2. **Wait** — do not touch files until the user approves
3. **Execute** — only after the user says "go ahead" / "do it" / "改" / "干"

## Project Overview

xio is a C++ asynchronous I/O and networking library derived from Boost.Asio, with integrated Raft consensus support.

Key features:
- **Async networking** — TCP/UDP sockets, acceptors, resolvers, local sockets, SSL (OpenSSL)
- **Async I/O** — timers, signals, files, pipes, serial ports
- **Modern C++** — completion tokens, `co_await` / `co_spawn` (C++20), thread pools, strands
- **Raft consensus** — leader election, log replication, snapshots, custom state machines (`xio/raft/`)
- **Linux io_uring** — optional (`XIO_ENABLE_URING=ON`)
- **Build system** — kmcmake + kmpkg

## Directory Layout

```
xio/
├── CMakeLists.txt              # Entry: includes kmcmake_module + user cmake/
├── CMakePresets.json
├── docs/
│   └── AI.md                   # AI context (this file)
├── kmcmake/                    # Framework layer — DO NOT MODIFY
│   ├── kmcmake_module.cmake
│   ├── kmcmake_option.cmake
│   ├── arch/                   # Per-CPU SIMD detection + level
│   └── tools/                  # Build functions (library, test, etc.)
├── cmake/                      # User config layer — MODIFY FREELY
│   ├── xio_user_option.cmake
│   ├── xio_deps.cmake
│   ├── xio_cxx_config.cmake
│   ├── xio_cpack_config.cmake
│   └── xio_config.cmake.in
├── xio/                        # Source code
│   ├── CMakeLists.txt
│   ├── *.h / *.cc
│   ├── version.h (generated from .in)
│   ├── skills.h (this file)
│   └── raft/                   # Raft consensus module
├── tests/
├── benchmark/
├── examples/
└── refs/
```

## Build Flow

1. `project()` sets project name and version
2. `include(kmcmake_module)` loads framework
3. `include(xio_user_option OPTIONAL)` — user overrides
4. `include(xio_deps)` — find_package for xlog, OpenSSL, Threads, optionally liburing
5. `include(xio_cxx_config)` — sets `KMCMAKE_CXX_OPTIONS`
6. `configure_file(xio/version.h.in)` — generates version.h
7. `add_subdirectory(xio)` — builds main library
8. `add_subdirectory(tests)` — if KMCMAKE_BUILD_TEST=ON
9. `add_subdirectory(benchmark)` — if KMCMAKE_BUILD_BENCHMARK=ON
10. `add_subdirectory(examples)` — if KMCMAKE_BUILD_EXAMPLES=ON

## Dependencies

| Library | Required | Notes |
|---------|----------|-------|
| xlog | Yes | Kumo logging (find_package) |
| OpenSSL | Yes | TLS/SSL (find_package CONFIG) |
| Threads | Yes | System pthreads |
| liburing | No | Linux-only, XIO_ENABLE_URING=ON |

## Version Header Macros

Generated `xio/version.h` provides:

| Macro | Description |
|-------|-------------|
| `XIO_VERSION_MAJOR/MINOR/PATCH` | Version components |
| `XIO_VERSION_STRING` | e.g. "0.0.5" |
| `XIO_SIMD_LEVEL` | Target SIMD level string |
| `XIO_SIMD_ENABLE_SSE..AVX512F` | 0/1 per feature |
| `XIO_CXX_COMPILER_ID` | Compiler name |
| `XIO_CXX_COMPILER_VERSION` | Compiler version |
| `XIO_BUILD_TYPE_STRING` | Debug/Release/etc |
| `XIO_GIT_COMMIT_HASH` | Full git commit |
| `XIO_GIT_VERSION_STRING` | "tag-hash[-dirty]" |
| `XIO_ENABLE_URING` | Defined if io_uring enabled |

## Key Conventions

- **DO NOT** modify files under `kmcmake/` — framework files, replaced on upgrade
- **DO** modify files under `cmake/` — project user configuration
- Public API lives in `namespace xio` in headers under `xio/`
- Raft module lives in `xio/raft/`
- Read `xio/skills.h` for a concise summary of the public API

## CI

See `.github/workflows/ci.yml` for the CI pipeline. Uses kmpkg for dependency management across Linux/macOS/Windows.
