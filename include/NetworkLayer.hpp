#pragma once
#include "RoutingTable.hpp"
#include "EthernetLayer.hpp"
namespace net
{
    class NetworkLayer
    {
    private:
        
        using ProtocolHandle = std::function<void(util::IpAddr, util::BufferPtr)>;
        using ProtocolTable = std::unordered_map<std::string, uint32_t>;
        using ProtocolHandles = std::unordered_map<uint32_t, ProtocolHandle>;
        using ChangedConnectionHandle=std::function<void(const std::unordered_map<util::IpAddr, uint32_t>&)>;

        ProtocolTable mprotocol_table_;
        uint32_t mprotocol_id_;
        ProtocolHandles mprotocol_handles_;
        ethernet::EthernetLayer *methernet_layer_;

        ChangedConnectionHandle mchanged_conn_handle_;

        struct IpHeader
        {
            util::IpAddr sip;
            util::IpAddr dip;
            uint32_t protocol;
        };

    public:
        RoutingTable *mforward_table_;
        util::IpAddr mlocal_ip_addr_;

    public:
        NetworkLayer(util::IpAddr local_ip_addr, ethernet::EthernetLayer *ethernet_layer)
            : mlocal_ip_addr_(local_ip_addr), methernet_layer_(ethernet_layer),mprotocol_id_(0)
        {
            methernet_layer_->RegisterNetworkLayerCallback(std::bind(&NetworkLayer::NetRecv, this, std::placeholders::_1));
            methernet_layer_->RegisterChangedConnectionHandle(std::bind(&NetworkLayer::HandleChangedConnection,this,std::placeholders::_1));
            mforward_table_=new RoutingTable{};
        };
        void NetSend(util::IpAddr dip, const std::string& protocol_name,const util::BufferPtr &buffer_ptr);
        void NetRecv(const util::BufferPtr &buffer_ptr);
        void RegisterProtocolHandle(const std::string &protocol_name, ProtocolHandle protocol_handle);
        void RegisterChangedConnectionHandle(ChangedConnectionHandle changed_connection_handle);
    private:
        void AddIpHeader(util::IpAddr sip,util::IpAddr dip, uint32_t protocol,const util::BufferPtr &buffer_ptr);
        IpHeader RemoveIpHeader(const util::BufferPtr &buffer_ptr);
        util::MacAddr IpToMac(util::IpAddr ip_addr)
        {
            return util::MacAddr(ip_addr);
        }
        void HandleChangedConnection(const std::unordered_map<util::MacAddr, uint32_t>&);
    };
}