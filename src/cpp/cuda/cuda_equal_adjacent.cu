#include "cuda_interface.cuh"
#include <vector>

template <typename T, size_t nDims, size_t... Dimensions>
__global__ void check_adjacent_kernel(const T* tensorData, EqualPair* results, size_t* resultSize)
{
    const unsigned idx{blockIdx.x * blockDim.x + threadIdx.x};
    constexpr size_t size{(Dimensions * ...)};
    constexpr size_t dimsList[nDims]{Dimensions...};
    if (idx >= size) return;

    size_t strides[nDims];
    strides[nDims - 1] = 1;

    for (long long d = static_cast<long long>(nDims) - 2; d >= 0; --d) {
        strides[d] = strides[d + 1] * dimsList[d + 1];
    }

    size_t pos[nDims];
    unsigned remaining{idx};
    for (size_t d{0}; d < nDims; ++d) {
        pos[d] = remaining * strides[d];
        remaining %= strides[d];
    }

    for (size_t d{0}; d < nDims; d++) {
        if (pos[d] + 1 < dimsList[d]) {
            // 计算相邻元素的线性索引
            const size_t next_idx = idx + strides[d];

            // 比较当前元素与相邻元素
            if (tensorData[idx] == tensorData[next_idx]) {
                // 记录相等对（原子操作）
                const size_t count = atomicAdd(resultSize, 1);
                EqualPair* result = &results[count];

                // 存储起始位置和维度
                for (size_t i = 0; i < nDims; i++) {
                    result->pos[i] = pos[i];
                }
                result->dim = d;
            }
        }
    }
}

template <typename T, size_t nDims, size_t... Dimensions>
std::vector<EqualPair> find_equal_adjacent(const T* tensor_data)
{
    constexpr size_t tensor_len{(Dimensions * ...)};
    // 1. 上传数据到GPU
    T* d_data;
    cudaMalloc(&d_data, tensor_len * sizeof(float));
    cudaMemcpy(d_data, tensor_data, tensor_len * sizeof(float), cudaMemcpyHostToDevice);

    // 2. 配置CUDA kernel
    int block_size = 256;
    size_t grid_size = (tensor_len + block_size - 1) / block_size;
    EqualPair* d_results;
    size_t* d_result_count;
    cudaMalloc(&d_results, nDims * sizeof(EqualPair));
    cudaMalloc(&d_result_count, sizeof(int));
    cudaMemset(d_result_count, 0, sizeof(int));

    // 3. 启动kernel
    check_adjacent_kernel<T, nDims, Dimensions...><<<grid_size, block_size>>>(d_data, d_results, d_result_count);

    // 4. 下载结果并处理
    size_t result_count;
    cudaMemcpy(&result_count, d_result_count, sizeof(int), cudaMemcpyDeviceToHost);
    std::vector<EqualPair> results(result_count);
    cudaMemcpy(results.data(), d_results, result_count * sizeof(EqualPair), cudaMemcpyDeviceToHost);

    // 5. 清理资源
    cudaFree(d_data);
    cudaFree(d_results);
    cudaFree(d_result_count);

    return results;
}


template __global__ void check_adjacent_kernel<size_t, 2, 4, 4>(const size_t* tensorData, EqualPair* results,
                                                                size_t* resultSize);
template __global__ void check_adjacent_kernel<size_t, 2, 6, 6>(const size_t* tensorData, EqualPair* results,
                                                                size_t* resultSize);
template __global__ void check_adjacent_kernel<size_t, 2, 8, 8>(const size_t* tensorData, EqualPair* results,
                                                                size_t* resultSize);
template __global__ void check_adjacent_kernel<size_t, 3, 4, 4, 4>(const size_t* tensorData, EqualPair* results,
                                                                   size_t* resultSize);
template __global__ void check_adjacent_kernel<size_t, 3, 6, 6, 6>(const size_t* tensorData, EqualPair* results,
                                                                   size_t* resultSize);
template __global__ void check_adjacent_kernel<size_t, 3, 8, 8, 8>(const size_t* tensorData, EqualPair* results,
                                                                   size_t* resultSize);

template std::vector<EqualPair> find_equal_adjacent<size_t, 2, 4, 4>(const size_t* tensor_data);

template  std::vector<EqualPair> find_equal_adjacent<size_t, 2, 6, 6>(const size_t* tensor_data);

template  std::vector<EqualPair> find_equal_adjacent<size_t, 2, 8, 8>(const size_t* tensor_data);

template  std::vector<EqualPair> find_equal_adjacent<size_t, 3, 4, 4, 4>(const size_t* tensor_data);

template  std::vector<EqualPair> find_equal_adjacent<size_t, 3, 6, 6, 6>(const size_t* tensor_data);

template  std::vector<EqualPair> find_equal_adjacent<size_t, 3, 8, 8, 8>(const size_t* tensor_data);

