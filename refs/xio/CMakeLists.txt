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

file(GLOB_RECURSE SRC "event/*.cc" "io/*.cc" "timer/*.cc" "compress/*.cc")
list(REMOVE_ITEM SRC "${CMAKE_CURRENT_SOURCE_DIR}/event/uring_poller.cc")
if (XIO_ENABLE_URING)
    list(APPEND SRC "${CMAKE_CURRENT_SOURCE_DIR}/event/uring_poller.cc")
endif ()

kmcmake_cc_library(
        NAMESPACE ${PROJECT_NAME}
        NAME xio
        SOURCES
        ${SRC}
        CXXOPTS
        ${KMCMAKE_CXX_OPTIONS}
        PLINKS
        ${KMCMAKE_DEPS_LINK}
        PUBLIC
)

