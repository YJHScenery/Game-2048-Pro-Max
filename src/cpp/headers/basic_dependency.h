//
// Created by jehor on 2026/3/31.
//

#ifndef GAME_2048_QUICK_BASIC_DEPENDENCY_H
#define GAME_2048_QUICK_BASIC_DEPENDENCY_H

struct StandardLineDesc
{
    std::uint64_t start; // element offset (not bytes)
    std::int64_t step;   // element stride (signed, not bytes)
};

#endif //GAME_2048_QUICK_BASIC_DEPENDENCY_H
