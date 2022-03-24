#ifndef __FLOW_GRAPH_HPP__
#define __FLOW_GRAPH_HPP__

#include "common.h"
#include "graph.hpp"
#include "solution.hpp"
#include "allocate_extreme.hpp"

class Dinic;
class Greedy;
class ExtremeAllocator;

class FlowGraph
{
    friend class Dinic;
    friend class Greedy;

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

    // 0-based
    inline i32 serverNode(i32 k) { return k + 1; }
    inline i32 customerNode(i32 k) { return k + n_server + 1; }

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

        for (i32 i = 0; i < n_server; ++i)
            addEdges(s_node, serverNode(i), g.capacities[i]);

        for (i32 i = 0; i < n_server; ++i)
            for (const auto &j : g.edges[i])
                addEdges(serverNode(i), customerNode(j),
                         std::numeric_limits<i32>::max());

        for (i32 i = 0; i < n_customer; ++i)
        {
            // NOTE: invoke changeDemand before solving
            addEdges(customerNode(i), t_node, 0);
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
        for (i32 i = 0; i < n_customer; ++i)
        {
            auto &e = edges[head[customerNode(i)]];
            e.cap = e.ori_cap = demands[t][i];
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

    // NOTE: this operation is time-consuming
    void fixPartialSol(const Solution &sol)
    {
        // NOTE: new sol doesn't contain solution here
        unordered_map<i32, i32> fixed_edges;
        for (u32 i = 0; i < n_customer; ++i)
            for (const auto &flow : sol.solution[i])
            {
                fixed_edges[serverNode(flow.first)] += flow.second;
                fixed_edges[customerNode(i)] += flow.second;
            }

        for (i32 i = head[s_node]; ~i; i = edges[i].nxt)
        {
            auto &e = edges[i];
            if (fixed_edges.count(e.v) > 0)
                e.cap = e.ori_cap -= fixed_edges[e.v];
        }
        for (i32 i = 0; i < n_customer; ++i)
        {
            auto &e = edges[head[customerNode(i)]];
            if (fixed_edges.count(e.v) > 0)
                e.cap = e.ori_cap -= fixed_edges[e.v];
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

    pair<bool, Solution> getGreedySol(
        vector<vector<int>> &costs, int valid_time,
        const vector<vector<double>> &weights,
        const vector<std::unordered_set<int>> &res,
        const vector<vector<Allocation>> &exres)
    {

        Solution solution(n_customer);
        vector<int> cur_demands(demands[t]);
        vector<int> remained_capacity(n_server);
        vector<pair<double, int>> available;

        // calculate remained capacity for all servers
        for (i32 j = head[s_node]; ~j; j = edges[j].nxt)
        {
            Edge *e = &(edges[j]);
            remained_capacity[e->v - 1] = e->ori_cap;
        }

        // preallocate based on AllocateExtreme
        for (auto &allo : exres[t])
        {
            cur_demands[allo.client] -= allo.bandwidth;
            solution.add(make_tuple(allo.client, allo.server, allo.bandwidth));
        }

        // allocate requirements for each customer
        for (i32 i = 1; i <= n_customer; ++i)
        {
            available.clear();
            // get all available servers for customer i(add into 'available')
            double total_weight = 0;
            for (i32 j = head[i + n_server]; ~j; j = edges[j].nxt)
            {
                const Edge &e = edges[j];
                if (e.v < n_server)
                {
                    available.emplace_back(weights[t][e.v - 1], e.v - 1);
                    total_weight += weights[t][e.v - 1];
                }
            }
            // add edges
            int unfinished = cur_demands[i - 1];
            for (int j = 0; j < available.size(); ++j) // for all possible edges
            {
                const pair<double, int> &pr = available[j];
                int cur_demand = (j == available.size() - 1) ? unfinished : (cur_demands[i - 1] * pr.first / total_weight); // edge value
                int target_s = pr.second;                                                                                   // edge target
                if (cur_demand == 0)
                    continue;

                // check flow amount
                if (remained_capacity[pr.second] >= cur_demand) // available for cur_demand
                {
                    unfinished -= cur_demand;
                    remained_capacity[pr.second] -= cur_demand;
                }
                else // unavailable, then flow into e as much as possible
                {
                    unfinished -= remained_capacity[pr.second];
                    cur_demand = remained_capacity[pr.second];
                    remained_capacity[pr.second] = 0;
                }

                solution.add(make_tuple(i - 1, pr.second, cur_demand)); // add into solutions
                costs[t][pr.second] += cur_demand;                      // update costs
            }
            if (unfinished != 0) // infeasible dispatch
                return make_pair(false, solution);
        }

        return make_pair(true, solution);
    }
};

#endif