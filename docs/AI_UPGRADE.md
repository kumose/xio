# kmcmake Upgrade Guide (for AI Agents)

This document describes how to upgrade an old-format kmcmake project to the
current framework structure. AI agents should read this when tasked with
migrating a legacy kmcmake project.

## Overview

The old kmcmake (pre-v1) had a flat structure with inline SIMD detection,
thick user config files, and no clear separation between framework and user
code. The current version introduces:

- Strict `kmcmake/` (framework) vs `cmake/` (user config) layer separation
- Per-architecture SIMD detection under `kmcmake/arch/`
- A dedicated `kmcmake_compiler_flags.cmake` module
- Thin `<project>_cxx_config.cmake` that just aggregates
- `KMCMAKE_CXX_OPTIONS` as the single aggregated flag variable
- `KMCMAKE_ARCH_ENABLE_*` exported as 0/1 integers for version.h
- `skills.h` for AI-readable project summaries
- `docs/AI.md` for AI context and API reference

## What Changed

| Old | New | Reason |
|-----|-----|--------|
| SIMD detection inline in `kmcmake_module.cmake` | `kmcmake/arch/kmcmake_arch.cmake` + per-arch detect/level | Architecture isolation, maintainability |
| `kmcmake/tools/simd_detect.cmake` | (deleted) | Replaced by arch subsystem |
| Compiler flags in `cmake/changeme_cxx_config.cmake` | `kmcmake/tools/kmcmake_compiler_flags.cmake` | Framework ownership, reusability |
| `kmcmake_apply_runtime_simd()` inline | (removed) | SIMD flags now set by arch level files |
| `KMCMAKE_SIMD_LEVEL_*_VAL` compat macros | (removed) | Use `KMCMAKE_ARCH_ENABLE_*` (0/1) instead |
| `cmake/changeme_test.cmake` | (deleted) | Test config folded into main flow |
| `USER_CXX_FLAGS` | `KMCMAKE_CXX_OPTIONS` | Standardized variable name |
| Mixed `COPTS` / `CXXOPTS` usage | Use `CXXOPTS ${KMCMAKE_CXX_OPTIONS}` | Consistent |
| `KMCMAKE_ARCH_DEFS` | (removed) | Never populated |

## Migration Steps

### Step 1: Separate framework from user config

Ensure `kmcmake/` contains only framework files (upgrade-safe) and `cmake/`
contains user modifications:

```
project/
├── kmcmake/                    # Framework — DO NOT MODIFY after migration
│   ├── kmcmake_module.cmake
│   ├── kmcmake_option.cmake
│   ├── arch/
│   │   ├── kmcmake_arch.cmake
│   │   ├── x86/kmcmake_arch_detect.cmake
│   │   ├── x86/kmcmake_arch_level.cmake
│   │   ├── arm/kmcmake_arch_detect.cmake
│   │   └── arm/kmcmake_arch_level.cmake
│   └── tools/
│       ├── kmcmake_compiler_flags.cmake
│       ├── default_setting.cmake
│       ├── kmcmake_cc_library.cmake
│       ├── kmcmake_cc_test.cmake
│       ├── kmcmake_cc_binary.cmake
│       ├── kmcmake_cc_interface.cmake
│       ├── kmcmake_cc_object.cmake
│       ├── kmcmake_cc_benchmark.cmake
│       ├── kmcmake_cc_proto.cmake
│       ├── kmcmake_cc_proto_object.cmake
│       └── git_commit.cmake
└── cmake/                      # User config — MODIFY FREELY
    ├── <project>_user_option.cmake
    ├── <project>_deps.cmake
    ├── <project>_cxx_config.cmake
    ├── <project>_cpack_config.cmake
    └── <project>_config.cmake.in
```

### Step 2: Replace SIMD detection

Old (inline in `kmcmake_module.cmake` or `simd_detect.cmake`):
```cmake
# ~250 lines of inline CheckCXXSourceRuns for each feature,
# mixed with compiler ID checks and flag selection
```

New:
- `kmcmake/arch/kmcmake_arch.cmake` — entry point, routes by `CMAKE_SYSTEM_PROCESSOR`
- `kmcmake/arch/x86/kmcmake_arch_detect.cmake` — per-feature detection, exports `KMCMAKE_X86_HAS_*`
- `kmcmake/arch/x86/kmcmake_arch_level.cmake` — level-by-level enable, exports `KMCMAKE_ARCH_ENABLE_*`
- `kmcmake/arch/arm/kmcmake_arch_detect.cmake` — ARM detection
- `kmcmake/arch/arm/kmcmake_arch_level.cmake` — ARM level enable

Include in `kmcmake_module.cmake`:
```cmake
include(kmcmake_option)
include(default_setting)
# ...
include(kmcmake_cc_library)
include(kmcmake_cc_test)
include(kmcmake_cc_binary)
# etc.
```

SIMD is loaded on-demand via `<project>_cxx_config.cmake`:
```cmake
include(kmcmake_compiler_flags)     # → KMCMAKE_BASE_CXX_FLAGS
include(kmcmake_arch)               # → KMCMAKE_SIMD_CXX_FLAGS + KMCMAKE_ARCH_ENABLE_*
set(KMCMAKE_CXX_OPTIONS
    ${KMCMAKE_BASE_CXX_FLAGS}
    ${KMCMAKE_SIMD_CXX_FLAGS}
    ${KMCMAKE_RANDEN_FLAGS}
)
```

### Step 3: Update compiler flags

Old (`<project>_cxx_config.cmake`):
```cmake
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    set(MY_CXX_FLAGS "-Wall -Wextra ...")
elseif(...)
    # etc.
endif()
```

New: flags are in `kmcmake_compiler_flags.cmake`. The `<project>_cxx_config.cmake`
is a thin aggregator (≈30 lines).

### Step 4: Replace SIMD compat macros

Old `version.h.in`:
```c
#define KMCMAKE_SIMD_LEVEL_SSE_VAL    1
#define KMCMAKE_SIMD_LEVEL_AVX2_VAL   1
```

New `version.h.in`:
```c
#define @CHANGEME_UP@_SIMD_ENABLE_AVX2   @KMCMAKE_ARCH_ENABLE_AVX2@
```

The `KMCMAKE_ARCH_ENABLE_*` variables are converted from BOOL to 0/1 integers
by `_kmcmake_to_int()` in `kmcmake_arch.cmake`.

### Step 5: Add skills.h

Create `<project>/skills.h.in` with Doxygen `///` style project summary.
See `docs/AI.md` → `## skills.h Convention` for details.

### Step 6: Add AI.md

Copy `docs/AI.md` to the project root `docs/` directory. This file provides
AI agents with project context, API reference, and behavioral constraints.

### Step 7: Clean up variables

- `USER_CXX_FLAGS` → `KMCMAKE_CXX_OPTIONS`
- Legacy `KMCMAKE_SIMD_LEVEL_*_VAL` → `KMCMAKE_ARCH_ENABLE_*`
- Remove any `KMCMAKE_ARCH_DEFS` references
- Remove any `KMCMAKE_BASE_TEST_FLAGS` references (was never consumed)
- Remove commented-out `include(require_gtest)` etc. from `<project>_deps.cmake`
- Remove commented-out `include(<project>_test)` from `CMakeLists.txt`

## Detection Checklist

After migration, verify:

- [ ] `cmake -S . -B build` succeeds
- [ ] `KMCMAKE_CXX_OPTIONS` contains base flags + SIMD flags + randen flags
- [ ] `version.h` has `*_SIMD_ENABLE_*` as 0/1 integers
- [ ] `kmcmake/` contains only framework files
- [ ] `cmake/` contains only user config files
- [ ] No stale `simd_detect.cmake` references remain
- [ ] No stale `changeme_test.cmake` references remain
- [ ] No `USER_CXX_FLAGS` usage remains
- [ ] No `KMCMAKE_SIMD_LEVEL_*_VAL` usage remains
- [ ] `skills.h` exists and is readable
- [ ] `docs/AI.md` exists
