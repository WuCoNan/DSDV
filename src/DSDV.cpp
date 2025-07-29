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
            if (entry.dip == mlocal_ip_addr_)
                entry.sequence += 2;
            SendDSDVPacket(entry);
        }
    }

    void DSDVProtocol::DSDVHandleChangedConnection(const std::unordered_map<util::IpAddr, uint32_t> &changed_connections)
    {
        // 修改邻接表
        for (auto &connection : changed_connections)
        {
            auto dip = connection.first, metric = connection.second;
            if (metric == UNREACHABLE)
                madjacent_table_.erase(dip);
            else
                madjacent_table_[dip] = metric;
        }
        // 修改路由表
        for (auto &connection : changed_connections)
        {
            auto dip = connection.first, metric = connection.second;

            if (metric == UNREACHABLE)
            {
                // 已有连接断开，查找转发表中下一跳为该地址的表项修改sequence并向所有邻居发送通知报文
                auto entries = mforward_table_->GetAllEntry();
                for (auto &entry : entries)
                {
                    if (entry.next_hop == dip && entry.sequence % 2 == 0)
                    {
                        entry.sequence++;
                        mforward_table_->UpdateRouteTable(entry);
                        SendDSDVPacket(entry);
                    }
                }
            }
            else
            {
                // 有新连接
                auto entry = mforward_table_->Find(dip);
                if (!entry.has_value() || entry->sequence % 2)
                {
                    auto self_entry = mforward_table_->Find(mlocal_ip_addr_);
                    SendDSDVPacket(*self_entry);
                }
            }
        }
    }
}