#include <QDebug>
#include <QElapsedTimer>

#include <cstddef>
#include <cstdint>
#include <random>
#include <vector>

#include "logic_2048_tmp.h"
#include <cmath>

namespace
{

QString archName(const std::uint64_t arch)
{
    if (arch == static_cast<std::uint64_t>(ArchCPU))
        return QStringLiteral("CPU");
    if (arch == static_cast<std::uint64_t>(ArchCUDA))
        return QStringLiteral("CUDA");
    if (arch == static_cast<std::uint64_t>(ArchDynamic))
        return QStringLiteral("Dynamic");
    return QStringLiteral("Unknown");
}

template <std::uint64_t Arch, std::size_t... Dims>
int run_random_game_test_arch(const std::size_t maxSteps, const std::uint64_t seed)
{
    static_assert(sizeof...(Dims) >= 2, "Board dimension must be >= 2");

    using Logic = Logic2048_tm<Arch, std::size_t, std::size_t, sizeof...(Dims), Dims...>;

    constexpr std::size_t kDimensionCount = sizeof...(Dims);
    constexpr std::size_t kTotalCells = (Dims * ...);

    std::mt19937_64 rng(seed ^ (Arch * 0x9E3779B185EBCA87ull));
    std::bernoulli_distribution occupyDist(0.12);
    std::uniform_int_distribution<int> tilePick(0, 2);
    static constexpr std::size_t kTileValues[3] = {2ull, 4ull, 8ull};

    std::vector<std::size_t> initBoard(kTotalCells, 0ull);
    for (std::size_t i = 0; i < kTotalCells; ++i) {
        if (occupyDist(rng))
            initBoard[i] = kTileValues[tilePick(rng)];
    }

    Logic logic;
    logic.setData<std::size_t, std::vector>(initBoard);

    std::uniform_int_distribution<std::size_t> dimDist(0, kDimensionCount - 1);
    std::bernoulli_distribution dirDist(0.5);

    qDebug().nospace() << "[RandomGameTest] arch=" << archName(Arch)
                       << " dims=" << kDimensionCount
                       << " totalCells=" << kTotalCells
                       << " maxSteps=" << maxSteps
                       << " seed=" << static_cast<qulonglong>(seed);

    constexpr size_t realMaxSteps{8 * 8 * 8 * 8 * 8 * 8 - 1};
    for (std::size_t step = 0; step < realMaxSteps ; ++step) {
        const std::size_t dim = dimDist(rng);
        const MoveDirection dir = dirDist(rng) ? MoveDirection::Positive : MoveDirection::Negative;

        const auto hashBefore = logic.getHash();

        QElapsedTimer operateTimer;
        operateTimer.start();
        logic.operate(dim, dir);
        const auto operateNs = operateTimer.nsecsElapsed();

        const auto hashAfter = logic.getHash();
        const bool changed = (hashBefore != hashAfter);

        // qDebug().nospace() << "[RandomGameTest][" << archName(Arch) << "]"
        //                    << " step=" << step
        //                    << " operate(dim=" << dim
        //                    << ", dir=" << (dir == MoveDirection::Positive ? "+" : "-")
        //                    << ") ns=" << operateNs
        //                    << " changed=" << changed;

        if (!changed && step >= maxSteps) {
            QElapsedTimer checkTimer;
            checkTimer.start();
            const auto overHint = logic.checkOver();
            const auto checkNs = checkTimer.nsecsElapsed();

            qDebug().nospace() << "[RandomGameTest][" << archName(Arch) << "]"
                               << " step=" << step
                               << " checkOver() ns=" << checkNs
                               << " resultSize=" << overHint.size();

            if (overHint.empty()) {
                qDebug().nospace() << "[RandomGameTest][" << archName(Arch) << "]"
                                   << " game over at step=" << step
                                   << " (checkOver result is empty).";
                return 0;
            }
        }
    }

    qDebug().nospace() << "[RandomGameTest][" << archName(Arch) << "]"
                       << " reached maxSteps without confirmed game over.";
    return 0;
}

} // namespace

int run_random_game_test()
{
    const std::size_t kSteps = 4 * powl(8, 5);
    const std::uint64_t seed = std::random_device{}();

    int rc = 0;
    rc |= run_random_game_test_arch<ArchCPU, 8, 8, 8, 8, 8, 8>(kSteps, seed);
    rc |= run_random_game_test_arch<ArchCUDA, 8, 8, 8, 8, 8, 8>(kSteps, seed);
    return rc;
}
