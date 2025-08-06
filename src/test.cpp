#include "Simulator.hpp"




util::Graph GenerateGraph()
{
    util::Graph graph(2);
    graph[0].push_back({1,5});
    graph[1].push_back({0,5});
    return graph;
}
int main()
{
    auto graph=GenerateGraph();

    simulator::Simulator sim(graph);
    sim.Start();
    
    return 0;
}