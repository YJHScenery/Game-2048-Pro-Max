//
// Created by jehor on 2026/3/29.
// Ye Jinghao All Rights Reserved
//

#ifndef GAME_2048_QUICK_LOGIC2048BASE_H
#define GAME_2048_QUICK_LOGIC2048BASE_H

#include <Eigen/Dense>
#include <unsupported/Eigen/CXX11/Tensor>

#include <array>
#include <algorithm>
#include <cmath>
#include <cfloat>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <optional>
#include <type_traits>
#include <utility>
#include <vector>
#include <random>
#include "hash_tools.h"
#include "basic_dependency.h"
#include "cpu_interface.h"
/**
 * @class Logic2048_tm
 * @brief 用于构建 2048 逻辑的底层抽象框架
 * @details 使用C++模板元编程实现，可以适配多维度多形状。
 * @param Dimension 2048的维度
 * @param DimensionSize... 各维度的长度
 * @note 此为 C++模板元编程版本，后期将要引入非模板版本
 * (详见 @file logic_2048_dynamic.h @class Logic2048Dynamic )
 * 以适应运行时自定义尺寸
 */
enum class MoveDirection : std::int8_t
{
    Negative = -1, // 沿此维度下索引减小方向
    Positive = 1,  // 沿此维度下索引增大方向
};

//! @brief 用于强制指定计算平台，便于进行性能测试。
enum Architecture : std::uint64_t
{
    ArchDynamic = 0ull,
    ArchCPU = 0xFull,
    ArchCUDA = 0x000000F0ull,
    ArchCANN = 0x00000F00ull
};

template <typename Size_T>
concept ValidSizeType = std::unsigned_integral<Size_T>;

template <typename Size_T, Size_T nDim, Size_T... Sizes>
concept ValidTensorSize =
    nDim >= 2 &&
    sizeof...(Sizes) == nDim &&
    ((Sizes != 0) && ...);

template <std::uint64_t Arch, typename MetaType = size_t, ValidSizeType SizeType = size_t, SizeType Dimension = 2, SizeType... DimensionSize>
    requires ValidTensorSize<SizeType, Dimension, DimensionSize...>
class Logic2048_tm
{
public:
    using meta_type_ = MetaType; //! 2048 的元数据类型

    using size_type_ = SizeType; //! 框架下标与尺寸标准类型

    using data_mesh_type_ = Eigen::Tensor<meta_type_, Dimension, Eigen::RowMajor, size_type_>;
    //! @brief 张量数据类型，m_data 的类型

    using size_mesh_type_ = Eigen::array<size_type_, Dimension>; //! @brief 此类型定义了棋盘的尺寸类型，可以用来创建下标

    using std_size_mesh_type_ = std::array<size_type_, Dimension>;

    using array_index_mesh_type_ = Eigen::array<size_type_, Dimension - 1>; //! @brief 此类型定义了查找某一个线性数组的下标所需要的类型

    using location_meta_type_ = std::pair<size_mesh_type_, meta_type_>; //! @brief 用于构建稀疏矩阵或定位单一元数据

    using hash_type_ = std::uint64_t;

    Logic2048_tm() = default;

    hash_type_ getHash();

    explicit Logic2048_tm(data_mesh_type_ data) : m_data(std::move(data))
    {
    }

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

    // 执行检查，判断整个张量是否无法通过任何 operation 改变
    std::vector<EqualPair> checkOver();

    template <typename Val_T, template <typename, typename...> typename Container>
    void setData(const Container<Val_T> &ctn);

    struct TileMoveTrace
    {
        size_type_ from{};  // flat index
        size_type_ to{};    // flat index
        meta_type_ value{}; // value before move
        bool merged{};      // whether this source tile participates in a merge
        bool primary{};     // for merged pair: the first tile (kept) vs second (consumed)
    };

    struct TileMergeTrace
    {
        size_type_ to{};
        size_type_ fromA{};
        size_type_ fromB{};
        meta_type_ newValue{};
    };

    struct SpawnTrace
    {
        size_type_ index{};
        meta_type_ value{};
    };

    struct MoveTrace
    {
        bool changed{false};
        std::vector<TileMoveTrace> moves;
        std::vector<TileMergeTrace> merges;
        std::optional<SpawnTrace> spawn;
    };

    /**
     * @brief 执行一次移动并返回移动/合并/生成的 trace（用于 UI 动画）。
     * @note 当前 trace 由 CPU 路径生成；与 move_lines_cpu 逻辑保持一致。
     */
    MoveTrace operateAndSpawnTrace(size_type_ dim, MoveDirection dir);

    /**
     * @brief 返回 RowMajor 扁平化数据（复制）。
     */
    [[nodiscard]] std::vector<meta_type_> flatData() const;

    /**
     * @brief 计算 2D 棋盘的 AI 最佳移动方向。
     * @param depth 搜索深度。小于 0 时按棋盘尺寸自动选择。
     * @return 方向编码：0 左、1 上、2 右、3 下；若无合法移动返回 -1。
     * @note 仅在 Dimension == 2 时有效。
     */
    [[nodiscard]] int getBestMove2D(int depth = -1) const;

    /**
     * @brief 执行 2D 棋盘的 AI 最佳一步，并返回包含生成块的 Trace。
     * @param depth 搜索深度。小于 0 时按棋盘尺寸自动选择。
     * @return 与 operateAndSpawnTrace 一致的移动轨迹；若无合法移动则 changed 为 false。
     * @note 仅在 Dimension == 2 时有效。
     */
    MoveTrace operateBestMoveAndSpawnTrace2D(int depth = -1);

    /**
     * 2048移动操作
     * @param dim 维度序列号
     * @param dir 移动方向
     */
    void operate(size_type_ dim, MoveDirection dir);

    template <typename OutputStream>
    void outputData(OutputStream &out);

    [[nodiscard]] data_mesh_type_ getData() { return m_data; }

    [[nodiscard]] const data_mesh_type_ &getDataRef() const { return m_data; }

    constexpr static size_mesh_type_ getSizeArray() { return sizes_; }

    template <typename T, typename OutputStream, size_t NDims>
    static void printTensor(const Eigen::Tensor<T, NDims, Eigen::RowMajor> &tensor, OutputStream &out, int indent = 0);

private:
    struct AiSearchResult2D
    {
        int move{-1};
        float score{0.0f};
    };

    [[nodiscard]] static int chooseDepth2D(size_type_ size);

    [[nodiscard]] static bool mapMoveToDimDir2D(int move, size_type_ &dim, MoveDirection &dir);

    [[nodiscard]] static bool applyMoveNoSpawn2D(Logic2048_tm &board, int move);

    [[nodiscard]] static bool setFlatCell2D(Logic2048_tm &board, size_type_ index, meta_type_ value);

    [[nodiscard]] static std::vector<size_type_> freeCells2D(const Logic2048_tm &board);

    [[nodiscard]] static float calculateEmpty2D(const std::vector<meta_type_> &data);

    [[nodiscard]] static float calculateMaxNum2D(const std::vector<meta_type_> &data);

    [[nodiscard]] static float calculateSmoothness2D(const std::vector<meta_type_> &data, size_type_ size);

    [[nodiscard]] static float calculateMonotonicity2D(const std::vector<meta_type_> &data, size_type_ size);
    static void markIsland2D(const std::vector<meta_type_> &data,
                             int size,
                             int x,
                             int y,
                             meta_type_ value,
                             std::vector<std::uint8_t> &marked);

    [[nodiscard]] static float calculateIslands2D(const std::vector<meta_type_> &data, size_type_ size);

    [[nodiscard]] static float calculateEvaluation2D(const std::vector<meta_type_> &data, size_type_ size);

    [[nodiscard]] static AiSearchResult2D searchBest2D(Logic2048_tm board,
                                                       int depth,
                                                       float alpha,
                                                       float beta,
                                                       bool playerTurn,
                                                       size_type_ size);

    bool operateInternal(size_type_ dim, MoveDirection dir);

    // 在空位填充一个数字
    bool spawnRandomTile();
    bool spawnRandomTile(SpawnTrace &out);

    //! 随机数生成，90% 2，10% 4
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

template <std::uint64_t Arch, typename MetaType, ValidSizeType SizeType, SizeType Dimension, SizeType... DimensionSize>
    requires ValidTensorSize<SizeType, Dimension, DimensionSize...>
constexpr auto Logic2048_tm<
    Arch, MetaType, SizeType, Dimension, DimensionSize...>::buildStrides() -> std_size_mesh_type_
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

template <std::uint64_t Arch, typename MetaType, ValidSizeType SizeType, SizeType Dimension, SizeType... DimensionSize>
    requires ValidTensorSize<SizeType, Dimension, DimensionSize...>
void Logic2048_tm<Arch, MetaType, SizeType, Dimension, DimensionSize...>::operate(
    size_type_ dim, const MoveDirection dir)
{
    std::ignore = operateInternal(dim, dir);
}

template <std::uint64_t Arch, typename MetaType, ValidSizeType SizeType, SizeType Dimension, SizeType... DimensionSize>
    requires ValidTensorSize<SizeType, Dimension, DimensionSize...>
bool Logic2048_tm<Arch, MetaType, SizeType, Dimension, DimensionSize...>::operateInternal(
    size_type_ dim, const MoveDirection dir)
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
                              ((dir == MoveDirection::Negative)
                                   ? static_cast<std::int64_t>(1)
                                   : static_cast<std::int64_t>(-1));

    std::vector<StandardLineDesc> lines;
    lines.reserve(static_cast<std::size_t>(line_count));

    std_size_mesh_type_ idx{};
    idx[dim] = start_index;

    while (true)
    {
        std::uint64_t offset = 0;
        for (size_type_ d = 0; d < Dimension; ++d)
            offset += static_cast<std::uint64_t>((idx.at(d) ? idx.at(d) : 0) * strides_[d]);

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

    move_lines_cpu(buf.data(), lines.data(), static_cast<std::size_t>(line_count),
                   static_cast<std::size_t>(line_len));

    const bool changed{buf != before};

    for (size_type_ i = 0; i < total_elems; ++i)
        raw[i] = static_cast<meta_type_>(buf[i]);

    return changed;
}

template <std::uint64_t Arch, typename MetaType, ValidSizeType SizeType, SizeType Dimension, SizeType... DimensionSize>
    requires ValidTensorSize<SizeType, Dimension, DimensionSize...>
Logic2048_tm<Arch, MetaType, SizeType, Dimension, DimensionSize...>::hash_type_ Logic2048_tm<Arch, MetaType,
                                                                                             SizeType, Dimension, DimensionSize...>::getHash()
{
    std::size_t seed = 0;

    // 1. 混合维度信息（防止不同形状但内存数据巧合相同的张量哈希冲突）
    // 例如：1x4 和 2x2 如果数据一样，不加维度哈希可能会冲突
    for (int i = 0; i < m_data.rank(); ++i)
    {
        const auto dimValue = m_data.dimension(i);
        hashCombineBytes(seed, reinterpret_cast<const unsigned char *>(&dimValue), sizeof(dimValue));
    }

    // 2. 混合数据内容
    // 直接对内存块进行哈希，效率最高
    const auto dataPtr = reinterpret_cast<const unsigned char *>(m_data.data());
    const std::size_t byteSize = m_data.size() * sizeof(typename data_mesh_type_::Scalar);

    hashCombineBytes(seed, dataPtr, byteSize);

    return seed;
}

template <std::uint64_t Arch, typename MetaType, ValidSizeType SizeType, SizeType Dimension, SizeType... DimensionSize>
    requires ValidTensorSize<SizeType, Dimension, DimensionSize...>
void Logic2048_tm<Arch, MetaType, SizeType, Dimension, DimensionSize...>::resetAndSeed(
    const std::size_t initialTileCount)
{
    m_data.setZero();
    for (std::size_t i = 0; i < initialTileCount; ++i)
        std::ignore = spawnRandomTile();
}

template <std::uint64_t Arch, typename MetaType, ValidSizeType SizeType, SizeType Dimension, SizeType... DimensionSize>
    requires ValidTensorSize<SizeType, Dimension, DimensionSize...>
bool Logic2048_tm<Arch, MetaType, SizeType, Dimension, DimensionSize...>::operateAndSpawn(
    size_type_ dim, const MoveDirection dir)
{
    const bool changed = operateInternal(dim, dir);
    if (changed)
        std::ignore = spawnRandomTile();
    return changed;
}

template <std::uint64_t Arch, typename MetaType, ValidSizeType SizeType, SizeType Dimension, SizeType... DimensionSize>
    requires ValidTensorSize<SizeType, Dimension, DimensionSize...>
std::vector<EqualPair> Logic2048_tm<Arch, MetaType, SizeType, Dimension, DimensionSize...>::checkOver()
{
    return find_equal_cpu<MetaType, Dimension, DimensionSize...>(m_data.data());
}

template <std::uint64_t Arch, typename MetaType, ValidSizeType SizeType, SizeType Dimension, SizeType... DimensionSize>
    requires ValidTensorSize<SizeType, Dimension, DimensionSize...>
template <typename Val_T, template <typename, typename...> class Container>
void Logic2048_tm<Arch, MetaType, SizeType, Dimension, DimensionSize...>::setData(const Container<Val_T> &ctn)
{
    typename std::remove_cvref_t<decltype(ctn)>::size_type ctnSize{ctn.size()};
    if (ctnSize != static_cast<decltype(ctnSize)>(total_elems_))
    {
        return;
    }

    if constexpr (std::is_same_v<std::remove_cv_t<Val_T>, meta_type_>)
    {
        using const_data_mesh_type_ = Eigen::Tensor<const meta_type_, Dimension, Eigen::RowMajor, size_type_>;
        const Eigen::TensorMap<const const_data_mesh_type_> mapped(ctn.data(), DimensionSize...);
        m_data = mapped;
    }
    else
    {
        // Fallback: element-wise copy when the source container's value type differs.
        const auto *ctnData = ctn.data();
        if (total_elems_ != 0 && ctnData == nullptr)
        {
            return;
        }

        const meta_type_ *src = nullptr;
        std::vector<meta_type_> tmp;
        tmp.reserve(static_cast<std::size_t>(total_elems_));
        for (size_type_ i = 0; i < total_elems_; ++i)
        {
            tmp.push_back(static_cast<meta_type_>(
                ctnData[static_cast<typename std::remove_cvref_t<decltype(ctn)>::size_type>(i)]));
        }
        src = tmp.data();

        using const_data_mesh_type_ = Eigen::Tensor<const meta_type_, Dimension, Eigen::RowMajor, size_type_>;
        const Eigen::TensorMap<const const_data_mesh_type_> mapped(src, DimensionSize...);
        m_data = mapped;
    }
}

template <std::uint64_t Arch, typename MetaType, ValidSizeType SizeType, SizeType Dimension, SizeType... DimensionSize>
    requires ValidTensorSize<SizeType, Dimension, DimensionSize...>
auto Logic2048_tm<Arch, MetaType, SizeType, Dimension, DimensionSize...>::operateAndSpawnTrace(
    size_type_ dim, const MoveDirection dir) -> MoveTrace
{
    MoveTrace trace;

    if (dim >= Dimension)
        return trace;

    const size_type_ line_len{sizes_[dim]};
    if (line_len == 0)
        return trace;

    constexpr size_type_ total_elems = total_elems_;
    if (total_elems % line_len != 0)
        return trace;

    const size_type_ line_count = total_elems / line_len;
    const size_type_ start_index = (dir == MoveDirection::Negative) ? 0 : (line_len - 1);
    const std::int64_t step = static_cast<std::int64_t>(strides_[dim]) *
                              ((dir == MoveDirection::Negative)
                                   ? static_cast<std::int64_t>(1)
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
        return trace;

    meta_type_ *raw = m_data.data();

    // operateInternal 当前使用 long long 作为统一缓冲；trace 也沿用以保持一致。
    std::vector<long long> buf(total_elems);
    for (size_type_ i = 0; i < total_elems; ++i)
        buf[i] = static_cast<long long>(raw[i]);

    const std::vector<long long> before{buf};

    trace.moves.reserve(static_cast<std::size_t>(total_elems));
    trace.merges.reserve(static_cast<std::size_t>(total_elems / 2));

    for (std::size_t line_id = 0; line_id < static_cast<std::size_t>(line_count); ++line_id)
    {
        const StandardLineDesc desc{lines[line_id]};
        const std::uint64_t base{desc.start};
        const std::int64_t lstep{desc.step};

        struct ReadTile
        {
            size_type_ pos;
            long long value;
        };

        std::vector<ReadTile> readTiles;
        readTiles.reserve(static_cast<std::size_t>(line_len));

        for (size_type_ read = 0; read < line_len; ++read)
        {
            const std::int64_t pos = static_cast<std::int64_t>(base) + static_cast<std::int64_t>(read) * lstep;
            const auto ipos = static_cast<size_type_>(pos);
            const long long v = before[ipos];
            if (v == 0)
                continue;
            readTiles.push_back(ReadTile{ipos, v});
        }

        bool has_prev = false;
        ReadTile prev{};
        size_type_ write = 0;

        for (const auto &t : readTiles)
        {
            if (!has_prev)
            {
                prev = t;
                has_prev = true;
                continue;
            }

            if (t.value == prev.value)
            {
                const long long merged = prev.value + t.value;
                const std::int64_t wpos = static_cast<std::int64_t>(base) + static_cast<std::int64_t>(write) * lstep;
                const auto iwpos = static_cast<size_type_>(wpos);
                buf[iwpos] = merged;

                trace.moves.push_back(TileMoveTrace{prev.pos, iwpos, static_cast<meta_type_>(prev.value), true, true});
                trace.moves.push_back(TileMoveTrace{t.pos, iwpos, static_cast<meta_type_>(t.value), true, false});
                trace.merges.push_back(TileMergeTrace{iwpos, prev.pos, t.pos, static_cast<meta_type_>(merged)});

                ++write;
                has_prev = false;
            }
            else
            {
                const std::int64_t wpos = static_cast<std::int64_t>(base) + static_cast<std::int64_t>(write) * lstep;
                const auto iwpos = static_cast<size_type_>(wpos);
                buf[iwpos] = prev.value;
                trace.moves.push_back(TileMoveTrace{prev.pos, iwpos, static_cast<meta_type_>(prev.value), false, true});
                ++write;

                prev = t;
                has_prev = true;
            }
        }

        if (has_prev)
        {
            const std::int64_t wpos = static_cast<std::int64_t>(base) + static_cast<std::int64_t>(write) * lstep;
            const auto iwpos = static_cast<size_type_>(wpos);
            buf[iwpos] = prev.value;
            trace.moves.push_back(TileMoveTrace{prev.pos, iwpos, static_cast<meta_type_>(prev.value), false, true});
            ++write;
        }

        for (size_type_ i = write; i < line_len; ++i)
        {
            const std::int64_t wpos = static_cast<std::int64_t>(base) + static_cast<std::int64_t>(i) * lstep;
            const auto iwpos = static_cast<size_type_>(wpos);
            buf[iwpos] = 0;
        }
    }

    trace.changed = (buf != before);
    for (size_type_ i = 0; i < total_elems; ++i)
        raw[i] = static_cast<meta_type_>(buf[i]);

    if (trace.changed)
    {
        SpawnTrace s;
        if (spawnRandomTile(s))
            trace.spawn = s;
    }

    return trace;
}

template <std::uint64_t Arch, typename MetaType, ValidSizeType SizeType, SizeType Dimension, SizeType... DimensionSize>
    requires ValidTensorSize<SizeType, Dimension, DimensionSize...>
auto Logic2048_tm<Arch, MetaType, SizeType, Dimension, DimensionSize...>::flatData() const -> std::vector<meta_type_>
{
    const meta_type_ *raw = m_data.data();
    return std::vector<meta_type_>(raw, raw + total_elems_);
}

template <std::uint64_t Arch, typename MetaType, ValidSizeType SizeType, SizeType Dimension, SizeType... DimensionSize>
    requires ValidTensorSize<SizeType, Dimension, DimensionSize...>
int Logic2048_tm<Arch, MetaType, SizeType, Dimension, DimensionSize...>::chooseDepth2D(const size_type_ size)
{
    switch (static_cast<int>(size))
    {
    case 4:
        return 4;
    case 6:
        return 1;
    case 8:
        return 1;
    default:
        return 1;
    }
}

template <std::uint64_t Arch, typename MetaType, ValidSizeType SizeType, SizeType Dimension, SizeType... DimensionSize>
    requires ValidTensorSize<SizeType, Dimension, DimensionSize...>
bool Logic2048_tm<Arch, MetaType, SizeType, Dimension, DimensionSize...>::mapMoveToDimDir2D(
    const int move,
    size_type_ &dim,
    MoveDirection &dir)
{
    switch (move)
    {
    case 0: // left
        dim = 1;
        dir = MoveDirection::Negative;
        return true;
    case 1: // up
        dim = 0;
        dir = MoveDirection::Negative;
        return true;
    case 2: // right
        dim = 1;
        dir = MoveDirection::Positive;
        return true;
    case 3: // down
        dim = 0;
        dir = MoveDirection::Positive;
        return true;
    default:
        return false;
    }
}

template <std::uint64_t Arch, typename MetaType, ValidSizeType SizeType, SizeType Dimension, SizeType... DimensionSize>
    requires ValidTensorSize<SizeType, Dimension, DimensionSize...>
bool Logic2048_tm<Arch, MetaType, SizeType, Dimension, DimensionSize...>::applyMoveNoSpawn2D(
    Logic2048_tm &board,
    const int move)
{
    size_type_ dim = 0;
    MoveDirection dir = MoveDirection::Negative;
    if (!mapMoveToDimDir2D(move, dim, dir))
        return false;

    const auto before = board.getHash();
    board.operate(dim, dir);
    return before != board.getHash();
}

template <std::uint64_t Arch, typename MetaType, ValidSizeType SizeType, SizeType Dimension, SizeType... DimensionSize>
    requires ValidTensorSize<SizeType, Dimension, DimensionSize...>
bool Logic2048_tm<Arch, MetaType, SizeType, Dimension, DimensionSize...>::setFlatCell2D(
    Logic2048_tm &board,
    const size_type_ index,
    const meta_type_ value)
{
    auto data = board.flatData();
    if (index >= data.size() || data[index] != static_cast<meta_type_>(0))
        return false;
    data[index] = value;
    board.template setData<meta_type_, std::vector>(data);
    return true;
}

template <std::uint64_t Arch, typename MetaType, ValidSizeType SizeType, SizeType Dimension, SizeType... DimensionSize>
    requires ValidTensorSize<SizeType, Dimension, DimensionSize...>
std::vector<SizeType> Logic2048_tm<Arch, MetaType, SizeType, Dimension, DimensionSize...>::freeCells2D(
    const Logic2048_tm &board)
{
    const auto data = board.flatData();
    std::vector<size_type_> out;
    out.reserve(data.size());
    for (size_type_ i = 0; i < static_cast<size_type_>(data.size()); ++i)
    {
        if (data[i] == static_cast<meta_type_>(0))
            out.push_back(i);
    }
    return out;
}

template <std::uint64_t Arch, typename MetaType, ValidSizeType SizeType, SizeType Dimension, SizeType... DimensionSize>
    requires ValidTensorSize<SizeType, Dimension, DimensionSize...>
float Logic2048_tm<Arch, MetaType, SizeType, Dimension, DimensionSize...>::calculateEmpty2D(const std::vector<meta_type_> &data)
{
    int empty = 0;
    for (const auto &v : data)
    {
        if (v == static_cast<meta_type_>(0))
            ++empty;
    }
    return static_cast<float>(empty);
}

template <std::uint64_t Arch, typename MetaType, ValidSizeType SizeType, SizeType Dimension, SizeType... DimensionSize>
    requires ValidTensorSize<SizeType, Dimension, DimensionSize...>
float Logic2048_tm<Arch, MetaType, SizeType, Dimension, DimensionSize...>::calculateMaxNum2D(const std::vector<meta_type_> &data)
{
    meta_type_ mx = static_cast<meta_type_>(0);
    for (const auto &v : data)
    {
        if (v > mx)
            mx = v;
    }
    return static_cast<float>(mx);
}

template <std::uint64_t Arch, typename MetaType, ValidSizeType SizeType, SizeType Dimension, SizeType... DimensionSize>
    requires ValidTensorSize<SizeType, Dimension, DimensionSize...>
float Logic2048_tm<Arch, MetaType, SizeType, Dimension, DimensionSize...>::calculateSmoothness2D(
    const std::vector<meta_type_> &data,
    const size_type_ size)
{
    auto idx = [size](const size_type_ r, const size_type_ c)
    {
        return static_cast<size_type_>(r * size + c);
    };

    float smooth = 0.0f;
    for (size_type_ i = 0; i < size; ++i)
    {
        for (size_type_ j = 0; j < size; ++j)
        {
            const auto v = data[idx(i, j)];
            if (v == static_cast<meta_type_>(0))
                continue;

            const float val = std::log2f(static_cast<float>(v) + 1.0f);
            if (i + 1 < size)
            {
                const float v2 = std::log2f(static_cast<float>(data[idx(i + 1, j)]) + 1.0f);
                smooth -= std::fabs(v2 - val);
            }
            if (j + 1 < size)
            {
                const float v2 = std::log2f(static_cast<float>(data[idx(i, j + 1)]) + 1.0f);
                smooth -= std::fabs(v2 - val);
            }
            if (i > 0)
            {
                const float v2 = std::log2f(static_cast<float>(data[idx(i - 1, j)]) + 1.0f);
                smooth -= std::fabs(v2 - val);
            }
            if (j > 0)
            {
                const float v2 = std::log2f(static_cast<float>(data[idx(i, j - 1)]) + 1.0f);
                smooth -= std::fabs(v2 - val);
            }
        }
    }
    return smooth;
}

template <std::uint64_t Arch, typename MetaType, ValidSizeType SizeType, SizeType Dimension, SizeType... DimensionSize>
    requires ValidTensorSize<SizeType, Dimension, DimensionSize...>
float Logic2048_tm<Arch, MetaType, SizeType, Dimension, DimensionSize...>::calculateMonotonicity2D(
    const std::vector<meta_type_> &data,
    const size_type_ size)
{
    auto idx = [size](const size_type_ r, const size_type_ c)
    {
        return static_cast<size_type_>(r * size + c);
    };

    float totals[4] = {0.0f, 0.0f, 0.0f, 0.0f};

    for (size_type_ i = 0; i < size; ++i)
    {
        size_type_ current = 0;
        size_type_ next = current + 1;
        while (next < size)
        {
            while (next < size && data[idx(i, next)] == static_cast<meta_type_>(0))
                ++next;
            if (next >= size)
                --next;

            const auto currentRaw = data[idx(i, current)];
            const auto nextRaw = data[idx(i, next)];
            const float currentVal =
                (currentRaw != static_cast<meta_type_>(0)) ? std::log2f(static_cast<float>(currentRaw)) : 0.0f;
            const float nextVal =
                (nextRaw != static_cast<meta_type_>(0)) ? std::log2f(static_cast<float>(nextRaw)) : 0.0f;

            if (currentVal > nextVal)
                totals[0] += nextVal - currentVal;
            else
                totals[1] += currentVal - nextVal;

            current = next;
            ++next;
        }
    }

    for (size_type_ j = 0; j < size; ++j)
    {
        size_type_ current = 0;
        size_type_ next = current + 1;
        while (next < size)
        {
            while (next < size && data[idx(next, j)] == static_cast<meta_type_>(0))
                ++next;
            if (next >= size)
                --next;

            const auto currentRaw = data[idx(current, j)];
            const auto nextRaw = data[idx(next, j)];
            const float currentVal =
                (currentRaw != static_cast<meta_type_>(0)) ? std::log2f(static_cast<float>(currentRaw)) : 0.0f;
            const float nextVal =
                (nextRaw != static_cast<meta_type_>(0)) ? std::log2f(static_cast<float>(nextRaw)) : 0.0f;

            if (currentVal > nextVal)
                totals[2] += nextVal - currentVal;
            else
                totals[3] += currentVal - nextVal;

            current = next;
            ++next;
        }
    }

    const float max1 = (totals[0] > totals[1]) ? totals[0] : totals[1];
    const float max2 = (totals[2] > totals[3]) ? totals[2] : totals[3];
    return max1 + max2;
}

template <std::uint64_t Arch, typename MetaType, ValidSizeType SizeType, SizeType Dimension, SizeType... DimensionSize>
    requires ValidTensorSize<SizeType, Dimension, DimensionSize...>
void Logic2048_tm<Arch, MetaType, SizeType, Dimension, DimensionSize...>::markIsland2D(
    const std::vector<meta_type_> &data,
    const int size,
    const int x,
    const int y,
    const meta_type_ value,
    std::vector<std::uint8_t> &marked)
{
    if (x < 0 || x >= size || y < 0 || y >= size)
        return;

    const auto index = static_cast<size_type_>(x * size + y);
    if (data[index] == static_cast<meta_type_>(0) || data[index] != value || marked[index] != 0)
        return;

    marked[index] = 1;
    markIsland2D(data, size, x + 1, y, value, marked);
    markIsland2D(data, size, x - 1, y, value, marked);
    markIsland2D(data, size, x, y + 1, value, marked);
    markIsland2D(data, size, x, y - 1, value, marked);
}

template <std::uint64_t Arch, typename MetaType, ValidSizeType SizeType, SizeType Dimension, SizeType... DimensionSize>
    requires ValidTensorSize<SizeType, Dimension, DimensionSize...>
float Logic2048_tm<Arch, MetaType, SizeType, Dimension, DimensionSize...>::calculateIslands2D(
    const std::vector<meta_type_> &data,
    const size_type_ size)
{
    const auto total = static_cast<size_type_>(size * size);
    std::vector<std::uint8_t> marked(static_cast<std::size_t>(total), 1);

    for (size_type_ i = 0; i < total; ++i)
    {
        if (data[i] != static_cast<meta_type_>(0))
            marked[i] = 0;
    }

    float islands = 0.0f;
    const int sizeInt = static_cast<int>(size);
    for (int r = 0; r < sizeInt; ++r)
    {
        for (int c = 0; c < sizeInt; ++c)
        {
            const auto index = static_cast<size_type_>(r * sizeInt + c);
            if (data[index] != static_cast<meta_type_>(0) && marked[index] == 0)
            {
                islands += 1.0f;
                markIsland2D(data, sizeInt, r, c, data[index], marked);
            }
        }
    }

    return islands;
}

template <std::uint64_t Arch, typename MetaType, ValidSizeType SizeType, SizeType Dimension, SizeType... DimensionSize>
    requires ValidTensorSize<SizeType, Dimension, DimensionSize...>
float Logic2048_tm<Arch, MetaType, SizeType, Dimension, DimensionSize...>::calculateEvaluation2D(
    const std::vector<meta_type_> &data,
    const size_type_ size)
{
    const float empty = calculateEmpty2D(data);
    const float emptyWeight = 2.7f + (std::logf(17.0f) - std::logf(empty + 1.0f)) * 0.1f;
    const float maxnumWeight = 1.0f;
    const float smoothWeight = 0.1f;
    const float monoWeight = 1.0f;

    return emptyWeight * std::logf(empty + 1.0f) +
           maxnumWeight * calculateMaxNum2D(data) +
           smoothWeight * calculateSmoothness2D(data, size) +
           monoWeight * calculateMonotonicity2D(data, size);
}

template <std::uint64_t Arch, typename MetaType, ValidSizeType SizeType, SizeType Dimension, SizeType... DimensionSize>
    requires ValidTensorSize<SizeType, Dimension, DimensionSize...>
auto Logic2048_tm<Arch, MetaType, SizeType, Dimension, DimensionSize...>::searchBest2D(
    Logic2048_tm board,
    int depth,
    const float alpha,
    const float beta,
    const bool playerTurn,
    const size_type_ size) -> AiSearchResult2D
{
    AiSearchResult2D result;

    if (playerTurn)
    {
        float bestScore = alpha;
        int bestMove = -1;

        for (int direction = 0; direction < 4; ++direction)
        {
            Logic2048_tm next = board;
            const bool changed = applyMoveNoSpawn2D(next, direction);
            if (!changed)
                continue;

            AiSearchResult2D sub;
            if (depth == 0)
            {
                sub.move = direction;
                sub.score = calculateEvaluation2D(next.flatData(), size);
            }
            else
            {
                sub = searchBest2D(next, depth - 1, bestScore, beta, false, size);
            }

            if (sub.score > bestScore)
            {
                bestScore = sub.score;
                bestMove = direction;
            }

            if (bestScore > beta)
            {
                result.move = bestMove;
                result.score = beta;
                return result;
            }
        }

        result.move = bestMove;
        result.score = bestScore;
        return result;
    }

    float bestScore = beta;
    const auto empty = freeCells2D(board);
    if (empty.empty())
    {
        result.move = -1;
        result.score = calculateEvaluation2D(board.flatData(), size);
        return result;
    }

    std::vector<float> score2(empty.size(), -FLT_MAX);
    std::vector<float> score4(empty.size(), -FLT_MAX);
    for (std::size_t i = 0; i < empty.size(); ++i)
    {
        Logic2048_tm t2 = board;
        if (setFlatCell2D(t2, empty[i], static_cast<meta_type_>(2)))
            score2[i] = -calculateSmoothness2D(t2.flatData(), size) + calculateIslands2D(t2.flatData(), size);

        Logic2048_tm t4 = board;
        if (setFlatCell2D(t4, empty[i], static_cast<meta_type_>(4)))
            score4[i] = -calculateSmoothness2D(t4.flatData(), size) + calculateIslands2D(t4.flatData(), size);
    }

    float maxScore = -FLT_MAX;
    for (std::size_t i = 0; i < empty.size(); ++i)
    {
        if (score2[i] > maxScore)
            maxScore = score2[i];
        if (score4[i] > maxScore)
            maxScore = score4[i];
    }

    struct WorstCase2D
    {
        size_type_ index;
        meta_type_ value;
    };

    std::vector<WorstCase2D> worst;
    worst.reserve(empty.size() * 2);
    for (std::size_t i = 0; i < empty.size(); ++i)
    {
        if (score2[i] == maxScore)
            worst.push_back(WorstCase2D{empty[i], static_cast<meta_type_>(2)});
        if (score4[i] == maxScore)
            worst.push_back(WorstCase2D{empty[i], static_cast<meta_type_>(4)});
    }

    for (const auto &w : worst)
    {
        Logic2048_tm next = board;
        if (!setFlatCell2D(next, w.index, w.value))
            continue;

        const auto sub = searchBest2D(next, depth, alpha, bestScore, true, size);
        if (sub.score < bestScore)
            bestScore = sub.score;

        if (bestScore < alpha)
        {
            result.move = -1;
            result.score = alpha;
            return result;
        }
    }

    result.move = -1;
    result.score = bestScore;
    return result;
}

template <std::uint64_t Arch, typename MetaType, ValidSizeType SizeType, SizeType Dimension, SizeType... DimensionSize>
    requires ValidTensorSize<SizeType, Dimension, DimensionSize...>
int Logic2048_tm<Arch, MetaType, SizeType, Dimension, DimensionSize...>::getBestMove2D(int depth) const
{
    if constexpr (Dimension != 2)
    {
        return -1;
    }
    else
    {
        const size_type_ size = sizes_[0];
        int localDepth = depth;
        if (localDepth < 0)
            localDepth = chooseDepth2D(size);

        Logic2048_tm board = *this;
        auto result = searchBest2D(board, localDepth, -1000000.0f, 1000000.0f, true, size);

        while (localDepth > 0)
        {
            if (result.move == -1)
                result = searchBest2D(board, --localDepth, -1000000.0f, 1000000.0f, true, size);
            else
                break;
        }

        if (result.move != -1)
        {
            Logic2048_tm test = board;
            if (applyMoveNoSpawn2D(test, result.move))
                return result.move;
        }

        for (int d = 0; d < 4; ++d)
        {
            Logic2048_tm test = board;
            if (applyMoveNoSpawn2D(test, d))
                return d;
        }
        return -1;
    }
}

template <std::uint64_t Arch, typename MetaType, ValidSizeType SizeType, SizeType Dimension, SizeType... DimensionSize>
    requires ValidTensorSize<SizeType, Dimension, DimensionSize...>
auto Logic2048_tm<Arch, MetaType, SizeType, Dimension, DimensionSize...>::operateBestMoveAndSpawnTrace2D(
    const int depth) -> MoveTrace
{
    if constexpr (Dimension != 2)
    {
        return MoveTrace{};
    }
    else
    {
        const int move = getBestMove2D(depth);
        size_type_ dim = 0;
        MoveDirection dir = MoveDirection::Negative;
        if (!mapMoveToDimDir2D(move, dim, dir))
            return MoveTrace{};
        return operateAndSpawnTrace(dim, dir);
    }
}

template <std::uint64_t Arch, typename MetaType, ValidSizeType SizeType, SizeType Dimension, SizeType... DimensionSize>
    requires ValidTensorSize<SizeType, Dimension, DimensionSize...>
auto Logic2048_tm<Arch, MetaType, SizeType, Dimension, DimensionSize...>::sampleNewTileValue() -> meta_type_
{
    // 90% -> 2, 10% -> 4
    std::uniform_int_distribution dist(0, 9);
    return (dist(m_rng) <= 4) ? static_cast<meta_type_>(4) : static_cast<meta_type_>(2);
}

template <std::uint64_t Arch, typename MetaType, ValidSizeType SizeType, SizeType Dimension, SizeType... DimensionSize>
    requires ValidTensorSize<SizeType, Dimension, DimensionSize...>
bool Logic2048_tm<Arch, MetaType, SizeType, Dimension, DimensionSize...>::spawnRandomTile()
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

template <std::uint64_t Arch, typename MetaType, ValidSizeType SizeType, SizeType Dimension, SizeType... DimensionSize>
    requires ValidTensorSize<SizeType, Dimension, DimensionSize...>
bool Logic2048_tm<Arch, MetaType, SizeType, Dimension, DimensionSize...>::spawnRandomTile(SpawnTrace &out)
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
    const auto val = sampleNewTileValue();
    raw[chosen] = val;

    out.index = chosen;
    out.value = val;
    return true;
}

template <std::uint64_t Arch, typename MetaType, ValidSizeType SizeType, SizeType Dimension, SizeType... DimensionSize>
    requires ValidTensorSize<SizeType, Dimension, DimensionSize...>
template <typename OutputStream>
void Logic2048_tm<Arch, MetaType, SizeType, Dimension, DimensionSize...>::outputData(OutputStream &out)
{
    printTensor<meta_type_, OutputStream, Dimension>(m_data, out, 0);
}

template <std::uint64_t Arch, typename MetaType, ValidSizeType SizeType, SizeType Dimension, SizeType... DimensionSize>
    requires ValidTensorSize<SizeType, Dimension, DimensionSize...>
template <typename T, typename OutputStream, size_t NDims>
void Logic2048_tm<Arch, MetaType, SizeType, Dimension, DimensionSize...>::printTensor(
    const Eigen::Tensor<T, NDims, Eigen::RowMajor> &tensor, OutputStream &out,
    int indent)
{
    std::string prefix(indent, ' ');
    if constexpr (NDims == 1)
    {
        for (int i = 0; i < tensor.dimension(0); ++i)
            out << prefix << tensor(i) << " ";
        out << "\n";
    }
    else
    {
        for (int i = 0; i < tensor.dimension(0); ++i)
        {
            out << prefix << "[Slice " << i << "]\n";
            auto slice = tensor.chip(i, 0); // 沿第0维切片，偏移量为i
            printTensor<T, OutputStream, NDims - 1>(slice, out, indent + 2);
        }
    }
}

template <std::uint64_t Arch, typename MetaType, ValidSizeType SizeType, SizeType Dimension, SizeType... DimensionSize>
    requires ValidTensorSize<SizeType, Dimension, DimensionSize...>
template <template <typename Val_T, typename...> typename Container>
bool Logic2048_tm<Arch, MetaType, SizeType, Dimension, DimensionSize...>::setRandomLocationValue(
    size_t numCount, const Container<meta_type_> &presetValues)
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
