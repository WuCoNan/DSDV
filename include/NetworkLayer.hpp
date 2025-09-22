#pragma once
#include "RoutingTable.hpp"
#include "EthernetLayer.hpp"
namespace net
{
    class NetworkLayer
    {
    private:
        struct IpHeader
        {
            util::IpAddr sip;
            util::IpAddr dip;
            uint16_t checksum;
            uint16_t identification;
            uint16_t offset;
            uint8_t flags;
            uint8_t protocol;
            //uint8_t ttl;
            //uint16_t flag_fragment;
        };

        using ProtocolHandle = std::function<void(util::IpAddr, util::BitStreamPtr& bit_ptr)>;
        using ProtocolTable = std::unordered_map<std::string, uint8_t>;
        using ProtocolHandles = std::unordered_map<uint8_t, ProtocolHandle>;
        using ChangedConnectionHandle = std::function<void(const std::unordered_map<util::IpAddr, uint32_t> &)>;
        using IpFragments = std::vector<util::BitStreamPtr>;
        using FragmentBuffers = std::unordered_map<uint64_t, std::list<std::pair<uint32_t, util::BitStreamPtr>>>;

        ProtocolTable mprotocol_table_;
        uint8_t mprotocol_id_;
        ProtocolHandles mprotocol_handles_;
        ethernet::EthernetLayer *methernet_layer_;

        ChangedConnectionHandle mchanged_conn_handle_;

        std::atomic<uint16_t> midentification_counter_;
        FragmentBuffers mfragment_buffers_;
        
    public:
        RoutingTable *mforward_table_;
        util::IpAddr mlocal_ip_addr_;

    public:
        NetworkLayer(util::IpAddr local_ip_addr, ethernet::EthernetLayer *ethernet_layer)
            : mlocal_ip_addr_(local_ip_addr), methernet_layer_(ethernet_layer), mprotocol_id_(0),midentification_counter_(0)
        {
            methernet_layer_->RegisterNetworkLayerCallback(std::bind(&NetworkLayer::NetRecv, this, std::placeholders::_1));
            methernet_layer_->RegisterChangedConnectionHandle(std::bind(&NetworkLayer::HandleChangedConnection, this, std::placeholders::_1));
            mforward_table_ = new RoutingTable{};
        };
        void NetSend(util::IpAddr dip, const std::string &protocol_name, util::BitStreamPtr& bit_ptr,bool flag);
        void NetRecv(util::BitStreamPtr& bit_ptr);
        void RegisterProtocolHandle(const std::string &protocol_name, ProtocolHandle protocol_handle);
        void RegisterChangedConnectionHandle(ChangedConnectionHandle changed_connection_handle);

    private:
        void AddIpHeader(util::IpAddr sip, util::IpAddr dip, uint32_t protocol, util::BitStreamPtr& bit_ptr,uint16_t identification,uint16_t offset,uint8_t flags);
        IpHeader RemoveIpHeader(util::BitStreamPtr& bit_ptr);
        util::MacAddr IpToMac(util::IpAddr ip_addr)
        {
            return util::MacAddr(ip_addr);
        }
        void HandleChangedConnection(const std::unordered_map<util::MacAddr, uint32_t> &);
        IpFragments CreateFragments(util::IpAddr dip, const std::string &protocol_name, util::BitStreamPtr& bit_ptr);
        std::optional<util::BitStreamPtr> ReassembleFragments(util::BitStreamPtr& bit_ptr,const IpHeader& ip_header);
    };
}