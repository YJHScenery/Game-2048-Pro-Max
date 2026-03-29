//
// Created by jehor on 2026/3/29.
//

#ifndef GAME_2048_QUICK_CUDAINTERFACE_H
#define GAME_2048_QUICK_CUDAINTERFACE_H

#include <cuda_runtime.h>
#include <device_launch_parameters.h>

#include <cstddef>
#include <cstdint>

struct CudaLineDesc
{
	std::uint64_t start; // element offset (not bytes)
	std::int64_t step;   // element stride (signed, not bytes)
};

// Host-callable entry points implemented in cuda_interface.cu
// Each CUDA entry processes all lines in parallel; each line is processed sequentially by one thread.
void cuda_move_lines_ll(long long *h_data, const CudaLineDesc *h_lines, std::size_t line_count, std::size_t line_len,
						cudaStream_t stream = nullptr);

void cuda_move_lines_ull(unsigned long long *h_data, const CudaLineDesc *h_lines, std::size_t line_count,
						 std::size_t line_len, cudaStream_t stream = nullptr);

// NOTE: CUDA device code does not reliably support long double math on all toolchains.
// We expose these as double-based pipelines for portability.
void cuda_move_lines_ld(double *h_data, const CudaLineDesc *h_lines, std::size_t line_count, std::size_t line_len,
						cudaStream_t stream = nullptr);

void cuda_move_lines_uld(double *h_data, const CudaLineDesc *h_lines, std::size_t line_count, std::size_t line_len,
						 cudaStream_t stream = nullptr);


#endif //GAME_2048_QUICK_CUDAINTERFACE_H
