#ifndef __SOLUTION_HPP__
#define __SOLUTION_HPP__

#include "common.h"
#include "settings.h"

struct Statistics
{
    i32 max, cost;
    f64 mean, var;
    // regret := max{f_i} * t - sum{f_i}
    i64 regret;

    Statistics()
        : max(std::numeric_limits<i32>::min()),
          mean(0), var(0), cost(0), regret(0) {}

    friend std::ostream &operator<<(
        std::ostream &out, const Statistics &stat)
    {
        out << "max: " << std::setw(12) << stat.max << ", "
            << "cost: " << std::setw(12) << stat.cost << ", "
            << "mean: " << std::setw(12) << stat.mean << ", "
            << "variance: " << std::setw(12) << stat.var << ", "
            << "regret: " << std::setw(12) << stat.regret << '\n';

        return out;
    }
};

void printStats(string msg, const vector<Statistics> &stats)
{
    std::cout << '\n'
              << msg << '\n';
    for (const auto &stat : stats)
        std::cout << '\t' << stat;
    std::cout << std::endl;
}

struct Solution
{
    i32 n_customer;
    // customer server flow
    vector<vector<pair<i32, i32>>> solution;

    // Tooooo slow.
    // vector<unordered_map<i32, i32>> solution;

    // problematic initialization
    Solution() {}

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
        // solution[u][v] += w;
    }

    // NOTE: this operation is time-consuming
    Solution operator+=(const Solution &rhs)
    {
        printError(
            solution.size() != rhs.solution.size(),
            "size mismatch");

        Solution result(n_customer);

        for (u32 i = 0; i < solution.size(); ++i)
        {
            unordered_map<i32, i32> flows;
            for (const auto &flow : solution[i])
                flows[flow.first] += flow.second;
            for (const auto &flow : rhs.solution[i])
                flows[flow.first] += flow.second;

            result.solution[i].reserve(flows.size());
            for (const auto &flow : flows)
                result.solution[i].push_back(flow);
        }

        return result;
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
    // problematic initialization
    Solutions() {}

    Solutions(tuple<vector<string>, vector<string>> ids)
    {
        std::tie(this->server_ids, this->customer_ids) = std::move(ids);
    }

    void add(Solution &sol)
    {
        solutions.emplace_back(sol);
    }

    tuple<u64, vector<Statistics>> evaluate(f32 quantile = Settings::quantile)
    {
        u64 cost = 0;
        vector<Statistics> stats(server_ids.size());
        vector<vector<i32>> flows(server_ids.size());

        for (const auto &solution : solutions)
        {
            vector<i32> flow(server_ids.size(), 0);
            for (const auto &customer : solution.solution)
                for (const auto &f : customer)
                    flow[f.first] += f.second;
            for (u32 i = 0; i < flow.size(); ++i)
            {
                auto &stat = stats[i];
                stat.regret -= flow[i];
                stat.mean += flow[i];
                stat.var += 1.0 * flow[i] * flow[i];
                stat.max = std::max(stat.max, flow[i]);
                flows[i].emplace_back(flow[i]);
            }
        }

        for (u32 i = 0; i < stats.size(); ++i)
        {
            auto &stat = stats[i];
            stat.mean /= flows[i].size();
            stat.var = stat.var / flows[i].size() - stat.mean * stat.mean;
            stat.regret += (i64)stat.max * flows[i].size();
        }

        // NOTE: 0-based vector
        int k_idx = std::ceil(solutions.size() * quantile) - 1;
        for (u32 i = 0; i < flows.size(); ++i)
        {
            auto k_large = flows[i].begin() + k_idx;
            std::nth_element(flows[i].begin(), k_large, flows[i].end());
            cost += stats[i].cost = *k_large;
        }

        return make_tuple(cost, stats);
    }
};

#endif