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

configure_file(${PROJECT_SOURCE_DIR}/benchmark/config.h.in ${PROJECT_SOURCE_DIR}/benchmark/config.h @ONLY)
#[[
find_package(benchmark REQUIRED)

kmcmake_cc_bm(
        NAME timer_bench
        MODULE timer
        SOURCES
        timer_bench.cc
        CXXOPTS
        ${KMCMAKE_CXX_OPTIONS}
        LINKS
        xio::xio
        benchmark::benchmark_main
        benchmark::benchmark
        ${KMCMAKE_DEPS_LINK}
)

kmcmake_cc_bm(
        NAME event_timer_bench
        MODULE event
        SOURCES
        event_timer_bench.cc
        CXXOPTS
        ${KMCMAKE_CXX_OPTIONS}
        LINKS
        xio::xio
        benchmark::benchmark_main
        benchmark::benchmark
        ${KMCMAKE_DEPS_LINK}
)
]]
add_subdirectory(raft)
