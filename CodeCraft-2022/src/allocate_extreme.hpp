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
        for (i32 t = 0; t < g.n_time; t++)
            for (i32 i = 0; i < g.n_server; i++)
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
        i32 total_selected = 0;
        // time * n_server
        vector<vector<i32>> dirty_update;
        dirty_update.resize(g.n_time);
        for (i32 i = 0; i < g.n_time; i++)
            dirty_update[i].resize(g.n_server, 0);

        while (!extreme_que.empty() &&
               total_selected < g.n_server * max_selected)
        {
            i32 server, time;
            std::tie(std::ignore, time, server) = extreme_que.top();
            extreme_que.pop();

            // ignore lazy data
            if (dirty_update[time][server] > 0)
            {
                dirty_update[time][server]--;
                continue;
            }

            if (selected_num[server]++ >= max_selected)
                continue;

            total_selected++;
            // compute allocation
            i32 res_band = g.capacities[server];
            for (i32 customer : time_edge[time][server])
            {
                if (res_band >= demands[time][customer])
                {
                    res_band -= demands[time][customer];
                    extreme_tuple[time].emplace_back(make_tuple(customer, server, demands[time][customer]));
                    // lazy priority_queue update
                    for (i32 server_update : inv_edges[customer])
                    {
                        if (server_update == server)
                            continue;

                        auto iter = std::remove(time_edge[time][server_update].begin(), time_edge[time][server_update].end(), customer);
                        time_edge[time][server_update].erase(iter, time_edge[time][server_update].end());

                        dirty_update[time][server_update]++;
                        requests[time][server_update] -= weigh_type == WeighType::sum
                                                             ? demands[time][customer]
                                                             : demands[time][customer] / float(inv_edges[customer].size());
                        extreme_que.emplace(make_tuple(requests[time][server_update], time, server_update));
                    }
                    demands[time][customer] = 0;
                }
                else if (res_band > 0)
                {
                    demands[time][customer] -= res_band;
                    extreme_tuple[time].emplace_back(make_tuple(customer, server, res_band));
                    // lazy priority_queue update
                    for (i32 server_update : inv_edges[customer])
                    {
                        if (server_update == server)
                            continue;
                        dirty_update[time][server_update]++;
                        requests[time][server_update] -= weigh_type == WeighType::sum
                                                             ? res_band
                                                             : res_band / float(inv_edges[customer].size());
                        extreme_que.emplace(make_tuple(requests[time][server_update], time, server_update));
                    }
                    break;
                }
                else
                    break;
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
        for (i32 i = 0; i < g.n_server; i++)
            for (const auto &v : g.edges[i])
                inv_edges[v].push_back(i);

        // initialize requests (average version)
        for (i32 t = 0; t < g.n_time; t++)
        {
            vector<i32> request;
            for (const auto &edge : g.edges)
            {
                i32 bandwidth = 0;
                for (const auto &v : edge)
                    bandwidth += weigh_type == WeighType::sum
                                     ? g.demands[t][v]
                                     : g.demands[t][v] / inv_edges[v].size();
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