# HUAWEI Preliminary

:link: [link](https://competition.huaweicloud.com/codecraft2022?utm_source=weibo&utm_medium=sm-huaweiyun&utm_campaign=developer&utm_content=20220301-1)



```
CodeCraft-2022
│   build.sh
│   build_and_run.sh
│   CodeCraft_zip.sh
│
└───src
    │   CMakeLists.txt
    │   common.h
    │   dinic.hpp
    │   flow_graph.hpp
    │   graph.hpp
    │   main.cpp
    │   settings.h
    └─  utils.hpp
```



### 文件说明

- `*.sh`: 运行测试脚本
- `/src`: 源代码文件夹
  - `common.h`: 头文件
  - `utils.hpp`: utility 
  - `settings.h`: 配置文件
  - `graph.hpp`: 处理输入文件
  - `flow_graph.hpp`: 建模完的最大流图
  - `dinic.hpp`: 求解最大流



### 如何编译

可以选择以下几种方式：

- 使用`*.sh`脚本

- 使用`cmake`编译

- 直接使用`g++`编译

  `g++ -Wall -std=c++14 main.cpp -o main`



### 如何运行

将`data`放入可执行文件所在的目录，运行后会在`solution.txt`输出一组可行解



### 如何使用

在你的代码中`include`上述文件，使用以下代码即可求解可行解

```c++
int findFeasibleSolution()
{
    // read input files & construct basic graph
    Graph g;
    g.display();
    // construct network flow graph
    FlowGraph fg(g);
    fg.display();
    // initialize solver
    Dinic solver(fg);

    vector<string> solutions; // store solutions
    auto n_time = g.getTime();
    for (u32 t = 0; t < n_time; ++t)
    {
        // find feasible solution at time t
        
        // strictly follow the following steps
        auto demand_sum = fg.changeDemand(t);
        fg.reset(); 
        auto max_flow = solver.run();
        printError(max_flow != demand_sum, "invalid solution");

        // you may want to modify fg.getSolution()
        //	so that it returns data instead of a string
        u32 timestep;
        vector<string> solution;
        std::tie(timestep, solution) = std::move(fg.getSolution());
        printError(t != timestep, "invalid time");
        solutions.insert(solutions.end(), solution.begin(), solution.end());
    }

    return solutions;
}
```



