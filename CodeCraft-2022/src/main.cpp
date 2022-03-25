#include "graph.hpp"
#include "flow_graph.hpp"
#include "dinic.hpp"
#include "min_max.hpp"
#include "allocate_extreme.hpp"

int main()
{
    Graph g("./data");
    g.display();
    ExtremeAllocator allocator(g);
    allocator.run();
    Solutions partial_sol = allocator.getSolution();

    printStats("partial solution:", std::get<1>(partial_sol.evaluate()));

    FlowGraph fg(g);
    fg.display();

    // BUG: FIXME: this is error, do not include partial solution.
    MinMax minmax_solver(g, fg);
    auto solutions = minmax_solver.run();

    std::ofstream f_out("solution.txt");
    f_out << solutions;
    f_out.close();

    return 0;
}
