#pragma once

#include "chess.h"
#include "uci.h"

typedef struct best_move {
	move_t move;
	int score;
} best_move_t;

void init_piece_square_tables(void);
int evaluate(chessboard_t* b);
best_move_t negamax(chessboard_t* b, int alpha, int beta, unsigned depth, search_parameter_t* p);
best_move_t best_move(chessboard_t* b);
best_move_t find_best_move(search_parameter_t* p);

move_t random_player(chessboard_t* b);
