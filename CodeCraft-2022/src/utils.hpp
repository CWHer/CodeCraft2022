#include "common.h"

tuple<string, vector<i32>> readLine(const string &line)
{
    i32 x;
    string name;
    vector<i32> arr;
    const u32 ignore_num = 100;

    std::istringstream s_in(line);
    getline(s_in, name, ',');
    while (s_in >> x)
    {
        arr.emplace_back(x);
        s_in.ignore(ignore_num, ',');
    }
    return make_tuple(name, arr);
}

vector<string> readNames(const string &line)
{
    const u32 ignore_num = 100;
    string name;
    vector<string> names;

    std::istringstream s_in(line);
    s_in.ignore(ignore_num, ',');
    while (getline(s_in, name, ','))
        names.emplace_back(name);

    return names;
}

void printInfo(string msg)
{
    std::cout << "[INFO]: " << msg << std::endl;
}

void printWarning(bool expr, string msg)
{
    if (expr)
        std::cout << "[WARN]: " << msg << std::endl;
}

void printError(bool expr, string msg)
{
    if (expr)
    {
        std::cout << "[ERROR]: " << msg << std::endl;
        exit(0);
    }
}