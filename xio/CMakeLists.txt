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

kmcmake_cc_library(
        NAMESPACE ${PROJECT_NAME}
        NAME xio
        SOURCES
        detail/impl/buffer_sequence_adapter.cc
        detail/impl/descriptor_ops.cc
        detail/impl/dev_poll_reactor.cc
        detail/impl/epoll_reactor.cc
        detail/impl/eventfd_select_interrupter.cc
        detail/impl/handler_tracking.cc
        detail/impl/io_uring_descriptor_service.cc
        detail/impl/io_uring_file_service.cc
        detail/impl/io_uring_service.cc
        detail/impl/io_uring_socket_service_base.cc
        detail/impl/kqueue_reactor.cc
        detail/impl/null_event.cc
        detail/impl/pipe_select_interrupter.cc
        detail/impl/posix_event.cc
        detail/impl/posix_mutex.cc
        detail/impl/posix_serial_port_service.cc
        detail/impl/posix_thread.cc
        detail/impl/posix_tss_ptr.cc
        detail/impl/reactive_descriptor_service.cc
        detail/impl/reactive_socket_service_base.cc
        detail/impl/resolver_service_base.cc
        detail/impl/resolver_thread_pool.cc
        detail/impl/scheduler.cc
        detail/impl/select_reactor.cc
        detail/impl/service_registry.cc
        detail/impl/signal_set_service.cc
        detail/impl/socket_ops.cc
        detail/impl/socket_select_interrupter.cc
        detail/impl/strand_executor_service.cc
        detail/impl/strand_service.cc
        detail/impl/thread_context.cc
        detail/impl/throw_error.cc
        detail/impl/timer_queue_set.cc
        detail/impl/win_critsec_mutex.cc
        detail/impl/win_event.cc
        detail/impl/win_iocp_file_service.cc
        detail/impl/win_iocp_handle_service.cc
        detail/impl/win_iocp_io_context.cc
        detail/impl/win_iocp_serial_port_service.cc
        detail/impl/win_iocp_socket_service_base.cc
        detail/impl/win_object_handle_service.cc
        detail/impl/winrt_ssocket_service_base.cc
        detail/impl/winrt_timer_scheduler.cc
        detail/impl/winsock_init.cc
        detail/impl/win_static_mutex.cc
        detail/impl/win_thread.cc
        detail/impl/win_tss_ptr.cc
        experimental/impl/channel_error.cc
        generic/detail/impl/endpoint.cc
        impl/any_completion_executor.cc
        impl/any_io_executor.cc
        impl/awaitable.cc
        impl/cancellation_signal.cc
        impl/config.cc
        impl/connect_pipe.cc
        impl/error.cc
        impl/error_code.cc
        impl/execution_context.cc
        impl/io_context.cc
        impl/serial_port_base.cc
        impl/system_context.cc
        impl/thread_pool.cc
        ip/detail/impl/endpoint.cc
        ip/impl/address.cc
        ip/impl/address_v4.cc
        ip/impl/address_v6.cc
        ip/impl/host_name.cc
        ip/impl/network_v4.cc
        ip/impl/network_v6.cc
        local/detail/impl/endpoint.cc
        raft/buffer.cc
        raft/buffer_serializer.cc
        raft/cluster_config.cc
        raft/crc32.cc
        raft/error_code.cc
        raft/global_mgr.cc
        raft/handle_append_entries.cc
        raft/handle_client_request.cc
        raft/handle_commit.cc
        raft/handle_custom_notification.cc
        raft/handle_join_leave.cc
        raft/handle_priority.cc
        raft/handle_snapshot_sync.cc
        raft/handle_timeout.cc
        raft/handle_user_cmd.cc
        raft/handle_vote.cc
        raft/in_memory_log_store.cc
        raft/launcher.cc
        raft/log_entry.cc
        raft/peer.cc
        raft/raft_server.cc
        raft/snapshot.cc
        raft/snapshot_sync_ctx.cc
        raft/snapshot_sync_req.cc
        raft/srv_config.cc
        raft/stat_mgr.cc
        raft/xio_service.cc
        ssl/detail/impl/engine.cc
        ssl/detail/impl/openssl_init.cc
        ssl/impl/context.cc
        ssl/impl/error.cc
        ssl/impl/host_name_verification.cc
        DEFINES
        -DXIO_DYN_LINK
        CXXOPTS
        ${KMCMAKE_CXX_OPTIONS}
        PLINKS
        ${KMCMAKE_DEPS_LINK}
        PUBLIC
)

