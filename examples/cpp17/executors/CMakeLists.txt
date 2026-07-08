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
        NAME cpp17_executors_actor
        SOURCES
        actor.cpp
        CXXOPTS ${KMCMAKE_CXX_OPTIONS} -std=c++17
        INCLUDES
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CPP17_EXAMPLE_ROOT}
        LINKS xio::xio_static ${KMCMAKE_DEPS_LINK}
)


kmcmake_cc_binary(
        NAME cpp17_executors_bank_account_1
        SOURCES
        bank_account_1.cpp
        CXXOPTS ${KMCMAKE_CXX_OPTIONS} -std=c++17
        INCLUDES
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CPP17_EXAMPLE_ROOT}
        LINKS xio::xio_static ${KMCMAKE_DEPS_LINK}
)


kmcmake_cc_binary(
        NAME cpp17_executors_bank_account_2
        SOURCES
        bank_account_2.cpp
        CXXOPTS ${KMCMAKE_CXX_OPTIONS} -std=c++17
        INCLUDES
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CPP17_EXAMPLE_ROOT}
        LINKS xio::xio_static ${KMCMAKE_DEPS_LINK}
)


kmcmake_cc_binary(
        NAME cpp17_executors_fork_join
        SOURCES
        fork_join.cpp
        CXXOPTS ${KMCMAKE_CXX_OPTIONS} -std=c++17
        INCLUDES
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CPP17_EXAMPLE_ROOT}
        LINKS xio::xio_static ${KMCMAKE_DEPS_LINK}
)


kmcmake_cc_binary(
        NAME cpp17_executors_pipeline
        SOURCES
        pipeline.cpp
        CXXOPTS ${KMCMAKE_CXX_OPTIONS} -std=c++17
        INCLUDES
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CPP17_EXAMPLE_ROOT}
        LINKS xio::xio_static ${KMCMAKE_DEPS_LINK}
)


kmcmake_cc_binary(
        NAME cpp17_executors_priority_scheduler
        SOURCES
        priority_scheduler.cpp
        CXXOPTS ${KMCMAKE_CXX_OPTIONS} -std=c++17
        INCLUDES
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CPP17_EXAMPLE_ROOT}
        LINKS xio::xio_static ${KMCMAKE_DEPS_LINK}
)

