#pragma once
// #include "ip_address.hpp"
#include "unordered_map"
#include <optional>
#include <memory>
#include <iostream>
#include "Utils.hpp"
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

    public:
        std::optional<util::IpAddr> GetNextHop(util::IpAddr dip) const
        {
            if (mtable_.find(dip) != mtable_.end())
                return mtable_.at(dip).next_hop;
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
                std::cout << "dip :" << entry.dip << "next hop: " << entry.next_hop << "metric: " << entry.metric << "sequence: " << entry.sequence << std::endl;
            }
        }

        void Clear()
        {
            mtable_.clear();
        }
    };

}