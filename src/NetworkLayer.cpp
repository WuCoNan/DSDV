#include "NetworkLayer.hpp"

namespace net
{
    void NetworkLayer::AddIpHeader(util::IpAddr sip, util::IpAddr dip, uint32_t protocol, const util::BufferPtr &buffer_ptr)
    {
        IpHeader ip_header;
        ip_header.sip = dip;
        ip_header.dip = dip;
        ip_header.protocol = protocol;

        util::Serialize(ip_header, buffer_ptr);
    }
    NetworkLayer::IpHeader NetworkLayer::RemoveIpHeader(const util::BufferPtr &buffer_ptr)
    {
        IpHeader ip_header = util::DeSerialize<IpHeader>(buffer_ptr);
        return ip_header;
    }
    void NetworkLayer::NetSend(util::IpAddr dip, const std::string &protocol_name, const util::BufferPtr &buffer_ptr)
    {
        AddIpHeader(mlocal_ip_addr_, dip, mprotocol_table_[protocol_name], buffer_ptr);

        util::MacAddr dmac = IpToMac(dip);

        methernet_layer_->EthernetSend(dmac, buffer_ptr);
    }
    void NetworkLayer::NetRecv(const util::BufferPtr &buffer_ptr)
    {
        IpHeader ip_header = RemoveIpHeader(buffer_ptr);

        util::IpAddr sip = ip_header.sip, dip = ip_header.dip;
        uint32_t protocol = ip_header.protocol;

        if (dip == mlocal_ip_addr_)
        {
            mprotocol_handles_[protocol](sip, buffer_ptr);
        }
        else
        {
            auto next_hop = mforward_table_->GetNextHop(dip);
            if (next_hop.has_value())
            {
                AddIpHeader(sip, dip, protocol, buffer_ptr);

                util::MacAddr dmac = IpToMac(dip);

                methernet_layer_->EthernetSend(dmac, buffer_ptr);
            }
            else
            {
            }
        }
    }
    void NetworkLayer::RegisterProtocolHandle(const std::string &protocol_name, ProtocolHandle protocol_handle)
    {
        if (mprotocol_table_.find(protocol_name) == mprotocol_table_.end())
            mprotocol_table_.emplace(protocol_name, mprotocol_id_++);
        mprotocol_handles_.emplace(mprotocol_table_[protocol_name], protocol_handle);
    }
}