#pragma once
#include <functional>
#include "Utils.hpp"

namespace simulator
{
    class Simulator;
}
namespace ethernet
{
    class EthernetLayer
    {
    private:
        using NetworkLayerCallback = std::function<void(util::BufferPtr)>;
        using ChangedConnectionHandle = std::function<void(const std::unordered_map<util::IpAddr, uint32_t> &)>;

        NetworkLayerCallback mnetwork_layer_callback_;
        ChangedConnectionHandle mchanged_conn_handle_;

        simulator::Simulator *msimulator_;

        util::MacAddr mlocal_mac_addr_;
        uint32_t MacToID(util::MacAddr mac_addr);

        static constexpr int mmtu_ = 1500;

    public:
        EthernetLayer(util::MacAddr local_mac_addr, simulator::Simulator *sim);
        void RegisterNetworkLayerCallback(NetworkLayerCallback network_layer_callback);
        void RegisterChangedConnectionHandle(ChangedConnectionHandle changed_connection_handle);
        void EthernetSend(util::MacAddr dmac, const util::BufferPtr &buffer_ptr);
        void EthernetRecv(const util::Buffer &buffer);

        void AddLinks(const std::unordered_map<uint32_t, uint32_t> &new_connections);

        void RemoveLinks(const std::unordered_map<uint32_t, uint32_t> &disconnections);

        int GetMTU() const;
    };
}