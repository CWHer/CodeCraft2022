#ifndef __GREEDY_HPP__
#define __GREEDY_HPP__

#include <vector>
#include "solution.hpp"
#include "flow_graph.hpp"
#include "allocate_extreme.hpp"
#include "common.h"

class FlowGraph;
class AllocateExtreme;
class Solutions;

class Greedy
{
private:
    u32 n_server, n_customer, iter_num;
    u32 n_time, valid_time;
    vector<vector<double>> weights;
    vector<vector<int>> costs;

public:
    Greedy(FlowGraph &fg, int num_time)
    {
        iter_num = 10;
        n_server = fg.n_server;
        n_customer = fg.n_customer;
        n_time = num_time;
        valid_time = n_time - std::ceil(double(n_time) * 0.95); // amount of top flows to be neglect

        // initalize costs and weights
        for (int i = 0; i < n_time; ++i)
        {
            vector<int> tmp1(n_server, 0);
            vector<double> tmp2(n_server, 1.0);
            costs.push_back(tmp1);
            weights.push_back(tmp2);
        }
    }

    // set weight as 0 if the server is preallocated at time t
    void initWeight(const Solutions &partial_sol)
    {
        for (int i = 0; i < n_time; ++i)
        {
            const auto solution = partial_sol.get(i);
            for (int j = 0; j < n_customer; ++j)
                for (const auto &flow : solution.solution[j])
                {
                    weights[i][flow.first] = 0;
                }
        }
    }

    // set weights based on previous costs
    void setWeight()
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
                    weights[i][j] = sum / costs[i][j] + double(rand() % 100) / 10;
            }
        }
    }

    void outputFilter(const Solutions &partial_sol)
    {
        std::ofstream f_out("flow_filter.txt");
        for (int t = 0; t < n_time; ++t)
        {
            vector<int> flow_amount(n_server, 0);
            const auto &solution = partial_sol.get(t);
            // calc
            for (int i = 0; i < n_customer; ++i)
            {
                for (auto flow : solution.solution[i])
                    flow_amount[flow.first] += flow.second;
            }

            // output
            for (int i = 0; i < n_server; ++i)
                f_out << flow_amount[i] << ' ';
            f_out << std::endl;
        }
        f_out.close();
    }

    void outputFlowDistribution(const Solutions &cur_sol)
    {
        std::ofstream f_out("flow_distribution.txt");
        for (int t = 0; t < n_time; ++t)
        {
            vector<int> flow_amount(n_server, 0);
            const auto &solution = cur_sol.get(t);
            // calc
            for (int i = 0; i < n_customer; ++i)
            {
                for (auto flow : solution.solution[i])
                    flow_amount[flow.first] += flow.second;
            }

            // output
            for (int i = 0; i < n_server; ++i)
                f_out << flow_amount[i] << ' ';
            f_out << std::endl;
        }
        f_out.close();
    }

    // use reward to update average strategy
    vector<int> run(FlowGraph &g, const vector<i32> &capacities, const Solutions &partial_sol)
    {
        // set capacity
        g.changeCapacity(capacities);
        initWeight(partial_sol);
        vector<int> max_flow(n_server, 0);
        // outputFilter(partial_sol);

        iter_num = 1;
        // iteration
        for (u32 iter = 0; iter < iter_num; ++iter)
        {
            // reset costs
            for (auto &piece : costs)
                for (auto &cost : piece)
                    cost = 0;

            Solutions solutions(g.getNames());
            // get solution
            for (u32 t = 0; t < n_time; ++t)
            {
                // find feasible solution at time t
                auto demand_sum = g.changeDemand(t);
                auto current_solution = partial_sol.get(t);
                auto solution_pair = g.getGreedySol(costs, valid_time, weights, current_solution, max_flow);
                if (!solution_pair.first)
                {
                    std::cout << "Unavailable Solution" << std::endl;
                }
                else
                    solutions.add(solution_pair.second);
            }

            // output flow_distribution
            // outputFlowDistribution(solutions);

            // update weight
            // setWeight();

            // return
            // if(iter == iter_num-1)
            return max_flow;
        }
        return max_flow;
    }
};

#endif