#include <math.h>
#include <stdio.h>
#include <float.h>
#include <stdlib.h>
#include <string.h>
#include "ai_mode.h" 

float calculate_empty(const Board *board, int boardsize) {
	int empty = 0;
    for (int i = 0; i < boardsize; i++)
        for (int j = 0; j < boardsize; j++)
            if (board->board[i][j] == 0) empty++;
    return (float)empty;
}
float calculate_maxnum(const Board *board, int boardsize) {
    int max = 0;
    for (int i = 0; i < boardsize; i++)
        for (int j = 0; j < boardsize; j++)
            if (board->board[i][j] > max) max = board->board[i][j];
    return (float)max;
}

float calculate_smoothness(const Board *board, int boardsize) {
	float smooth = 0.0f;
    for (int i = 0; i < boardsize; i++) {
        for (int j = 0; j < boardsize; j++) {
            if (board->board[i][j] != 0) {
                float val = log2f(board->board[i][j] + 1);
                if (i + 1 < boardsize) {
                    float v2 = log2f(board->board[i + 1][j] + 1);
                    smooth -= fabsf(v2 - val);
                }
                if (j + 1 < boardsize) {
                    float v2 = log2f(board->board[i][j + 1] + 1);
                    smooth -= fabsf(v2 - val);
                }
                if (i - 1 >= 0) {
                    float v2 = log2f(board->board[i - 1][j] + 1);
                    smooth -= fabsf(v2 - val);
                }
                if (j - 1 >= 0) {
                    float v2 = log2f(board->board[i][j - 1] + 1);
                    smooth -= fabsf(v2 - val);
                }
            }
        }
    }
    return smooth;
}
float calculate_monotonicity(const Board *board, int boardsize) {
    float totals[4] = {0};
    for (int i = 0; i < boardsize; i++) {
        int current = 0;
        int next = current + 1;
        while (next < boardsize) {
            while (next < boardsize && board->board[i][next] == 0) next++;
            if (next >= boardsize) next--;
            float current_val = (board->board[i][current] != 0) ? log2f(board->board[i][current]) : 0;
            float next_val = (board->board[i][next] != 0) ? log2f(board->board[i][next]) : 0;
            if (current_val > next_val)
                totals[0] += next_val - current_val;
            else
                totals[1] += current_val - next_val;
            current = next;
            next++;
        }
    }
    for (int j = 0; j < boardsize; j++) {
        int current = 0;
        int next = current + 1;
        while (next < boardsize) {
            while (next < boardsize && board->board[next][j] == 0) next++;
            if (next >= boardsize) next--;
            float current_val = (board->board[current][j] != 0) ? log2f(board->board[current][j]) : 0;
            float next_val = (board->board[next][j] != 0) ? log2f(board->board[next][j]) : 0;
            if (current_val > next_val)
                totals[2] += next_val - current_val;
            else
                totals[3] += current_val - next_val;
            current = next;
            next++;
        }
    }
    float max1 = (totals[0] > totals[1]) ? totals[0] : totals[1];
    float max2 = (totals[2] > totals[3]) ? totals[2] : totals[3];
    return max1 + max2;
}
void mark_island(const Board *board, int x, int y, int value, int marked[MAX_BOARD_SIZE][MAX_BOARD_SIZE], int boardsize) {
    if (x < 0 || x >= boardsize || y < 0 || y >= boardsize) return;
    if (board->board[x][y] == 0) return;
    if (board->board[x][y] != value) return;
    if (marked[x][y]) return;
    marked[x][y] = 1;
    mark_island(board, x + 1, y, value, marked, boardsize);
    mark_island(board, x - 1, y, value, marked, boardsize);
    mark_island(board, x, y + 1, value, marked, boardsize);
    mark_island(board, x, y - 1, value, marked, boardsize);
}
float calculate_islands(const Board *board, int boardsize) {
    int marked[MAX_BOARD_SIZE][MAX_BOARD_SIZE];
    for (int i = 0; i < boardsize; i++) for (int j = 0; j < boardsize; j++) marked[i][j] = 1;
    float islands = 0;
    for (int i = 0; i < boardsize; i++) {
        for (int j = 0; j < boardsize; j++) {
            if (board->board[i][j] != 0) {
            	marked[i][j] = 0;
			}
        }
    }
    for (int i = 0; i < boardsize; i++) {
        for (int j = 0; j < boardsize; j++) {
            if (board->board[i][j] != 0 && !marked[i][j]) {
                islands++;
                mark_island(board, i, j, board->board[i][j], marked, boardsize);
            }
        }
    }
    return islands;
}
float calculate_evaluation(const Board *board, int boardsize) {
	float empty = calculate_empty(board, boardsize);
    float emptyWeight = 2.7f + (logf(17) - logf(empty + 1)) * 0.1f;
    float maxnumWeight = 1.0f;
    float smoothWeight = 0.1f;
    float monoWeight = 1.0f;
    return emptyWeight * logf(empty + 1) +
           maxnumWeight * calculate_maxnum(board, boardsize) +
           smoothWeight * calculate_smoothness(board, boardsize) +
           monoWeight * calculate_monotonicity(board, boardsize);
}
AI2048Result search_best(Board *this_board, int depth, float alpha, float beta, int playerTurn, int boardsize) {
	AI2048Result result;
    result.move = -1;
    result.score = 0.0f;
    if (playerTurn) {
        float bestScore = alpha;
        int bestMove = -1;
        for (int direction = 0; direction < 4; direction++) {
            Board newBoard;
            copy_board(&newBoard, this_board);
            int changed = move(&newBoard, direction, 1, boardsize);
            if (changed) {
                AI2048Result subResult;
                if (depth == 0) {
                    subResult.move = direction;
                    subResult.score = calculate_evaluation(&newBoard, boardsize);
                } else {
                    subResult = search_best(&newBoard, depth - 1, bestScore, beta, 0, boardsize);
                }
                if (subResult.score > bestScore) {
                    bestScore = subResult.score;
                    bestMove = direction;
                }
                if (bestScore > beta) {
                    result.move = bestMove;
                    result.score = beta;
                    return result;
                }
            }
        }
        result.move = bestMove;
        result.score = bestScore;
        return result;
    } else {
        float bestScore = beta;
        Board newBoard;
        copy_board(&newBoard, this_board);
        int freeBlocks[64][2];
        int freeCount = get_free_blocks(&newBoard, freeBlocks, boardsize);
        float *score_2 = (float*)malloc(freeCount * sizeof(float));
        float *score_4 = (float*)malloc(freeCount * sizeof(float));
        for (int i = 0; i < freeCount; i++) {
            Board tempBoard;
            copy_board(&tempBoard, &newBoard);
            add_xy(&tempBoard, freeBlocks[i][0], freeBlocks[i][1], 2);
            score_2[i] = -calculate_smoothness(&tempBoard, boardsize) + calculate_islands(&tempBoard, boardsize);
            copy_board(&tempBoard, &newBoard);
            add_xy(&tempBoard, freeBlocks[i][0], freeBlocks[i][1], 4);
            score_4[i] = -calculate_smoothness(&tempBoard, boardsize) + calculate_islands(&tempBoard, boardsize);
        }
        float maxScore = -FLT_MAX;
        for (int i = 0; i < freeCount; i++) {
            if (score_2[i] > maxScore) maxScore = score_2[i];
            if (score_4[i] > maxScore) maxScore = score_4[i];
        }
        int worstCount = 0;
        int worstSituations[16][3];
        for (int i = 0; i < freeCount; i++) {
            if (score_2[i] == maxScore) {
                worstSituations[worstCount][0] = freeBlocks[i][0];
                worstSituations[worstCount][1] = freeBlocks[i][1];
                worstSituations[worstCount][2] = 2;
                worstCount++;
            }
            if (score_4[i] == maxScore) {
                worstSituations[worstCount][0] = freeBlocks[i][0];
                worstSituations[worstCount][1] = freeBlocks[i][1];
                worstSituations[worstCount][2] = 4;
                worstCount++;
            }
        }
        for (int k = 0; k < worstCount; k++) {
            Board newBoard2;
            copy_board(&newBoard2, this_board);
            add_xy(&newBoard2, worstSituations[k][0], worstSituations[k][1], worstSituations[k][2]);
            AI2048Result subResult = search_best(&newBoard2, depth, alpha, bestScore, 1, boardsize);
            if (subResult.score < bestScore) {
                bestScore = subResult.score;
            }
            if (bestScore < alpha) {
                free(score_2); free(score_4);
                result.move = -1;
                result.score = alpha;
                return result;
            }
        }
        free(score_2); free(score_4);
        result.move = -1;
        result.score = bestScore;
        return result;
    }
}
int getBestMove(Board *board, int boardsize) {
//	return rand() % 4;
	int depth = 0;
	switch(boardsize) {
		case 4:
			depth = 4;
			break;
		case 5:
			depth = 2;
			break;
		case 6:
			depth = 1;
			break;
		case 8:
			depth = 1;
			break;
		default:
			break;
	}
	AI2048Result result = search_best(board, depth, -1000000.0f, 1000000.0f, 1, boardsize);
	while (depth > 0) {
		if (result.move == -1) result = search_best(board, --depth, -1000000.0f, 1000000.0f, 1, boardsize);
		else break;
	}
	if (result.move == -1) result.move = rand() % 4;
	if (result.move != -1) {
        Board testBoard;
        copy_board(&testBoard, board);
        if (move(&testBoard, result.move, 0, boardsize)) {
            return result.move;
        }
    }
    for (int d = 0; d < 4; d++) {
        Board testBoard;
        copy_board(&testBoard, board);
        if (move(&testBoard, d, 0, boardsize)) {
            return d;
        }
    }
	return -1;
}
int get_action(Board *board, int boardsize) {
    return getBestMove(board, boardsize);
}
void test_moves(Board *board, int boardsize) {
    for (int d = 0; d < 4; d++) {
        Board copy;
        copy_board(&copy, board);
        int changed = move(&copy, d, 0, boardsize);
        printf("     %d: %s\n", d, changed ? "   ƶ " : "     ƶ ");
    }
}
void print_board(Board *board, int boardsize) {
	printf(" ");
	for (int i = 0; i < boardsize; i++) printf("------- ");
	printf("\n");
	for (int i = 0; i < boardsize; i++) {
		printf("|");
		for (int j = 0; j < boardsize; j++) printf("  \t|");
		printf("\n|");
		for (int j = 0; j < boardsize; j++) {
			if (board->board[i][j] == 0) printf("  \t|");
			else printf("%d\t|", board->board[i][j]);
		}
		printf("\n|");
		for (int j = 0; j < boardsize; j++) printf("  \t|");
		printf("\n ");
		for (int j = 0; j < boardsize; j++) printf("------- ");
		printf("\n");
	}
}
void print_board_fast(Board *board, int boardsize) {
    char buffer[8192];
    char *p = buffer;
    int remaining = sizeof(buffer);
    int len;
    len = snprintf(p, remaining, " ");
    p += len; remaining -= len;
    for (int i = 0; i < boardsize; i++) {
        len = snprintf(p, remaining, "------- ");
        p += len; remaining -= len;
    }
    len = snprintf(p, remaining, "\n");
    p += len; remaining -= len;
    for (int i = 0; i < boardsize; i++) {
        len = snprintf(p, remaining, "|");
        p += len; remaining -= len;
        for (int j = 0; j < boardsize; j++) {
            len = snprintf(p, remaining, "  \t|");
            p += len; remaining -= len;
        }
        len = snprintf(p, remaining, "\n|");
        p += len; remaining -= len;
        for (int j = 0; j < boardsize; j++) {
            if (board->board[i][j] == 0) {
                len = snprintf(p, remaining, "  \t|");
            } else {
                len = snprintf(p, remaining, "%d\t|", board->board[i][j]);
            }
            p += len; remaining -= len;
        }
        len = snprintf(p, remaining, "\n|");
        p += len; remaining -= len;
        for (int j = 0; j < boardsize; j++) {
            len = snprintf(p, remaining, "  \t|");
            p += len; remaining -= len;
        }
        len = snprintf(p, remaining, "\n ");
        p += len; remaining -= len;
        for (int j = 0; j < boardsize; j++) {
            len = snprintf(p, remaining, "------- ");
            p += len; remaining -= len;
        }
        len = snprintf(p, remaining, "\n");
        p += len; remaining -= len;
    }
    fputs(buffer, stdout);
}
void print_board_fastest(Board *board, int boardsize) {
    switch(boardsize) {
    	case 4:
    		printf(" ------- ------- ------- ------- \n"
			       "|  \t|  \t|  \t|  \t|\n"
			       "|%d\t|%d\t|%d\t|%d\t|\n"
			       "|  \t|  \t|  \t|  \t|\n"
			       " ------- ------- ------- ------- \n"
			       "|  \t|  \t|  \t|  \t|\n"
			       "|%d\t|%d\t|%d\t|%d\t|\n"
			       "|  \t|  \t|  \t|  \t|\n"
			       " ------- ------- ------- ------- \n"
			       "|  \t|  \t|  \t|  \t|\n"
			       "|%d\t|%d\t|%d\t|%d\t|\n"
			       "|  \t|  \t|  \t|  \t|\n"
			       " ------- ------- ------- ------- \n"
			       "|  \t|  \t|  \t|  \t|\n"
			       "|%d\t|%d\t|%d\t|%d\t|\n"
			       "|  \t|  \t|  \t|  \t|\n"
			       " ------- ------- ------- ------- \n", 
			       board->board[0][0], board->board[0][1], board->board[0][2], board->board[0][3],
			       board->board[1][0], board->board[1][1], board->board[1][2], board->board[1][3],
			       board->board[2][0], board->board[2][1], board->board[2][2], board->board[2][3],
			       board->board[3][0], board->board[3][1], board->board[3][2], board->board[3][3]);
			break;
		case 5:
		    printf(" ------- ------- ------- ------- ------- \n"
		           "|  \t|  \t|  \t|  \t|  \t|\n"
		           "|%d\t|%d\t|%d\t|%d\t|%d\t|\n"
		           "|  \t|  \t|  \t|  \t|  \t|\n"
		           " ------- ------- ------- ------- ------- \n"
		           "|  \t|  \t|  \t|  \t|  \t|\n"
		           "|%d\t|%d\t|%d\t|%d\t|%d\t|\n"
		           "|  \t|  \t|  \t|  \t|  \t|\n"
		           " ------- ------- ------- ------- ------- \n"
		           "|  \t|  \t|  \t|  \t|  \t|\n"
		           "|%d\t|%d\t|%d\t|%d\t|%d\t|\n"
		           "|  \t|  \t|  \t|  \t|  \t|\n"
		           " ------- ------- ------- ------- ------- \n"
		           "|  \t|  \t|  \t|  \t|  \t|\n"
		           "|%d\t|%d\t|%d\t|%d\t|%d\t|\n"
		           "|  \t|  \t|  \t|  \t|  \t|\n"
		           " ------- ------- ------- ------- ------- \n"
		           "|  \t|  \t|  \t|  \t|  \t|\n"
		           "|%d\t|%d\t|%d\t|%d\t|%d\t|\n"
		           "|  \t|  \t|  \t|  \t|  \t|\n"
		           " ------- ------- ------- ------- ------- \n",
		           board->board[0][0], board->board[0][1], board->board[0][2], board->board[0][3], board->board[0][4],
		           board->board[1][0], board->board[1][1], board->board[1][2], board->board[1][3], board->board[1][4],
		           board->board[2][0], board->board[2][1], board->board[2][2], board->board[2][3], board->board[2][4],
		           board->board[3][0], board->board[3][1], board->board[3][2], board->board[3][3], board->board[3][4],
		           board->board[4][0], board->board[4][1], board->board[4][2], board->board[4][3], board->board[4][4]);
		    break;
		case 6:
		    printf(" ------- ------- ------- ------- ------- ------- \n"
		           "|  \t|  \t|  \t|  \t|  \t|  \t|\n"
		           "|%d\t|%d\t|%d\t|%d\t|%d\t|%d\t|\n"
		           "|  \t|  \t|  \t|  \t|  \t|  \t|\n"
		           " ------- ------- ------- ------- ------- ------- \n"
		           "|  \t|  \t|  \t|  \t|  \t|  \t|\n"
		           "|%d\t|%d\t|%d\t|%d\t|%d\t|%d\t|\n"
		           "|  \t|  \t|  \t|  \t|  \t|  \t|\n"
		           " ------- ------- ------- ------- ------- ------- \n"
		           "|  \t|  \t|  \t|  \t|  \t|  \t|\n"
		           "|%d\t|%d\t|%d\t|%d\t|%d\t|%d\t|\n"
		           "|  \t|  \t|  \t|  \t|  \t|  \t|\n"
		           " ------- ------- ------- ------- ------- ------- \n"
		           "|  \t|  \t|  \t|  \t|  \t|  \t|\n"
		           "|%d\t|%d\t|%d\t|%d\t|%d\t|%d\t|\n"
		           "|  \t|  \t|  \t|  \t|  \t|  \t|\n"
		           " ------- ------- ------- ------- ------- ------- \n"
		           "|  \t|  \t|  \t|  \t|  \t|  \t|\n"
		           "|%d\t|%d\t|%d\t|%d\t|%d\t|%d\t|\n"
		           "|  \t|  \t|  \t|  \t|  \t|  \t|\n"
		           " ------- ------- ------- ------- ------- ------- \n"
		           "|  \t|  \t|  \t|  \t|  \t|  \t|\n"
		           "|%d\t|%d\t|%d\t|%d\t|%d\t|%d\t|\n"
		           "|  \t|  \t|  \t|  \t|  \t|  \t|\n"
		           " ------- ------- ------- ------- ------- ------- \n",
		           board->board[0][0], board->board[0][1], board->board[0][2], board->board[0][3], board->board[0][4], board->board[0][5],
		           board->board[1][0], board->board[1][1], board->board[1][2], board->board[1][3], board->board[1][4], board->board[1][5],
		           board->board[2][0], board->board[2][1], board->board[2][2], board->board[2][3], board->board[2][4], board->board[2][5],
		           board->board[3][0], board->board[3][1], board->board[3][2], board->board[3][3], board->board[3][4], board->board[3][5],
		           board->board[4][0], board->board[4][1], board->board[4][2], board->board[4][3], board->board[4][4], board->board[4][5],
		           board->board[5][0], board->board[5][1], board->board[5][2], board->board[5][3], board->board[5][4], board->board[5][5]);
		    break;
		case 8:
		    printf(" ------- ------- ------- ------- ------- ------- ------- ------- \n"
		           "|  \t|  \t|  \t|  \t|  \t|  \t|  \t|  \t|\n"
		           "|%d\t|%d\t|%d\t|%d\t|%d\t|%d\t|%d\t|%d\t|\n"
		           "|  \t|  \t|  \t|  \t|  \t|  \t|  \t|  \t|\n"
		           " ------- ------- ------- ------- ------- ------- ------- ------- \n"
		           "|  \t|  \t|  \t|  \t|  \t|  \t|  \t|  \t|\n"
		           "|%d\t|%d\t|%d\t|%d\t|%d\t|%d\t|%d\t|%d\t|\n"
		           "|  \t|  \t|  \t|  \t|  \t|  \t|  \t|  \t|\n"
		           " ------- ------- ------- ------- ------- ------- ------- ------- \n"
		           "|  \t|  \t|  \t|  \t|  \t|  \t|  \t|  \t|\n"
		           "|%d\t|%d\t|%d\t|%d\t|%d\t|%d\t|%d\t|%d\t|\n"
		           "|  \t|  \t|  \t|  \t|  \t|  \t|  \t|  \t|\n"
		           " ------- ------- ------- ------- ------- ------- ------- ------- \n"
		           "|  \t|  \t|  \t|  \t|  \t|  \t|  \t|  \t|\n"
		           "|%d\t|%d\t|%d\t|%d\t|%d\t|%d\t|%d\t|%d\t|\n"
		           "|  \t|  \t|  \t|  \t|  \t|  \t|  \t|  \t|\n"
		           " ------- ------- ------- ------- ------- ------- ------- ------- \n"
		           "|  \t|  \t|  \t|  \t|  \t|  \t|  \t|  \t|\n"
		           "|%d\t|%d\t|%d\t|%d\t|%d\t|%d\t|%d\t|%d\t|\n"
		           "|  \t|  \t|  \t|  \t|  \t|  \t|  \t|  \t|\n"
		           " ------- ------- ------- ------- ------- ------- ------- ------- \n"
		           "|  \t|  \t|  \t|  \t|  \t|  \t|  \t|  \t|\n"
		           "|%d\t|%d\t|%d\t|%d\t|%d\t|%d\t|%d\t|%d\t|\n"
		           "|  \t|  \t|  \t|  \t|  \t|  \t|  \t|  \t|\n"
		           " ------- ------- ------- ------- ------- ------- ------- ------- \n"
		           "|  \t|  \t|  \t|  \t|  \t|  \t|  \t|  \t|\n"
		           "|%d\t|%d\t|%d\t|%d\t|%d\t|%d\t|%d\t|%d\t|\n"
		           "|  \t|  \t|  \t|  \t|  \t|  \t|  \t|  \t|\n"
		           " ------- ------- ------- ------- ------- ------- ------- ------- \n"
		           "|  \t|  \t|  \t|  \t|  \t|  \t|  \t|  \t|\n"
		           "|%d\t|%d\t|%d\t|%d\t|%d\t|%d\t|%d\t|%d\t|\n"
		           "|  \t|  \t|  \t|  \t|  \t|  \t|  \t|  \t|\n"
		           " ------- ------- ------- ------- ------- ------- ------- ------- \n",
		           board->board[0][0], board->board[0][1], board->board[0][2], board->board[0][3], board->board[0][4], board->board[0][5], board->board[0][6], board->board[0][7],
		           board->board[1][0], board->board[1][1], board->board[1][2], board->board[1][3], board->board[1][4], board->board[1][5], board->board[1][6], board->board[1][7],
		           board->board[2][0], board->board[2][1], board->board[2][2], board->board[2][3], board->board[2][4], board->board[2][5], board->board[2][6], board->board[2][7],
		           board->board[3][0], board->board[3][1], board->board[3][2], board->board[3][3], board->board[3][4], board->board[3][5], board->board[3][6], board->board[3][7],
		           board->board[4][0], board->board[4][1], board->board[4][2], board->board[4][3], board->board[4][4], board->board[4][5], board->board[4][6], board->board[4][7],
		           board->board[5][0], board->board[5][1], board->board[5][2], board->board[5][3], board->board[5][4], board->board[5][5], board->board[5][6], board->board[5][7],
		           board->board[6][0], board->board[6][1], board->board[6][2], board->board[6][3], board->board[6][4], board->board[6][5], board->board[6][6], board->board[6][7],
		           board->board[7][0], board->board[7][1], board->board[7][2], board->board[7][3], board->board[7][4], board->board[7][5], board->board[7][6], board->board[7][7]);
		    break;
		default:
			break;
	}
}
void choose_print(Board *board, int boardsize, int choose) {
	switch(choose) {
		case 1:
			print_board(board, boardsize);
		case 2:
			print_board_fast(board, boardsize);
		default:
			print_board_fastest(board, boardsize);
	}
}
void show_ai(Board *board, int steps, int each_steps, int test, int boardsize, int choose) {
	const char *directions[] = {"  ", "  ", "  ", "  "};
	printf("  ʼ   ̣ \n");
	choose_print(board, boardsize, choose);
	for (int i = 0; i < steps; i++) {
		int ai_move = getBestMove(board, boardsize);
		if (test != -1 && i >= test) {
			test_moves(board, boardsize);
		}
		if (ai_move == -1) {
			printf(" ޷  ƶ     Ϸ      ");
			printf("   յ÷֣ %d\n", board->score);
            break;
		}
		move(board, ai_move, 1, boardsize);
		if (i % each_steps == 0) {
			printf("   %d     ִ   ƶ     : %s\n", i + 1, directions[ai_move]);
			add_random(board, boardsize);
			if (i == steps - 1) {
				printf("       ̣ \n");
				choose_print(board, boardsize, choose);
				printf("   յ÷֣ %d\n", board->score);
			} else {
				printf(" ƶ       ̣ \n");
				choose_print(board, boardsize, choose);
			}
		} else {
			add_random(board, boardsize);
		}
		if (i == steps - 1) {
			printf("       ̣ \n");
			choose_print(board, boardsize, choose);
			printf("   յ÷֣ %d\n", board->score);
		} else {
//			printf("   س     ...\n");
//			getchar();
		}
	}
}

