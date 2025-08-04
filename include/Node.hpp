#pragma once
#include "DSDV.hpp"
namespace simulator
{
    class Simulator;
}
class Node
{
private:
    ethernet::EthernetLayer methernet_layer_;
    
    net::NetworkLayer mnetwork_layer_;

    net::DSDVProtocol mdsdv_;

    uint32_t mnode_id_;

public:
    Node(uint32_t node_id,simulator::Simulator* sim)
        :mnode_id_(node_id)
        ,methernet_layer_(node_id,sim) 
        ,mnetwork_layer_(node_id,&methernet_layer_)
        ,mdsdv_(&mnetwork_layer_)
    {};
    void NodeInit()
    {

    }
};