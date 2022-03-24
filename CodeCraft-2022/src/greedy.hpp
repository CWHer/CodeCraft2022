#ifndef __GREEDY_HPP__
#define __GREEDY_HPP__

#include <vector>
#include "solution.hpp"
#include "flow_graph.hpp"
#include "allocate_extreme.hpp"
#include "common.h"

class FlowGraph;
class ExtremeAllocator;
class Solutions;

class Greedy
{
private:
    u32 n_server;
    u32 n_time, valid_time;
    vector<vector<double>> weights;
    vector<vector<int>> costs;

public:
    Greedy(FlowGraph &fg, int num_time)
    {
        n_server = fg.n_server;
        n_time = num_time;
        valid_time = n_time - std::ceil(double(n_time) * 0.95); // amount of top flows to be neglect

        // initialize costs and weights
        for (int i = 0; i < n_time; ++i)
        {
            vector<int> tmp1(n_server, 0);
            vector<double> tmp2(n_server, 1.0);
            costs.push_back(tmp1);
            weights.push_back(tmp2);
        }
    }

    // set weights based on previous costs
    void set_weight()
    {
        for (int i = 0; i < n_time; ++i)
        {
            double sum = 0;
            // calculate sum
            for (int j = 0; j < n_server; ++j)
            {
                if (costs[i][j] != 0)
                    sum += 100000 / costs[i][j];
            }
            // update
            for (int j = 0; j < n_server; ++j)
            {
                if (costs[i][j] != 0)
                    weights[i][j] = sum / costs[i][j];
            }
        }
    }

    // set weight as 0 if the server is preallocated at time t
    void set_weight_2(const vector<std::unordered_set<int>> &res)
    {
        for (int i = 0; i < n_time; ++i)
        {
            for (auto &iter : res[i])
                weights[i][iter] = 0;
        }
    }

    // use reward to update average strategy
    pair<bool, Solutions> recurrent_search(FlowGraph &g, const vector<i32> &capacities, const ExtremeAllocator &ae)
    {
        // set capacity
        g.changeCapacity(capacities);

        // reallocate
        const auto &ae_res = ae.result;
        const auto &ae_exres = ae.extreme_result;

        // amount of iterations
        int iter_num = 10;

        // iteration
        for (u32 iter = 0; iter < iter_num; ++iter)
        {
            // reset costs, check weights
            for (auto &piece : costs)
                for (auto &cost : piece)
                    cost = 0;
            set_weight_2(ae_res);

            Solutions solutions(g.getNames());
            // get solution
            for (u32 t = 0; t < n_time; ++t)
            {
                // find feasible solution at time t
                auto demand_sum = g.changeDemand(t);
                auto solution_pair = g.getGreedySol(costs, valid_time, weights, ae_res, ae_exres);
                if (!solution_pair.first)
                {
                    std::cout << "Unavailable Solution" << std::endl;
                }
                else
                    solutions.add(solution_pair.second);
            }

            // evaluate
            auto result = solutions.evaluate();
            std::cout << "Evaluation:" << iter << std::endl;
            std::cout << "Answer:" << std::get<0>(result) << std::endl;

            // update weight
            set_weight();

            // return
            if (iter == iter_num - 1)
                return make_pair(true, solutions);
        }
        return make_pair(false, Solutions(g.getNames()));
    }
};

#endif