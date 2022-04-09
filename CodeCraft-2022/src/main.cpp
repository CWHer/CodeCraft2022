#include "graph.hpp"
#include "flow_graph.hpp"
#include "dinic.hpp"
#include "min_max.hpp"
#include "allocate_extreme.hpp"
#include "greedy.hpp"

int main()
{
    Graph g("/data");
    g.display();
    ExtremeAllocator allocator(
        g, ExtremeAllocator::WeighType::sum);
    allocator.run();
    Solutions partial_sol = allocator.getSolution();

    printStats("partial solution:", std::get<1>(partial_sol.evaluate()));

    FlowGraph fg(g);
    fg.display();

    Greedy greedy_solver(fg, g.getTime());
    auto max_flow = greedy_solver.run(fg, g.getCapacity(), partial_sol);

    // BUG: FIXME: this is error, do not include partial solution.
    MinMax minmax_solver(
        g, fg, partial_sol, max_flow);
    auto solutions = minmax_solver.run();

    solutions += partial_sol;
    printStats("final solution:", std::get<1>(solutions.evaluate()));

    std::ofstream f_out("/output/solution.txt");
    f_out << solutions;
    f_out.close();

    return 0;
}
