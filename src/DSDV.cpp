#include "DSDV.hpp"
namespace net
{

    void DSDVProtocol::ReadCallback(util::IpAddr src, util::BufferPtr buffer_ptr)
    {
        DSDVPacket packet = util::DeSerialize<DSDVPacket>(buffer_ptr);

        auto entry = mforward_table_.Find(packet.dip);

        if (entry.has_value())
        {
        }
        // New Route!!!
        else
        {
            mnetwork_layer_->mforward_table_.UpdateRouteTable(packet.dip, *entry);
            mforward_table_.UpdateRouteTable(packet.dip, *entry);
            mbroadcast_table_.UpdateRouteTable(packet.dip, *entry);
        }
    }
}