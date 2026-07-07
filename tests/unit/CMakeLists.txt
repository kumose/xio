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

set(XIO_TEST_INCLUDES
        ${PROJECT_SOURCE_DIR}/tests
        ${PROJECT_SOURCE_DIR}/refs/asio/include
)

set(XIO_TEST_DEFINES
        ASIO_DISABLE_DEPRECATED_MSG
        ASIO_STANDALONE
)

find_package(OpenSSL QUIET)
find_package(Boost QUIET COMPONENTS context)

# Core tests
set(CORE_TESTS
        any_completion_executor
        any_completion_handler
        any_io_executor
        append
        as_tuple
        associated_allocator
        associated_cancellation_slot
        associated_executor
        associated_immediate_executor
        associator
        async_result
        awaitable
        basic_datagram_socket
        basic_deadline_timer
        basic_file
        basic_random_access_file
        basic_raw_socket
        basic_readable_pipe
        basic_seq_packet_socket
        basic_serial_port
        basic_signal_set
        basic_socket
        basic_socket_acceptor
        basic_stream_file
        basic_stream_socket
        basic_streambuf
        basic_waitable_timer
        basic_writable_pipe
        bind_allocator
        bind_cancellation_slot
        bind_executor
        bind_immediate_executor
        buffered_read_stream
        buffered_stream
        buffered_write_stream
        buffer
        buffer_registration
        buffers_iterator
        cancel_after
        cancel_at
        cancellation_signal
        cancellation_state
        cancellation_type
        co_composed
        co_spawn
        completion_condition
        compose
        composed
        config
        connect
        connect_pipe
        consign
        coroutine
        deadline_timer
        defer
        deferred
        detached
        dispatch
        disposition
        error
        execution_context
        executor
        executor_work_guard
        file_base
        fwd_first
        fwd_last
        high_resolution_timer
        immediate
        inline_executor
        inline_or_executor
        io_context
        io_context_strand
        is_read_buffered
        is_write_buffered
        packaged_task
        placeholders
        post
        prepend
        random_access_file
        read
        read_at
        read_until
        readable_pipe
        recycling_allocator
        redirect_disposition
        redirect_error
        registered_buffer
        serial_port
        serial_port_base
        signal_set
        signal_set_base
        socket_base
        static_thread_pool
        steady_timer
        strand
        stream_file
        streambuf
        system_context
        system_executor
        system_timer
        this_coro
        thread
        thread_pool
        time_traits
        use_awaitable
        use_future
        uses_executor
        wait_traits
        writable_pipe
        write
        write_at
)

set(SUBDIR_TESTS
        execution/any_executor
        execution/blocking
        execution/blocking_adaptation
        execution/context_as
        execution/executor
        execution/inline_exception_handling
        execution/invocable_archetype
        execution/mapping
        execution/outstanding_work
        execution/prefer_only
        execution/relationship
        generic/basic_endpoint
        generic/datagram_protocol
        generic/raw_protocol
        generic/seq_packet_protocol
        generic/stream_protocol
        ip/address
        ip/address_v4
        ip/address_v4_iterator
        ip/address_v4_range
        ip/address_v6
        ip/address_v6_iterator
        ip/address_v6_range
        ip/basic_endpoint
        ip/basic_resolver
        ip/basic_resolver_entry
        ip/basic_resolver_iterator
        ip/basic_resolver_query
        ip/host_name
        ip/icmp
        ip/multicast
        ip/network_v4
        ip/network_v6
        ip/resolver_query_base
        ip/tcp
        ip/udp
        ip/unicast
        ip/v6_only
        local/basic_endpoint
        local/connect_pair
        local/datagram_protocol
        local/seq_packet_protocol
        local/stream_protocol
        posix/basic_descriptor
        posix/basic_stream_descriptor
        posix/descriptor
        posix/descriptor_base
        posix/stream_descriptor
        ts/buffer
        ts/executor
        ts/internet
        ts/io_context
        ts/net
        ts/netfwd
        ts/socket
        ts/timer
        windows/basic_object_handle
        windows/basic_overlapped_handle
        windows/basic_random_access_handle
        windows/basic_stream_handle
        windows/object_handle
        windows/overlapped_handle
        windows/overlapped_ptr
        windows/random_access_handle
        windows/stream_handle
)

foreach(t ${CORE_TESTS})
    kmcmake_cc_test(NAME ${t} MODULE unit
            SOURCES ${t}.cpp
            INCLUDES ${XIO_TEST_INCLUDES}
            DEFINES ${XIO_TEST_DEFINES}
            LINKS xio::xio_static
    )
endforeach()

foreach(t ${SUBDIR_TESTS})
    string(REPLACE "/" "_" name ${t})
    kmcmake_cc_test(NAME ${name} MODULE unit
            SOURCES ${t}.cpp
            INCLUDES ${XIO_TEST_INCLUDES}
            DEFINES ${XIO_TEST_DEFINES}
            LINKS xio::xio_static
    )
endforeach()

# C++11 experimental tests
if(NOT CMAKE_CXX_STANDARD OR CMAKE_CXX_STANDARD GREATER_EQUAL 11)
    foreach(t experimental/basic_channel
               experimental/basic_concurrent_channel
               experimental/channel
               experimental/channel_traits
               experimental/concurrent_channel
               experimental/parallel_group)
        string(REPLACE "/" "_" name ${t})
        kmcmake_cc_test(NAME ${name} MODULE unit
                SOURCES ${t}.cpp
                INCLUDES ${XIO_TEST_INCLUDES}
                DEFINES ${XIO_TEST_DEFINES}
                LINKS xio::xio_static
        )
    endforeach()
endif()

# C++20 / coroutine tests
if(CMAKE_CXX_STANDARD GREATER_EQUAL 20)
    foreach(t experimental/promise
               experimental/awaitable_operators
               experimental/co_composed
               experimental/coro/allocator
               experimental/coro/cancel
               experimental/coro/co_spawn
               experimental/coro/exception
               experimental/coro/executor
               experimental/coro/partial
               experimental/coro/simple_test
               experimental/coro/stack_test
               experimental/coro/use_coro)
        string(REPLACE "/" "_" name ${t})
        kmcmake_cc_test(NAME ${name} MODULE unit
                SOURCES ${t}.cpp
                INCLUDES ${XIO_TEST_INCLUDES}
                DEFINES ${XIO_TEST_DEFINES}
                LINKS xio::xio_static
        )
    endforeach()
endif()

# Boost.Coroutine (spawn)
if(Boost_context_FOUND)
    kmcmake_cc_test(NAME spawn MODULE unit
            SOURCES spawn.cpp
            INCLUDES ${XIO_TEST_INCLUDES}
            DEFINES ${XIO_TEST_DEFINES}
            LINKS xio::xio_static Boost::context
    )
endif()

# OpenSSL tests
if(OpenSSL_FOUND)
    foreach(t ssl/context_base ssl/context ssl/error
               ssl/host_name_verification ssl/stream_base ssl/stream)
        string(REPLACE "/" "_" name ${t})
        kmcmake_cc_test(NAME ${name} MODULE unit
                SOURCES ${t}.cpp
                INCLUDES ${XIO_TEST_INCLUDES}
                DEFINES ${XIO_TEST_DEFINES}
                LINKS xio::xio_static OpenSSL::SSL OpenSSL::Crypto
        )
    endforeach()
endif()
