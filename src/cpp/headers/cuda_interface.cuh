//
// Created by jehor on 2026/3/29.
//

#ifndef GAME_2048_QUICK_CUDAINTERFACE_H
#define GAME_2048_QUICK_CUDAINTERFACE_H

#if __has_include(<cuda_runtime.h>)
	#include <cuda_runtime.h>
	#include <device_launch_parameters.h>
#else
	#define CUDA_UNAVAILABLE
#endif


#ifndef CUDA_UNAVAILABLE
#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

#include "basic_dependency.h"


template <typename T>
void cuda_move_lines(T* h_data, const StandardLineDesc* h_lines, std::size_t line_count, std::size_t line_len, cudaStream_t stream);

template <typename T, size_t nDims, size_t... Dimensions>
std::vector<EqualPair> cuda_find_equal(const T* tensor_data);


#endif
#endif //GAME_2048_QUICK_CUDAINTERFACE_H
