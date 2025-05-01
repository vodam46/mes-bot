#pragma once

#include "chess.h"

typedef struct best_move {
	move_t move;
	int score;
} best_move_t;

void init_piece_square_tables(void);
int evaluate(chessboard_t* b);
best_move_t negamax(chessboard_t* b, int alpha, int beta, unsigned depth);
best_move_t find_best_move(chessboard_t* b);

move_t random_player(chessboard_t* b);
