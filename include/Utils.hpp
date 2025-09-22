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
#include <list>
#include "Noncopyable.hpp"
namespace util
{

    struct Edge
    {
        uint32_t dst_id;
        uint32_t metric;
    };
    class BitStream;
    using BitStreamPtr = std::shared_ptr<BitStream>;
    using IpAddr = uint32_t;
    using MacAddr = uint32_t;
    using Graph = std::vector<std::list<Edge>>;
    using NICSendFunc = std::function<void(uint32_t, uint32_t, util::BitStreamPtr)>;

    constexpr uint32_t IP_BROADCAST = 0xffffffff;

    
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
            mactive_ = false;
            while (mthread_.joinable())
                mthread_.join();
        }
        std::atomic<bool> mactive_;
        std::thread mthread_;
    };



    class BitStream : public std::enable_shared_from_this<BitStream>
    {
    private:
        BitStream() {}
        std::deque<std::byte> buffer_;

    public:
        static BitStreamPtr Create() { return BitStreamPtr(new BitStream()); }

        size_t Size() const { return buffer_.size(); }

        template <typename T>
        void Serialize(const T &packet)
        {
            const std::byte * packet_begin = reinterpret_cast<const std::byte *>(&packet);
            buffer_.insert(buffer_.begin(), packet_begin, packet_begin + sizeof(T));
        }

        template<typename T>
        void Serialize(const T* first,const T* last)
        {
            auto start=reinterpret_cast<const std::byte*>(first),end=reinterpret_cast<const std::byte*>(last);
            buffer_.insert(buffer_.begin(),start,end);
        }

        template <typename T>
        T DeSerialize()
        {
            T packet;
            std::copy_n(buffer_.begin(), sizeof(packet), reinterpret_cast<std::byte *>(&packet));
            buffer_.erase(buffer_.begin(), buffer_.begin() + sizeof(packet));
            return packet;
        }

        void Append(const BitStreamPtr &other, size_t len)
        {
            buffer_.insert(buffer_.end(), other->buffer_.begin(), other->buffer_.begin() + len);
        }
        void Append(const BitStreamPtr &other)
        {
            Append(other, other->Size());
        }

        
        BitStreamPtr Extract(size_t len)
        {
            auto ret = BitStream::Create();
            ret->Append(shared_from_this(), len);
            buffer_.erase(buffer_.begin(), buffer_.begin() + len);
            return ret;
        }
        const std::byte* Data() const {return &*buffer_.begin();}
    };
    
}