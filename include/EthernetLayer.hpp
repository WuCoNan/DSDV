#pragma once
#include <functional>
#include "Utils.hpp"
namespace ethernet
{
    class EthernetLayer
    {
    private:
        using NetworkLayerCallback = std::function<void(util::BufferPtr)>;
        using NICSendFunc = std::function<void(uint32_t, uint32_t, util::Buffer)>;

        NetworkLayerCallback mnetwork_layer_callback_;
        NICSendFunc mnic_send_func_;
        util::MacAddr mlocal_mac_addr_;
        uint32_t MacToID(util::MacAddr mac_addr)
        {
            return mac_addr;
        }

    public:
        EthernetLayer(util::MacAddr local_mac_addr, NICSendFunc nic_send_func)
            : mlocal_mac_addr_(local_mac_addr), mnic_send_func_(nic_send_func)
        {
        }
        void RegisterNetworkLayerCallback(NetworkLayerCallback network_layer_callback)
        {
            mnetwork_layer_callback_ = network_layer_callback;
        }
        void EthernetSend(util::MacAddr dmac, const util::BufferPtr &buffer_ptr)
        {
            uint32_t dst_id = MacToID(dmac), src_id = MacToID(mlocal_mac_addr_);
            mnic_send_func_(src_id, dst_id, *buffer_ptr);
        }
        void EthernetRecv(const util::Buffer &buffer)
        {
            auto buffer_ptr = util::make_buffer(buffer);
            mnetwork_layer_callback_(buffer_ptr);
        }
    };
}