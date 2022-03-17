#include "common.h"
#include "utils.hpp"

class Graph
{

private:
    u32 n_time, n_server, n_customer;

    // capacity of edge nodes
    //  NOTE: invariant over time
    vector<i32> capacities;

    // demand of customer nodes
    //  times x customers
    vector<vector<i32>> demands;

    // edges: adjacent table
    vector<vector<i32>> edges;

public:
    Graph(string data_dir = "data")
    {
        loadData(data_dir);
    }

    void loadData(string data_dir)
    {
        const u32 ignore_num = 100;
        std::ifstream f_in;
        string line;

        // >>>>> config.ini
        i32 QoS_lim;
        f_in.open(data_dir + "/config.ini");
        printError(!f_in.is_open(), "file not found!");
        getline(f_in, line);
        f_in.ignore(ignore_num, '=');
        f_in >> QoS_lim;
        f_in.close();

        // >>>> site_bandwidth.csv
        i32 capacity;
        f_in.open(data_dir + "/site_bandwidth.csv");
        printError(!f_in.is_open(), "file not found!");
        getline(f_in, line);
        while (getline(f_in, line))
            capacities.emplace_back(readLine(line).front());
        n_server = capacities.size();
        edges.resize(n_server);
        f_in.close();

        // >>>> demand.csv
        f_in.open(data_dir + "/demand.csv");
        printError(!f_in.is_open(), "file not found!");
        getline(f_in, line);
        while (getline(f_in, line))
            demands.emplace_back(readLine(line));
        n_time = demands.size();
        n_customer = demands.front().size();
        f_in.close();

        // >>>> qos.csv
        i32 k_server = 0;
        f_in.open(data_dir + "/qos.csv");
        printError(!f_in.is_open(), "file not found!");
        getline(f_in, line);
        while (getline(f_in, line))
        {
            auto arr = readLine(line);
            for (auto i = 0; i < arr.size(); ++i)
                if (arr[i] < QoS_lim)
                    edges[k_server].emplace_back(i);
        }
        f_in.close();
    }

    // DEBUG function
    void display()
    {
        printInfo(
            "Time: " + std::to_string(n_time) + ", " +
            "Server: " + std::to_string(n_server) + ", " +
            "Customer: " + std::to_string(n_customer));
    }
};