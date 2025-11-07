#pragma once
#include "EthernetLayer.hpp"
#include "Utils.hpp"
#include <optional>
namespace net
{
    class RoutingTable;
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
        constexpr static uint8_t subnetNodeNum_ =5;
    public:
        NetworkLayer(util::IpAddr local_ip_addr, ethernet::EthernetLayer *ethernet_layer);
        
        void NetSend(util::IpAddr dip, const std::string &protocol_name, util::BitStreamPtr& bit_ptr,bool flag);
        void NetRecv(util::BitStreamPtr& bit_ptr);
        void RegisterProtocolHandle(const std::string &protocol_name, ProtocolHandle protocol_handle);
        void RegisterChangedConnectionHandle(ChangedConnectionHandle changed_connection_handle);
        static bool isInSameSubnet(util::IpAddr ip1,util::IpAddr ip2)  {return (ip1/NetworkLayer::subnetNodeNum_)==(ip2/NetworkLayer::subnetNodeNum_);};
        static bool IsGateway(util::IpAddr ip)  {return (ip%NetworkLayer::subnetNodeNum_)==0;};
        static util::IpAddr GetSubnetGateway(util::IpAddr ip) {return util::IpAddr(ip - (ip % NetworkLayer::subnetNodeNum_));};
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