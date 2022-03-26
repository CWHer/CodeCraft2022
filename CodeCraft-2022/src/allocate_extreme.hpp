// credit to Ruitao Zhu

#ifndef __ALLOCATE_EXTREME_HPP__
#define __ALLOCATE_EXTREME_HPP__

// Handling extreme requests
#include "common.h"
#include "graph.hpp"
#include "solution.hpp"

class ExtremeAllocator
{
public:
    enum class WeighType
    {
        sum = 0,
        average = 1
    };

private:
    // NOTE: < max_selected
    i32 max_selected;
    vector<i32> selected_num;

    const Graph &g;
    WeighType weigh_type;

    using QueTuple = tuple<i32, i32, i32>;
    // <band_request, time, server>
    std::priority_queue<QueTuple, vector<QueTuple>> extreme_que;

    // time * (server -> customer)
    vector<vector<vector<i32>>> time_edge;
    // customer -> server
    vector<vector<i32>> inv_edges;

    // time * { <customer, server, bandwidth> }
    vector<vector<tuple<i32, i32, i32>>> extreme_tuple;

    // requests[TimeNum][FringeNum]: 在 TimeNum 时刻对编号为 FringeNum 的边缘节点的带宽请求之和
    vector<vector<i32>> requests;
    //  times x customers
    vector<vector<i32>> demands;

    Solutions solutions;

private:
    void initQueue()
    {
        for (u32 t = 0; t < g.n_time; t++)
            for (u32 i = 0; i < g.n_server; i++)
                extreme_que.emplace(make_tuple(requests[t][i], t, i));
    }

    void toSolution()
    {
        for (u32 t = 0; t < g.n_time; t++)
        {
            Solution solution(g.n_customer);
            for (const auto &flow : extreme_tuple[t])
                solution.add(flow);
            solutions.add(solution);
        }
    }

    void popQueue()
    {
        u32 total_selected = 0;
        // time * n_server
        vector<vector<i32>> dirty_update;
        dirty_update.resize(g.n_time);
        for (u32 i = 0; i < g.n_time; i++)
            dirty_update[i].resize(g.n_server, 0);

        while (!extreme_que.empty() &&
               total_selected < g.n_server * max_selected)
        {
            i32 t, server;
            std::tie(std::ignore, t, server) = extreme_que.top();
            extreme_que.pop();

            // ignore lazy data
            if (dirty_update[t][server] > 0)
            {
                dirty_update[t][server]--;
                continue;
            }

            if (selected_num[server]++ >= max_selected)
                continue;

            total_selected++;
            // compute allocation
            i32 res_band = g.capacities[server];
            for (const auto &customer : time_edge[t][server])
            {
                auto allocation = std::min(
                    res_band, demands[t][customer]);
                if (allocation == 0)
                    continue;

                demands[t][customer] -= allocation;
                extreme_tuple[t].emplace_back(
                    make_tuple(customer, server, allocation));
                // lazy priority_queue update
                for (const auto &peer_server : inv_edges[customer])
                {
                    if (peer_server == server)
                        continue;

                    dirty_update[t][peer_server]++;
                    requests[t][peer_server] -= weigh_type == WeighType::average
                                                    ? allocation / inv_edges[customer].size()
                                                    : allocation;
                    extreme_que.emplace(
                        make_tuple(requests[t][peer_server], t, peer_server));
                }
            }
        }
    }

public:
    ExtremeAllocator(
        const Graph &g, WeighType weigh_type = WeighType::sum)
        : g(g), demands(g.demands), weigh_type(weigh_type),
          solutions(make_tuple(g.server_ids, g.customer_ids))
    {
        max_selected = std::floor(g.n_time * (1 - Settings::quantile));
        selected_num.resize(g.n_server, 0);
        time_edge.resize(g.n_time, g.edges);
        extreme_tuple.resize(g.n_time);

        inv_edges.resize(g.n_customer);
        for (u32 i = 0; i < g.n_server; i++)
            for (const auto &v : g.edges[i])
                inv_edges[v].push_back(i);

        // initialize requests (average version)
        for (u32 t = 0; t < g.n_time; t++)
        {
            vector<i32> request;
            for (const auto &edge : g.edges)
            {
                i32 bandwidth = 0;
                for (const auto &v : edge)
                    bandwidth += weigh_type == WeighType::average
                                     ? g.demands[t][v] / inv_edges[v].size()
                                     : g.demands[t][v];
                request.emplace_back(bandwidth);
            }
            requests.emplace_back(request);
        }
    }

    void run()
    {
        initQueue();
        popQueue();
        toSolution();

        printInfo("ExtremeAllocator finish.");
    }

    Solutions getSolution() { return solutions; }
};

#endif