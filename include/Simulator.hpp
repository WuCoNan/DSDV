#pragma once
#include "Node.hpp"
#include <mutex>
#include <condition_variable>

namespace simulator
{
    class Simulator
    {
    private:
        using Data = std::deque<std::byte>;
        using DataQueue = std::deque<Data>;
        using ReadCallback = std::function<void(Data)>;
        using AddLinksCallback = std::function<void(const std::unordered_map<uint32_t, uint32_t> &)>;
        using RemoveLinksCallback = std::function<void(const std::unordered_map<uint32_t, uint32_t> &)>;

        std::vector<std::vector<DataQueue>> mtransmit_queue_;
        std::vector<DataQueue> mreceive_queue_;
        std::vector<std::mutex> mtrans_locks_, mrecv_locks_;
        std::vector<std::condition_variable> mtrans_cvs_, mrecv_cvs_;

        util::Graph mgraph_;

        std::vector<Node *> mnodes_;

        std::vector<ReadCallback> mread_callbacks_;
        std::vector<AddLinksCallback> madd_links_callbacks_, mremove_links_callbacks_;

        std::vector<std::thread> mtrans_threads_, mrecv_threads_;

        std::vector<std::vector<int>> mchanged_conns_;
        int mchanged_conns_index_ = 0;

        void Transmit(uint32_t src_id)
        {
            while (true)
            {
                std::vector<DataQueue> trans_copy(mnodes_.size());
                {
                    std::unique_lock<std::mutex> trans_lock(mtrans_locks_[src_id]);

                    mtrans_cvs_[src_id].wait(trans_lock, [src_id, this]()
                                             { return !Empty(mtransmit_queue_[src_id]); });

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
                    // std::cout<<"simulator push data from   "<<src_id<<"   to   "<<dst_id<<std::endl;

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

                    mrecv_cvs_[self_id].wait(recv_lock, [self_id, this]()
                                             { return !mreceive_queue_[self_id].empty(); });

                    std::swap(recv_copy, mreceive_queue_[self_id]);
                }
                // std::cout<<"simulator receive data in   "<<self_id<<std::endl;
                for (auto &data : recv_copy)
                {
                    mread_callbacks_[self_id](data);
                }
            }
        }
        void TransThread(uint32_t src_id)
        {
            Transmit(src_id);
        }
        void RecvThread(uint32_t self_id)
        {
            std::unordered_map<uint32_t, uint32_t> adjacent_table;
            for (util::Edge &edge : mgraph_[self_id])
            {
                adjacent_table[edge.dst_id] = edge.metric;
            }
            madd_links_callbacks_[self_id](adjacent_table);
            Receive(self_id);
        }
        bool Empty(const std::vector<DataQueue> &queue) const
        {
            for (auto &elem : queue)
            {
                if (!elem.empty())
                    return false;
            }
            return true;
        }

    public:
        explicit Simulator(int node_num)
            : mgraph_(node_num)
        {
        }
        explicit Simulator(const util::Graph &graph)
            : mgraph_(graph), mtransmit_queue_(graph.size(), std::vector<DataQueue>(graph.size())), mtrans_locks_(graph.size()), mtrans_cvs_(graph.size()), mrecv_locks_(graph.size()), mreceive_queue_(graph.size()), mrecv_cvs_(graph.size()), mread_callbacks_(graph.size()), mtrans_threads_(graph.size()), mrecv_threads_(graph.size()), mnodes_(graph.size()), madd_links_callbacks_(graph.size()), mremove_links_callbacks_(graph.size())
        {
            for (int cur_node = 0; cur_node < mgraph_.size(); cur_node++)
            {
                mnodes_[cur_node] = new Node{(uint32_t)cur_node, this};
            }
        }
        void Start()
        {
            for (int cur_node = 0; cur_node < mnodes_.size(); cur_node++)
            {
                mtrans_threads_[cur_node] = std::thread([this, cur_node]()
                                                        { this->TransThread(cur_node); });
                mrecv_threads_[cur_node] = std::thread([this, cur_node]()
                                                       { this->RecvThread(cur_node); });
            }
            auto modify_thread=std::thread(std::bind(&Simulator::ModifyThread,this));

            for(int cur_node=0;cur_node<mnodes_.size();cur_node++)
            {
                mnodes_[cur_node]->BussinessStart();
            }

            while (true)
                ;
        }

        void PushTransQueue(uint32_t src_id, uint32_t dst_id, const Data &bytes)
        {
            // std::cout<<"simulator push data to transmit queue   "<<src_id<<"------>"<<dst_id<<std::endl;

            std::unique_lock<std::mutex> u_lock(mtrans_locks_[src_id]);
            mtransmit_queue_[src_id][dst_id].push_back(bytes);
            mtrans_cvs_[src_id].notify_one();
        }
        void RegisterReadCallback(uint32_t node_id, ReadCallback read_callback)
        {
            mread_callbacks_[node_id] = read_callback;
        }
        void RegisterAddLinksCallback(uint32_t node_id, AddLinksCallback addlinks_callback)
        {
            madd_links_callbacks_[node_id] = addlinks_callback;
        }
        void RegisterRemoveLinksCallback(uint32_t node_id, RemoveLinksCallback removelinks_callback)
        {
            mremove_links_callbacks_[node_id] = removelinks_callback;
        }
        int ModifyLinks()
        {
            auto time = mchanged_conns_[mchanged_conns_index_][3];
            bool flag=true;
            while (mchanged_conns_index_ < mchanged_conns_.size() && (flag || mchanged_conns_[mchanged_conns_index_][3] == mchanged_conns_[mchanged_conns_index_ - 1][3]))
            {
                flag=false;
                auto s = mchanged_conns_[mchanged_conns_index_][0], t = mchanged_conns_[mchanged_conns_index_][1], metric = mchanged_conns_[mchanged_conns_index_][2];
                std::unordered_map<uint32_t, uint32_t> modify_links1, remove_links1, modify_links2, remove_links2;
                if (metric == UNREACHABLE)
                {
                    remove_links1[s] = metric;
                    remove_links2[t] = metric;
                }
                else
                {
                    modify_links1[s] = metric;
                    modify_links2[t] = metric;
                }
                //std::cout<<mchanged_conns_index_<<std::endl;
                madd_links_callbacks_[t](modify_links1);
                madd_links_callbacks_[s](modify_links2);
                mremove_links_callbacks_[t](remove_links1);
                mremove_links_callbacks_[s](remove_links2);

                mchanged_conns_index_++;
            }

            if (mchanged_conns_index_ == mchanged_conns_.size())
                return -1;
            std::cout<<"next time:  "<<mchanged_conns_[mchanged_conns_index_][3] - time<<std::endl;
            return mchanged_conns_[mchanged_conns_index_][3] - time;
        }
        void ModifyThread()
        {
            if(mchanged_conns_.size()==0)
                return;
            std::this_thread::sleep_until(std::chrono::steady_clock::now()+std::chrono::milliseconds(mchanged_conns_[0][3]));
            while(true)
            {
                int next_time=ModifyLinks();
                if(next_time<0)
                {
                    std::cout<<"return"<<std::endl;
                    return;
                }
                    
                std::this_thread::sleep_until(std::chrono::steady_clock::now()+std::chrono::milliseconds(next_time));
            }
        }
        void SetModifyLinks(const std::vector<std::vector<int>>& modify_links)
        {
            this->mchanged_conns_=modify_links;
        }
    };
}