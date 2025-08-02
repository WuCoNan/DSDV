#pragma once
#include <vector>
#include <string>
#include <deque>
#include <cstdint>
#include <memory>
#include <cstddef>
#include <algorithm>
#include <thread>
#include <chrono>
#include <atomic>
#include <functional>
namespace util
{
    using Buffer = std::deque<std::byte>;
    using BufferPtr = std::shared_ptr<Buffer>;
    using IpAddr = uint32_t;
    using MacAddr = uint32_t;
    
    constexpr uint32_t IP_BROADCAST=0xffffffff;

    BufferPtr make_buffer(util::Buffer buffer={})
    {
        return std::make_shared<Buffer>(buffer);
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
    class PeriodicExecutor
    {
    public:
        PeriodicExecutor() : mactive_(false) {};
        ~PeriodicExecutor()
        {
            stop();
        }
        void start(int interval_ms, std::function<void()> task)
        {
            if (mactive_)
                return;
            mactive_ = true;
            mthread_ = std::thread([this, interval_ms, task]()
                                   { this->run(interval_ms, task); });
        }

    private:
        void run(int interval_ms, std::function<void()> task)
        {
            auto next_time = std::chrono::steady_clock::now() + std::chrono::milliseconds(interval_ms);
            while (mactive_)
            {
                task();
                std::this_thread::sleep_until(next_time);
                next_time += std::chrono::milliseconds(interval_ms);
            }
        }
        void stop()
        {
            mactive_ = true;
            while (mthread_.joinable())
                mthread_.join();
        }
        std::atomic<bool> mactive_;
        std::thread mthread_;
    };
}