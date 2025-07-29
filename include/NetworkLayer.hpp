#pragma once
#include "RoutingTable.hpp"
#include "Utils.hpp"
#include "EthernetLayer.hpp"
namespace net
{
    class NetworkLayer
    {
    private:
        using ProtocolHandle = std::function<void(util::IpAddr, util::BufferPtr)>;
        using ProtocolHandles = std::unordered_map<std::string, ProtocolHandle>;

        ProtocolHandles mprotocol_handles_;
        ethernet::EthernetLayer* methernet_layer_;
    public:
        RoutingTable *mforward_table_;
        util::IpAddr mlocal_ip_addr_;

    public:
        NetworkLayer(util::IpAddr local_ip_addr,ethernet::EthernetLayer* ethernet_layer)
                    :mlocal_ip_addr_(local_ip_addr)
                    ,methernet_layer_(ethernet_layer)
        {
            methernet_layer_->RegisterNetworkLayerCallback(std::bind(&NetworkLayer::NetRecv,this,std::placeholders::_1));
        };
        void NetSend(util::IpAddr ip_addr, util::BufferPtr buffer_ptr);
        void NetRecv(util::BufferPtr buffer_ptr);
        void RegisterProtocolHandle(const std::string &protocol_name, ProtocolHandle protocol_handle);
    };
}