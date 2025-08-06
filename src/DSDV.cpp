#include "DSDV.hpp"

namespace net
{
    void DSDVProtocol::SendDSDVPacket(util::IpAddr dip, const RouteEntry &entry)
    {
        DSDVPacket packet{entry.dip, entry.metric, entry.sequence};

        if (dip != util::IP_BROADCAST)
        {
            util::BufferPtr buffer_ptr = util::make_buffer();
            util::Serialize(packet, buffer_ptr);

            mnetwork_layer_->NetSend(dip, "DSDV", buffer_ptr);
            return;
        }

        for (auto &adjacent_table_entry : madjacent_table_)
        {
            util::BufferPtr buffer_ptr = util::make_buffer();
            util::Serialize(packet, buffer_ptr);

            dip = adjacent_table_entry.first;
            mnetwork_layer_->NetSend(dip, "DSDV", buffer_ptr);
            std::cout << "node   " << mlocal_ip_addr_ << "   send to   " << dip << "   about   " << packet.dip << std::endl;
        }
    }
    void DSDVProtocol::ReadCallback(util::IpAddr src, const util::BufferPtr &buffer_ptr)
    {
        // 将字节流反序列化为packet
        DSDVPacket packet = util::DeSerialize<DSDVPacket>(buffer_ptr);
        // 在转发表中查找目标ip一致的表项
        auto entry = mforward_table_->Find(packet.dip);
        // 根据发来的packet和邻接表生成新的entry
        RouteEntry new_entry{packet.dip, src, std::min(packet.metric + madjacent_table_[src], UNREACHABLE), packet.sequence};

        std::cout << "node   " << mlocal_ip_addr_ << "   DSDV receive entry: dip   " << new_entry.dip << "   metric   " << new_entry.metric << "   sequence   " << new_entry.sequence << std::endl;

        // 已经发现的路由表项
        if (entry.has_value())
        {
            // 断链
            if (new_entry.sequence % 2)
            {
                // 下一跳正好为发送者，修改表项sequence为奇数，立即发送断链信息
                if (src == entry->next_hop)
                {
                    // 更新转发表
                    mforward_table_->UpdateRouteTable(new_entry);
                    // 广播不可达消息
                    SendDSDVPacket(util::IP_BROADCAST, new_entry);
                    // 如果有，删除广播表中相应的表项
                    mbroadcast_table_->Erase(new_entry.dip);
                }
            }
            // 有效链路
            else
            {
                // 新序号，不管metric直接更新，应该会导致路由振荡？
                if (entry->sequence < new_entry.sequence)
                {
                    // 更新转发表
                    mforward_table_->UpdateRouteTable(new_entry);
                    // 更新广播表
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
            if (new_entry.sequence % 2)
            {
            }
            // 有效链路，立即更新路由表并发送新路由信息
            else
            {
                // 更新转发表
                // std::cout << "New Route!!!" << std::endl;

                mforward_table_->UpdateRouteTable(new_entry);
                // 广播新路由
                std::cout << "node  " << mlocal_ip_addr_ << "   broadcast new entry about " << new_entry.dip << "   to   ";
                for (auto &adjacent_node : madjacent_table_)
                {
                    std::cout << adjacent_node.first << "   ";
                }
                std::cout << std::endl;

                SendDSDVPacket(util::IP_BROADCAST, new_entry);
            }
        }
    }

    void DSDVProtocol::BroadcastRouteTable()
    {
        auto entries = mbroadcast_table_->GetAllEntry();

        mbroadcast_table_->Clear();
        if (mlocal_ip_addr_ == 2)
        {
            std::cout << "node " << mlocal_ip_addr_ << " RouteTable:" << std::endl;
            mforward_table_->PrintRouteTable();
        }

        for (auto &entry : entries)
        {
            // 自增本节点的sequence
            // std::cout << "node " << mlocal_ip_addr_ << " broadcast!!!" << std::endl;

            SendDSDVPacket(util::IP_BROADCAST, entry);
        }

        auto self_entry = mforward_table_->Find(mlocal_ip_addr_);
        self_entry->sequence += 2;
        mforward_table_->UpdateRouteTable(*self_entry);
        SendDSDVPacket(util::IP_BROADCAST, *self_entry);
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
                        entry.metric = metric;
                        mforward_table_->UpdateRouteTable(entry);
                        SendDSDVPacket(util::IP_BROADCAST, entry);
                    }
                }
            }
            else
            {
                // 有新连接
                auto entry = mforward_table_->Find(dip);
                if (!entry.has_value() || entry->sequence % 2)
                {
                    // auto self_entry = mforward_table_->Find(mlocal_ip_addr_);
                    //  只向该节点发送hello报文，是否应该给他发广播表？
                    // std::cout << "node   " << mlocal_ip_addr_ << "   发送self_entry to   " << dip << std::endl;
                    SendHelloPacket(dip);
                }
            }
        }
    }

    void DSDVProtocol::SendHelloPacket(util::IpAddr dip)
    {
        std::cout << "node   " << mlocal_ip_addr_ << "   send Hello packet to   " << dip << std::endl;

        auto entries = mforward_table_->GetAllEntry();
        for (auto &entry : entries)
        {
            SendDSDVPacket(dip, entry);
        }
    }
}