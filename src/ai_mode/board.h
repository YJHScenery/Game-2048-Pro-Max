/**
 * @file board.h
 * @brief 2048 棋盘状态与移动规则的公开接口。
 */
#ifndef BOARD_H
#define BOARD_H

/** @brief 支持的棋盘边长上限。 */
#define MAX_BOARD_SIZE 8
/** @brief 移动方向编码：左。 */
#define MOVE_LEFT 0
/** @brief 移动方向编码：上。 */
#define MOVE_UP 1
/** @brief 移动方向编码：右。 */
#define MOVE_RIGHT 2
/** @brief 移动方向编码：下。 */
#define MOVE_DOWN 3

/**
 * @brief 可变的 2048 棋盘状态。
 * @details
 * board 在固定 8x8 缓冲区中保存方块值，实际生效区域为左上角
 * boardsize x boardsize。score 用于累计合并得分。
 */
typedef struct
{
    int board[MAX_BOARD_SIZE][MAX_BOARD_SIZE];
    int score;
} Board;

/**
 * @brief 初始化棋盘并放置两个起始方块。
 * @param[out] board 目标棋盘对象。
 * @param[in] boardsize 当前有效棋盘边长。
 */
void init_board(Board *board, int boardsize);

/**
 * @brief 将一个棋盘的方块缓冲区复制到另一个棋盘。
 * @param[out] board 目标棋盘。
 * @param[in] grid 源棋盘。
 * @note 该函数仅复制 board 数组，不复制 score 字段。
 */
void copy_board(Board *board, const Board *grid);

/**
 * @brief 用一维行优先数组初始化棋盘。
 * @param[out] board 目标棋盘对象。
 * @param[in] boardsize 当前有效棋盘边长。
 * @param[in] arr 包含 boardsize * boardsize 个值的一维数组。
 * @param[in] score 初始分数。
 */
void init_board_from_flat(Board *board, int boardsize, const int arr[], int score);

/**
 * @brief 在随机空位放置指定数值方块。
 * @param[in,out] board 棋盘状态。
 * @param[in] boardsize 当前有效棋盘边长。
 * @param[in] value 要放置的方块值。
 * @return 放置成功返回 1；若无空位返回 0。
 */
int add_value(Board *board, int boardsize, int value);

/**
 * @brief 随机添加方块（高概率为 2，否则为 4）。
 * @param[in,out] board 棋盘状态。
 * @param[in] boardsize 当前有效棋盘边长。
 * @return 放置成功返回 1；若无空位返回 0。
 */
int add_random(Board *board, int boardsize);

/**
 * @brief 旧版兼容接口：在原始矩阵上随机加块。
 * @param[in,out] num 原始棋盘矩阵。
 * @param[in] boardsize 当前有效棋盘边长。
 */
void add(int num[MAX_BOARD_SIZE][MAX_BOARD_SIZE], int boardsize);

/**
 * @brief 枚举所有空格坐标。
 * @param[in] board 棋盘状态。
 * @param[out] blocks 输出坐标列表，元素格式为 [x, y]。
 * @param[in] boardsize 当前有效棋盘边长。
 * @return 找到的空格数量。
 */
int get_free_blocks(const Board *board, int blocks[64][2], int boardsize);

/**
 * @brief 若指定坐标为空，则在该位置放置数值。
 * @param[in,out] board 棋盘状态。
 * @param[in] x 行索引。
 * @param[in] y 列索引。
 * @param[in] value 要放置的方块值。
 * @return 成功返回 1；若该格已被占用返回 0。
 */
int add_xy(Board *board, int x, int y, int value);

/**
 * @brief 清空指定坐标的方块。
 * @param[in,out] board 棋盘状态。
 * @param[in] x 行索引。
 * @param[in] y 列索引。
 */
void remove_xy(Board *board, int x, int y);

/**
 * @brief 模拟或执行一次向左移动。
 * @param[in,out] board 棋盘状态。
 * @param[in] is_change 非 0 表示写回移动结果和分数。
 * @param[in] boardsize 当前有效棋盘边长。
 * @return 若棋盘会发生变化返回 1，否则返回 0。
 */
int move_left(Board *board, int is_change, int boardsize);

/**
 * @brief 模拟或执行一次向右移动。
 * @param[in,out] board 棋盘状态。
 * @param[in] is_change 非 0 表示写回移动结果和分数。
 * @param[in] boardsize 当前有效棋盘边长。
 * @return 若棋盘会发生变化返回 1，否则返回 0。
 */
int move_right(Board *board, int is_change, int boardsize);

/**
 * @brief 将棋盘原地顺时针旋转 90 度。
 * @param[in,out] board 棋盘状态。
 * @param[in] boardsize 当前有效棋盘边长。
 */
void rotate_right(Board *board, int boardsize);

/**
 * @brief 将棋盘原地逆时针旋转 90 度。
 * @param[in,out] board 棋盘状态。
 * @param[in] boardsize 当前有效棋盘边长。
 */
void rotate_left(Board *board, int boardsize);

/**
 * @brief 模拟或执行一次向上移动。
 * @param[in,out] board 棋盘状态。
 * @param[in] is_change 非 0 表示写回移动结果和分数。
 * @param[in] boardsize 当前有效棋盘边长。
 * @return 若棋盘会发生变化返回 1，否则返回 0。
 */
int move_up(Board *board, int is_change, int boardsize);

/**
 * @brief 模拟或执行一次向下移动。
 * @param[in,out] board 棋盘状态。
 * @param[in] is_change 非 0 表示写回移动结果和分数。
 * @param[in] boardsize 当前有效棋盘边长。
 * @return 若棋盘会发生变化返回 1，否则返回 0。
 */
int move_down(Board *board, int is_change, int boardsize);

/**
 * @brief 根据方向编码分发移动逻辑。
 * @param[in,out] board 棋盘状态。
 * @param[in] direction 取值为 MOVE_LEFT/MOVE_UP/MOVE_RIGHT/MOVE_DOWN 之一。
 * @param[in] is_change 非 0 表示写回移动结果和分数。
 * @param[in] boardsize 当前有效棋盘边长。
 * @return 若棋盘会发生变化返回 1，否则返回 0。
 */
int move(Board *board, int direction, int is_change, int boardsize);

/**
 * @brief 检查是否存在 2048 方块。
 * @param[in] board 棋盘状态。
 * @param[in] boardsize 当前有效棋盘边长。
 * @return 若存在胜利方块返回 1，否则返回 0。
 */
int isGameWon(Board *board, int boardsize);

/**
 * @brief 检查是否已无合法移动。
 * @param[in] board 棋盘状态。
 * @param[in] boardsize 当前有效棋盘边长。
 * @return 若游戏结束返回 1，否则返回 0。
 */
int isGameOver(Board *board, int boardsize);

#endif
