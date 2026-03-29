#include "cuda_interface.h"

#include <cstdio>
#include <string>
#include <stdexcept>

static inline void cuda_throw_if_failed(cudaError_t err, const char *what)
{
    if (err != cudaSuccess)
    {
        throw std::runtime_error(std::string("CUDA error: ") + what + ": " + cudaGetErrorString(err));
    }
}

// -------------------- device-side line resolve (no templates) --------------------

__global__ void move_lines_ll_kernel(long long *data, const CudaLineDesc *lines, std::size_t line_count, std::size_t len)
{
    const std::size_t line_id = blockIdx.x * blockDim.x + threadIdx.x;
    if (line_id >= line_count)
        return;

    const CudaLineDesc desc = lines[line_id];
    const std::uint64_t base = desc.start;
    const std::int64_t step = desc.step;

    bool has_prev = false;
    long long prev{};
    std::size_t write = 0;

    for (std::size_t read = 0; read < len; ++read)
    {
        const std::int64_t pos = static_cast<std::int64_t>(base) + static_cast<std::int64_t>(read) * step;
        const long long v = data[pos];
        if (v == 0)
            continue;

        if (has_prev && v == prev)
        {
            const long long merged = prev + v;
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
        data[wpos] = 0;
    }
}

__global__ void move_lines_ull_kernel(unsigned long long *data, const CudaLineDesc *lines, std::size_t line_count,
                                      std::size_t len)
{
    const std::size_t line_id = blockIdx.x * blockDim.x + threadIdx.x;
    if (line_id >= line_count)
        return;

    const CudaLineDesc desc = lines[line_id];
    const std::uint64_t base = desc.start;
    const std::int64_t step = desc.step;

    bool has_prev = false;
    unsigned long long prev{};
    std::size_t write = 0;

    for (std::size_t read = 0; read < len; ++read)
    {
        const std::int64_t pos = static_cast<std::int64_t>(base) + static_cast<std::int64_t>(read) * step;
        const unsigned long long v = data[pos];
        if (v == 0ull)
            continue;

        if (has_prev && v == prev)
        {
            const unsigned long long merged = prev + v;
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
        data[wpos] = 0ull;
    }
}

__global__ void move_lines_ld_kernel(double *data, const CudaLineDesc *lines, std::size_t line_count, std::size_t len)
{
    const std::size_t line_id = blockIdx.x * blockDim.x + threadIdx.x;
    if (line_id >= line_count)
        return;

    const CudaLineDesc desc = lines[line_id];
    const std::uint64_t base = desc.start;
    const std::int64_t step = desc.step;

    bool has_prev = false;
    double prev{};
    std::size_t write = 0;

    for (std::size_t read = 0; read < len; ++read)
    {
        const std::int64_t pos = static_cast<std::int64_t>(base) + static_cast<std::int64_t>(read) * step;
        const double v = data[pos];
        if (v == 0.0)
            continue;

        if (has_prev && v == prev)
        {
            const double merged = prev + v;
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

__global__ void move_lines_uld_kernel(double *data, const CudaLineDesc *lines, std::size_t line_count, std::size_t len)
{
    const std::size_t line_id = blockIdx.x * blockDim.x + threadIdx.x;
    if (line_id >= line_count)
        return;

    const CudaLineDesc desc = lines[line_id];
    const std::uint64_t base = desc.start;
    const std::int64_t step = desc.step;

    bool has_prev = false;
    double prev{};
    std::size_t write = 0;

    for (std::size_t read = 0; read < len; ++read)
    {
        const std::int64_t pos = static_cast<std::int64_t>(base) + static_cast<std::int64_t>(read) * step;
        const double v = data[pos];
        if (v == 0.0)
            continue;

        if (has_prev && v == prev)
        {
            const double merged = prev + v;
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

static inline dim3 choose_block_dim(std::size_t n)
{
    (void)n;
    return dim3(256, 1, 1);
}

static inline dim3 choose_grid_dim(std::size_t n, dim3 block)
{
    const std::size_t blocks = (n + block.x - 1) / block.x;
    return dim3(static_cast<unsigned int>(blocks), 1, 1);
}

// -------------------- public API --------------------

void cuda_move_lines_ll(long long *h_data, const CudaLineDesc *h_lines, std::size_t line_count, std::size_t line_len,
                        cudaStream_t stream)
{
    if (line_count == 0 || line_len == 0)
        return;

    const std::size_t total_elems = line_count * line_len;

    long long *d_data = nullptr;
    CudaLineDesc *d_lines = nullptr;

    cuda_throw_if_failed(cudaMalloc(reinterpret_cast<void **>(&d_data), total_elems * sizeof(long long)),
                         "cudaMalloc d_data");
    cuda_throw_if_failed(cudaMalloc(reinterpret_cast<void **>(&d_lines), line_count * sizeof(CudaLineDesc)),
                         "cudaMalloc d_lines");

    cuda_throw_if_failed(
        cudaMemcpyAsync(d_data, h_data, total_elems * sizeof(long long), cudaMemcpyHostToDevice, stream),
        "cudaMemcpyAsync H2D data");
    cuda_throw_if_failed(
        cudaMemcpyAsync(d_lines, h_lines, line_count * sizeof(CudaLineDesc), cudaMemcpyHostToDevice, stream),
        "cudaMemcpyAsync H2D lines");

    const dim3 block = choose_block_dim(line_count);
    const dim3 grid = choose_grid_dim(line_count, block);
    move_lines_ll_kernel<<<grid, block, 0, stream>>>(d_data, d_lines, line_count, line_len);
    cuda_throw_if_failed(cudaGetLastError(), "kernel launch");

    cuda_throw_if_failed(
        cudaMemcpyAsync(h_data, d_data, total_elems * sizeof(long long), cudaMemcpyDeviceToHost, stream),
        "cudaMemcpyAsync D2H data");
    cuda_throw_if_failed(cudaStreamSynchronize(stream), "cudaStreamSynchronize");

    cudaFree(d_lines);
    cudaFree(d_data);
}

void cuda_move_lines_ull(unsigned long long *h_data, const CudaLineDesc *h_lines, std::size_t line_count,
                         std::size_t line_len, cudaStream_t stream)
{
    if (line_count == 0 || line_len == 0)
        return;

    const std::size_t total_elems = line_count * line_len;

    unsigned long long *d_data = nullptr;
    CudaLineDesc *d_lines = nullptr;

    cuda_throw_if_failed(cudaMalloc(reinterpret_cast<void **>(&d_data), total_elems * sizeof(unsigned long long)),
                         "cudaMalloc d_data");
    cuda_throw_if_failed(cudaMalloc(reinterpret_cast<void **>(&d_lines), line_count * sizeof(CudaLineDesc)),
                         "cudaMalloc d_lines");

    cuda_throw_if_failed(
        cudaMemcpyAsync(d_data, h_data, total_elems * sizeof(unsigned long long), cudaMemcpyHostToDevice, stream),
        "cudaMemcpyAsync H2D data");
    cuda_throw_if_failed(
        cudaMemcpyAsync(d_lines, h_lines, line_count * sizeof(CudaLineDesc), cudaMemcpyHostToDevice, stream),
        "cudaMemcpyAsync H2D lines");

    const dim3 block = choose_block_dim(line_count);
    const dim3 grid = choose_grid_dim(line_count, block);
    move_lines_ull_kernel<<<grid, block, 0, stream>>>(d_data, d_lines, line_count, line_len);
    cuda_throw_if_failed(cudaGetLastError(), "kernel launch");

    cuda_throw_if_failed(
        cudaMemcpyAsync(h_data, d_data, total_elems * sizeof(unsigned long long), cudaMemcpyDeviceToHost, stream),
        "cudaMemcpyAsync D2H data");
    cuda_throw_if_failed(cudaStreamSynchronize(stream), "cudaStreamSynchronize");

    cudaFree(d_lines);
    cudaFree(d_data);
}

void cuda_move_lines_ld(double *h_data, const CudaLineDesc *h_lines, std::size_t line_count, std::size_t line_len,
                        cudaStream_t stream)
{
    if (line_count == 0 || line_len == 0)
        return;

    const std::size_t total_elems = line_count * line_len;

    double *d_data = nullptr;
    CudaLineDesc *d_lines = nullptr;

    cuda_throw_if_failed(cudaMalloc(reinterpret_cast<void **>(&d_data), total_elems * sizeof(double)),
                         "cudaMalloc d_data");
    cuda_throw_if_failed(cudaMalloc(reinterpret_cast<void **>(&d_lines), line_count * sizeof(CudaLineDesc)),
                         "cudaMalloc d_lines");

    cuda_throw_if_failed(cudaMemcpyAsync(d_data, h_data, total_elems * sizeof(double), cudaMemcpyHostToDevice, stream),
                         "cudaMemcpyAsync H2D data");
    cuda_throw_if_failed(
        cudaMemcpyAsync(d_lines, h_lines, line_count * sizeof(CudaLineDesc), cudaMemcpyHostToDevice, stream),
        "cudaMemcpyAsync H2D lines");

    const dim3 block = choose_block_dim(line_count);
    const dim3 grid = choose_grid_dim(line_count, block);
    move_lines_ld_kernel<<<grid, block, 0, stream>>>(d_data, d_lines, line_count, line_len);
    cuda_throw_if_failed(cudaGetLastError(), "kernel launch");

    cuda_throw_if_failed(
        cudaMemcpyAsync(h_data, d_data, total_elems * sizeof(double), cudaMemcpyDeviceToHost, stream),
        "cudaMemcpyAsync D2H data");
    cuda_throw_if_failed(cudaStreamSynchronize(stream), "cudaStreamSynchronize");

    cudaFree(d_lines);
    cudaFree(d_data);
}

void cuda_move_lines_uld(double *h_data, const CudaLineDesc *h_lines, std::size_t line_count, std::size_t line_len,
                         cudaStream_t stream)
{
    if (line_count == 0 || line_len == 0)
        return;

    const std::size_t total_elems = line_count * line_len;

    double *d_data = nullptr;
    CudaLineDesc *d_lines = nullptr;

    cuda_throw_if_failed(cudaMalloc(reinterpret_cast<void **>(&d_data), total_elems * sizeof(double)),
                         "cudaMalloc d_data");
    cuda_throw_if_failed(cudaMalloc(reinterpret_cast<void **>(&d_lines), line_count * sizeof(CudaLineDesc)),
                         "cudaMalloc d_lines");

    cuda_throw_if_failed(cudaMemcpyAsync(d_data, h_data, total_elems * sizeof(double), cudaMemcpyHostToDevice, stream),
                         "cudaMemcpyAsync H2D data");
    cuda_throw_if_failed(
        cudaMemcpyAsync(d_lines, h_lines, line_count * sizeof(CudaLineDesc), cudaMemcpyHostToDevice, stream),
        "cudaMemcpyAsync H2D lines");

    const dim3 block = choose_block_dim(line_count);
    const dim3 grid = choose_grid_dim(line_count, block);
    move_lines_uld_kernel<<<grid, block, 0, stream>>>(d_data, d_lines, line_count, line_len);
    cuda_throw_if_failed(cudaGetLastError(), "kernel launch");

    cuda_throw_if_failed(
        cudaMemcpyAsync(h_data, d_data, total_elems * sizeof(double), cudaMemcpyDeviceToHost, stream),
        "cudaMemcpyAsync D2H data");
    cuda_throw_if_failed(cudaStreamSynchronize(stream), "cudaStreamSynchronize");

    cudaFree(d_lines);
    cudaFree(d_data);
}
