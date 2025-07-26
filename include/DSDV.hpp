#pragma once
#include "RoutingTable.hpp"
#include "NetworkLayer.hpp"
#include "Utils.hpp"
namespace net
{
    class DSDVProtocol
    {
    private:
        struct DSDVPacket
        {
            util::IpAddr dip;
            uint32_t metric;
            uint32_t sequence;
        };
        std::unordered_map<util::IpAddr,uint32_t> madjacent_table_;

        NetworkLayer *mnetwork_layer_;
        RoutingTable *mforward_table_;
        RoutingTable *mbroadcast_table_;

        util::PeriodicExecutor mperiodic_broadcast_;
        static constexpr uint32_t mbroadcast_interval_ms_ = 1000;

    public:
        DSDVProtocol(NetworkLayer *network_layer) : mnetwork_layer_(network_layer), mforward_table_(network_layer->mforward_table_), mbroadcast_table_(new RoutingTable{})
        {
            mperiodic_broadcast_.start(mbroadcast_interval_ms_, [this]()
                                       { this->BroadcastRouteTable(); });
        }
        void ReadCallback(util::IpAddr src, util::BufferPtr buffer_ptr);
        void DSDVHandleChangedConnection(const std::unordered_map<util::IpAddr, uint32_t> &changed_connections);

    private:
        void BroadcastRouteTable();
        void SendDSDVPacket(const RouteEntry& entry);
    };
}