#pragma once
#include <functional>
#include "Utils.hpp"
namespace ethernet
{
    class EthernetLayer
    {
    private:
        using NetworkLayerCallback = std::function<void(util::BufferPtr)>;
        NetworkLayerCallback mnetwork_layer_callback_;

    public:
        void RegisterNetworkLayerCallback(NetworkLayerCallback network_layer_callback);
        void EthernetSend();
        void EthernetRecv();
    };
}