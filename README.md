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
    │   solution.hpp
    └─  utils.hpp
```



### 文件说明

- `*.sh`: 运行测试脚本
- `/src`: 源代码文件夹
  - `common.h`: 头文件
  - `utils.hpp`: utility 
  - `settings.h`: 配置文件
  - `solution.hpp`: 用于存储解
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

参考`dinic.hpp`中的`getFeasibleSol()`
