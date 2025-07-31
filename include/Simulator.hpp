#pragma once
#include "Node.hpp"
#include <mutex>
#include <condition_variable>
namespace simulator
{
    class Simulator
    {
    private:
        std::unordered_map<uint32_t,std::deque<std::deque<std::byte>>> mtransmit_queue_;
        std::unordered_map<uint32_t,std::deque<std::deque<std::byte>>> mreceive_queue_;
        std::unordered_map<uint32_t,std::mutex> mtransmit_queue_locks_;
        std::unordered_map<uint32_t,std::mutex> mreceive_queue_locks_;
        void PushTransQueue(uint32_t node_id,const std::deque<std::byte>& bytes)
        {
            {
                std::unique_lock<std::mutex> u_lock(mtransmit_queue_locks_[node_id]);
                mtransmit_queue_[node_id].push_back(bytes);
            }
        }
    };
}