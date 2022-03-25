#include "graph.hpp"
#include "flow_graph.hpp"
#include "dinic.hpp"
#include "allocate_extreme.hpp"

int main()
{
    Graph g("./data");
    g.display();
    ExtremeAllocator allocator(1, g);
    allocator.run();
    Solutions partial_sol = allocator.getSolution();

    printStats("partial solution:", std::get<1>(partial_sol.evaluate()));

    // FlowGraph fg(g);
    // fg.display();
    // Dinic solver(fg);

    // get answer

    // printError(!answer.first, "no solution");

    // std::ofstream f_out("solution.txt");
    // f_out << answer.second;
    // f_out.close();

    return 0;
}
