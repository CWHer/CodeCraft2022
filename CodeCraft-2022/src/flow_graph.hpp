#ifndef __FLOW_GRAPH_HPP__
#define __FLOW_GRAPH_HPP__

#include "common.h"
#include "graph.hpp"

class Dinic;

class FlowGraph
{
    friend class Dinic;

private:
    i32 t; // current timestep

    i32 n_server, n_customer, n_node;
    i32 s_node, t_node;

    struct Edge
    {
        i32 v;       // point to v
        i32 ori_cap; // only used in reset()
        i32 cap;     // rest capicity, i.e. original capacity - flow
        i32 nxt;     // next edge
    };

    // adjacent list
    vector<i32> head;
    // edges
    vector<Edge> edges;

    vector<vector<i32>> demands;
    vector<string> server_ids;
    vector<string> customer_ids;

private:
    void addEdge(i32 u, i32 v, i32 cap)
    {
        Edge e = {v, cap, cap, head[u]};
        edges.emplace_back(e);
        head[u] = edges.size() - 1;
    }

    void addEdges(i32 u, i32 v, i32 cap)
    {
        addEdge(u, v, cap);
        addEdge(v, u, 0);
    }

public:
    FlowGraph(Graph &g)
    {
        t = -1;
        n_server = g.n_server;
        n_customer = g.n_customer;
        n_node = n_server + n_customer + 2;
        // server: 1 ~ n_server
        // customer: n_server + 1 ~ n_server + n_customer
        s_node = 0;
        t_node = n_server + n_customer + 1;

        head.resize(n_node, -1);
        demands = std::move(g.demands);
        server_ids = std::move(g.server_ids);
        customer_ids = std::move(g.customer_ids);

        for (i32 i = 1; i <= n_server; ++i)
            addEdges(s_node, i, g.capacities[i - 1]);

        for (i32 i = 1; i <= n_server; ++i)
            for (const auto &j : g.edges[i - 1])
                addEdges(i, n_server + j + 1,
                         std::numeric_limits<i32>::max());

        for (i32 i = n_server + 1; i < t_node; ++i)
        {
            // NOTE: invoke changeDemand before solving
            addEdges(i, t_node, 0);
        }
    }

    void reset()
    {
        // reset edges
        for (auto &e : edges)
            e.cap = e.ori_cap;
    }

    u64 changeDemand(i32 t)
    {
        printError(
            t < 0 || t >= demands.size(),
            "no such time");

        this->t = t;
        u64 demand_sum = 0;
        for (i32 i = 1; i <= n_customer; ++i)
        {
            auto &e = edges[head[i + n_server]];
            e.cap = e.ori_cap = demands[t][i - 1];
            demand_sum += e.cap;
        }
        return demand_sum;
    }

    void display()
    {
        printInfo(
            "Nodes: " + std::to_string(n_node) + ", " +
            "Edges: " + std::to_string(edges.size()));
    }

    tuple<u32, vector<string>> getSolution()
    {
        vector<string> solution;

        auto format = [&](i32 k, i32 flow, bool add_sep)
        { return (add_sep ? ",<" : "<") + server_ids[k] + "," + std::to_string(flow) + ">"; };

        for (i32 i = 1; i <= n_customer; ++i)
        {
            string line = customer_ids[i - 1] + ":";
            for (i32 j = head[i + n_server]; ~j; j = edges[j].nxt)
            {
                const auto &e = edges[j];
                if (e.v != t_node && e.cap > 0)
                    line += format(e.v - 1, e.cap, line.back() == '>');
            }
            solution.emplace_back(line);
        }

        return make_tuple(t, solution);
    }
};

#endif