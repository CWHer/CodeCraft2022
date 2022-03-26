// credit to Ruitao Zhu

#ifndef __ALLOCATE_EXTREME_HPP__
#define __ALLOCATE_EXTREME_HPP__

// Handling extreme requests
#include "common.h"
#include "graph.hpp"
#include "solution.hpp"

using namespace std;

struct QueCmp
{
    bool operator()(tuple<i32, i32, i32> x, tuple<i32, i32, i32> y)
    {
        return std::get<2>(x) < std::get<2>(y);
    }
};

class ExtremeAllocator
{
private:
    // size = n_server
    vector<i32> ratio_flag;
    // <time, server, band_request>
    priority_queue<tuple<i32, i32, i32>, vector<tuple<i32, i32, i32>>, QueCmp> extreme_que;
    // time * (server -> customer)
    vector<vector<vector<i32>>> time_edge;
    // customer -> server
    vector<vector<i32>> inv_edges;
    // time* { <server,customer,bandwidth> }
    vector<vector<std::tuple<i32, i32, i32>>> extreme_tuple;

    // Get Solution
    Solutions solutions;

    // requests[TimeNum][FringeNum]: �� TimeNum ʱ�̶Ա��Ϊ FringeNum �ı�Ե�ڵ�Ĵ�������֮��
    vector<vector<i32>> requests;

    const Graph &g;

    //  times x customers
    vector<vector<i32>> demands;

private:
    void initQueue()
    {
        for (i32 time = 0; time < requests.size(); time++)
        {
            for (i32 server = 0; server < requests[time].size(); server++)
            {
                extreme_que.push(std::make_tuple(time, server, requests[time][server]));
            }
        }
    }

    void popQueue()
    {
        i32 total_select = 0;
        // time * n_server
        vector<vector<int>> dirty_update;
        dirty_update.resize(g.n_time);
        for (i32 i = 0; i < g.n_time; i++)
        {
            dirty_update[i].resize(g.n_server);
            for (i32 j = 0; j < g.n_server; j++)
            {
                dirty_update[i][j] = 0;
            }
        }
        while (!extreme_que.empty())
        {
            if (total_select >= g.n_server * g.n_time * 0.05)
                break;
            tuple<i32, i32, i32> node = extreme_que.top();
            i32 server = std::get<1>(node);
            i32 time = std::get<0>(node);
            i32 band_request = std::get<2>(node);
            extreme_que.pop();
            if (dirty_update[time][server] > 0)
            {
                dirty_update[time][server]--;
                continue;
            }
            if (ratio_flag[server] >= g.n_time * 0.05)
                continue;
            ratio_flag[server]++;
            total_select++;
            // compute alocation
            i32 res_band = g.capacities[server];
            for (i32 customer : time_edge[time][server])
            {
                if (res_band >= demands[time][customer])
                {
                    res_band -= demands[time][customer];
                    extreme_tuple[time].emplace_back(make_tuple(server, customer, demands[time][customer]));
                    // lazy priority_queue update
                    for (i32 server_update : inv_edges[customer])
                    {
                        if (server_update == server)
                            continue;

                        auto iter = std::remove(time_edge[time][server_update].begin(), time_edge[time][server_update].end(), customer);
                        time_edge[time][server_update].erase(iter, time_edge[time][server_update].end());

                        dirty_update[time][server_update]++;
                        requests[time][server_update] -= demands[time][customer] / float(inv_edges[customer].size());
                        extreme_que.push(make_tuple(time, server_update, requests[time][server_update]));
                    }
                }
                else if (res_band > 0)
                {
                    demands[time][customer] -= res_band;
                    extreme_tuple[time].emplace_back(make_tuple(server, customer, res_band));
                    // lazy priority_queue update
                    for (i32 server_update : inv_edges[customer])
                    {
                        if (server_update == server)
                            continue;
                        dirty_update[time][server_update]++;
                        requests[time][server_update] -= res_band / float(inv_edges[customer].size());
                        extreme_que.push(make_tuple(time, server_update, requests[time][server_update]));
                    }
                    break;
                }
                else
                    break;
            }
        }
    }

    void toSolution()
    {
        for (i32 time = 0; time < g.n_time; time++)
        {
            Solution cursolution(g.n_customer);
            for (i32 i = 0; i < extreme_tuple[time].size(); i++)
            {
                tuple<i32, i32, i32> node = extreme_tuple[time][i];
                cursolution.add(make_tuple(std::get<1>(node), std::get<0>(node), std::get<2>(node)));
            }
            solutions.add(cursolution);
        }
    }

public:
    ExtremeAllocator(const Graph &g)
        : g(g), demands(g.demands),
          solutions(make_tuple(g.server_ids, g.customer_ids))
    {
        ratio_flag.resize(g.n_server);
        for (i32 i = 0; i < ratio_flag.size(); i++)
            ratio_flag[i] = 0;

        extreme_tuple.resize(g.n_time);

        inv_edges.resize(g.n_customer);
        for (i32 i = 0; i < g.n_server; i++)
            for (const auto &v : g.edges[i])
                inv_edges[v].push_back(i);

        time_edge.resize(g.n_time);
        for (i32 time = 0; time < g.n_time; time++)
        {
            time_edge[time] = g.edges;
        }

        // initiate requests (average version)
        for (i32 t = 0; t < g.n_time; t++)
        {
            vector<i32> request;
            for (const auto &edge : g.edges)
            {
                i32 bandwidth = 0;
                for (const auto &v : edge)
                    bandwidth += g.demands[t][v] / float(inv_edges[v].size());
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
    }

    Solutions getSolution()
    {
        return this->solutions;
    }
};

#endif