#include <chrono>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <random>
#include <unordered_set>
#include <vector>

#include "logic_2048_tmp.h"

// 6D: 8*8*8*8*8*8 tensor benchmark for Logic2048_tm::operate()
// Note: This file is intentionally self-contained and does not modify CMake.

void benchmark_logic2048_tensor_6d_operate()
{
    using Logic = Logic2048_tm<Architecture::ArchDynamic, std::size_t, std::size_t, 6, 8, 8, 8, 8, 8, 8>;

    // Build an 8^6 sparse tensor with a few random {2,4,8} values.
    constexpr std::size_t kTotalElems = 8ull * 8ull * 8ull * 8ull * 8ull * 8ull; // 262144
    constexpr std::size_t kNonZeros = 128;                                       // sparse: adjust if needed

    auto sizes = Logic::getSizeArray();
    Logic::data_mesh_type_ tensor(sizes);
    tensor.setZero();

    std::mt19937_64 rng{std::random_device{}()};
    std::uniform_int_distribution<std::size_t> index_dist(0, kTotalElems - 1);
    std::uniform_int_distribution<int> value_pick(0, 2);
    static constexpr std::size_t kValues[3] = {2ull, 4ull, 8ull};

    std::unordered_set<std::size_t> picked;
    picked.reserve(kNonZeros * 2);
    while (picked.size() < kNonZeros)
    {
        picked.insert(index_dist(rng));
    }

    for (std::size_t idx : picked)
    {
        tensor.data()[idx] = kValues[value_pick(rng)];
    }

    Logic logic{tensor};

    // Benchmark: time repeated operate() calls.
    constexpr std::size_t kIters = 200;
    std::uniform_int_distribution<std::size_t> dim_dist(0, 5);
    std::bernoulli_distribution dir_dist(0.5);

    // Warm-up (helps reduce first-call overhead effects)
    for (std::size_t i = 0; i < 10; ++i)
    {
        const auto dim = dim_dist(rng);
        const auto dir = dir_dist(rng) ? MoveDirection::Positive : MoveDirection::Negative;
        logic.operate(dim, dir);
    }

    std::vector<std::uint64_t> ns_samples;
    ns_samples.reserve(kIters);

    for (std::size_t i = 0; i < kIters; ++i)
    {
        const auto dim = dim_dist(rng);
        const auto dir = dir_dist(rng) ? MoveDirection::Positive : MoveDirection::Negative;

        const auto t0 = std::chrono::steady_clock::now();
        logic.operate(dim, dir);
        const auto t1 = std::chrono::steady_clock::now();

        const auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count();
        ns_samples.push_back(static_cast<std::uint64_t>(ns));
    }

    // Compute average time per call.
    long double sum_ns = 0.0L;
    std::uint64_t min_ns = ns_samples.empty() ? 0 : ns_samples[0];
    std::uint64_t max_ns = ns_samples.empty() ? 0 : ns_samples[0];

    for (auto v : ns_samples)
    {
        sum_ns += static_cast<long double>(v);
        if (v < min_ns)
            min_ns = v;
        if (v > max_ns)
            max_ns = v;
    }

    const long double avg_ns = ns_samples.empty() ? 0.0L : (sum_ns / static_cast<long double>(ns_samples.size()));
    const long double avg_us = avg_ns / 1000.0L;

    std::cout << "[Logic2048_tm] 6D 8^6 sparse tensor operate() benchmark\n"
              << "  nonzeros=" << kNonZeros << ", iters=" << kIters << "\n"
              << "  avg=" << static_cast<double>(avg_us) << " us/call"
              << "  (min=" << min_ns << " ns, max=" << max_ns << " ns)\n";
}
