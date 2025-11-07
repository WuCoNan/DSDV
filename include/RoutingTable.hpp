#pragma once
// #include "ip_address.hpp"
#include "unordered_map"
#include <optional>
#include <memory>
#include <iostream>
#include "Utils.hpp"
#include "NetworkLayer.hpp"
namespace net
{
#define UNREACHABLE 1000U
    struct RouteEntry
    {
        util::IpAddr dip;
        util::IpAddr next_hop;
        uint32_t metric;
        uint32_t sequence;
    };

    class RoutingTable
    {
    private:
        using Table = std::unordered_map<util::IpAddr, RouteEntry>;
        Table mtable_;
        util::IpAddr mlocal_ip_addr_;
        void UpdateSelfRouteEntry()
        {
            RouteEntry self_entry{mlocal_ip_addr_, mlocal_ip_addr_, 0, 0};
            UpdateRouteTable(self_entry);
        }

    public:
        RoutingTable() = default;
        explicit RoutingTable(util::IpAddr local_ip_addr) : mlocal_ip_addr_(local_ip_addr)
        {
            UpdateSelfRouteEntry();
        }
        std::optional<util::IpAddr> GetNextHop(util::IpAddr dip) const
        {
            if (mtable_.find(dip) != mtable_.end()&&mtable_.at(dip).sequence%2==0)
                return mtable_.at(dip).next_hop;
            if(!NetworkLayer::isInSameSubnet(dip,mlocal_ip_addr_)&&!NetworkLayer::IsGateway(mlocal_ip_addr_)&&mtable_.find(NetworkLayer::GetSubnetGateway(mlocal_ip_addr_))!=mtable_.end()&&mtable_.at(NetworkLayer::GetSubnetGateway(mlocal_ip_addr_)).sequence%2==0)
                return mtable_.at(NetworkLayer::GetSubnetGateway(mlocal_ip_addr_)).next_hop;
            if(NetworkLayer::IsGateway(mlocal_ip_addr_)&&mtable_.find(NetworkLayer::GetSubnetGateway(dip))!=mtable_.end()&&mtable_.at(NetworkLayer::GetSubnetGateway(dip)).sequence%2==0)
                return mtable_.at(NetworkLayer::GetSubnetGateway(dip)).next_hop;
                
            return std::nullopt;
        }

        void UpdateRouteTable(const RouteEntry &entry)
        {
            util::IpAddr dip = entry.dip;
            if (mtable_.find(dip) != mtable_.end())
            {
                mtable_[dip] = entry;
            }
            else
            {
                mtable_.emplace(dip, entry);
            }
        }
        void Erase(util::IpAddr dip)
        {
            mtable_.erase(dip);
        }
        std::optional<RouteEntry> Find(util::IpAddr dip) const
        {
            if (mtable_.find(dip) != mtable_.end())
                return mtable_.at(dip);
            return std::nullopt;
        }
        std::vector<RouteEntry> GetAllEntry() const
        {
            std::vector<RouteEntry> entries;
            for (auto &table_entry : mtable_)
            {
                entries.push_back(table_entry.second);
            }
            return entries;
        }
        void PrintRouteTable() const
        {
            for (auto &table_entry : mtable_)
            {
                auto entry = table_entry.second;
                std::cout << "dip :   " << entry.dip << "   next hop :   " << entry.next_hop << "   metric :   " << entry.metric << "   sequence :   " << entry.sequence << std::endl;
            }
        }

        void Clear()
        {
            mtable_.clear();
        }
    };

}