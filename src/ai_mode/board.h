#ifndef BOARD_H
#define BOARD_H

#define MAX_BOARD_SIZE 8
#define MOVE_LEFT  0
#define MOVE_UP    1
#define MOVE_RIGHT 2
#define MOVE_DOWN  3

typedef struct {
    int board[MAX_BOARD_SIZE][MAX_BOARD_SIZE];
    int score;
} Board;

void init_board(Board *board, int boardsize);
void copy_board(Board *board, const Board *grid);
void init_board_from_flat(Board *board, int boardsize, const int arr[], int score);
int add_value(Board *board, int boardsize, int value);
int add_random(Board *board, int boardsize);
void add(int num[MAX_BOARD_SIZE][MAX_BOARD_SIZE], int boardsize);
int get_free_blocks(const Board *board, int blocks[64][2], int boardsize);
int add_xy(Board *board, int x, int y, int value);
void remove_xy(Board *board, int x, int y);
int move_left(Board *board, int is_change, int boardsize);
int move_right(Board *board, int is_change, int boardsize);
void rotate_right(Board *board, int boardsize);
void rotate_left(Board *board, int boardsize);
int move_up(Board *board, int is_change, int boardsize);
int move_down(Board *board, int is_change, int boardsize);
int move(Board *board, int direction, int is_change, int boardsize);
int isGameWon(Board *board, int boardsize);
int isGameOver(Board *board, int boardsize);

#endif

