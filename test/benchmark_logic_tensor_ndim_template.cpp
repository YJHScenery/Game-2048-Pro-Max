#include <algorithm>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <random>
#include <unordered_set>
#include <vector>

#include "logic_2048_tmp.h"

// Generic N-D tensor benchmark for Logic2048_tm::operate().
// This file is intentionally self-contained and does not modify CMake.

namespace
{

    template <std::size_t NDim, std::size_t... Dims>
    void benchmark_logic2048_tensor_operate_ndim()
    {
        static_assert(NDim >= 2, "NDim must be >= 2 for Logic2048_tm");
        static_assert(sizeof...(Dims) == NDim, "NDim must equal number of Dims");
        static_assert(((Dims != 0) && ...), "All Dims must be non-zero");

        using Logic = Logic2048_tm<Architecture::ArchDynamic, std::size_t, std::size_t, static_cast<std::size_t>(NDim), static_cast<std::size_t>(Dims)...>;

        constexpr std::size_t kTotalElems = (static_cast<std::size_t>(Dims) * ...);
        constexpr std::size_t kRequestedNonZeros = 128;
        constexpr std::size_t kIters = 200;

        auto sizes = Logic::getSizeArray();
        typename Logic::data_mesh_type_ tensor(sizes);
        tensor.setZero();

        std::mt19937_64 rng{std::random_device{}()};
        std::uniform_int_distribution<std::size_t> index_dist(0, kTotalElems - 1);
        std::uniform_int_distribution<int> value_pick(0, 2);
        static constexpr std::size_t kValues[3] = {2ull, 4ull, 8ull};

        const std::size_t nonzeros = std::min<std::size_t>(kRequestedNonZeros, kTotalElems);

        std::unordered_set<std::size_t> picked;
        picked.reserve(nonzeros * 2);
        while (picked.size() < nonzeros)
            picked.insert(index_dist(rng));

        for (std::size_t idx : picked)
            tensor.data()[idx] = kValues[value_pick(rng)];

        Logic logic{tensor};

        std::uniform_int_distribution<std::size_t> dim_dist(0, NDim - 1);
        std::bernoulli_distribution dir_dist(0.5);

        for (std::size_t i = 0; i < 10; ++i)
        {
            const auto dim = dim_dist(rng);
            const auto dir = dir_dist(rng) ? MoveDirection::Positive : MoveDirection::Negative;
            logic.operate(dim, dir);
        }

        long double sum_ns = 0.0L;
        std::uint64_t min_ns = 0;
        std::uint64_t max_ns = 0;

        for (std::size_t i = 0; i < kIters; ++i)
        {
            const auto dim = dim_dist(rng);
            const auto dir = dir_dist(rng) ? MoveDirection::Positive : MoveDirection::Negative;

            const auto t0 = std::chrono::steady_clock::now();
            logic.operate(dim, dir);
            const auto t1 = std::chrono::steady_clock::now();

            const auto ns = static_cast<std::uint64_t>(
                std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count());

            sum_ns += static_cast<long double>(ns);
            if (i == 0 || ns < min_ns)
                min_ns = ns;
            if (i == 0 || ns > max_ns)
                max_ns = ns;
        }

        const long double avg_ns = (kIters == 0) ? 0.0L : (sum_ns / static_cast<long double>(kIters));
        const long double avg_us = avg_ns / 1000.0L;

        std::cout << "[Logic2048_tm] N-D tensor operate() benchmark\n"
                  << "  NDim=" << NDim << ", total=" << kTotalElems << ", nonzeros=" << nonzeros << ", iters=" << kIters << "\n"
                  << "  avg=" << static_cast<double>(avg_us) << " us/call"
                  << "  (min=" << min_ns << " ns, max=" << max_ns << " ns)\n";
    }

} // namespace

// Customize sizes by editing ONLY the template arguments below.
// Example: 6D 8*8*8*8*8*8
void benchmark_logic2048_tensor_operate_custom()
{
    benchmark_logic2048_tensor_operate_ndim<8, 8, 8, 8, 8, 8, 8, 8, 8>();
}
