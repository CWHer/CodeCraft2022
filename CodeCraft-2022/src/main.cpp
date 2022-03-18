#include "graph.hpp"
#include "flow_graph.hpp"
#include "dinic.hpp"

int main()
{
    Graph g;
    g.display();
    FlowGraph fg(g);
    fg.display();
    Dinic solver(fg);

    auto answer = getFeasibleSol(fg, g.getTime(), g.getCapacity());

    printError(!answer.first, "no solution");

    std::ofstream f_out("solution.txt");
    f_out << answer.second;
    f_out.close();

    return 0;
}
