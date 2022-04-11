# HUAWEI CodeCraft2022 Preliminary

:link: [link](https://competition.huaweicloud.com/codecraft2022?utm_source=weibo&utm_medium=sm-huaweiyun&utm_campaign=developer&utm_content=20220301-1)

```
CodeCraft-2022
│   build.sh
│   build_and_run.sh
│   CodeCraft_zip.sh
│
└───src
    │   CMakeLists.txt
    │   allocate_extreme.hpp
    │   common.h
    │   dinic.hpp
    │   flow_graph.hpp
    │   graph.hpp
    │   greedy.hpp
    │   main.cpp
    │   min_max.hpp
    │   settings.h
    │   solution.hpp
    └─  utils.hpp
```



### 分支说明

<u>注</u>：说明文档仅在`preliminary`分支，且不同分支之间并没有良好同步某些代码

- `preliminary`：feasible solution求解 + local search 基本框架
- `extreme_greedy`：方法1 + 方法3
- `extreme_minmax`：方法1 + 方法2（$\epsilon_i$优化器）
- `final_version`：最终提交的版本



### 总体思路

:warning:这是一个勉强压线进入上合赛区复赛的**初赛**解题思路，且这种思路和复赛赛题完全不兼容

0. 如何得到一个可行解

   按照如下方式建图，求最大流即可（例如Dinic）

   <u>注</u>：一般可行解效果较差且分布难以预测
   $$
   \text{S} \longmapsto^{\text{bandwidth}}  \text{server} \longmapsto^{\infty} \text{customer} \longmapsto^{\text{demand}} \text{T}
   $$

1. 考虑到可以skip 5%时刻的流量，先贪心地给每个server尽可能**填满**不超过5%的时刻

   一种可能贪心思路：考虑所有时刻所有的server，分别计算$\sum$demand（作为优先级，可以考虑采用不同的方式，例如模仿page ranking），从大到小依次分配

2. 那么接下来，仅需考虑剩下95%的时刻

   我们用$\bold{M}=[m_1,m_2,\cdots]$来表示每个server在剩下时刻的带宽**最大值**，问题转换为$\min \sum m_i$

   <u>注</u>：其实这部分可以直接线性规划，但实现难度较大

   考虑到我们可以较快地求出可行解（不同time step可以良好地并行处理），可以使用local search的方式，迭代优化$\bold M$，一种可能的框架如下：

   - 求出feasible solution，计算$\bold M$

   - 选择某个$m_i$进行降低，重复

   如何选择$m_i$：（考虑到local search的性质，建议不要使用deterministic的选择策略）

   - 等概率随机选择
   - 选择$m_i$较大的server

   - 选择方差较大的server
   - 对于每个server，计算$\text{regret}= n \cdot \max \{\text{bandwidth}_i\} - \sum \text{bandwidth}_i $，选择regret较大的server

   如何降低$m_i$：（调参开始:hugs:）

   - 对于每个server，维护一个$\Delta_i$

     当选择第i个server时，令$m^\prime=m-\Delta_i$；若$m'$不存在可行解，考虑discount，例如$\Delta_i\leftarrow \Delta_i \times 0.5$

     主要难点：$\Delta$初始值难以确定，discount次数过多浪费时间

   - 对于每个server，维护一个$\epsilon_i$（上侧分位数）

     当选择第i个server时，降低$m_i$直到有$\epsilon_i$比例的时刻的bandwidth超过$m_i$

     同样可以根据$m'$可行解的情况来增大或减少$\epsilon_i$（例如$\epsilon_i \leftarrow \epsilon_i \times \alpha$）

3. 一种来自队友的parallel的思路：尽可能地平均分配



:hugs:在最终提交的版本里，我们使用了玄学杂糅

- 先使用方法1来分配5%的时刻
- 接下来，使用方法3估计$\overline m_i$
- 最后，在$\overline m_i$的基础上，使用regret + $\Delta_i$的方式进行local search



### 文件说明

- `*.sh`: 运行测试脚本
- `/src`: 源代码文件夹
  - `common.h`: 头文件
  - `utils.hpp`: utility functions
  - `settings.h`: 配置文件
  - `solution.hpp`: 用于存储解
  - `graph.hpp`: 处理输入文件
  - `flow_graph.hpp`: 建模完的最大流图
  - `dinic.hpp`: 求解最大流
  - `allocate_extreme.hpp`: 方法1
  - `min_max.hpp`: 方法2
  - `greedy.hpp`: 方法3
  - `main.cpp`: 主函数



### 如何编译

可以选择以下几种方式：

- 使用`*.sh`脚本

- 使用`cmake`编译

- 直接使用`g++`编译

  `g++ -Wall -std=c++14 main.cpp -o main`



### 如何运行

将`data`放入可执行文件所在的目录，运行后会在`solution.txt`输出一组可行解
