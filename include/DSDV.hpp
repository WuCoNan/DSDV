#pragma once
#include "NetworkLayer.hpp"

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

        std::unordered_map<util::IpAddr, uint32_t> madjacent_table_;

        NetworkLayer *mnetwork_layer_;
        RoutingTable *mforward_table_;
        RoutingTable *mbroadcast_table_;

        util::PeriodicExecutor mperiodic_broadcast_;
        static constexpr uint32_t mbroadcast_interval_ms_ = 1000;

        util::IpAddr mlocal_ip_addr_;

    public:
        DSDVProtocol(NetworkLayer *network_layer) : mnetwork_layer_(network_layer), mforward_table_(network_layer->mforward_table_), mbroadcast_table_(new RoutingTable{}), mlocal_ip_addr_(network_layer->mlocal_ip_addr_)
        {
            mnetwork_layer_->RegisterProtocolHandle("DSDV", std::bind(&DSDVProtocol::ReadCallback, this, std::placeholders::_1, std::placeholders::_2));

            mforward_table_->UpdateRouteTable({mlocal_ip_addr_, mlocal_ip_addr_, 0, 0});

            mperiodic_broadcast_.start(mbroadcast_interval_ms_, [this]()
                                       { this->BroadcastRouteTable(); });
        }
        void ReadCallback(util::IpAddr src, const util::BufferPtr& buffer_ptr);
        void DSDVHandleChangedConnection(const std::unordered_map<util::IpAddr, uint32_t> &changed_connections);

    private:
        void BroadcastRouteTable();
        void SendDSDVPacket(util::IpAddr,const RouteEntry &entry);
    };
}