#pragma once

#include <stdint.h>

typedef uint64_t bitboard_t;

void print_bitboard(bitboard_t b);
bitboard_t flip_horizontal(bitboard_t b);
int bitboard_contains(bitboard_t b, int i);
int bitboard_lowest(bitboard_t b);
int bitboard_poplsb(bitboard_t* b);
int bitboard_count(bitboard_t b);
bitboard_t random_bitboard(void);
bitboard_t bitboard_fewbits(void);
void init_bitboard_between(void);
bitboard_t bitboard_between(int start, int end);
