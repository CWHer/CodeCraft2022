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

    // TODO: calculate total cost
};

#endif