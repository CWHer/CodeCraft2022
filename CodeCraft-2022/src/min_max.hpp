#ifndef __MIN_MAX_HPP__
#define __MIN_MAX_HPP__

#include "common.h"
#include "flow_graph.hpp"
#include "dinic.hpp"
#include "solution.hpp"
#include "settings.h"

// use local search to minimize maximal bandwith
class MinMax
{
private:
    struct Optimizer
    {
        // NOTE: reduce the bound until a certain
        //  proportion of data is above the bound

        f32 epsilon;
        i32 fail_cnt, last_cap;

        Optimizer()
        {
            fail_cnt = 0;
            epsilon = Settings::init_proportion;
        }

        bool isEnd() { return fail_cnt > Settings::fail_threshold; }

        // not feasible
        i32 reduce()
        {
            fail_cnt++;
            epsilon *= Settings::discounting_factor;
            epsilon = std::max(Settings::min_epsilon, epsilon);
            return last_cap;
        }

        // feasible
        void step()
        {
            fail_cnt = 0;
            epsilon *= Settings::step_factor;
            epsilon = std::min(Settings::max_epsilon, epsilon);
        }

        i32 operator()(i32 cap, vector<i32> &flow)
        {
            last_cap = cap;
            i32 k_idx = flow.size() * (1 - epsilon);
            auto k_large = flow.begin() + k_idx;
            std::nth_element(flow.begin(), k_large, flow.end());
            return *k_large;
        }
    };

private:
    i32 n_time;
    FlowGraph &flow_g;

    vector<Optimizer> optimizers;
    vector<i32> capacities;

    Solutions best_solutions, last_solutions;
    vector<Statistics> last_stats;
    vector<vector<i32>> last_flows;
    u64 last_cost, best_cost;

    vector<u64> trajectory;

    const Solutions &partial_sol;

private:
    void step(i32 k)
    {
        capacities[k] = optimizers[k](capacities[k], last_flows[k]);

        // DO NOT change solution
        if (capacities[k] >= last_stats[k].max)
        {
            optimizers[k].step();
            return;
        }

        auto answer = getFeasibleSols(
            flow_g, n_time, capacities, partial_sol);

        // no feasible solution
        if (!answer.first)
        {
            capacities[k] = optimizers[k].reduce();
            trajectory.push_back(std::numeric_limits<u64>::max());
            return;
        }

        optimizers[k].step();
        last_solutions = std::move(answer.second);
        std::tie(last_cost, last_stats, last_flows) = std::move(last_solutions.evaluate(1.0));
        trajectory.push_back(last_cost);

        if (last_cost < best_cost)
        {
            best_cost = last_cost;
            best_solutions = last_solutions;
        }
    }

public:
    MinMax(Graph &g, FlowGraph &flow_g,
           const Solutions &partial_sol, const vector<i32> &capcities)
        : flow_g(flow_g), partial_sol(partial_sol)
    {
        this->n_time = g.getTime();
        this->capacities = std::move(capcities);
        last_stats.resize(capacities.size());
        best_cost = std::numeric_limits<u64>::max();
        optimizers.reserve(capacities.size());
        for (u32 i = 0; i < capacities.size(); ++i)
            optimizers.emplace_back(Optimizer());
    }

    Solutions run(f64 run_time = 295)
    {
        auto getTime = []
        { return std::chrono::duration_cast<std::chrono::seconds>(
                     std::chrono::system_clock::now().time_since_epoch())
              .count(); };

        auto answer = getFeasibleSols(flow_g, n_time, capacities, partial_sol);
        printError(!answer.first, "invalid partial solution.");
        last_solutions = std::move(answer.second);
        std::tie(last_cost, last_stats, last_flows) = std::move(last_solutions.evaluate(1.0));

        f64 start_time = getTime();
        i32 cnt = 0, k;
        // std::priority_queue<pair<i32, i32>> cands;
        // for (u32 i = 0; i < last_stats.size(); ++i)
        //     cands.emplace(make_pair(last_stats[i].regret, i));

        // while (!cands.empty() && cnt < 2000 &&
        //        start_time + run_time > getTime())
        // {
        //     k = cands.top().second;
        //     cands.pop();

        //     step(k);
        //     std::cerr << "\rSearch Times: " << ++cnt << std::flush;
        //     display();
        //     if (cnt > 10)
        //     {
        //         printInfo("1");
        //     }

        //     for (u32 i = 0; i < last_stats.size(); ++i)
        //         if (!optimizers[i].isEnd())
        //             cands.emplace(make_pair(last_stats[i].regret, i));
        // }

        do
        {
            do
            {
                vector<i64> last_regrets;
                last_regrets.reserve(last_stats.size());
                for (const auto &stat : last_stats)
                    last_regrets.push_back(stat.regret);

                k = randomChoice<i64>(last_regrets);
                // k = randomInt(0, capacities.size() - 1);
                // k = randomChoice<u32>(last_costs);
                // k = std::max_element(last_costs.begin(), last_costs.end()) - last_costs.begin();
            } while (optimizers[k].isEnd());
            step(k);
            std::cerr << "\rSearch Times: " << ++cnt << std::flush;
        } while (start_time + run_time > getTime());

        display();
        return best_solutions;
    }

    void display()
    {
        std::cout << "\nlast states: \n";
        for (const auto &stat : last_stats)
            std::cout << '\t' << stat;
        std::cout << std::endl;
        std::cout << "cost: " << best_cost << std::endl;
    }
};

#endif