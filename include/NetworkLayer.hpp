#pragma once
#include "RoutingTable.hpp"
#include "Utils.hpp"
namespace net
{
    class NetworkLayer
    {
    private:
        using RoutingTable = RoutingTableBase<RouteEntry>;

    public:
        RoutingTable mforward_table_;

    public:
        void NetSend(util::MacAddr mac, util::BufferPtr buffer_ptr);
        void NetRecv(util::BufferPtr buffer_ptr);
    };
}