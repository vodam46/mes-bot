#pragma once

#include "bitboard.h"

typedef enum piece {
	pempty = -1,
	wpawn,
	wknight,
	wbishop,
	wrook,
	wqueen,
	wking,
	bpawn,
	bknight,
	bbishop,
	brook,
	bqueen,
	bking,
} piece_t;

extern char piece_chars[12];
extern piece_t chars_piece[128];

//  4 bits for flags
	// bit 1 - promotion
	// bit 2 - capture
	// bit 3, 4 - specials
	// https://www.chessprogramming.org/Encoding_Moves#From-To_Based
//  6 bits for to square
//  6 bits for from square
typedef uint16_t move_t;

typedef struct moves {
	move_t* moves;
	int num_moves;
	int arr_len;
} moves_t;

// TODO: more information?
typedef struct meta_move {
	int enpassant;
	int fiftymove;
	move_t move;
	piece_t piece;
	piece_t capture;
	unsigned char castling;
	struct meta_move* next;
} meta_move_t;

typedef struct chessboard {
	bitboard_t pieces[12];	// lower 6 - white
							// upper 6 - black
							// PNBRQK


	unsigned side: 1;	// white = 0, black = 1

	meta_move_t* moves;

	int en_passant_square;
	unsigned char castling_rights;

	unsigned fiftymove;
	unsigned ply;

	/*
	 * repetition checking
		 * linked list of previous positions?
		 * only check if pawn bitboards are the same
		 * and the number of pieces are the same?
	 */
} chessboard_t;

extern bitboard_t pawn_attacks[64];
extern bitboard_t pawn_moves[64][2];
extern bitboard_t knight_attacks[64];
extern bitboard_t bishop_attacks[64];
extern bitboard_t rook_attacks[64];
extern bitboard_t queen_attacks[64];
extern bitboard_t king_attacks[64];

void init_attack_bitboard(void);

void print_chessboard(chessboard_t* b);
chessboard_t* init_chessboard(char* fen);

bitboard_t pieces_color(chessboard_t* b, int color);
piece_t piece_at(chessboard_t* b, unsigned pos);
int piece_side(piece_t p);

unsigned char string_to_square(char square[2]);
void square_to_string(unsigned char square, char string[2]);

move_t string_to_move(char s[5]);
move_t string_to_move_flags(chessboard_t* b, char s[5]);
void move_to_string(move_t m, char s[5]);

void print_moves(moves_t m);
void print_game_history(chessboard_t* b);

int play_move(chessboard_t* b, move_t m);
int undo_move(chessboard_t* b);

bitboard_t get_attacked_squares(chessboard_t* b);
void append_move(moves_t* moves, move_t move);
moves_t generate_moves(chessboard_t* b);
unsigned long long perft(chessboard_t* b, int depth, int print, unsigned long long* total_visited);

// TODO: add more of those
typedef enum game_result {
	ongoing,
	checkmate,
	draw
} game_result_t;
game_result_t game_result(chessboard_t* b);
