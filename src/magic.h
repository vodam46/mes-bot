#pragma once

#include "bitboard.h"

extern int magics_generated;
typedef struct magic {
	bitboard_t magic;
	bitboard_t mask;
	int shift;
} magic_t;
extern magic_t rook_magics[64];
extern bitboard_t rook_magic_attacks[64][4096];
extern bitboard_t rook_magic_blockers[64][4096];
extern magic_t bishop_magics[64];
extern bitboard_t bishop_magic_attacks[64][4096];
extern bitboard_t bishop_magic_blockers[64][4096];

void generate_magics();
int magic_index(bitboard_t b, magic_t magic);
bitboard_t get_rook_attacks(int square, bitboard_t blockers);
bitboard_t get_bishop_attacks(int square, bitboard_t blockers);


// only for testing
extern int rook_slides[4][2];
extern int bishop_slides[4][2];
bitboard_t create_mask(int sq, int dir[4][2]);
bitboard_t create_attacks(int sq, bitboard_t block, int dir[4][2]);
bitboard_t index_to_bitboard(int i, int shift, bitboard_t mask);
