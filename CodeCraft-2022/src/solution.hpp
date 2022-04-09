#ifndef __SOLUTION_HPP__
#define __SOLUTION_HPP__

#include "common.h"
#include "settings.h"

struct Solution
{
    i32 n_customer;
    // customer: (server, stream_name)
    vector<unordered_map<i32, vector<string>>> solution;

    Solution(i32 n_customer)
    {
        this->n_customer = n_customer;
        solution.resize(n_customer);
    }

    void add(tuple<string, i32, i32> flow)
    {
        // stream_name, customer, server
        string name;
        i32 u, v;
        std::tie(name, u, v) = flow;
        if (!solution[u].count(v))
            solution[u][v] = vector<string>();
        solution[u][v].emplace_back(name);
    }
};

class Solutions
{

    friend std::ostream &operator<<(
        std::ostream &out, const Solutions &sol)
    {
        auto format = [&](i32 k, const vector<string> &names, bool add_sep)
        {
            string answer = (add_sep ? ",<" : "<") + sol.server_ids[k];
            for (const auto &name : names)
                answer += "," + name;
            return answer + ">";
        };

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
};

#endif