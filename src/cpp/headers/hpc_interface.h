//
// Created by jehor on 2026/3/30.
//

#ifndef GAME_2048_QUICK_HPC_INTERFACE_H
#define GAME_2048_QUICK_HPC_INTERFACE_H

#include "cuda_interface.h"
#include "cann_interface.h"
#include "cpu_interface.h"
#include <utility>


template <typename T, typename LineDesc, typename Device>
void move_lines_gpu(T* h_data, const LineDesc* h_lines, std::size_t line_count, std::size_t line_len, Device* stream = nullptr);


template<typename T, typename LineDesc, typename Device>
void move_lines_gpu(T *h_data, const LineDesc *h_lines, const std::size_t line_count, const std::size_t line_len, Device* stream)
{
    using meta_type_ = T;
    if constexpr (std::is_integral_v<meta_type_> && std::is_signed_v<meta_type_>)
    {
        cuda_move_lines_ll(h_data, h_lines, static_cast<std::size_t>(line_count), static_cast<std::size_t>(line_len));
    }
    else if constexpr (std::is_integral_v<meta_type_> && std::is_unsigned_v<meta_type_>)
    {

        cuda_move_lines_ull(h_data, h_lines, static_cast<std::size_t>(line_count), static_cast<std::size_t>(line_len));

    }
    else if constexpr (std::is_floating_point_v<meta_type_> && std::is_signed_v<meta_type_>)
    {
        cuda_move_lines_ld(h_data, h_lines, static_cast<std::size_t>(line_count), static_cast<std::size_t>(line_len));
    }
    else
    {
        cuda_move_lines_uld(h_data, h_lines, static_cast<std::size_t>(line_count), static_cast<std::size_t>(line_len));

    }
}



#endif //GAME_2048_QUICK_HPC_INTERFACE_H
