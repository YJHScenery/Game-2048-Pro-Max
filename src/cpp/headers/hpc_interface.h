//
// Created by jehor on 2026/3/30.
//

#ifndef GAME_2048_QUICK_HPC_INTERFACE_H
#define GAME_2048_QUICK_HPC_INTERFACE_H

#include "cuda_interface.h"
#include "cann_interface.h"
#include "cpu_interface.h"
#include <utility>



struct CudaLineDesc
{
    std::uint64_t start; // element offset (not bytes)
    std::int64_t step;   // element stride (signed, not bytes)
};

struct CannLineDesc
{
    std::uint64_t start;
    std::int64_t step;
};

template <typename T, typename LineDesc, typename Device>
void move_lines_gpu(T* h_data, const LineDesc* h_lines, std::size_t line_count, std::size_t line_len, Device* stream = nullptr);


template<typename T, typename LineDesc, typename Device>
void move_lines_gpu(T *h_data, const LineDesc *h_lines, std::size_t line_count, std::size_t line_len, Device* stream)
{
}



#endif //GAME_2048_QUICK_HPC_INTERFACE_H
