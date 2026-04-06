//
// Created by jehor on 2026/3/31.
//

#ifndef GAME_2048_QUICK_BASIC_DEPENDENCY_H
#define GAME_2048_QUICK_BASIC_DEPENDENCY_H
#include <vector>
#include <utility>

struct StandardLineDesc
{
    std::uint64_t start; // element offset (not bytes)
    std::int64_t step;   // element stride (signed, not bytes)
};

struct EqualPair
{
    std::vector<size_t> pos; // 相等发生的起始位置（坐标较小的）
    size_t dim; // 相等发生的维度
};

#endif //GAME_2048_QUICK_BASIC_DEPENDENCY_H
