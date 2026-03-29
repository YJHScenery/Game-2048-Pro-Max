//
// Created by jehor on 2026/3/29.
//

#ifndef GAME_2048_QUICK_LOGIC2048BASE_H
#define GAME_2048_QUICK_LOGIC2048BASE_H

#include <Eigen/Dense>
#include <unsupported/Eigen/CXX11/Tensor>

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

template <size_t Dimension = 2, size_t... DimensionSize>
    requires(sizeof...(DimensionSize) == Dimension && ((DimensionSize != 0) && ...))
class Logic2048_tm
{
public:
    using data_type_ = Eigen::Tensor<size_t, Dimension>;
    using size_type_ = Eigen::array<size_t, Dimension>;

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

#define CURRENT_TEMPLATE_DEFINITION template <size_t Dimension, size_t... DimensionSize> \
    requires(sizeof...(DimensionSize) == Dimension && ((DimensionSize != 0) && ...))

CURRENT_TEMPLATE_DEFINITION
void Logic2048_tm<Dimension, DimensionSize...>::move(size_t dim, MoveDirection dir)
{
    if (dim >= Dimension)
        return;
}

#endif // GAME_2048_QUICK_LOGIC2048BASE_H
