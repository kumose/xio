/************************************************************************
Modifications Copyright 2017-2019 eBay Inc.
Author/Developer(s): Jung-Sang Ahn

Original Copyright:
See URL: https://github.com/datatechnology/cornerstone

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    https://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
**************************************************************************/

#ifndef _ASIO_SERVICE_HXX_
#define _ASIO_SERVICE_HXX_

#include <xio/raft/xio_service_options.h>
#include <xio/raft/delayed_task_scheduler.h>
#include <xio/raft/delayed_task.h>
#include <xio/raft/rpc_cli_factory.h>

namespace nuraft {
    /**
     * Declaring this to hide the dependency of xio.hpp
     * from root header file, which can boost the compilation time.
     */
    class xio_service_impl;
    class logger;
    class rpc_listener;

    class xio_service
            : public delayed_task_scheduler
              , public rpc_client_factory {
    public:
        using meta_cb_params = xio_service_meta_cb_params;
        using options = xio_service_options;

        xio_service(const options &_opt = options(),
                    ptr<logger> _l = nullptr);

        ~xio_service();

        __nocopy__(xio_service);

    public:
        virtual void schedule(ptr<delayed_task> &task,
                              int32 milliseconds) override;

        virtual ptr<rpc_client> create_client(const std::string &endpoint)
        __override__;

        ptr<rpc_listener> create_rpc_listener(ushort listening_port,
                                              ptr<logger> &l);

        void stop();

        uint32_t get_active_workers();

    private:
        void cancel_impl(ptr<delayed_task> &task) override;

        xio_service_impl *impl_;

        ptr<logger> l_;
    };
};

#endif //_ASIO_SERVICE_HXX_
