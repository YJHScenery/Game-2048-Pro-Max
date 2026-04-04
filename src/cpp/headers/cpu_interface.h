//
// Created by jehor on 2026/3/30.
//

#ifndef GAME_2048_QUICK_CPU_INTERFACE_H
#define GAME_2048_QUICK_CPU_INTERFACE_H
#include "basic_dependency.h"

template <typename T>
void move_lines_cpu(T* h_data, const StandardLineDesc *h_lines, std::size_t line_count, std::size_t line_len)
{
    if (line_count == 0 || line_len == 0)
        return;

    for (std::size_t line_id = 0; line_id < line_count; ++line_id)
    {
        const StandardLineDesc desc = h_lines[line_id];
        const std::uint64_t base = desc.start;
        const std::int64_t step = desc.step;

        bool has_prev = false;
        T prev{};
        std::size_t write = 0;

        for (std::size_t read = 0; read < line_len; ++read)
        {
            const std::int64_t pos = static_cast<std::int64_t>(base) + static_cast<std::int64_t>(read) * step;
            const T v = h_data[pos];
            if (v == T{}) // 检查是否为零值
                continue;

            if (has_prev && v == prev)
            {
                const T merged = prev + v;
                const std::int64_t wpos = static_cast<std::int64_t>(base) + static_cast<std::int64_t>(write) * step;
                h_data[wpos] = merged;
                ++write;
                has_prev = false;
            }
            else
            {
                if (has_prev)
                {
                    const std::int64_t wpos = static_cast<std::int64_t>(base) + static_cast<std::int64_t>(write) * step;
                    h_data[wpos] = prev;
                    ++write;
                }
                prev = v;
                has_prev = true;
            }
        }

        if (has_prev)
        {
            const std::int64_t wpos = static_cast<std::int64_t>(base) + static_cast<std::int64_t>(write) * step;
            h_data[wpos] = prev;
            ++write;
        }

        for (std::size_t i = write; i < line_len; ++i)
        {
            const std::int64_t wpos = static_cast<std::int64_t>(base) + static_cast<std::int64_t>(i) * step;
            h_data[wpos] = T{}; // 填充零值
        }
    }
}


template <typename T, size_t nDims, size_t... Dimensions>
requires (sizeof...(Dimensions) == nDims)
std::vector<EqualPair> find_equal_cpu(const T* tensor_data)
{
    // static_assert(sizeof...(Dimensions) == nDims, "nDims must match number of Dimensions");

    constexpr size_t dims_list[nDims]{Dimensions...};
    constexpr size_t tensor_len{(Dimensions * ...)};

    size_t strides[nDims];
    strides[nDims - 1] = 1;
    for (long long d = static_cast<long long>(nDims) - 2; d >= 0; --d)
        strides[d] = strides[d + 1] * dims_list[d + 1];

    std::vector<EqualPair> results;
    results.reserve(tensor_len * nDims);

    for (size_t idx = 0; idx < tensor_len; ++idx)
    {
        size_t pos[nDims];
        size_t remaining = idx;
        for (size_t d = 0; d < nDims; ++d)
        {
            pos[d] = remaining / strides[d];
            remaining %= strides[d];
        }

        for (size_t d = 0; d < nDims; ++d)
        {
            if (pos[d] + 1 >= dims_list[d])
                continue;

            const size_t next_idx = idx + strides[d];
            if (tensor_data[idx] == tensor_data[next_idx])
            {
                EqualPair eq;
                eq.pos.reserve(nDims);
                for (size_t i = 0; i < nDims; ++i)
                    eq.pos.push_back(pos[i]);
                eq.dim = d;
                results.push_back(std::move(eq));
            }
        }
    }

    return results;
}

#endif //GAME_2048_QUICK_CPU_INTERFACE_H
