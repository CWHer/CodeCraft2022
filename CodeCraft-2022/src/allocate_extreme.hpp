// "This is not written by me....", said CWHer.
// "And I am NOT responsible for this code."

#ifndef __ALLOCATE_EXTREME_HPP__
#define __ALLOCATE_EXTREME_HPP__

// Handling extreme requests
#include "common.h"
#include "graph.hpp"
#include "solution.hpp"

struct Allocation
{
    i32 server;
    i32 client;
    i32 bandwidth;
};

class ExtremeAllocator
{
private:
    // Get Solution
    Solutions solutions;
    
    f32 peak_ratio;
    // requests[TimeNum][FringeNum]: 在 TimeNum 时刻对编号为 FringeNum 的边缘节点的带宽请求之和
    vector<vector<i32>> requests;

    const Graph &g;

    vector<unordered_set<i32>> conflicts;    // 每个边缘节点的冲突集合
    vector<unordered_set<i32>> max_requests; // 每个边缘节点的 TOP 时刻集合

private:
    void getExtreme()
    {
        i32 select_time = g.n_time * peak_ratio;
        for (i32 i = 0; i < g.n_server; i++)
        {
            unordered_set<i32> times;
            vector<pair<i32, i32>> fringes; // <bandwidth,time>
            for (i32 j = 0; j < g.n_time; j++)
                fringes.emplace_back(make_pair(requests[j][i], j));

            std::sort(fringes.rbegin(), fringes.rend());
            for (i32 k = 0; k < select_time; k++)
                times.insert(fringes[k].second);

            max_requests.emplace_back(times);
        }
    }

    void allocate()
    {
        vector<i32> random_perm(g.n_time);
        random_perm.reserve(g.n_time);
        for (i32 i = 0; i < g.n_time; ++i)
            random_perm[i] = i;
        std::random_shuffle(random_perm.begin(), random_perm.end());

        for (const auto &t : random_perm)
        {
            vector<i32> fringes;
            unordered_map<i32, unordered_set<i32>> conflict;
            for (i32 i = 0; i < g.n_server; i++)
                if (max_requests[i].count(t) > 0)
                    fringes.emplace_back(i);

            for (u32 i = 0; i < fringes.size(); i++)
                for (u32 j = 0; j < fringes.size(); j++)
                    if (conflicts[fringes[i]].count(fringes[j]) > 0)
                        conflict[fringes[i]].insert(fringes[j]);

            unordered_set<i32> node_set;
            while (!fringes.empty())
            {
                i32 max_index = 0;
                f32 max_val = 0;
                for (u32 i = 0; i < fringes.size(); i++)
                {
                    if (max_requests[fringes[i]].size() <= g.n_time * (peak_ratio - 0.05))
                        continue;
                    if (conflict[fringes[i]].size() == 0)
                    {
                        max_index = i;
                        break;
                    }
                    if (float(max_requests[fringes[i]].size()) / conflict[fringes[i]].size() > max_val)
                    {
                        max_val = float(max_requests[fringes[i]].size() / conflict[fringes[i]].size());
                        max_index = i;
                    }
                }
                i32 node = fringes[max_index];
                node_set.insert(node);
                max_requests[node].erase(t);
                // use average to balance
                // if (nodeset.size() >= TmpFringe.size() * ratio) break;
                auto iter = fringes.erase(fringes.begin() + max_index);
                iter = fringes.begin();
                while (iter != fringes.end())
                {
                    if (conflict[node].find(*iter) != conflict[node].end() || max_requests[*iter].size() <= g.n_time * (peak_ratio - 0.05))
                    {
                        iter = fringes.erase(iter);
                    }
                    else
                        iter++;
                }
            }
            result[t] = node_set;
        }
    }

    void computeResult()
    {
        for (i32 time = 0; time < g.n_time; time++)
        {
            unordered_set<i32> current_set = result[time];
            vector<Allocation> tmpset;
            for (const auto &server : current_set)
            {
                i32 resband = g.capacities[server];
                for (i32 client : g.edges[server])
                {
                    if (g.demands[time][client] <= resband)
                    {
                        resband -= g.demands[time][client];
                        Allocation newnode = {server, client, g.demands[time][client]};
                        tmpset.emplace_back(newnode);
                    }
                }
            }
            extreme_result.emplace_back(tmpset);
        }
    }

    void Turn2Solution() {
        for (int time = 0;time < extreme_result.size();time++) {
            Solution cursolution(g.n_customer);
            for (int i = 0;i < extreme_result[time].size();i++) {
                cursolution.add(make_tuple(extreme_result[time][i].client, extreme_result[time][i].server, 
                extreme_result[time][i].bandwidth));
            }
            solutions.add(cursolution);
        }
    }

public:
    // result[time] 为 time 时刻使得带宽请求拉满的边缘节点编号集合
    vector<unordered_set<i32>> result;
    // time * {<server,client,bandwidth>} 具体分配方式
    vector<vector<Allocation>> extreme_result;

    ExtremeAllocator(f32 peak_ratio, const Graph &g) : g(g)
    {
        peak_ratio = peak_ratio;

        result.resize(g.n_time);
        conflicts.resize(g.n_server);

        // compute and initiate conflict
        vector<vector<i32>> inv_edges;
        for (i32 i = 0; i < g.n_server; i++)
            for (const auto &v : g.edges[i])
                inv_edges[v].push_back(i);

        conflicts.resize(g.n_server);
        for (i32 i = 0; i < g.n_server; i++)
            for (const auto &v : g.edges[i])
                for (const auto &u : inv_edges[v])
                    conflicts[i].insert(u);

        // initiate requests
        for (i32 t = 0; t < g.n_time; t++)
        {
            vector<i32> request;
            for (const auto &edge : g.edges)
            {
                i32 bandwidth = 0;
                for (const auto &v : edge)
                    bandwidth += g.demands[t][v];
                request.emplace_back(bandwidth);
            }
            requests.emplace_back(request);
        }
    }

    void run()
    {
        getExtreme();
        allocate();
        computeResult();
        Turn2Solution();
    }

    // 获取类型为 Solutions 的极端请求分配结果
    Solutions GetSolution() {
        return this->solutions;
    }

};

#endif