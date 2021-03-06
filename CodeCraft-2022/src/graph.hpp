#ifndef __GRAPH_HPP__
#define __GRAPH_HPP__

#include "common.h"
#include "utils.hpp"

class FlowGraph;

// NOTE: DO NOT use Graph,
//  once it is used to construct FlowGraph
class Graph
{
    friend class FlowGraph;
    friend class ExtremeAllocator;

private:
    u32 n_time, n_server, n_customer;

    // capacity of edge nodes
    //  NOTE: invariant over time
    vector<i32> capacities;

    // demand of customer nodes
    //  times x customers
    vector<vector<i32>> demands;

    // edges: adjacent table
    //  server -> customer
    vector<vector<i32>> edges;

    vector<string> server_ids;
    vector<string> customer_ids;

public:
    Graph(string data_dir)
    {
        loadData(data_dir);
    }

    void loadData(string data_dir)
    {
        const u32 ignore_num = 100;
        std::ifstream f_in;
        string line, name;
        vector<i32> arr;

        // >>>>> config.ini
        i32 QoS_lim;
        f_in.open(data_dir + "/config.ini");
        printError(!f_in.is_open(), "file not found!");
        getline(f_in, line);
        f_in.ignore(ignore_num, '=');
        f_in >> QoS_lim;
        f_in.close();

        // >>>> site_bandwidth.csv
        f_in.open(data_dir + "/site_bandwidth.csv");
        printError(!f_in.is_open(), "file not found!");
        getline(f_in, line);
        while (getline(f_in, line))
        {
            std::tie(name, arr) = readLine(line);
            server_ids.emplace_back(name);
            capacities.emplace_back(arr.front());
        }
        n_server = capacities.size();
        edges.resize(n_server);
        f_in.close();

        // >>>> demand.csv
        f_in.open(data_dir + "/demand.csv");
        printError(!f_in.is_open(), "file not found!");
        getline(f_in, line);
        while (line.back() == '\r' || line.back() == '\n')
            line.pop_back();
        customer_ids = std::move(readNames(line));
        while (getline(f_in, line))
        {
            std::tie(std::ignore, arr) = readLine(line);
            demands.emplace_back(arr);
        }
        n_time = demands.size();
        n_customer = demands.front().size();
        f_in.close();

        // >>>> qos.csv
        f_in.open(data_dir + "/qos.csv");
        printError(!f_in.is_open(), "file not found!");
        getline(f_in, line);
        while (line.back() == '\r' || line.back() == '\n')
            line.pop_back();
        vector<string> names = readNames(line);
        vector<i32> customer_indices(n_customer);
        for (u32 i = 0; i < n_customer; ++i)
            customer_indices[i] = std::find(customer_ids.begin(),
                                            customer_ids.end(), names[i]) -
                                  customer_ids.begin();

        while (getline(f_in, line))
        {
            std::tie(name, arr) = readLine(line);
            i32 k_server = std::find(server_ids.begin(),
                                     server_ids.end(), name) -
                           server_ids.begin();
            for (u32 i = 0; i < arr.size(); ++i)
                if (arr[i] < QoS_lim)
                    edges[k_server].push_back(customer_indices[i]);
        }
        f_in.close();
    }

    i32 getTime() { return n_time; }
    vector<i32> getCapacity() { return capacities; }

    // DEBUG function
    void display()
    {
        printInfo(
            "Time: " + std::to_string(n_time) + ", " +
            "Server: " + std::to_string(n_server) + ", " +
            "Customer: " + std::to_string(n_customer));

        for (const auto &server_id : server_ids)
            std::cout << server_id << ' ';
        std::cout << std::endl;

        for (const auto &customer_id : customer_ids)
            std::cout << customer_id << ' ';
        std::cout << std::endl;
    }
};

#endif