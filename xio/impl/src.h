//
// impl/src.hpp
// ~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_IMPL_SRC_HPP
#define ASIO_IMPL_SRC_HPP

#define ASIO_SOURCE

#include <xio/detail/config.h>

#if defined(ASIO_HEADER_ONLY)
# error Do not compile Asio library source with ASIO_HEADER_ONLY defined
#endif

#include "xio/impl/any_completion_executor.ipp"
#include "xio/impl/any_io_executor.ipp"
#include "xio/impl/awaitable.ipp"
#include "xio/impl/cancellation_signal.ipp"
#include "xio/impl/config.ipp"
#include "xio/impl/connect_pipe.ipp"
#include "xio/impl/error.ipp"
#include "xio/impl/error_code.ipp"
#include "xio/impl/execution_context.ipp"
#include "xio/impl/io_context.ipp"
#include "xio/impl/serial_port_base.ipp"
#include "xio/impl/system_context.ipp"
#include "xio/impl/thread_pool.ipp"
#include "xio/detail/impl/buffer_sequence_adapter.ipp"
#include "xio/detail/impl/descriptor_ops.ipp"
#include "xio/detail/impl/dev_poll_reactor.ipp"
#include "xio/detail/impl/epoll_reactor.ipp"
#include "xio/detail/impl/eventfd_select_interrupter.ipp"
#include "xio/detail/impl/handler_tracking.ipp"
#include "xio/detail/impl/io_uring_descriptor_service.ipp"
#include "xio/detail/impl/io_uring_file_service.ipp"
#include "xio/detail/impl/io_uring_socket_service_base.ipp"
#include "xio/detail/impl/io_uring_service.ipp"
#include "xio/detail/impl/kqueue_reactor.ipp"
#include "xio/detail/impl/null_event.ipp"
#include "xio/detail/impl/pipe_select_interrupter.ipp"
#include "xio/detail/impl/posix_event.ipp"
#include "xio/detail/impl/posix_mutex.ipp"
#include "xio/detail/impl/posix_serial_port_service.ipp"
#include "xio/detail/impl/posix_thread.ipp"
#include "xio/detail/impl/posix_tss_ptr.ipp"
#include "xio/detail/impl/reactive_descriptor_service.ipp"
#include "xio/detail/impl/reactive_socket_service_base.ipp"
#include "xio/detail/impl/resolver_service_base.ipp"
#include "xio/detail/impl/resolver_thread_pool.ipp"
#include "xio/detail/impl/scheduler.ipp"
#include "xio/detail/impl/select_reactor.ipp"
#include "xio/detail/impl/service_registry.ipp"
#include "xio/detail/impl/signal_set_service.ipp"
#include "xio/detail/impl/socket_ops.ipp"
#include "xio/detail/impl/socket_select_interrupter.ipp"
#include "xio/detail/impl/strand_executor_service.ipp"
#include "xio/detail/impl/strand_service.ipp"
#include "xio/detail/impl/thread_context.ipp"
#include "xio/detail/impl/throw_error.ipp"
#include "xio/detail/impl/timer_queue_set.ipp"
#include "xio/detail/impl/win_iocp_file_service.ipp"
#include "xio/detail/impl/win_iocp_handle_service.ipp"
#include "xio/detail/impl/win_iocp_io_context.ipp"
#include "xio/detail/impl/win_iocp_serial_port_service.ipp"
#include "xio/detail/impl/win_iocp_socket_service_base.ipp"
#include "xio/detail/impl/win_event.ipp"
#include "xio/detail/impl/win_critsec_mutex.ipp"
#include "xio/detail/impl/win_object_handle_service.ipp"
#include "xio/detail/impl/win_static_mutex.ipp"
#include "xio/detail/impl/win_thread.ipp"
#include "xio/detail/impl/win_tss_ptr.ipp"
#include "xio/detail/impl/winrt_ssocket_service_base.ipp"
#include "xio/detail/impl/winrt_timer_scheduler.ipp"
#include "xio/detail/impl/winsock_init.ipp"
#include "xio/experimental/impl/channel_error.ipp"
#include "xio/generic/detail/impl/endpoint.ipp"
#include "xio/ip/impl/address.ipp"
#include "xio/ip/impl/address_v4.ipp"
#include "xio/ip/impl/address_v6.ipp"
#include "xio/ip/impl/host_name.ipp"
#include "xio/ip/impl/network_v4.ipp"
#include "xio/ip/impl/network_v6.ipp"
#include "xio/ip/detail/impl/endpoint.ipp"
#include "xio/local/detail/impl/endpoint.ipp"

#endif // ASIO_IMPL_SRC_HPP
