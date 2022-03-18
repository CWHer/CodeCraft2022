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

    i32 delta;
    vector<i32> capacities;
    Solutions solutions;

private:
    void step(i32 k)
    {
        // TODO
        //  1. save best solution
        //  2. coordinate descent (i.e. find out min{capacity[k]})
        capacities[k] -= delta;

        auto answer = getFeasibleSol(
            flow_g, n_time, capacities);

        if (!answer.first)
        {
            capacities[k] += delta;
            delta *= Settings::discounting_factor;
            delta = std::max(delta, Settings::min_delta);
            return;
        }

        solutions = std::move(answer.second);
    }

public:
    MinMax(Graph &g, FlowGraph &flow_g)
        : flow_g(flow_g), solutions(std::move(flow_g.getNames()))
    {
        this->n_time = g.getTime();
        delta = Settings::init_delta;
        capacities = std::move(g.getCapacity());
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
        std::cout << std::endl;
        std::cout << "delta: " << delta << std::endl;

        return solutions;
    }
};

#endif