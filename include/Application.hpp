#pragma once
#include "NetworkLayer.hpp"
namespace application
{
    class Application
    {
    public:
        explicit Application(uint node_id,net::NetworkLayer* network_layer,uint interval_ms=1000,uint once_size=10000)
                            :node_id_(node_id)
                            ,network_layer_(network_layer)
                            ,interval_ms_(interval_ms)
                            ,once_size_(once_size)
        {
            network_layer->RegisterProtocolHandle("Bussiness",std::bind(&Application::AppRecv,this,std::placeholders::_1,std::placeholders::_2));
        }
        void Start();
    private:
        
        uint node_id_;
        uint interval_ms_;
        uint once_size_;
        net::NetworkLayer* network_layer_;
        util::PeriodicExecutor periodic_bussiness_;

        void SetInterval(uint interval);
        void SetOnceSize(uint once_size);
        
        void AppRecv(util::IpAddr,util::BitStreamPtr&);
        void AppSend();
    };
}