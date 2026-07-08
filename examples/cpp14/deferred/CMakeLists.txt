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
        NAME cpp14_deferred_deferred_1
        SOURCES
        deferred_1.cpp
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
        INCLUDES
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CPP14_EXAMPLE_ROOT}
        LINKS xio::xio_static ${KMCMAKE_DEPS_LINK}
)


kmcmake_cc_binary(
        NAME cpp14_deferred_deferred_2
        SOURCES
        deferred_2.cpp
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
        INCLUDES
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CPP14_EXAMPLE_ROOT}
        LINKS xio::xio_static ${KMCMAKE_DEPS_LINK}
)


kmcmake_cc_binary(
        NAME cpp14_deferred_deferred_3
        SOURCES
        deferred_3.cpp
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
        INCLUDES
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CPP14_EXAMPLE_ROOT}
        LINKS xio::xio_static ${KMCMAKE_DEPS_LINK}
)


kmcmake_cc_binary(
        NAME cpp14_deferred_deferred_4
        SOURCES
        deferred_4.cpp
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
        INCLUDES
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CPP14_EXAMPLE_ROOT}
        LINKS xio::xio_static ${KMCMAKE_DEPS_LINK}
)


kmcmake_cc_binary(
        NAME cpp14_deferred_deferred_5
        SOURCES
        deferred_5.cpp
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
        INCLUDES
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CPP14_EXAMPLE_ROOT}
        LINKS xio::xio_static ${KMCMAKE_DEPS_LINK}
)


kmcmake_cc_binary(
        NAME cpp14_deferred_deferred_6
        SOURCES
        deferred_6.cpp
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
        INCLUDES
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CPP14_EXAMPLE_ROOT}
        LINKS xio::xio_static ${KMCMAKE_DEPS_LINK}
)


kmcmake_cc_binary(
        NAME cpp14_deferred_deferred_7
        SOURCES
        deferred_7.cpp
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
        INCLUDES
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CPP14_EXAMPLE_ROOT}
        LINKS xio::xio_static ${KMCMAKE_DEPS_LINK}
)

