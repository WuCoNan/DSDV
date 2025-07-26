#pragma once
#include "RoutingTable.hpp"
#include "Utils.hpp"

namespace net
{
    class NetworkLayer
    {
    private:
        

    public:
        RoutingTable *mforward_table_;

    public:
        void NetSend(util::IpAddr ip_addr, util::BufferPtr buffer_ptr);
        void NetRecv(util::BufferPtr buffer_ptr);
    };
}