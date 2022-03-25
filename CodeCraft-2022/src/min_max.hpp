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
        i32 delta, min_delta;

        Optimizer(i32 cap)
        {
            delta = std::min(
                Settings::init_delta, (i32)(cap * Settings::init_delta_factor));
            min_delta = std::max(
                Settings::min_delta, (i32)(cap * Settings::min_delta_factor));
        }

        void reduce()
        {
            delta *= Settings::discounting_factor;
            delta = std::max(delta, min_delta);
        }

        void update(i32 cap)
        {
            delta = std::min(
                delta, (i32)(cap * Settings::init_delta_factor));
            delta = std::max(delta, min_delta);
        }
    };

private:
    i32 n_time;
    FlowGraph &flow_g;

    vector<Optimizer> optimizers;
    vector<i32> capacities;

    Solutions best_solutions, last_solutions;
    vector<Statistics> last_stats;
    u64 last_cost, best_cost;

    vector<u64> trajectory;

private:
    // void calcRegret()
    // {
    //     std::fill(
    //         last_max_flow.begin(), last_max_flow.end(),
    //         std::numeric_limits<i32>::min());
    //     std::fill(last_regrets.begin(), last_regrets.end(), 0);

    //     for (u32 i = 0; i < last_flows.size(); ++i)
    //     {
    //         for (const auto &flow : last_flows[i])
    //         {
    //             last_regrets[i] -= flow;
    //             last_max_flow[i] = std::max(last_max_flow[i], flow);
    //         }
    //         last_regrets[i] += (i64)last_max_flow[i] * last_flows[i].size();
    //     }
    // }

    void step(i32 k)
    {
        // TODO
        //  1. coordinate descent (i.e. find out min{capacity[k]})
        //  2. Linear programming
        capacities[k] -= optimizers[k].delta;
        // capacities[k] = last_costs[k] / 2;

        // DO NOT change solution
        if (capacities[k] >= last_stats[k].max)
        {
            optimizers[k].update(capacities[k]);
            return;
        }

        auto answer = getFeasibleSols(flow_g, n_time, capacities);

        // no feasible solution
        if (!answer.first)
        {
            capacities[k] += optimizers[k].delta;
            optimizers[k].reduce();
            trajectory.push_back(std::numeric_limits<u64>::max());
            return;
        }

        optimizers[k].update(capacities[k]);

        last_solutions = std::move(answer.second);
        std::tie(last_cost, last_stats) = std::move(last_solutions.evaluate());
        trajectory.push_back(last_cost);

        if (last_cost < best_cost)
        {
            best_cost = last_cost;
            best_solutions = last_solutions;
        }
    }

public:
    MinMax(Graph &g, FlowGraph &flow_g) : flow_g(flow_g)
    {
        this->n_time = g.getTime();
        capacities = std::move(g.getCapacity());
        last_stats.resize(capacities.size());
        best_cost = std::numeric_limits<u64>::max();
    }

    Solutions run(f64 run_time = 275)
    {
        auto getTime = []
        { return std::chrono::duration_cast<std::chrono::seconds>(
                     std::chrono::system_clock::now().time_since_epoch())
              .count(); };

        auto answer = getFeasibleSols(flow_g, n_time, capacities);
        last_solutions = std::move(answer.second);
        std::tie(last_cost, last_stats) = std::move(last_solutions.evaluate());

        // agressive optimization
        auto max_flow = std::max_element(
                            last_stats.begin(), last_stats.end(),
                            [](const auto &lhs, const auto &rhs)
                            { return lhs.max < rhs.max; })
                            ->max;
        optimizers.reserve(capacities.size());
        for (u32 i = 0; i < capacities.size(); ++i)
        {
            capacities[i] = std::min(capacities[i], 2 * max_flow);
            optimizers.emplace_back(Optimizer(capacities[i]));
        }

        // i32 max_cost = *std::max_element(
        //     last_costs.begin(), last_costs.end());
        // std::fill(capacities.begin(), capacities.end(), max_cost);

        f64 start_time = getTime();
        i32 cnt = 0, k;
        do
        {
            do
            {
                vector<i64> last_regrets;
                last_regrets.reserve(last_stats.size());
                for (const auto &stat : last_stats)
                    last_regrets.push_back(stat.regret);
                // TODO: choose max regret
                k = randomChoice<i64>(last_regrets);
                // k = randomInt(0, capacities.size() - 1);
                // k = randomChoice<u32>(last_costs);
                // k = std::max_element(last_costs.begin(), last_costs.end()) - last_costs.begin();
            } while (capacities[k] < optimizers[k].delta);
            step(k);
            std::cerr << "\rSearch Times: " << ++cnt << std::flush;
        } while (start_time + run_time > getTime() && cnt < 2000);

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