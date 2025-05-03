#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include "engine.h"
#include "chess.h"
#include "uci.h"
#include "hash.h"

int mg_value[6] = { 82, 337, 365, 477, 1025,  0};
int eg_value[6] = { 94, 281, 297, 512,  936,  0};

int mg_pawn_table[64] = {
      0,   0,   0,   0,   0,   0,  0,   0,
     98, 134,  61,  95,  68, 126, 34, -11,
     -6,   7,  26,  31,  65,  56, 25, -20,
    -14,  13,   6,  21,  23,  12, 17, -23,
    -27,  -2,  -5,  12,  17,   6, 10, -25,
    -26,  -4,  -4, -10,   3,   3, 33, -12,
    -35,  -1, -20, -23, -15,  24, 38, -22,
      0,   0,   0,   0,   0,   0,  0,   0,
};

int eg_pawn_table[64] = {
      0,   0,   0,   0,   0,   0,   0,   0,
    178, 173, 158, 134, 147, 132, 165, 187,
     94, 100,  85,  67,  56,  53,  82,  84,
     32,  24,  13,   5,  -2,   4,  17,  17,
     13,   9,  -3,  -7,  -7,  -8,   3,  -1,
      4,   7,  -6,   1,   0,  -5,  -1,  -8,
     13,   8,   8,  10,  13,   0,   2,  -7,
      0,   0,   0,   0,   0,   0,   0,   0,
};

int mg_knight_table[64] = {
    -167, -89, -34, -49,  61, -97, -15, -107,
     -73, -41,  72,  36,  23,  62,   7,  -17,
     -47,  60,  37,  65,  84, 129,  73,   44,
      -9,  17,  19,  53,  37,  69,  18,   22,
     -13,   4,  16,  13,  28,  19,  21,   -8,
     -23,  -9,  12,  10,  19,  17,  25,  -16,
     -29, -53, -12,  -3,  -1,  18, -14,  -19,
    -105, -21, -58, -33, -17, -28, -19,  -23,
};

int eg_knight_table[64] = {
    -58, -38, -13, -28, -31, -27, -63, -99,
    -25,  -8, -25,  -2,  -9, -25, -24, -52,
    -24, -20,  10,   9,  -1,  -9, -19, -41,
    -17,   3,  22,  22,  22,  11,   8, -18,
    -18,  -6,  16,  25,  16,  17,   4, -18,
    -23,  -3,  -1,  15,  10,  -3, -20, -22,
    -42, -20, -10,  -5,  -2, -20, -23, -44,
    -29, -51, -23, -15, -22, -18, -50, -64,
};


int mg_bishop_table[64] = {
    -29,   4, -82, -37, -25, -42,   7,  -8,
    -26,  16, -18, -13,  30,  59,  18, -47,
    -16,  37,  43,  40,  35,  50,  37,  -2,
     -4,   5,  19,  50,  37,  37,   7,  -2,
     -6,  13,  13,  26,  34,  12,  10,   4,
      0,  15,  15,  15,  14,  27,  18,  10,
      4,  15,  16,   0,   7,  21,  33,   1,
    -33,  -3, -14, -21, -13, -12, -39, -21,
};

int eg_bishop_table[64] = {
    -14, -21, -11,  -8, -7,  -9, -17, -24,
     -8,  -4,   7, -12, -3, -13,  -4, -14,
      2,  -8,   0,  -1, -2,   6,   0,   4,
     -3,   9,  12,   9, 14,  10,   3,   2,
     -6,   3,  13,  19,  7,  10,  -3,  -9,
    -12,  -3,   8,  10, 13,   3,  -7, -15,
    -14, -18,  -7,  -1,  4,  -9, -15, -27,
    -23,  -9, -23,  -5, -9, -16,  -5, -17,
};

int mg_rook_table[64] = {
     32,  42,  32,  51, 63,  9,  31,  43,
     27,  32,  58,  62, 80, 67,  26,  44,
     -5,  19,  26,  36, 17, 45,  61,  16,
    -24, -11,   7,  26, 24, 35,  -8, -20,
    -36, -26, -12,  -1,  9, -7,   6, -23,
    -45, -25, -16, -17,  3,  0,  -5, -33,
    -44, -16, -20,  -9, -1, 11,  -6, -71,
    -19, -13,   1,  17, 16,  7, -37, -26,
};

int eg_rook_table[64] = {
    13, 10, 18, 15, 12,  12,   8,   5,
    11, 13, 13, 11, -3,   3,   8,   3,
     7,  7,  7,  5,  4,  -3,  -5,  -3,
     4,  3, 13,  1,  2,   1,  -1,   2,
     3,  5,  8,  4, -5,  -6,  -8, -11,
    -4,  0, -5, -1, -7, -12,  -8, -16,
    -6, -6,  0,  2, -9,  -9, -11,  -3,
    -9,  2,  3, -1, -5, -13,   4, -20,
};


int mg_queen_table[64] = {
    -28,   0,  29,  12,  59,  44,  43,  45,
    -24, -39,  -5,   1, -16,  57,  28,  54,
    -13, -17,   7,   8,  29,  56,  47,  57,
    -27, -27, -16, -16,  -1,  17,  -2,   1,
     -9, -26,  -9, -10,  -2,  -4,   3,  -3,
    -14,   2, -11,  -2,  -5,   2,  14,   5,
    -35,  -8,  11,   2,   8,  15,  -3,   1,
     -1, -18,  -9,  10, -15, -25, -31, -50,
};

int eg_queen_table[64] = {
     -9,  22,  22,  27,  27,  19,  10,  20,
    -17,  20,  32,  41,  58,  25,  30,   0,
    -20,   6,   9,  49,  47,  35,  19,   9,
      3,  22,  24,  45,  57,  40,  57,  36,
    -18,  28,  19,  47,  31,  34,  39,  23,
    -16, -27,  15,   6,   9,  17,  10,   5,
    -22, -23, -30, -16, -16, -23, -36, -32,
    -33, -28, -22, -43,  -5, -32, -20, -41,
};

int mg_king_table[64] = {
    -65,  23,  16, -15, -56, -34,   2,  13,
     29,  -1, -20,  -7,  -8,  -4, -38, -29,
     -9,  24,   2, -16, -20,   6,  22, -22,
    -17, -20, -12, -27, -30, -25, -14, -36,
    -49,  -1, -27, -39, -46, -44, -33, -51,
    -14, -14, -22, -46, -44, -30, -15, -27,
      1,   7,  -8, -64, -43, -16,   9,   8,
    -15,  36,  12, -54,   8, -28,  24,  14,
};

int eg_king_table[64] = {
    -74, -35, -18, -18, -11,  15,   4, -17,
    -12,  17,  14,  17,  17,  38,  23,  11,
     10,  17,  23,  15,  20,  45,  44,  13,
     -8,  22,  24,  27,  26,  33,  26,   3,
    -18,  -4,  21,  24,  27,  23,   9, -11,
    -19,  -3,  11,  21,  23,  16,   7,  -9,
    -27, -11,   4,  13,  14,   4,  -5, -17,
    -53, -34, -21, -11, -28, -14, -24, -43
};


int* mg_pesto_table[6] =
{
    mg_pawn_table,
    mg_knight_table,
    mg_bishop_table,
    mg_rook_table,
    mg_queen_table,
    mg_king_table
};

int* eg_pesto_table[6] =
{
    eg_pawn_table,
    eg_knight_table,
    eg_bishop_table,
    eg_rook_table,
    eg_queen_table,
    eg_king_table
};

int gamephaseInc[12] = {0,1,1,2,4,0};
int mg_table[12][64];
int eg_table[12][64];

void init_piece_square_tables(void) {
	for (int i = 0; i < 6; i++) {
		for (int sq = 0; sq < 64; sq++) {
			mg_table[i][sq]   = mg_value[i] + mg_pesto_table[i][sq];
			eg_table[i][sq]   = eg_value[i] + eg_pesto_table[i][sq];
			mg_table[i+6][sq] = mg_value[i] + mg_pesto_table[i][sq^56];
			eg_table[i+6][sq] = eg_value[i] + eg_pesto_table[i][sq^56];
		}
	}
}


int evaluate(chessboard_t* b) {
	game_result_t game_res = game_result(b);
	if (game_res == draw) return 0;
	if (game_res == checkmate) return b->fullmove-INT_MAX;
	int mg[2] = {0, 0};
	int eg[2] = {0, 0};
	int phase = 0;
	for (int pc = 0; pc < 6; pc++) {
		bitboard_t white = b->pieces[pc];
		while (white) {
			int sq = bitboard_lowest(white);
			white &= ~(1ull<<sq);
			mg[0] += mg_table[pc][sq];
			eg[0] += eg_table[pc][sq];
			phase += gamephaseInc[pc];
		}
		bitboard_t black = b->pieces[pc+6];
		while (black) {
			int sq = bitboard_lowest(black);
			black &= ~(1ull<<sq);
			mg[1] += mg_table[pc][sq];
			eg[1] += eg_table[pc][sq];
			phase += gamephaseInc[pc];
		}
	}
	int mgScore = mg[b->side] - mg[!b->side];
	int egScore = eg[b->side] - eg[!b->side];
	if (phase > 24) return mgScore;
	return (mgScore*phase + egScore*(24-phase))/24;
}

best_move_t negamax(chessboard_t* b, int alpha, int beta, unsigned depth,
		search_parameter_t* p) {

	if (p) {
		pthread_rwlock_rdlock(&p->locks[3]);
		unsigned stop = p->stop;
		pthread_rwlock_unlock(&p->locks[3]);
		if (stop) return (best_move_t){0, 0};
	}

	int alphaorig = alpha;

	uint64_t key = hash_key(b);
	hash_element_t* entry = hash_index(key);
	best_move_t bm = {.move=0, .score=INT_MIN};

	if (entry->type != node_empty && entry->depth >= depth) {
		if (entry->type == node_exact) {
			return entry->best_move;
		}
		if (entry->type == node_lower && entry->best_move.score >= beta) {
			return entry->best_move;
		}
		if (entry->type == node_upper && entry->best_move.score <= alpha) {
			return entry->best_move;
		}
		bm = entry->best_move;
	}
	if (depth == 0 || (game_result(b) != ongoing))
		// TODO: quiescence search
		return (best_move_t){.move=0, .score=evaluate(b)};

	entry->type = node_searching;

	moves_t moves = generate_moves(b);
	for (int i = 0; i < moves.num_moves; i++) {
		move_t move = moves.moves[i];
		play_move(b, move);
		int score = -negamax(b, -beta, -alpha, depth-1, p).score;
		undo_move(b);

		if (p) {
			pthread_rwlock_rdlock(&p->locks[3]);
			unsigned stop = p->stop;
			pthread_rwlock_unlock(&p->locks[3]);
			if (stop) return (best_move_t){0, 0};
		}

		if (score >= bm.score) {
			bm.score = score;
			bm.move = move;
		}
		alpha = score > alpha ? score : alpha;	// some sort of max function?
		if (score >= beta)
			break;

	}

	node_type_t type = node_exact;
	if (bm.score <= alphaorig) {
		type = node_upper;
	} else if (bm.score >= beta) {
		type = node_lower;
	}
	entry = hash_index(key);
	entry->type = type;
	entry->best_move = bm;
	entry->depth = depth;

	free(moves.moves);
	return bm;
}

unsigned maxdepth = 10;

best_move_t best_move(chessboard_t* b) {
	return negamax(b, -INT_MAX, INT_MAX, 8, NULL);
}

best_move_t find_best_move(search_parameter_t* p) {
	pthread_rwlock_rdlock(&p->locks[0]);
	limit_t limit = p->limit;
	pthread_rwlock_unlock(&p->locks[0]);

	best_move_t best_move = negamax(p->chessboard, -INT_MAX, INT_MAX, 1, NULL);
	if (best_move.move == 0) {
		printf("null move!!!\n");
	}
	char string[6] = {0};
	move_to_string(best_move.move, string);
	printf("info depth %u score cp %d pv %s\n", 1, best_move.score, string);
	fflush(stdout);

	unsigned cur_depth = 1;
	for (cur_depth = 2; cur_depth <= limit.depth; cur_depth++) {
		best_move_t bm = negamax(p->chessboard, -INT_MAX, INT_MAX, cur_depth, p);

		pthread_rwlock_rdlock(&p->locks[3]);
		unsigned stop = p->stop;
		pthread_rwlock_unlock(&p->locks[3]);
		if (stop) break;

		char string[6] = {0};
		move_to_string(bm.move, string);
		printf("info depth %u score cp %d pv %s\n", cur_depth, bm.score, string);
		fflush(stdout);

		best_move = bm;
	}
	string[4] = 0;
	move_to_string(best_move.move, string);
	printf("info depth %u score cp %d pv %s\n", cur_depth-1, best_move.score, string);
	fflush(stdout);
	return best_move;
}

move_t random_player(chessboard_t* b) {
	moves_t moves = generate_moves(b);
	move_t move = moves.moves[random() % moves.num_moves];
	free(moves.moves);
	return move;
}
