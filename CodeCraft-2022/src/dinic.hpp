#ifndef __DINIC_HPP__
#define __DINIC_HPP__

#include "common.h"
#include "flow_graph.hpp"
#include "solution.hpp"

class Dinic
{
private:
    FlowGraph *g;

    vector<i32> depth;
    vector<i32> cur_head;
    queue<i32> q;

private:
    bool BFS()
    {
        std::fill(depth.begin(), depth.end(), 0);
        depth[g->s_node] = 1;

        q.push(g->s_node);
        while (!q.empty())
        {
            auto x = q.front();
            q.pop();
            for (i32 i = g->head[x]; ~i; i = g->edges[i].nxt)
            {
                const auto &e = g->edges[i];
                if (!depth[e.v] && e.cap > 0)
                {
                    depth[e.v] = depth[x] + 1;
                    q.push(e.v);
                }
            }
        }

        return depth[g->t_node] > 0;
    }

    u64 DFS(i32 x, u64 cur_flow)
    {
        if (cur_flow == 0 || x == g->t_node)
            return cur_flow;

        if (depth[x] >= depth[g->t_node])
            return 0;

        u64 ret = 0;
        for (auto &i = cur_head[x]; ~i; i = g->edges[i].nxt)
        {
            auto &e = g->edges[i];
            auto &inv_e = g->edges[i ^ 1];

            if (depth[x] + 1 == depth[e.v])
            {
                u64 new_flow = DFS(
                    e.v, std::min(cur_flow, (u64)e.cap));
                e.cap -= new_flow;
                inv_e.cap += new_flow;
                ret += new_flow;
                cur_flow -= new_flow;
            }

            if (cur_flow == 0)
                break;
        }

        return ret;
    }

public:
    Dinic(FlowGraph &g)
    {
        this->g = &g;
        depth.resize(g.n_node, 0);
    }

    u64 run()
    {
        printError(g->t == -1, "invalid time");

        u64 max_flow = 0;
        while (BFS())
        {
            cur_head = g->head;
            max_flow += DFS(
                g->s_node, std::numeric_limits<u64>::max());
        }
        return max_flow;
    }
};

pair<bool, Solutions> getFeasibleSol(
    FlowGraph &g, u32 n_time, const vector<i32> &capacities)
{
    // initialize solver
    Dinic solver(g);
    g.changeCapacity(capacities);

    Solutions solutions(g.getNames()); // store solutions
    for (u32 t = 0; t < n_time; ++t)
    {
        // find feasible solution at time t
        auto demand_sum = g.changeDemand(t);
        g.reset();
        auto max_flow = solver.run();

        if (max_flow != demand_sum)
            return make_pair(false, solutions);

        auto solution = std::move(g.getSolution());
        solutions.add(solution);
    }

    return make_pair(true, solutions);
}

#endif