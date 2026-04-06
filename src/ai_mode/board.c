/**
 * @file board.c
 * @brief 2048 棋盘核心操作与移动规则实现。
 * @details
 * 包含棋盘初始化、加块、移动变换、旋转辅助函数与终局判定逻辑。
 */
#include <cstdlib>
#include <cstring>
#include "board.h"

void init_board(Board *board, int boardsize)
{
    memset(board->board, 0, sizeof(board->board));
    board->score = 0;
    add_value(board, boardsize, 2);
    add_random(board, boardsize);
}

void copy_board(Board *board, const Board *grid)
{
    memcpy(board->board, grid->board, sizeof(board->board));
}

void init_board_from_flat(Board *board, int boardsize, const int arr[], int score)
{
    for (int i = 0; i < boardsize; i++)
    {
        for (int j = 0; j < boardsize; j++)
        {
            board->board[i][j] = arr[i * boardsize + j];
        }
    }
    board->score = score;
}

int add_value(Board *board, int boardsize, int value)
{
    int blocks[64][2];
    int free_cnt = get_free_blocks(board, blocks, boardsize);
    if (free_cnt == 0)
        return 0;
    int idx = rand() % free_cnt;
    int x = blocks[idx][0];
    int y = blocks[idx][1];
    board->board[x][y] = value;
    return 1;
}

int add_random(Board *board, int boardsize)
{
    int value = (rand() % 10 < 9) ? 2 : 4;
    return add_value(board, boardsize, value);
}

void add(int num[MAX_BOARD_SIZE][MAX_BOARD_SIZE], int boardsize)
{
    Board board;
    memcpy(&board.board, num, sizeof(int) * 64);
    add_random(&board, boardsize);
    memcpy(num, &board.board, sizeof(int) * 64);
}

int get_free_blocks(const Board *board, int blocks[64][2], int boardsize)
{
    int count = 0;
    for (int i = 0; i < boardsize; i++)
    {
        for (int j = 0; j < boardsize; j++)
        {
            if (board->board[i][j] == 0)
            {
                blocks[count][0] = i;
                blocks[count][1] = j;
                ++count;
                if (count >= 64)
                    return count;
            }
        }
    }
    return count;
}

int add_xy(Board *board, int x, int y, int value)
{
    if (board->board[x][y] == 0)
    {
        board->board[x][y] = value;
        return 1;
    }
    return 0;
}

void remove_xy(Board *board, int x, int y)
{
    board->board[x][y] = 0;
}

int move_left(Board *board, int is_change, int boardsize)
{
    int single_score = 0;
    int is_ok = 0;
    int temp_board[MAX_BOARD_SIZE][MAX_BOARD_SIZE];
    int last_board[MAX_BOARD_SIZE][MAX_BOARD_SIZE];
    memcpy(temp_board, board->board, sizeof(board->board));
    memcpy(last_board, board->board, sizeof(board->board));
    for (int i = 0; i < boardsize; i++)
    {
        int row[MAX_BOARD_SIZE];
        for (int j = 0; j < boardsize; j++)
            row[j] = temp_board[i][j];
        int no_zero[MAX_BOARD_SIZE] = {0};
        int nz_position = 0;
        for (int j = 0; j < boardsize; j++)
            if (row[j] != 0)
                no_zero[nz_position++] = row[j];
        for (int j = 0; j < boardsize; j++)
            row[j] = (j < nz_position) ? no_zero[j] : 0;
        for (int j = 0; j < boardsize - 1; j++)
        {
            if (row[j] == row[j + 1] && row[j] != 0)
            {
                single_score += row[j] * 2;
                row[j] += row[j + 1];
                row[j + 1] = 0;
            }
        }
        nz_position = 0;
        for (int j = 0; j < boardsize; j++)
            if (row[j] != 0)
                no_zero[nz_position++] = row[j];
        for (int j = 0; j < boardsize; j++)
            row[j] = (j < nz_position) ? no_zero[j] : 0;
        for (int j = 0; j < boardsize; j++)
            temp_board[i][j] = row[j];
    }
    if (memcmp(temp_board, last_board, sizeof(board->board)) != 0)
        is_ok = 1;
    if (is_change)
    {
        memcpy(board->board, temp_board, sizeof(board->board));
        board->score += single_score;
    }
    return is_ok;
}

int move_right(Board *board, int is_change, int boardsize)
{
    for (int i = 0; i < boardsize; i++)
    {
        for (int j = 0; j < boardsize / 2; j++)
        {
            int temp = board->board[i][j];
            board->board[i][j] = board->board[i][boardsize - 1 - j];
            board->board[i][boardsize - 1 - j] = temp;
        }
    }
    int ret = move_left(board, is_change, boardsize);
    for (int i = 0; i < boardsize; i++)
    {
        for (int j = 0; j < boardsize / 2; j++)
        {
            int temp = board->board[i][j];
            board->board[i][j] = board->board[i][boardsize - 1 - j];
            board->board[i][boardsize - 1 - j] = temp;
        }
    }
    return ret;
}

void rotate_right(Board *board, int boardsize)
{
    int row, temp;
    row = (boardsize % 2 == 0) ? boardsize / 2 : boardsize / 2 + 1;
    for (int i = 0; i < row; i++)
    {
        for (int j = 0; j < boardsize / 2; j++)
        {
            temp = board->board[i][j];
            board->board[i][j] = board->board[boardsize - 1 - j][i];
            board->board[boardsize - 1 - j][i] = board->board[boardsize - 1 - i][boardsize - 1 - j];
            board->board[boardsize - 1 - i][boardsize - 1 - j] = board->board[j][boardsize - 1 - i];
            board->board[j][boardsize - 1 - i] = temp;
        }
    }
}

void rotate_left(Board *board, int boardsize)
{
    int row, temp;
    row = (boardsize % 2 == 0) ? boardsize / 2 : boardsize / 2 + 1;
    for (int i = 0; i < row; i++)
    {
        for (int j = 0; j < boardsize / 2; j++)
        {
            temp = board->board[i][j];
            board->board[i][j] = board->board[j][boardsize - 1 - i];
            board->board[j][boardsize - 1 - i] = board->board[boardsize - 1 - i][boardsize - 1 - j];
            board->board[boardsize - 1 - i][boardsize - 1 - j] = board->board[boardsize - 1 - j][i];
            board->board[boardsize - 1 - j][i] = temp;
        }
    }
}

int move_up(Board *board, int is_change, int boardsize)
{
    rotate_left(board, boardsize);
    int ret = move_left(board, is_change, boardsize);
    rotate_right(board, boardsize);
    return ret;
}

int move_down(Board *board, int is_change, int boardsize)
{
    rotate_right(board, boardsize);
    int ret = move_left(board, is_change, boardsize);
    rotate_left(board, boardsize);
    return ret;
}

int move(Board *board, int direction, int is_change, int boardsize)
{
    switch (direction)
    {
    case MOVE_LEFT:
        return move_left(board, is_change, boardsize);
    case MOVE_UP:
        return move_up(board, is_change, boardsize);
    case MOVE_RIGHT:
        return move_right(board, is_change, boardsize);
    case MOVE_DOWN:
        return move_down(board, is_change, boardsize);
    default:
        return 0;
    }
}

int isGameWon(Board *board, int boardsize)
{
    for (int i = 0; i < boardsize; i++)
        for (int j = 0; j < boardsize; j++)
            if (board->board[i][j] == 2048)
                return 1;
    return 0;
}

int isGameOver(Board *board, int boardsize)
{
    for (int i = 0; i < boardsize; i++)
        for (int j = 0; j < boardsize; j++)
            if (board->board[i][j] == 0)
                return 0;
    for (int i = 0; i < boardsize; i++)
        for (int j = 0; j < boardsize - 1; j++)
            if (board->board[i][j] == board->board[i][j + 1])
                return 0;
    for (int j = 0; j < boardsize; j++)
        for (int i = 0; i < boardsize - 1; i++)
            if (board->board[i][j] == board->board[i + 1][j])
                return 0;
    return 1;
}
