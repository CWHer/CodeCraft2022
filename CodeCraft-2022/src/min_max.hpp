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
    i32 n_time;
    FlowGraph &flow_g;

    i32 delta, min_delta;
    vector<i32> capacities;
    Solutions solutions;
    u64 cost;

    vector<u64> trajectory;

private:
    void step(i32 k)
    {
        // TODO
        //  1. coordinate descent (i.e. find out min{capacity[k]})
        //  2. KKT condition ????
        capacities[k] -= delta;

        auto answer = getFeasibleSol(
            flow_g, n_time, capacities);

        if (!answer.first)
        {
            capacities[k] += delta;
            delta *= Settings::discounting_factor;
            delta = std::max(delta, min_delta);
            return;
        }

        auto result = std::move(answer.second.evaluate());
        trajectory.push_back(result.first);
        if (result.first < cost)
        {
            cost = result.first;
            solutions = std::move(answer.second);
        }
    }

public:
    MinMax(Graph &g, FlowGraph &flow_g)
        : flow_g(flow_g), solutions(std::move(flow_g.getNames()))
    {
        this->n_time = g.getTime();
        capacities = std::move(g.getCapacity());
        i32 max_cap = *std::max_element(capacities.begin(), capacities.end());
        delta = std::min(Settings::init_delta, (i32)(max_cap * 0.5));
        min_delta = std::max(Settings::min_delta, (i32)(max_cap * 0.0001));
        cost = std::numeric_limits<u64>::max();
    }

    Solutions run(f64 run_time = 200)
    {
        auto getTime = []
        { return std::chrono::duration_cast<std::chrono::seconds>(
                     std::chrono::system_clock::now().time_since_epoch())
              .count(); };

        f64 start_time = getTime();
        i32 cnt = 0;
        do
        {
            step(randomInt(0, capacities.size() - 1));
            std::cerr << "\rSearch Times: " << ++cnt << std::flush;
        } while (start_time + run_time > getTime() && cnt < 10000);

        display();
        return solutions;
    }

    void display()
    {
        std::cout << "\ncapacities: ";
        for (const auto &cap : capacities)
            std::cout << cap << ' ';
        std::cout << std::endl;
        std::cout << "delta: " << delta << std::endl;
        std::cout << "cost: " << cost << std::endl;
    }
};

#endif