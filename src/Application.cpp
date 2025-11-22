#include "Application.hpp"
#include <random>
#include "iostream"
#include "Logger.hpp"
namespace application
{
    void Application::SetInterval(uint interval_ms)
    {
        interval_ms_ = interval_ms;
    }
    void Application::SetOnceSize(uint once_size)
    {
        once_size_ = once_size;
    }
    void Application::AppSend()
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, node_num_-1);
        int random_id = dis(gen);
        while (random_id == node_id_)
            random_id = dis(gen);

        //std::vector<char> bussiness_data(once_size_, 'a');
        char bussiness_data[10000];

        auto bit_ptr=util::BitStream::Create();
        bit_ptr->Serialize(bussiness_data,bussiness_data+sizeof(bussiness_data));
        //std::cout<<"bussiness data size is "<<buffer_ptr->size()<<std::endl;

        LOG_INFO("(Node %u Application) : Application sending %u bytes to %d.\n", node_id_, bit_ptr->Size(), random_id);
        network_layer_->NetSend(random_id, "Bussiness", bit_ptr,false);
    }
    void Application::AppRecv(util::IpAddr sip,util::BitStreamPtr& bit_ptr)
    {
        LOG_INFO("(Node %u Application) : Application received %u bytes from %d.\n", node_id_, bit_ptr->Size(), sip);
    }
    void Application::Start()
    {
        LOG_INFO("(Node %u Application) : Application started with interval %u ms and once size %u bytes.\n", node_id_, interval_ms_, once_size_);
        periodic_bussiness_.start(interval_ms_, [this]()
                                 { AppSend(); });
    }
}