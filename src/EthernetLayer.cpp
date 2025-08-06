#include "EthernetLayer.hpp"
#include "Simulator.hpp"

namespace ethernet
{
    uint32_t EthernetLayer::MacToID(util::MacAddr mac_addr)
    {
        return mac_addr;
    }
    EthernetLayer::EthernetLayer(util::MacAddr local_mac_addr, simulator::Simulator *sim)
        : mlocal_mac_addr_(local_mac_addr), msimulator_(sim)
    {
        msimulator_->RegisterReadCallback(MacToID(mlocal_mac_addr_), std::bind(&EthernetLayer::EthernetRecv, this, std::placeholders::_1));
        msimulator_->RegisterAddLinksCallback(MacToID(mlocal_mac_addr_), std::bind(&EthernetLayer::AddLinks, this, std::placeholders::_1));
        msimulator_->RegisterRemoveLinksCallback(MacToID(mlocal_mac_addr_), std::bind(&EthernetLayer::RemoveLinks, this, std::placeholders::_1));
    }
    void EthernetLayer::RegisterNetworkLayerCallback(NetworkLayerCallback network_layer_callback)
    {
        mnetwork_layer_callback_ = network_layer_callback;
    }
    void EthernetLayer::RegisterChangedConnectionHandle(ChangedConnectionHandle changed_connection_handle)
    {
        mchanged_conn_handle_ = changed_connection_handle;
    }
    void EthernetLayer::EthernetSend(util::MacAddr dmac, const util::BufferPtr &buffer_ptr)
    {
        std::cout<<"EthernetLayer "<<mlocal_mac_addr_<<" send frame to "<<dmac<<std::endl;

        uint32_t dst_id = MacToID(dmac), src_id = MacToID(mlocal_mac_addr_);
        msimulator_->PushTransQueue(src_id, dst_id, *buffer_ptr);

    }
    void EthernetLayer::EthernetRecv(const util::Buffer &buffer)
    {
        std::cout<<"EthernetLayer "<<mlocal_mac_addr_<<" receive frame"<<std::endl;

        auto buffer_ptr = util::make_buffer(buffer);
        mnetwork_layer_callback_(buffer_ptr);
    }
    void EthernetLayer::AddLinks(const std::unordered_map<uint32_t, uint32_t> &new_connections)
    {
        mchanged_conn_handle_(new_connections);
    }
    void EthernetLayer::RemoveLinks(const std::unordered_map<uint32_t, uint32_t> &disconnections)
    {
        mchanged_conn_handle_(disconnections);
    }
}