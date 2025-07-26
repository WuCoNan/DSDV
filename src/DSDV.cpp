#include "DSDV.hpp"
namespace net
{
    void DSDVProtocol::SendDSDVPacket(const RouteEntry &entry)
    {
        DSDVPacket packet{entry.dip, entry.metric, entry.sequence};
        util::BufferPtr buffer_ptr = util::make_buffer();
        util::Serialize(packet, buffer_ptr);

        for (auto &adjacent_table_entry : madjacent_table_)
        {
            util::IpAddr dip = adjacent_table_entry.second;
            mnetwork_layer_->NetSend(dip, buffer_ptr);
        }
    }
    void DSDVProtocol::ReadCallback(util::IpAddr src, util::BufferPtr buffer_ptr)
    {
        DSDVPacket packet = util::DeSerialize<DSDVPacket>(buffer_ptr);

        auto entry = mforward_table_->Find(packet.dip);

        RouteEntry new_entry{packet.dip, src, packet.metric + madjacent_table_[src], packet.sequence};

        // 已经发现的路由表项
        if (entry.has_value())
        {
            // 断链
            if (new_entry.sequence % 2)
            {
                // 下一跳正好为发送者，删除表项，立即发送断链信息
                if (src == entry->next_hop)
                {
                    mforward_table_->Erase(entry->dip);

                    SendDSDVPacket(new_entry);
                }
            }
            // 有效链路
            else
            {
                // 新序号，不管metric直接更新，应该会导致路由振荡？
                if (entry->sequence < new_entry.sequence)
                {
                    mforward_table_->UpdateRouteTable(new_entry);

                    mbroadcast_table_->UpdateRouteTable(new_entry);
                }
                // 相同序号
                else if (entry->sequence == new_entry.sequence)
                {
                    // metric更优才更新
                    if (entry->metric > new_entry.metric)
                    {
                        mforward_table_->UpdateRouteTable(new_entry);

                        mbroadcast_table_->UpdateRouteTable(new_entry);
                    }
                }
                // 旧序号，不做任何处理
                else
                {
                }
            }
        }
        // New Route!!!
        else
        {
            // 断链，不做任何处理
            if (entry->sequence % 2)
            {
            }
            // 有效链路，立即更新路由表并发送新路由信息
            else
            {
                mforward_table_->UpdateRouteTable(new_entry);

                SendDSDVPacket(new_entry);
            }
        }
    }

    void DSDVProtocol::BroadcastRouteTable()
    {
        auto entries = mbroadcast_table_->GetAllEntry();
        for (auto &entry : entries)
        {
            SendDSDVPacket(entry);
        }
    }

    void DSDVProtocol::DSDVHandleChangedConnection(const std::unordered_map<util::IpAddr, uint32_t> &changed_connections)
    {
        for (auto &connection : changed_connections)
        {
            auto dip = connection.first, metric = connection.second;

            if (metric == UNREACHABLE)
            {

            }
            else
            {
                
            }
        }
    }
}