#include "graph.hpp"
#include "flow_graph.hpp"
#include "dinic.hpp"
#include "allocate_extreme.hpp"
#include "greedy.hpp"

int main()
{
    // for submission:
    /*
    Graph g("/data");
    AllocateExtreme ae(0.13, "/data");
    std::ofstream f_out("/output/solution.txt");
    */

    // for testing:

    Graph g;
    ExtremeAllocator allocator(0.1, g);

    g.display();
    FlowGraph fg(g);
    fg.display();
    Dinic solver(fg);

    // get answer
    Greedy gy(fg, g.getTime());
    auto answer = gy.recurrent_search(fg, g.getCapacity(), allocator);

    printError(!answer.first, "no solution");

    std::ofstream f_out("solution.txt");
    f_out << answer.second;
    f_out.close();

    return 0;
}
