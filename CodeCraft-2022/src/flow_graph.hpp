#ifndef __FLOW_GRAPH_HPP__
#define __FLOW_GRAPH_HPP__

#include "common.h"
#include "graph.hpp"
#include "solution.hpp"

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
        s_node = 0, t_node = n_node - 1;

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

    tuple<vector<string>, vector<string>> getNames()
    {
        return make_tuple(server_ids, customer_ids);
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

    void changeCapacity(const vector<i32> &capacities)
    {
        printError(
            capacities.size() != n_server,
            "invalid capacity");

        for (i32 i = head[s_node]; ~i; i = edges[i].nxt)
        {
            auto &e = edges[i];
            e.cap = e.ori_cap = capacities[e.v - 1];
        }
    }

    Solution getSolution()
    {
        Solution solution(n_customer);

        for (i32 i = 1; i <= n_customer; ++i)
            for (i32 j = head[i + n_server]; ~j; j = edges[j].nxt)
            {
                const auto &e = edges[j];
                if (e.v != t_node && e.cap > 0)
                    solution.add(make_tuple(i - 1, e.v - 1, e.cap));
            }
        return solution;
    }

    // DEBUG function
    void display()
    {
        printInfo(
            "Nodes: " + std::to_string(n_node) + ", " +
            "Edges: " + std::to_string(edges.size()));
    }
};

#endif