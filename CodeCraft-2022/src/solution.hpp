#ifndef __SOLUTION_HPP__
#define __SOLUTION_HPP__

#include "common.h"

struct Solution
{
    i32 n_customer;
    vector<vector<pair<i32, i32>>> solution;

    Solution(i32 n_customer)
    {
        this->n_customer = n_customer;
        solution.resize(n_customer);
    }

    void add(tuple<i32, i32, i32> flow)
    {
        // customer server flow
        i32 u, v, w;
        std::tie(u, v, w) = flow;
        solution[u].push_back(make_pair(v, w));
    }
};

class Solutions
{

    friend std::ostream &operator<<(
        std::ostream &out, const Solutions &sol)
    {
        auto format = [&](i32 k, i32 flow, bool add_sep)
        { return (add_sep ? ",<" : "<") + sol.server_ids[k] +
                 "," + std::to_string(flow) + ">"; };

        for (const auto &solution : sol.solutions)
            for (i32 i = 0; i < solution.n_customer; ++i)
            {
                string line = sol.customer_ids[i] + ":";
                for (const auto &flow : solution.solution[i])
                    line += format(flow.first, flow.second, line.back() == '>');
                out << line << '\n';
            }
        out << std::endl;

        return out;
    }

private:
    vector<string> server_ids;
    vector<string> customer_ids;

    vector<Solution> solutions;

public:
    Solutions(tuple<vector<string>, vector<string>> ids)
    {
        std::tie(this->server_ids, this->customer_ids) = std::move(ids);
    }

    void add(Solution &sol)
    {
        solutions.emplace_back(sol);
    }

    pair<u64, vector<vector<i32>>> evaluate()
    {
        u64 cost = 0;
        vector<vector<i32>> flows(server_ids.size());

        for (const auto &solution : solutions)
        {
            vector<i32> flow(server_ids.size(), 0);
            for (const auto &customer : solution.solution)
                for (const auto &f : customer)
                    flow[f.first] += f.second;
            for (u32 i = 0; i < flow.size(); ++i)
                flows[i].emplace_back(flow[i]);
        }

        // NOTE: 0-based vector
        int k_idx = std::ceil(solutions.size() * 0.95) - 1;
        for (auto &flow : flows)
        {
            auto k_large = flow.begin() + k_idx;
            std::nth_element(flow.begin(), k_large, flow.end());
            cost += *k_large;
        }

        return make_pair(cost, flows);
    }
};

#endif