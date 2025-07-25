#pragma once
#include "RoutingTable.hpp"
#include "NetworkLayer.hpp"
#include "Utils.hpp"
namespace net
{
    class DSDVProtocol
    {
    private:
        using DSDVRoutingTable = RoutingTableBase<DSDVRouteEntry>;

        struct DSDVPacket
        {
            util::IpAddr dip;
            uint32_t metric;
            uint32_t sequence;
        };

        NetworkLayer *mnetwork_layer_;
        DSDVRoutingTable mforward_table_;
        DSDVRoutingTable mbroadcast_table_;

    public:
        DSDVProtocol(NetworkLayer *network_layer) : mnetwork_layer_(network_layer) {}
        void ReadCallback(util::IpAddr src, util::BufferPtr buffer_ptr);

    private:
    };
}