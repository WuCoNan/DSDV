#pragma once
#include "NetworkLayer.hpp"
#include "DSDV.hpp"
class Node
{
private:
    net::DSDVProtocol *mdsdv_;
    net::NetworkLayer *mnetwork_layer_;
    ethernet::EthernetLayer *methernet_layer_;

    void NetworkInit(ethernet::EthernetLayer *ethernet_layer)
    {
        net::NetworkLayer network_layer(0, ethernet_layer);
        net::DSDVProtocol dsdv(&network_layer);
    }
    void EthernetInit()
    {

    }
public:
    Node() {};
    void NodeInit()
    {

    }
};