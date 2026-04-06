#ifndef AI_MODE_H
#define AI_MODE_H

#include "board.h"

typedef struct {
    int move;
    float score;
} AI2048Result;

float calculate_empty(const Board *board, int boardsize);
float calculate_maxnum(const Board *board, int boardsize);
float calculate_smoothness(const Board *board, int boardsize);
float calculate_monotonicity(const Board *board, int boardsize);
void mark_island(const Board *board, int x, int y, int value, int marked[MAX_BOARD_SIZE][MAX_BOARD_SIZE], int boardsize);
float calculate_islands(const Board *board, int boardsize);
float calculate_evaluation(const Board *board, int boardsize);
AI2048Result search_best(Board *this_board, int depth, float alpha, float beta, int playerTurn, int boardsize);
int getBestMove(Board *board, int boardsize);
int get_action(Board *board, int boardsize);
void test_moves(Board *board, int boardsize);
void print_board(Board *board, int boardsize);
void print_board_fast(Board *board, int boardsize);
void print_board_fastest(Board *board, int boardsize);
void choose_print(Board *board, int boardsize, int choose);
void show_ai(Board *board, int steps, int each_steps, int test, int boardsize, int choose);

#endif

