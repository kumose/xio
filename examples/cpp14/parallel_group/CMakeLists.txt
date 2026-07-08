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
kmcmake_cc_binary(
        NAME cpp14_parallel_group_parallel_sort
        SOURCES
        parallel_sort.cpp
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
        INCLUDES
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CPP14_EXAMPLE_ROOT}
        LINKS xio::xio_static ${KMCMAKE_DEPS_LINK}
)


kmcmake_cc_binary(
        NAME cpp14_parallel_group_ranged_wait_for_all
        SOURCES
        ranged_wait_for_all.cpp
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
        INCLUDES
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CPP14_EXAMPLE_ROOT}
        LINKS xio::xio_static ${KMCMAKE_DEPS_LINK}
)


kmcmake_cc_binary(
        NAME cpp14_parallel_group_wait_for_all
        SOURCES
        wait_for_all.cpp
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
        INCLUDES
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CPP14_EXAMPLE_ROOT}
        LINKS xio::xio_static ${KMCMAKE_DEPS_LINK}
)


kmcmake_cc_binary(
        NAME cpp14_parallel_group_wait_for_one
        SOURCES
        wait_for_one.cpp
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
        INCLUDES
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CPP14_EXAMPLE_ROOT}
        LINKS xio::xio_static ${KMCMAKE_DEPS_LINK}
)


kmcmake_cc_binary(
        NAME cpp14_parallel_group_wait_for_one_error
        SOURCES
        wait_for_one_error.cpp
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
        INCLUDES
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CPP14_EXAMPLE_ROOT}
        LINKS xio::xio_static ${KMCMAKE_DEPS_LINK}
)


kmcmake_cc_binary(
        NAME cpp14_parallel_group_wait_for_one_success
        SOURCES
        wait_for_one_success.cpp
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
        INCLUDES
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CPP14_EXAMPLE_ROOT}
        LINKS xio::xio_static ${KMCMAKE_DEPS_LINK}
)

