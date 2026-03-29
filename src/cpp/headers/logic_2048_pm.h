//
// Created by jehor on 2026/3/29.
//

#ifndef GAME_2048_QUICK_LOGIC2048BASE_H
#define GAME_2048_QUICK_LOGIC2048BASE_H

#include <Eigen/Dense>
#include <unsupported/Eigen/CXX11/Tensor>

#include <array>
#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <vector>

#include "cuda_interface.h"

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

template <typename MetaType = size_t, size_t Dimension = 2, size_t... DimensionSize>
    requires((Dimension >= 2) && sizeof...(DimensionSize) == Dimension && ((DimensionSize != 0) && ...))
class Logic2048_tm
{
public:
    using meta_type_ = MetaType; //! 2048 的元数据类型
    using data_type_ = Eigen::Tensor<meta_type_, Dimension, Eigen::RowMajor>; //! @brief 张量数据类型
    using size_type_ = Eigen::array<size_t, Dimension>; //! @brief 此类型定义了棋盘的尺寸
    using array_index_type_ = Eigen::array<size_t, Dimension - 1>; //! @brief 此类型定义了查找某一个线性数组的下标所需要的类型

    /**
     * 2048移动操作
     * @param dim 维度序列号
     * @param dir 移动方向
     */
    void move(size_t dim, MoveDirection dir);

    [[nodiscard]] data_type_ getData() { return m_data; }
    [[nodiscard]] const data_type_ &getDataRef() const { return m_data; }

    constexpr static size_type_ getSizeArray() { return sizes_; }

private:
    static constexpr size_type_ sizes_{DimensionSize...};
    data_type_ m_data{sizes_};


};

#define CURRENT_TEMPLATE_DEFINITION template <typename MetaType, size_t Dimension, size_t... DimensionSize> \
requires((Dimension >= 2) && sizeof...(DimensionSize) == Dimension && ((DimensionSize != 0) && ...))

CURRENT_TEMPLATE_DEFINITION
void Logic2048_tm<MetaType, Dimension, DimensionSize...>::move(size_t dim, MoveDirection dir)
{
    if (dim >= Dimension)
        return;

    const auto sizes = sizes_;
    const std::size_t line_len = sizes[dim];
    if (line_len == 0)
        return;

    std::array<std::size_t, Dimension> strides{};
    strides[Dimension - 1] = 1;
    for (std::size_t d = Dimension - 1; d-- > 0;)
    {
        strides[d] = strides[d + 1] * sizes[d + 1];
    }

    std::size_t line_count = 1;
    for (std::size_t d = 0; d < Dimension; ++d)
    {
        if (d == dim)
            continue;
        line_count *= sizes[d];
    }

    const std::size_t total_elems = static_cast<std::size_t>(m_data.size());
    if (line_count * line_len != total_elems)
        return;

    const std::size_t start_index = (dir == MoveDirection::Negative) ? 0 : (line_len - 1);
    const std::int64_t step = static_cast<std::int64_t>(strides[dim]) *
                              ((dir == MoveDirection::Negative) ? static_cast<std::int64_t>(1) : static_cast<std::int64_t>(-1));

    std::vector<CudaLineDesc> lines;
    lines.reserve(line_count);

    std::array<std::size_t, Dimension> idx{};
    for (std::size_t d = 0; d < Dimension; ++d)
        idx[d] = 0;
    idx[dim] = start_index;

    while (true)
    {
        std::uint64_t offset = 0;
        for (std::size_t d = 0; d < Dimension; ++d)
            offset += static_cast<std::uint64_t>(idx[d] * strides[d]);

        lines.push_back(CudaLineDesc{offset, step});

        // odometer increment on all dims except dim
        std::int64_t carry_dim = static_cast<std::int64_t>(Dimension) - 1;
        for (; carry_dim >= 0; --carry_dim)
        {
            const std::size_t d = static_cast<std::size_t>(carry_dim);
            if (d == dim)
                continue;
            ++idx[d];
            if (idx[d] < sizes[d])
                break;
            idx[d] = 0;
        }

        if (carry_dim < 0)
            break;
    }

    if (lines.size() != line_count)
        return;

    meta_type_ *raw = m_data.data();
    if constexpr (std::is_integral_v<meta_type_> && std::is_signed_v<meta_type_>)
    {
        std::vector<long long> buf(total_elems);
        for (std::size_t i = 0; i < total_elems; ++i)
            buf[i] = static_cast<long long>(raw[i]);

        cuda_move_lines_ll(buf.data(), lines.data(), line_count, line_len);

        for (std::size_t i = 0; i < total_elems; ++i)
            raw[i] = static_cast<meta_type_>(buf[i]);
    }
    else if constexpr (std::is_integral_v<meta_type_> && std::is_unsigned_v<meta_type_>)
    {
        std::vector<unsigned long long> buf(total_elems);
        for (std::size_t i = 0; i < total_elems; ++i)
            buf[i] = static_cast<unsigned long long>(raw[i]);

        cuda_move_lines_ull(buf.data(), lines.data(), line_count, line_len);

        for (std::size_t i = 0; i < total_elems; ++i)
            raw[i] = static_cast<meta_type_>(buf[i]);
    }
    else if constexpr (std::is_floating_point_v<meta_type_> && std::is_signed_v<meta_type_>)
    {
        std::vector<double> buf(total_elems);
        for (std::size_t i = 0; i < total_elems; ++i)
            buf[i] = static_cast<double>(raw[i]);

        cuda_move_lines_ld(buf.data(), lines.data(), line_count, line_len);

        for (std::size_t i = 0; i < total_elems; ++i)
            raw[i] = static_cast<meta_type_>(buf[i]);
    }
    else
    {
        // "unsigned floating" isn't a standard C++ type; keep a dedicated entry for symmetry.
        std::vector<double> buf(total_elems);
        for (std::size_t i = 0; i < total_elems; ++i)
            buf[i] = static_cast<double>(raw[i]);

        cuda_move_lines_uld(buf.data(), lines.data(), line_count, line_len);

        for (std::size_t i = 0; i < total_elems; ++i)
            raw[i] = static_cast<meta_type_>(buf[i]);
    }
}


#endif // GAME_2048_QUICK_LOGIC2048BASE_H
