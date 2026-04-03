//
// Created by jehor on 2026/4/3.
//

#ifndef GAME_2048_QUICK_HASH_TOOLS_H
#define GAME_2048_QUICK_HASH_TOOLS_H
#include <iostream>
#include <functional> // std::hash
#include <cstddef>    // std::size_t

// 简单的 FNV-1a 哈希混合算法，用于将字节流混合进 seed
inline void hashCombineBytes(std::size_t& seed, const unsigned char* data, const std::size_t len) {
    for (std::size_t i = 0; i < len; ++i) {
        seed ^= static_cast<std::size_t>(data[i]) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }
}

#endif //GAME_2048_QUICK_HASH_TOOLS_H
