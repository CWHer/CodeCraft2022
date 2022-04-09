#include "graph.hpp"
#include "flow_graph.hpp"
#include "dinic.hpp"
#include "min_max.hpp"
#include "allocate_extreme.hpp"
#include "greedy.hpp"

int main()
{
    Graph g("./data");
    ExtremeAllocator allocator(
        g, ExtremeAllocator::WeighType::sum);
    allocator.run();
    Solutions partial_sol = allocator.getSolution();

    FlowGraph fg(g);

    Greedy greedy_solver(fg, g.getTime());
    auto max_flow = greedy_solver.run(fg, g.getCapacity(), partial_sol);

    std::ofstream f_out("/output/solution.txt");
    f_out << solutions;
    f_out.close();

    return 0;
}
