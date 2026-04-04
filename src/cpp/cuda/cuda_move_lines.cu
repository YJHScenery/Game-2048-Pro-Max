#include "cuda_interface.cuh"

#include <cstdio>
#include <string>
#include <stdexcept>

static void cuda_throw_if_failed(cudaError_t err, const char *what)
{
    if (err != cudaSuccess)
    {
        throw std::runtime_error(std::string("CUDA error: ") + what + ": " + cudaGetErrorString(err));
    }
}

static dim3 choose_block_dim(std::size_t n)
{
    (void)n;
    return {256, 1, 1};
}

static dim3 choose_grid_dim(std::size_t n, dim3 block)
{
    const std::size_t blocks = (n + block.x - 1) / block.x;
    return {static_cast<unsigned int>(blocks), 1, 1};
}

template <typename T>
__global__ void move_lines_kernel(T *data, const StandardLineDesc *lines, std::size_t line_count, std::size_t len)
{
    const std::size_t line_id = blockIdx.x * blockDim.x + threadIdx.x;
    if (line_id >= line_count)
        return;

    const StandardLineDesc desc = lines[line_id];
    const std::uint64_t base = desc.start;
    const std::int64_t step = desc.step;

    bool has_prev = false;
    T prev{};
    std::size_t write = 0;

    for (std::size_t read = 0; read < len; ++read)
    {
        const std::int64_t pos = static_cast<std::int64_t>(base) + static_cast<std::int64_t>(read) * step;
        const T v = data[pos];
        if (v == 0.0)
            continue;

        if (has_prev && v == prev)
        {
            const T merged = prev + v;
            const std::int64_t wpos = static_cast<std::int64_t>(base) + static_cast<std::int64_t>(write) * step;
            data[wpos] = merged;
            ++write;
            has_prev = false;
        }
        else
        {
            if (has_prev)
            {
                const std::int64_t wpos = static_cast<std::int64_t>(base) + static_cast<std::int64_t>(write) * step;
                data[wpos] = prev;
                ++write;
            }
            prev = v;
            has_prev = true;
        }
    }

    if (has_prev)
    {
        const std::int64_t wpos = static_cast<std::int64_t>(base) + static_cast<std::int64_t>(write) * step;
        data[wpos] = prev;
        ++write;
    }

    for (std::size_t i = write; i < len; ++i)
    {
        const std::int64_t wpos = static_cast<std::int64_t>(base) + static_cast<std::int64_t>(i) * step;
        data[wpos] = 0.0;
    }
}



template <typename T>
void cuda_move_lines(T* h_data, const StandardLineDesc* h_lines, std::size_t line_count, std::size_t line_len, cudaStream_t stream)
{
    if (line_count == 0 || line_len == 0)
        return;

    const std::size_t total_elems = line_count * line_len;

    T *d_data = nullptr;
    StandardLineDesc *d_lines = nullptr;

    cuda_throw_if_failed(cudaMalloc(reinterpret_cast<void **>(&d_data), total_elems * sizeof(T)),
                         "cudaMalloc d_data");
    cuda_throw_if_failed(cudaMalloc(reinterpret_cast<void **>(&d_lines), line_count * sizeof(StandardLineDesc)),
                         "cudaMalloc d_lines");

    cuda_throw_if_failed(cudaMemcpyAsync(d_data, h_data, total_elems * sizeof(T), cudaMemcpyHostToDevice, stream),
                         "cudaMemcpyAsync H2D data");
    cuda_throw_if_failed(
        cudaMemcpyAsync(d_lines, h_lines, line_count * sizeof(StandardLineDesc), cudaMemcpyHostToDevice, stream),
        "cudaMemcpyAsync H2D lines");

    const dim3 block = choose_block_dim(line_count);
    const dim3 grid = choose_grid_dim(line_count, block);
    move_lines_kernel<T><<<grid, block, 0, stream>>>(d_data, d_lines, line_count, line_len);
    cuda_throw_if_failed(cudaGetLastError(), "kernel launch");

    cuda_throw_if_failed(
        cudaMemcpyAsync(h_data, d_data, total_elems * sizeof(T), cudaMemcpyDeviceToHost, stream),
        "cudaMemcpyAsync D2H data");
    cuda_throw_if_failed(cudaStreamSynchronize(stream), "cudaStreamSynchronize");

    cudaFree(d_lines);
    cudaFree(d_data);
}

template __global__ void move_lines_kernel<size_t>(size_t *data, const StandardLineDesc *lines, std::size_t line_count, std::size_t len);

template __global__ void move_lines_kernel<long long>(long long *data, const StandardLineDesc *lines, std::size_t line_count, std::size_t len);

template __global__ void move_lines_kernel<double>(double *data, const StandardLineDesc *lines, std::size_t line_count, std::size_t len);

template void cuda_move_lines<size_t>(size_t* h_data, const StandardLineDesc* h_lines, std::size_t line_count, std::size_t line_len, cudaStream_t stream);

template void cuda_move_lines<long long>(long long* h_data, const StandardLineDesc* h_lines, std::size_t line_count, std::size_t line_len, cudaStream_t stream);

template void cuda_move_lines<double>(double* h_data, const StandardLineDesc* h_lines, std::size_t line_count, std::size_t line_len, cudaStream_t stream);