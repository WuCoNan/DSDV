#include "Simulator.hpp"
/*
std::vector<std::vector<uint32_t>> edges={
    {0,1,1},
    {0,2,3},
    {0,3,3},
    {1,2,2},
    {1,4,2},
    {1,5,4},
    {2,3,2},
    {2,5,1},
    {3,6,2},
    {4,5,2},
    {5,6,1}
};
*/

std::vector<std::vector<uint32_t>> edges=
{
    {0,1,1},
    {0,2,3},
    {0,3,3},
    {0,5,4},
    {1,2,2},
    {1,4,2},
    {2,3,3},
    {5,6,1},
    {5,7,3},
    {5,8,3},
    {6,7,2},
    {6,9,2},
    {7,8,3}
};
std::vector<std::vector<int>> modify_edges={
    {1,5,1,5000},
    {1,5,UNREACHABLE,10000}
};


util::Graph GenerateGraph(const std::vector<std::vector<uint32_t>>& edges)
{
    util::Graph graph;
    for(auto& edge:edges)
    {
        auto s=edge[0],t=edge[1],metric=edge[2];
        if(s>=graph.size())
            graph.resize(s+1);
        graph[s].push_back({t,metric});
        if(t>=graph.size())
            graph.resize(t+1);
        graph[t].push_back({s,metric});
    }
    return graph;
}
int main()
{
    auto graph=GenerateGraph(edges);

    simulator::Simulator sim(graph);
    //sim.SetModifyLinks(modify_edges);
    sim.Start();
    
    return 0;
}