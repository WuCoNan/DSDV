#include "Simulator.hpp"




util::Graph GenerateGraph()
{
    util::Graph graph(3);
    graph[0].push_back({1,5});
    graph[1].push_back({0,5});
    graph[1].push_back({2,3});
    graph[2].push_back({1,3});
    return graph;
}
int main()
{
    auto graph=GenerateGraph();

    simulator::Simulator sim(graph);
    sim.Start();
    
    return 0;
}