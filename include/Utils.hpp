#pragma once
#include <vector>
#include <string>
#include <deque>
#include <cstdint>
#include <memory>
#include <cstddef>
#include <algorithm>
namespace util
{
    using Buffer = std::deque<std::byte>;
    using BufferPtr = std::shared_ptr<Buffer>;
    using IpAddr = uint32_t;
    using MacAddr = uint32_t;
    
    BufferPtr make_buffer()
    {
        return std::make_shared<Buffer>();
    }

    template <typename Packet>
    void Serialize(const Packet &packet, BufferPtr buffer_ptr)
    {
        auto packet_begin = reinterpret_cast<const std::byte *>(&packet);
        buffer_ptr->insert(buffer_ptr->begin(), packet_begin, packet_begin + sizeof(Packet));
    }

    template <typename Packet>
    Packet DeSerialize(BufferPtr buffer_ptr)
    {
        Packet packet;
        std::copy_n(buffer_ptr->begin(), sizeof(Packet), reinterpret_cast<std::byte *>(&packet));
        buffer_ptr->erase(buffer_ptr->begin(), buffer_ptr->begin() + sizeof(Packet));
        return packet;
    }
}