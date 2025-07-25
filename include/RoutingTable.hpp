#pragma once
// #include "ip_address.hpp"
#include "unordered_map"
#include <optional>
#include <memory>
#include "Utils.hpp"
namespace net
{
    struct RouteEntry
    {
        util::IpAddr dip;
        util::IpAddr next_hop;
    };

    struct DSDVRouteEntry : RouteEntry
    {
        uint32_t metric;
        uint32_t sequence;
    };

    template <typename EntryType>
    class RoutingTableBase
    {
    public:
        std::optional<util::IpAddr> GetNextHop(util::IpAddr dip) const
        {
            if (mtable_.find(dip) != mtable_.end())
                return mtable_.at(dip).next_hop;
            return std::nullopt;
        }

        void UpdateRouteTable(util::IpAddr dip, const EntryType &entry)
        {
            if (mtable_.find(dip) != mtable_.end())
            {
                mtable_[dip] = entry;
            }
            else
            {
                mtable_.emplace(dip, entry);
            }
        }

        std::optional<EntryType> Find(util::IpAddr dip) const
        {
            if (mtable_.find(dip) != mtable_.end())
                return mtable_.at(dip);
            return std::nullopt;
        }

    private:
        using Table = std::unordered_map<util::IpAddr, EntryType>;
        Table mtable_;
    };

}