#ifndef __UTILS_HPP__
#define __UTILS_HPP__

#include "common.h"

// >>> input utils
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
// <<< input utils

// softmax with temperature
vector<f64> softMax(
    const vector<f64> logits, f64 temp = 1.0)
{
    f64 inv_Z = 0, inv_temp = 1.0 / temp;
    f64 max_logit = *std::max_element(logits.begin(), logits.end());

    vector<f64> probs(logits.size());
    for (u32 i = 0; i < logits.size(); ++i)
        inv_Z += probs[i] = std::exp((logits[i] - max_logit) * inv_temp);
    inv_Z = 1.0 / inv_Z;
    for (u32 i = 0; i < probs.size(); ++i)
        probs[i] *= inv_Z;

    return probs;
}

// x ~ [min, max]
i32 randomInt(i32 min, i32 max)
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<i32> dist(min, max);
    return dist(gen);
}

// random select with given probability
template <class T>
i32 randomChoice(const vector<T> &probs)
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::discrete_distribution<> dist(probs.begin(), probs.end());
    return dist(gen);
}

// [0, 1)
f32 randomReal()
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_real_distribution<f32> dist(0, 1);
    return dist(gen);
}

f32 randomReal(f32 min, f32 max)
{
    return min + (max - min) * randomReal();
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
        exit(1);
    }
}

#endif