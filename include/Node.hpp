#pragma once
#include "DSDV.hpp"
#include "Application.hpp"
namespace simulator
{
    class Simulator;
}
class Node
{
private:
    uint32_t mnodes_num_;

    ethernet::EthernetLayer methernet_layer_;
    
    net::NetworkLayer mnetwork_layer_;

    net::DSDVProtocol mdsdv_;

    application::Application mapplication_layer_;

    uint32_t mnode_id_;


public:
    Node(uint32_t node_id,simulator::Simulator* sim,uint32_t nodes_num)
        :mnodes_num_(nodes_num)
        ,mnode_id_(node_id)
        ,methernet_layer_(node_id,sim) 
        ,mnetwork_layer_(node_id,&methernet_layer_)
        ,mdsdv_(&mnetwork_layer_)
        ,mapplication_layer_(node_id,&mnetwork_layer_,nodes_num)
    {};
    void NodeInit()
    {

    }
    void BussinessStart()
    {
        mapplication_layer_.Start();
    }
};