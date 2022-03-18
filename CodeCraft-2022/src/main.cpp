#include "graph.hpp"
#include "flow_graph.hpp"
#include "dinic.hpp"
#include "min_max.hpp"

int main()
{
    Graph g("data");
    g.display();
    FlowGraph flow_g(g);
    flow_g.display();
    Dinic solver(flow_g);

    // >>> feasible solution
    // auto answer = getFeasibleSol(
    //     flow_g, g.getTime(), g.getCapacity());
    // printError(!answer.first, "no solution");

    // std::ofstream f_out("solution.txt");
    // f_out << answer.second;
    // f_out.close();
    // <<< feasible solution

    MinMax minmax_solver(g, flow_g);
    auto solutions = minmax_solver.run();

    std::ofstream f_out("solution.txt");
    f_out << solutions;
    f_out.close();

    return 0;
}
