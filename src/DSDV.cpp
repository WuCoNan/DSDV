#include "DSDV.hpp"

namespace net
{
    //向指定ip地址发送路由表项
    void DSDVProtocol::SendDSDVPacket(util::IpAddr dip, const RouteEntry &entry)
    {
        //根据entry构造dsdv packet
        DSDVPacket packet{entry.dip, entry.metric, entry.sequence};

        //单播地址
        if (dip != util::IP_BROADCAST)
        {
            util::BufferPtr buffer_ptr = util::make_buffer();     //上下层统一使用buffer容器存放数据，但好像可以不用智能指针？？？？
            util::Serialize(packet, buffer_ptr);

            mnetwork_layer_->NetSend(dip, "DSDV", buffer_ptr);
            return;
        }
        
        //广播地址
        for (auto &adjacent_table_entry : madjacent_table_)
        {
            util::BufferPtr buffer_ptr = util::make_buffer();
            util::Serialize(packet, buffer_ptr);

            dip = adjacent_table_entry.first;
            mnetwork_layer_->NetSend(dip, "DSDV", buffer_ptr);
            // std::cout << "node   " << mlocal_ip_addr_ << "   send to   " << dip << "   about   " << packet.dip << std::endl;
        }
    }

    //收到dsdv packet
    void DSDVProtocol::ReadCallback(util::IpAddr src, const util::BufferPtr &buffer_ptr)
    {
        // 将字节流反序列化为packet
        DSDVPacket packet = util::DeSerialize<DSDVPacket>(buffer_ptr);
        // 在转发表中查找目标ip一致的表项
        auto entry = mforward_table_->Find(packet.dip);
        // 根据发来的packet和邻接表生成新的entry
        RouteEntry new_entry{packet.dip, src, std::min(packet.metric + madjacent_table_[src], UNREACHABLE), packet.sequence};

        //std::cout << "node   " << mlocal_ip_addr_ << "   DSDV receive entry: dip   " << new_entry.dip << "   metric   " << new_entry.metric << "   sequence   " << new_entry.sequence << std::endl;

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
                    // metric更优
                    if (entry->metric > new_entry.metric)
                    {
                        // 更新转发表并广播
                        mforward_table_->UpdateRouteTable(new_entry);
                        SendDSDVPacket(util::IP_BROADCAST, new_entry);
                    }
                    // metric相等
                    else if (entry->metric == new_entry.metric)
                    {
                        // 更新转发表并广播
                        mforward_table_->UpdateRouteTable(new_entry);
                        SendDSDVPacket(util::IP_BROADCAST, new_entry);
                    }
                    //metric更大
                    else
                    {
                        //当前表项metric变大了
                        if (entry->next_hop == src)
                        {
                            //更新转发表并广播
                            mforward_table_->UpdateRouteTable(new_entry);
                            SendDSDVPacket(util::IP_BROADCAST, new_entry);
                        }
                        //不是当前表项
                        else
                        {
                            //将其更新到广播表（优先取metric更小的）
                            auto old_entry = mbroadcast_table_->Find(new_entry.dip);
                            if (!old_entry.has_value() || old_entry->metric > new_entry.metric)
                                mbroadcast_table_->UpdateRouteTable(new_entry);
                        }
                    }
                }
                // 相同序号
                else if (entry->sequence == new_entry.sequence)
                {
                    // metric更优
                    if (entry->metric > new_entry.metric)
                    {
                        // 更新转发表并广播
                        mforward_table_->UpdateRouteTable(new_entry);
                        SendDSDVPacket(util::IP_BROADCAST, new_entry);
                    }
                    // metric相等
                    else if (entry->metric == new_entry.metric)
                    {
                        //不是当前表项 将其更新到广播表（优先取metric更小的）
                        if (entry->next_hop != src)
                        {
                            auto old_entry = mbroadcast_table_->Find(new_entry.dip);
                            if (!old_entry.has_value() || old_entry->metric > new_entry.metric)
                                mbroadcast_table_->UpdateRouteTable(new_entry);
                        }
                    }
                    //metric更大
                    else
                    {
                        //当前表项metric变大了
                        if (entry->next_hop == src)
                        {
                            //更新转发表并广播
                            mforward_table_->UpdateRouteTable(new_entry);
                            SendDSDVPacket(util::IP_BROADCAST, new_entry);
                        }
                        //不是当前表项
                        else
                        {
                            //将其更新到广播表（优先取metric更小的）
                            auto old_entry = mbroadcast_table_->Find(new_entry.dip);
                            if (!old_entry.has_value() || old_entry->metric > new_entry.metric)
                                mbroadcast_table_->UpdateRouteTable(new_entry);
                        }
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
                SendDSDVPacket(util::IP_BROADCAST, new_entry);
            }
        }
    }

    //周期广播路由表
    void DSDVProtocol::BroadcastRouteTable()
    {
        auto entries=mbroadcast_table_->GetAllEntry();
        mbroadcast_table_->Clear();

        //自增序号并广播
        auto self_entry=mforward_table_->Find(mlocal_ip_addr_);
        self_entry->sequence+=2;
        mforward_table_->UpdateRouteTable(*self_entry);
        SendDSDVPacket(util::IP_BROADCAST,*self_entry);

        //从广播表中选择路由表项发送
        for(auto& entry:entries)
        {
            auto old_entry=mforward_table_->Find(entry.dip);
            if(!old_entry.has_value()||((old_entry->sequence%2||old_entry->metric>entry.metric)&&old_entry->sequence<=entry.sequence))
            {
                mforward_table_->UpdateRouteTable(entry);
                SendDSDVPacket(util::IP_BROADCAST,entry);
            }
        }


        if(mlocal_ip_addr_==1)
        {
            std::cout << "node " << mlocal_ip_addr_ << " RouteTable:" << std::endl;
            mforward_table_->PrintRouteTable();
        }
        
    }

    //链路状态改变处理函数
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
                std::cout<<"disconnected"<<std::endl;

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
                auto entry = mforward_table_->Find(dip);

                // 有新连接 向其发送hello报文
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

    //hello报文发送函数,向目标发送本节点的路由表
    void DSDVProtocol::SendHelloPacket(util::IpAddr dip)
    {
        // std::cout << "node   " << mlocal_ip_addr_ << "   send Hello packet to   " << dip << std::endl;

        auto entries = mforward_table_->GetAllEntry();
        for (auto &entry : entries)
        {
            SendDSDVPacket(dip, entry);
        }
    }
}