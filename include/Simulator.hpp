#pragma once
#include "Node.hpp"
#include <mutex>
#include <condition_variable>
#include <list>
namespace simulator
{
    class Simulator
    {
    private:
        using Data = std::deque<std::byte>;
        using DataQueue = std::deque<Data>;
        using RecvCallback = std::function<void(Data)>;
        using Graph=std::vector<std::list<uint32_t>>;

        std::vector<std::vector<DataQueue>> mtransmit_queue_;
        std::vector<DataQueue> mreceive_queue_;
        std::vector<std::mutex> mtrans_locks_, mrecv_locks_;
        std::vector<std::condition_variable> mtrans_cvs_, mrecv_cvs_;

        std::vector<RecvCallback> mrecv_callbacks_;

        Graph mgraph_;
        
        void PushTransQueue(uint32_t src_id, uint32_t dst_id, const Data &bytes)
        {

            std::unique_lock<std::mutex> u_lock(mtrans_locks_[src_id]);
            mtransmit_queue_[src_id][dst_id].push_back(bytes);
            mtrans_cvs_[src_id].notify_one();
        }

        void Transmit(uint32_t src_id)
        {
            while (true)
            {
                std::vector<DataQueue> trans_copy;
                {
                    std::unique_lock<std::mutex> trans_lock(mtrans_locks_[src_id]);

                    mtrans_cvs_[src_id].wait(trans_lock);

                    std::swap(trans_copy, mtransmit_queue_[src_id]);
                }
                for (int dst_id = 0; dst_id < trans_copy.size(); dst_id++)
                {
                    if (trans_copy[dst_id].empty())
                        continue;

                    std::unique_lock<std::mutex> recv_lock(mrecv_locks_[dst_id]);

                    for (auto &data : trans_copy[dst_id])
                    {
                        mreceive_queue_[dst_id].push_back(data);
                    }

                    mrecv_cvs_[dst_id].notify_one();
                }
            }
        }

        void Receive(uint32_t self_id)
        {
            while (true)
            {
                DataQueue recv_copy;
                {
                    std::unique_lock<std::mutex> recv_lock(mrecv_locks_[self_id]);

                    mrecv_cvs_[self_id].wait(recv_lock);

                    std::swap(recv_copy, mreceive_queue_[self_id]);
                }

                for (auto &data : recv_copy)
                {
                    mrecv_callbacks_[self_id](data);
                }
            }
        }

    public:
        explicit Simulator(int node_num)
                :mgraph_(node_num)
        {}
        explicit Simulator(const Graph& graph)
                        :mgraph_(graph)
        {}
        void Start()
        {
            
        }
    };
}