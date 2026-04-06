/**
 * @file ai_mode.h
 * @brief 2048 AI 评估与搜索的公开接口。
 */
#ifndef AI_MODE_H
#define AI_MODE_H

#include "board.h"

/**
 * @brief AI 搜索返回的结果对。
 * @details
 * move 为 [0, 3] 方向编码；无可用动作时为 -1。
 * score 为当前节点对应的评估分值（极大极小搜索分数）。
 */
typedef struct
{
    int move;
    float score;
} AI2048Result;

/**
 * @brief 统计当前棋盘中的空格数量。
 * @param[in] board 棋盘状态。
 * @param[in] boardsize 当前有效棋盘边长。
 * @return 空格数量（float）。
 */
float calculate_empty(const Board *board, int boardsize);

/**
 * @brief 获取棋盘中的最大方块值。
 * @param[in] board 棋盘状态。
 * @param[in] boardsize 当前有效棋盘边长。
 * @return 最大方块值（float）。
 */
float calculate_maxnum(const Board *board, int boardsize);

/**
 * @brief 计算相邻方块的平滑性指标。
 * @param[in] board 棋盘状态。
 * @param[in] boardsize 当前有效棋盘边长。
 * @return 平滑性分数（按本实现符号约定，数值越大表示越平滑）。
 */
float calculate_smoothness(const Board *board, int boardsize);

/**
 * @brief 计算行列方向的单调性趋势。
 * @param[in] board 棋盘状态。
 * @param[in] boardsize 当前有效棋盘边长。
 * @return 单调性分数。
 */
float calculate_monotonicity(const Board *board, int boardsize);

/**
 * @brief 用 DFS 标记同值连通分量。
 * @param[in] board 棋盘状态。
 * @param[in] x 起始行索引。
 * @param[in] y 起始列索引。
 * @param[in] value 目标方块值。
 * @param[in,out] marked 访问标记矩阵。
 * @param[in] boardsize 当前有效棋盘边长。
 */
void mark_island(const Board *board, int x, int y, int value, int marked[MAX_BOARD_SIZE][MAX_BOARD_SIZE],
                 int boardsize);

/**
 * @brief 统计非零同值连通块数量。
 * @param[in] board 棋盘状态。
 * @param[in] boardsize 当前有效棋盘边长。
 * @return 连通块数量（float）。
 */
float calculate_islands(const Board *board, int boardsize);

/**
 * @brief 极大极小搜索叶子节点的启发式评估函数。
 * @param[in] board 棋盘状态。
 * @param[in] boardsize 当前有效棋盘边长。
 * @return 综合启发式分数。
 */
float calculate_evaluation(const Board *board, int boardsize);

/**
 * @brief 使用 Alpha-Beta 剪枝搜索最佳动作。
 * @param[in,out] this_board 当前棋盘状态。
 * @param[in] depth 剩余搜索深度。
 * @param[in] alpha Alpha 下界。
 * @param[in] beta Beta 上界。
 * @param[in] playerTurn 非 0 表示 AI 极大层，0 表示环境落子层。
 * @param[in] boardsize 当前有效棋盘边长。
 * @return 当前节点的最佳动作与对应分数。
 */
AI2048Result search_best(Board *this_board, int depth, float alpha, float beta, int playerTurn, int boardsize);

/**
 * @brief 获取 AI 的下一步动作（顶层接口）。
 * @param[in,out] board 当前棋盘状态。
 * @param[in] boardsize 当前有效棋盘边长。
 * @return 返回 [0, 3] 的方向编码；若无合法动作返回 -1。
 */
int getBestMove(Board *board, int boardsize);

/**
 * @brief 动作推断的兼容包装接口。
 * @param[in,out] board 当前棋盘状态。
 * @param[in] boardsize 当前有效棋盘边长。
 * @return 返回 [0, 3] 的方向编码；若无合法动作返回 -1。
 */
int get_action(Board *board, int boardsize);

/**
 * @brief 打印每个方向当前是否可移动。
 * @param[in] board 当前棋盘状态。
 * @param[in] boardsize 当前有效棋盘边长。
 */
void test_moves(Board *board, int boardsize);

/**
 * @brief 通过基础 printf 循环打印棋盘。
 * @param[in] board 棋盘状态。
 * @param[in] boardsize 当前有效棋盘边长。
 */
void print_board(Board *board, int boardsize);

/**
 * @brief 通过缓冲 snprintf 方式打印棋盘。
 * @param[in] board 棋盘状态。
 * @param[in] boardsize 当前有效棋盘边长。
 */
void print_board_fast(Board *board, int boardsize);

/**
 * @brief 使用常见尺寸的预展开格式串打印棋盘。
 * @param[in] board 棋盘状态。
 * @param[in] boardsize 当前有效棋盘边长。
 */
void print_board_fastest(Board *board, int boardsize);

/**
 * @brief 选择并调用一种打印策略。
 * @param[in] board 棋盘状态。
 * @param[in] boardsize 当前有效棋盘边长。
 * @param[in] choose 打印策略选择值。
 */
void choose_print(Board *board, int boardsize, int choose);

/**
 * @brief 运行并展示 AI 自博弈过程。
 * @param[in,out] board 初始并持续更新的棋盘状态。
 * @param[in] steps 最大执行步数。
 * @param[in] each_steps 打印频率控制参数。
 * @param[in] test 从该步开始打印合法性调试信息。
 * @param[in] boardsize 当前有效棋盘边长。
 * @param[in] choose 打印策略选择值。
 */
void show_ai(Board *board, int steps, int each_steps, int test, int boardsize, int choose);

#endif
