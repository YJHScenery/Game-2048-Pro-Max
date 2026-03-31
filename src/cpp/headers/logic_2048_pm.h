//
// Created by jehor on 2026/3/29.
//

#ifndef GAME_2048_QUICK_LOGIC2048BASE_H
#define GAME_2048_QUICK_LOGIC2048BASE_H

#include <Eigen/Dense>
#include <unsupported/Eigen/CXX11/Tensor>

#include <array>
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <type_traits>
#include <vector>
#include <random>

#include "hpc_interface.h"

/**
 * @class Logic2048_tm
 * @brief 用于构建 2048 逻辑的底层抽象框架
 * @details 使用C++模板元编程实现，可以适配多维度多形状。
 * @param Dimension 2048的维度
 * @param DimensionSize... 各维度的长度
 * @note 此为 C++模板元编程版本，后期将要引入非模板版本。
 */
enum class MoveDirection : char
{
    Negative = -1, // 沿此维度下索引减小方向
    Positive = 1,  // 沿此维度下索引增大方向
};

template <typename MetaType = size_t, typename SizeType = size_t, SizeType Dimension = 2, SizeType... DimensionSize>
    requires((Dimension >= 2) && sizeof...(DimensionSize) == Dimension && ((DimensionSize != 0) && ...))
class Logic2048_tm
{
public:
    using meta_type_ = MetaType; //! 2048 的元数据类型

    using size_type_ = SizeType; //! 框架下标与尺寸标准类型

    using data_mesh_type_ = Eigen::Tensor<meta_type_, Dimension, Eigen::RowMajor, size_type_>; //! @brief 张量数据类型，m_data 的类型

    using size_mesh_type_ = Eigen::array<size_type_, Dimension>; //! @brief 此类型定义了棋盘的尺寸类型，可以用来创建下标

    using std_size_mesh_type_ = std::array<size_type_, Dimension>;

    using array_index_mesh_type_ = Eigen::array<size_type_, Dimension - 1>; //! @brief 此类型定义了查找某一个线性数组的下标所需要的类型

    using location_meta_type_ = std::pair<size_mesh_type_, meta_type_>; //! @brief 用于构建稀疏矩阵或定位单一元数据

    Logic2048_tm() = default;

    explicit Logic2048_tm(data_mesh_type_ data) : m_data(std::move(data)) {}

    /**
     * @brief 清空棋盘并随机生成初始块（默认2个）。
     * @note 仅使用 STL 随机数；不依赖 Qt。
     */
    void resetAndSeed(std::size_t initialTileCount = 2);

    /**
     * @brief 执行一次移动；若棋盘发生变化，则在空位随机生成一个新块（2/4）。
     * @return 若移动导致棋盘变化则返回 true，否则返回 false。
     */
    bool operateAndSpawn(size_type_ dim, MoveDirection dir);

    /**
     * @brief 返回 RowMajor 扁平化数据（复制）。
     */
    [[nodiscard]] std::vector<meta_type_> flatData() const;

    /**
     * 2048移动操作
     * @param dim 维度序列号
     * @param dir 移动方向
     */
    void operate(size_type_ dim, MoveDirection dir);

    void outputData();

    [[nodiscard]] data_mesh_type_ getData() { return m_data; }
    [[nodiscard]] const data_mesh_type_ &getDataRef() const { return m_data; }

    constexpr static size_mesh_type_ getSizeArray() { return sizes_; }

    template <typename T, size_t NDims>
    static void printTensor(const Eigen::Tensor<T, NDims, Eigen::RowMajor> &tensor, int indent = 0);

private:
    bool operateInternal(size_type_ dim, MoveDirection dir);
    bool spawnRandomTile();
    meta_type_ sampleNewTileValue();

    /**
     * 构建 @ref strides_ 编译期常量
     * @return 返回生成的 strides 编译期数据
     */
    constexpr static std_size_mesh_type_ buildStrides();

    /**
     * 在 @ref m_data 中为 0 的位置添加给定数目的随机预设值，取值范围由 presetValues 给出
     * @tparam Container 包含预设值的容器类型
     * @param numCount 添加值的数量
     * @param presetValues 预设值列表
     * @return 添加成功后返回 true，否则返回 false
     */
    template <template <typename Val_T, typename...> typename Container>
    bool setRandomLocationValue(size_t numCount, const Container<meta_type_> &presetValues);

    constexpr static size_mesh_type_ sizes_{DimensionSize...};
    constexpr static size_type_ total_elems_{(DimensionSize * ...)};
    constexpr static std_size_mesh_type_ strides_{buildStrides()};

    data_mesh_type_ m_data{sizes_};

    std::mt19937_64 m_rng{std::random_device{}()};
};

#define CURRENT_TEMPLATE_DEFINITION template <typename MetaType, typename SizeType, SizeType Dimension, SizeType... DimensionSize> \
    requires((Dimension >= 2) && sizeof...(DimensionSize) == Dimension && ((DimensionSize != 0) && ...))

CURRENT_TEMPLATE_DEFINITION
constexpr typename Logic2048_tm<MetaType, SizeType, Dimension, DimensionSize...>::std_size_mesh_type_ Logic2048_tm<
    MetaType, SizeType, Dimension, DimensionSize...>::buildStrides()
{
    {
        std_size_mesh_type_ strides{};
        strides[Dimension - 1] = 1;
        for (size_type_ d = Dimension - 1; d-- > 0;)
        {
            strides[d] = strides[d + 1] * sizes_[d + 1];
        }
        return strides;
    }
}

CURRENT_TEMPLATE_DEFINITION
void Logic2048_tm<MetaType, SizeType, Dimension, DimensionSize...>::operate(size_type_ dim, const MoveDirection dir)
{
    (void)operateInternal(dim, dir);
}

CURRENT_TEMPLATE_DEFINITION
bool Logic2048_tm<MetaType, SizeType, Dimension, DimensionSize...>::operateInternal(size_type_ dim, const MoveDirection dir)
{
    if (dim >= Dimension)
        return false;

    const size_type_ line_len{sizes_[dim]};
    if (line_len == 0)
        return false;

    constexpr size_type_ total_elems = total_elems_;
    if (total_elems % line_len != 0)
        return false;

    const size_type_ line_count = total_elems / line_len;
    const size_type_ start_index = (dir == MoveDirection::Negative) ? 0 : (line_len - 1);
    const std::int64_t step = static_cast<std::int64_t>(strides_[dim]) *
                              ((dir == MoveDirection::Negative) ? static_cast<std::int64_t>(1)
                                                                : static_cast<std::int64_t>(-1));

    std::vector<StandardLineDesc> lines;
    lines.reserve(static_cast<std::size_t>(line_count));

    std_size_mesh_type_ idx{};
    idx[dim] = start_index;

    while (true)
    {
        std::uint64_t offset = 0;
        for (size_type_ d = 0; d < Dimension; ++d)
            offset += static_cast<std::uint64_t>(idx[d] * strides_[d]);

        lines.push_back(StandardLineDesc{offset, step});

        // odometer increment on all dims except dim
        std::int64_t carry_dim = static_cast<std::int64_t>(Dimension) - 1;
        for (; carry_dim >= 0; --carry_dim)
        {
            const auto d = static_cast<size_type_>(carry_dim);
            if (d == dim)
                continue;
            ++idx[d];
            if (idx[d] < sizes_[d])
                break;
            idx[d] = 0;
        }

        if (carry_dim < 0)
            break;
    }

    if (lines.size() != static_cast<std::size_t>(line_count))
        return false;

    meta_type_ *raw = m_data.data();

    std::vector<long long> buf(total_elems);
    for (size_type_ i = 0; i < total_elems; ++i)
        buf[i] = static_cast<long long>(raw[i]);

    const std::vector<long long> before = buf;

    int *device{nullptr};

    if constexpr (Dimension <= 10)
    {
        move_lines_cpu(buf.data(), lines.data(), static_cast<std::size_t>(line_count), static_cast<std::size_t>(line_len));
    }
    else
    {
        move_lines_gpu(buf.data(), lines.data(), static_cast<std::size_t>(line_count), static_cast<std::size_t>(line_len), device);
    }

    const bool changed = (buf != before);

    for (size_type_ i = 0; i < total_elems; ++i)
        raw[i] = static_cast<meta_type_>(buf[i]);

    return changed;
}

CURRENT_TEMPLATE_DEFINITION
void Logic2048_tm<MetaType, SizeType, Dimension, DimensionSize...>::resetAndSeed(const std::size_t initialTileCount)
{
    m_data.setZero();
    for (std::size_t i = 0; i < initialTileCount; ++i)
        (void)spawnRandomTile();
}

CURRENT_TEMPLATE_DEFINITION
bool Logic2048_tm<MetaType, SizeType, Dimension, DimensionSize...>::operateAndSpawn(size_type_ dim, const MoveDirection dir)
{
    const bool changed = operateInternal(dim, dir);
    if (changed)
        (void)spawnRandomTile();
    return changed;
}

CURRENT_TEMPLATE_DEFINITION
std::vector<typename Logic2048_tm<MetaType, SizeType, Dimension, DimensionSize...>::meta_type_>
Logic2048_tm<MetaType, SizeType, Dimension, DimensionSize...>::flatData() const
{
    const meta_type_ *raw = m_data.data();
    return std::vector<meta_type_>(raw, raw + total_elems_);
}

CURRENT_TEMPLATE_DEFINITION
typename Logic2048_tm<MetaType, SizeType, Dimension, DimensionSize...>::meta_type_
Logic2048_tm<MetaType, SizeType, Dimension, DimensionSize...>::sampleNewTileValue()
{
    // 90% -> 2, 10% -> 4
    std::uniform_int_distribution<int> dist(0, 9);
    return (dist(m_rng) == 0) ? static_cast<meta_type_>(4) : static_cast<meta_type_>(2);
}

CURRENT_TEMPLATE_DEFINITION
bool Logic2048_tm<MetaType, SizeType, Dimension, DimensionSize...>::spawnRandomTile()
{
    meta_type_ *raw = m_data.data();
    std::vector<size_type_> empties;
    empties.reserve(static_cast<std::size_t>(total_elems_));
    for (size_type_ i = 0; i < total_elems_; ++i)
    {
        if (raw[i] == static_cast<meta_type_>(0))
            empties.push_back(i);
    }
    if (empties.empty())
        return false;

    std::uniform_int_distribution<std::size_t> pick(0, empties.size() - 1);
    const auto chosen = empties[pick(m_rng)];
    raw[chosen] = sampleNewTileValue();
    return true;
}

CURRENT_TEMPLATE_DEFINITION
void Logic2048_tm<MetaType, SizeType, Dimension, DimensionSize...>::outputData()
{
    printTensor<meta_type_, Dimension>(m_data, 0);
}

CURRENT_TEMPLATE_DEFINITION
template <typename T, size_t NDims>
void Logic2048_tm<MetaType, SizeType, Dimension, DimensionSize...>::printTensor(const Eigen::Tensor<T, NDims, Eigen::RowMajor> &tensor,
                                                                                int indent)
{
    std::string prefix(indent, ' ');
    if constexpr (NDims == 1)
    {
        for (int i = 0; i < tensor.dimension(0); ++i)
            std::cout << prefix << tensor(i) << " ";
        std::cout << "\n";
    }
    else
    {
        for (int i = 0; i < tensor.dimension(0); ++i)
        {
            std::cout << prefix << "[Slice " << i << "]\n";
            auto slice = tensor.chip(i, 0); // 沿第0维切片，偏移量为i
            printTensor<T, NDims - 1>(slice, indent + 2);
        }
    }
}

CURRENT_TEMPLATE_DEFINITION
template <template <typename Val_T, typename...> typename Container>
bool Logic2048_tm<MetaType, SizeType, Dimension, DimensionSize...>::setRandomLocationValue(size_t numCount, const Container<meta_type_> &presetValues)
{
    if (numCount == 0)
        return true;

    meta_type_ *raw = m_data.data();

    std::vector<size_type_> empties;
    empties.reserve(static_cast<std::size_t>(total_elems_));
    for (size_type_ i = 0; i < total_elems_; ++i)
    {
        if (raw[i] == static_cast<meta_type_>(0))
            empties.push_back(i);
    }
    if (empties.empty())
        return false;

    if (presetValues.begin() == presetValues.end())
        return false;

    std::shuffle(empties.begin(), empties.end(), m_rng);

    // preset value random pick
    const auto presetCount = static_cast<std::size_t>(std::distance(presetValues.begin(), presetValues.end()));
    std::uniform_int_distribution<std::size_t> presetPick(0, presetCount - 1);

    const std::size_t actualCount = std::min<std::size_t>(numCount, empties.size());
    for (std::size_t n = 0; n < actualCount; ++n)
    {
        const auto cell = empties[n];
        const auto pvIndex = presetPick(m_rng);
        auto it = presetValues.begin();
        std::advance(it, static_cast<std::ptrdiff_t>(pvIndex));
        raw[cell] = *it;
    }

    return true;
}

#endif // GAME_2048_QUICK_LOGIC2048BASE_H
