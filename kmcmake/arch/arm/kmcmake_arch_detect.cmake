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
# AI: ARM/AArch64 SIMD feature detection.
# AI: Exports one variable per feature. All start with KMCMAKE_ARM_HAS_.
# AI:
# AI: Output variables (BOOL):
# AI:   KMCMAKE_ARM_HAS_NEON
# AI:   KMCMAKE_ARM_HAS_VFPv4
# AI:   KMCMAKE_ARM_HAS_FMA
# AI:
# AI: Also exports flag variables:
# AI:   NEON_FLAG, VFPv4_FLAG, FMA_FLAG
# AI:
# AI: Prerequisites:
# AI:   include(CheckCXXSourceRuns)

set(KMCMAKE_ARM_HAS_NEON FALSE)
set(KMCMAKE_ARM_HAS_VFPv4 FALSE)
set(KMCMAKE_ARM_HAS_FMA FALSE)

# AI: On AArch64 (ARM64), NEON/FMA are part of the base ISA — no -mfpu flags needed.
# AI: On ARM32, -mfpu flags are required for detection and compilation.
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(NEON_FLAG  "")
    set(VFPv4_FLAG "")
    set(FMA_FLAG   "")
else()
    set(NEON_FLAG  "-mfpu=neon")
    set(VFPv4_FLAG "-mfpu=vfpv4")
    set(FMA_FLAG   "-mfpu=neon -mfma")
endif()

include(CheckCXXSourceRuns)

# NEON check
set(NEON_CODE "
#include <arm_neon.h>
int main() { float32x4_t a = vdupq_n_f32(0.0f); return 0; }")
check_cxx_source_runs("${NEON_CODE}" _KMCMAKE_ARM_NEON_OK COMPILE_FLAGS "${NEON_FLAG}")
if(_KMCMAKE_ARM_NEON_OK)
    set(KMCMAKE_ARM_HAS_NEON TRUE)
endif()

# VFPv4 check
set(VFPv4_CODE "
#if defined(__ARM_FP) && (__ARM_FP & 0x8)
#include <arm_math.h>
#endif
int main() { float a = 0.0f; return 0; }")
check_cxx_source_runs("${VFPv4_CODE}" _KMCMAKE_ARM_VFPv4_OK COMPILE_FLAGS "${VFPv4_FLAG}")
if(_KMCMAKE_ARM_VFPv4_OK)
    set(KMCMAKE_ARM_HAS_VFPv4 TRUE)
endif()

# FMA check
set(FMA_CODE "
#if defined(__ARM_FEATURE_FMA)
#include <arm_neon.h>
#endif
int main() { float32x4_t a = vdupq_n_f32(1.0f); float32x4_t b = vdupq_n_f32(2.0f); float32x4_t c = vfmaq_f32(a, b, a); return 0; }")
check_cxx_source_runs("${FMA_CODE}" _KMCMAKE_ARM_FMA_OK COMPILE_FLAGS "${FMA_FLAG}")
if(_KMCMAKE_ARM_FMA_OK)
    set(KMCMAKE_ARM_HAS_FMA TRUE)
endif()

kmcmake_print("ARM SIMD detection complete")
kmcmake_print_label("ARM HAS_NEON"  ${KMCMAKE_ARM_HAS_NEON})
kmcmake_print_label("ARM HAS_VFPv4" ${KMCMAKE_ARM_HAS_VFPv4})
kmcmake_print_label("ARM HAS_FMA"   ${KMCMAKE_ARM_HAS_FMA})