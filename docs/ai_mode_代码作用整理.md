# ai_mode 目录代码作用整理

## 1. 模块总体职责

src/ai_mode 主要实现 2048 的两层能力：

- 棋盘底层规则：棋盘初始化、随机加块、四方向移动、旋转复用、胜负判定。
- AI 决策层：基于启发式评估 + alpha-beta 搜索，输出下一步动作。

对应文件分工：

- `board.h` / `board.c`：棋盘数据结构与规则引擎。
- `ai_mode.h` / `ai_mode.c`：AI 评估函数、搜索流程、调试/展示输出。

## 2. board 模块（规则引擎）

### 2.1 数据结构与常量

- `MAX_BOARD_SIZE = 8`：支持到 8x8。
- 方向常量：`MOVE_LEFT/UP/RIGHT/DOWN`。
- `Board`：
  - `board[8][8]`：固定缓冲区，实际使用左上角 `boardsize x boardsize`。
  - `score`：累计得分。

### 2.2 关键函数职责

- `init_board`：清盘、清分、放置两个初始块。
- `copy_board`：复制棋盘格子数据（不复制 score）。
- `init_board_from_flat`：从一维数组加载局面，便于测试/回放。
- `get_free_blocks`：枚举空格坐标。
- `add_value` / `add_random` / `add_xy` / `remove_xy`：各种加块与删块接口。
- `move_left`：核心合并逻辑（去零 -> 合并 -> 再去零），并计算本步得分。
- `move_right`：水平翻转 + `move_left` + 翻转恢复。
- `move_up` / `move_down`：通过旋转复用 `move_left`。
- `move`：方向分发。
- `isGameWon` / `isGameOver`：终局判定。

### 2.3 设计特点

- 通过旋转与翻转复用 `move_left`，减少重复实现。
- `is_change` 参数允许“只模拟不落盘”，这对 AI 搜索非常关键。

## 3. ai_mode 模块（AI 决策）

### 3.1 启发式特征

- `calculate_empty`：空格数量。
- `calculate_maxnum`：最大块值。
- `calculate_smoothness`：相邻块差异平滑度（log 尺度）。
- `calculate_monotonicity`：行列单调性趋势。
- `calculate_islands` + `mark_island`：同值连通块数量（用于惩罚碎片化）。

### 3.2 综合评分

- `calculate_evaluation` 将特征线性组合：
  - 空格权重是动态的（空格越少，权重会调节）。
  - 同时考虑最大值、平滑度、单调性。
- 该评分用于搜索叶子节点（深度耗尽时）。

### 3.3 搜索策略

- `search_best`：alpha-beta 框架，分为两层：
  - `playerTurn = 1`：AI 层（max），遍历 4 个方向并取最大。
  - `playerTurn = 0`：环境层（近似 min/chance），枚举空格放置 2/4 后选“最坏局面”推进。
- `getBestMove`：
  - 依据棋盘尺寸选择搜索深度（4x4 更深，6x6/8x8 更浅）。
  - 如果最优步无效，降深重试。
  - 仍失败时随机兜底，再做合法性校验。

## 4. 输出与调试

- `test_moves`：打印每个方向是否可走。
- `print_board` / `print_board_fast` / `print_board_fastest`：三种不同速度/维护性的打印方案。
- `choose_print`：按参数选择打印策略。
- `show_ai`：驱动 AI 连续执行并按步输出棋盘。

## 5. 当前实现注意点

- `choose_print` 的 `switch` 缺少 `break`，会发生贯穿执行，最终总会走到 `print_board_fastest`。
- `copy_board` 仅复制 `board` 数组，不复制 `score`，调用方需明确是否需要保留分数。
- 环境层使用“最坏候选集合”而非严格概率期望（更偏对抗式）。

## 6. 代码阅读建议路径

建议按以下顺序阅读最容易建立整体心智模型：

1. `board.c::move_left`（理解规则核心）
2. `board.c::move` + `move_up/down/right`（理解复用）
3. `ai_mode.c::calculate_*`（理解启发式来源）
4. `ai_mode.c::search_best`（理解决策流程）
5. `ai_mode.c::getBestMove`（理解工程化兜底）
