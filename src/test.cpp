#include "ip_address.hpp"
#include <iostream>

int main()
{
    net::IpAddress ip_addr("192.168.0.1");
    std::cout<<ip_addr.get_ip_t()<<std::endl;
    return 0;
}