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
set(CPP17_EXAMPLE_ROOT ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(allocation)
add_subdirectory(buffers)
add_subdirectory(chat)
add_subdirectory(deferred)
add_subdirectory(echo)
add_subdirectory(executors)
add_subdirectory(files)
add_subdirectory(fork)
add_subdirectory(futures)
add_subdirectory(handler_tracking)
add_subdirectory(icmp)
add_subdirectory(invocation)
add_subdirectory(iostreams)
add_subdirectory(local)
add_subdirectory(multicast)
add_subdirectory(nonblocking)
add_subdirectory(operations)
add_subdirectory(parallel_group)
add_subdirectory(porthopper)
add_subdirectory(serialization)
add_subdirectory(services)
add_subdirectory(socks4)
add_subdirectory(spawn)
add_subdirectory(ssl)
add_subdirectory(timeouts)
add_subdirectory(timers)
add_subdirectory(type_erasure)

if(WIN32)
    add_subdirectory(windows)
endif()
add_subdirectory(http)
add_subdirectory(tutorial)
