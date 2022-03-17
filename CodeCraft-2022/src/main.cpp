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

    vector<string> solutions;
    auto n_time = g.getTime();
    for (u32 t = 0; t < n_time; ++t)
    {
        // find feasible solution at time t
        auto demand_sum = fg.changeDemand(t);
        fg.reset();
        auto max_flow = solver.run();
        printError(max_flow != demand_sum, "invalid solution");

        u32 timestep;
        vector<string> solution;
        std::tie(timestep, solution) = std::move(fg.getSolution());
        printError(t != timestep, "invalid time");
        solutions.insert(solutions.end(), solution.begin(), solution.end());
    }

    std::ofstream f_out("solution.txt");
    for (const auto &s : solutions)
        f_out << s << std::endl;
    f_out.close();

    return 0;
}
