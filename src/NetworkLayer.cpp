#include "NetworkLayer.hpp"
#include "RoutingTable.hpp"
namespace net
{
    NetworkLayer::NetworkLayer(util::IpAddr local_ip_addr, ethernet::EthernetLayer *ethernet_layer)
        : mlocal_ip_addr_(local_ip_addr), methernet_layer_(ethernet_layer), mprotocol_id_(0), midentification_counter_(0)
    {
        methernet_layer_->RegisterNetworkLayerCallback(std::bind(&NetworkLayer::NetRecv, this, std::placeholders::_1));
        methernet_layer_->RegisterChangedConnectionHandle(std::bind(&NetworkLayer::HandleChangedConnection, this, std::placeholders::_1));
        mforward_table_ = new RoutingTable(mlocal_ip_addr_);
    };

    void NetworkLayer::AddIpHeader(util::IpAddr sip, util::IpAddr dip, uint32_t protocol, util::BitStreamPtr &bit_ptr, uint16_t identification, uint16_t offset, uint8_t flags)
    {
        IpHeader ip_header;
        ip_header.sip = sip;
        ip_header.dip = dip;
        ip_header.protocol = protocol;
        ip_header.identification = identification;
        ip_header.offset = offset;
        ip_header.flags = flags;

        bit_ptr->Serialize(ip_header);
    }
    NetworkLayer::IpHeader NetworkLayer::RemoveIpHeader(util::BitStreamPtr &bit_ptr)
    {
        IpHeader ip_header = bit_ptr->DeSerialize<IpHeader>();
        return ip_header;
    }
    void NetworkLayer::NetSend(util::IpAddr dip, const std::string &protocol_name, util::BitStreamPtr &bit_ptr, bool flag)
    {
        // std::cout<<"NetworkLayer   "<<mlocal_ip_addr_<<"   send packet to   "<<dip<<std::endl;

        // AddIpHeader(mlocal_ip_addr_, dip, mprotocol_table_[protocol_name], buffer_ptr);
        auto next_hop = mforward_table_->GetNextHop(dip);

        if (flag)
            *next_hop = dip;

        if (!next_hop.has_value() && !flag)
            return;

        auto fragments = CreateFragments(dip, protocol_name, bit_ptr);

        util::MacAddr dmac = IpToMac(*next_hop);

        for (auto &fragment : fragments)
        {
            methernet_layer_->EthernetSend(dmac, fragment);
        }
    }
    void NetworkLayer::NetRecv(util::BitStreamPtr &bit_ptr)
    {
        IpHeader ip_header = RemoveIpHeader(bit_ptr);

        util::IpAddr sip = ip_header.sip, dip = ip_header.dip;
        uint32_t protocol = ip_header.protocol;
        uint16_t identification = ip_header.identification, offset = ip_header.offset;
        uint8_t flags = ip_header.flags;

        if (dip == mlocal_ip_addr_)
        {
            // std::cout<<"NetworkLayer   "<<mlocal_ip_addr_<<"   receive packet from sip   "<<sip<<std::endl;
            auto bit_ptr_new = ReassembleFragments(bit_ptr, ip_header);
            if (bit_ptr_new.has_value())
                mprotocol_handles_[protocol](sip, *bit_ptr_new);
        }
        else
        {
            auto next_hop = mforward_table_->GetNextHop(dip);
            if (next_hop.has_value())
            {
                AddIpHeader(sip, dip, protocol, bit_ptr, identification, offset, flags);

                util::MacAddr dmac = IpToMac(*next_hop);

                methernet_layer_->EthernetSend(dmac, bit_ptr);
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

    void NetworkLayer::RegisterChangedConnectionHandle(ChangedConnectionHandle changed_connection_handle)
    {
        mchanged_conn_handle_ = changed_connection_handle;
    }

    void NetworkLayer::HandleChangedConnection(const std::unordered_map<util::MacAddr, uint32_t> &changed_connections)
    {
        mchanged_conn_handle_(changed_connections);
    }

    NetworkLayer::IpFragments NetworkLayer::CreateFragments(util::IpAddr dip, const std::string &protocol_name, util::BitStreamPtr &bit_ptr)
    {
        IpFragments fragments;
        uint step = methernet_layer_->GetMTU() - sizeof(IpHeader), length = bit_ptr->Size();

        // 长度足够 不需要分片
        if (step >= length)
        {
            AddIpHeader(mlocal_ip_addr_, dip, mprotocol_table_[protocol_name], bit_ptr, midentification_counter_++, 0, 0b010);
            fragments.emplace_back(bit_ptr);
        }
        else
        {
            uint16_t offset = 0;
            for (offset = 0; length > 0; offset += step)
            {
                auto fragment = bit_ptr->Extract(std::min(length, step));
                // auto fragment=util::make_buffer(util::Buffer(buffer_ptr->begin()+offset,buffer_ptr->begin()+offset+std::min(length,step)));
                length -= std::min(length, step);

                uint8_t flags = 0;

                if (length == 0)
                    flags = 0;
                else
                    flags = 0b001;
                AddIpHeader(mlocal_ip_addr_, dip, mprotocol_table_[protocol_name], fragment, midentification_counter_, offset, flags);

                fragments.emplace_back(fragment);
            }
            midentification_counter_++;
        }
        return fragments;
    }
    std::optional<util::BitStreamPtr> NetworkLayer::ReassembleFragments(util::BitStreamPtr &bit_ptr, const IpHeader &ip_header)
    {
        uint32_t sip = ip_header.sip;
        uint16_t identification = ip_header.identification, offset = ip_header.offset;
        uint8_t flags = ip_header.flags, DF = (ip_header.flags) & 0b010, MF = (ip_header.flags) & 0b001;

        // 不支持分片 直接返回
        if (DF)
            return bit_ptr;
        // 支持分片 加入分片缓存中
        else
        {
            // 以sip和identification标识一个完整的数据包
            uint64_t key = (sip << 32) + identification;
            // 以offset和flag标识一个分片
            uint32_t flag_fragment = (offset << 8) | flags;

            // 此分片是第一个到达的分片
            if (mfragment_buffers_.find(key) == mfragment_buffers_.end())
            {
                // mfragment_buffers_.emplace(key,std::list<std::pair<uint32_t, util::BufferPtr>>{});
                mfragment_buffers_[key].push_back({flag_fragment, bit_ptr});
            }
            // 找到准确位置并插入
            else
            {
                auto iter = mfragment_buffers_[key].begin();
                for (; iter != mfragment_buffers_[key].end(); ++iter)
                {
                    uint16_t offset_tmp = iter->first >> 8;
                    if (offset_tmp > offset)
                        break;
                }
                mfragment_buffers_[key].insert(iter, {flag_fragment, bit_ptr});
            }

            // 判断是否所有分片均已到达
            auto fragment_list = mfragment_buffers_[key];

            uint16_t offset_pre = fragment_list.front().first >> 8, size_pre = fragment_list.front().second->Size();

            // 判断每个分片是否连续
            for (auto iter = ++fragment_list.begin(); iter != fragment_list.end(); ++iter)
            {
                uint16_t offset_cur = iter->first >> 8;
                if (offset_pre + size_pre != offset_cur)
                    return std::nullopt;
                offset_pre = offset_cur;
                size_pre = iter->second->Size();
            }

            // 判断最后一个分片是否到达
            if (fragment_list.back().first & 0b001)
                return std::nullopt;

            // 拼接每一个分片
            auto ret = util::BitStream::Create();
            for (auto iter = fragment_list.begin(); iter != fragment_list.end(); ++iter)
            {
                ret->Append(iter->second);
            }

            // 清除缓存
            mfragment_buffers_.erase(key);

            return ret;
        }
    }
}