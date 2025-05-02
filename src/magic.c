#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "bitboard.h"
#include "magic.h"

// idk really how this works
bitboard_t index_to_bitboard(int index, int bits, bitboard_t m) {
	bitboard_t result = 0ULL;
	for(int i = 0; i < bits; i++) {
		int j = bitboard_lowest(m);
		m &= ~(1ull<<j);
		if(index & (1 << i)) result |= (1ULL << j);
	}
	return result;
}

int rook_slides[4][2] = {
	{+1, 0},
	{-1, 0},
	{ 0,+1},
	{ 0,-1},
};
int bishop_slides[4][2] = {
	{+1,+1},
	{+1,-1},
	{-1,+1},
	{-1,-1}
};

#define edge(n, o) ((n == 0 || n == 7) && !o)
bitboard_t create_mask(int sq, int dir[4][2]) {
	bitboard_t b = 0ull;
	int rk = sq/8, fl = sq%8;
	for (int i = 0; i < 4; i++) {
		int ro = dir[i][0],
			fo = dir[i][1];
		for (int r = rk+ro, f = fl+fo;
				(edge(r, ro) || (1 <= r && r <= 6)) &&
				(edge(f, fo) || (1 <= f && f <= 6));
				r += ro, f += fo) {
			b |= 1ull << (r*8 + f);
		}
	}
	return b;
}

bitboard_t create_attacks(int sq, bitboard_t block, int dir[4][2]) {
	bitboard_t b = 0ull;
	int rk = sq/8, fl = sq%8;
	for (int i = 0; i < 4; i++) {
		int ro = dir[i][0],
			fo = dir[i][1];
		for (int r = rk+ro, f = fl+fo;
				(edge(r, ro) || (0 <= r && r <= 7)) &&
				(edge(f, fo) || (0 <= f && f <= 7));
				r += ro, f += fo
				) {
			b |= 1ull << (r*8 + f);
			if (block & (1ull << (r*8 + f))) break;
		}
	}
	return b;
}
#undef edge

int magic_index(bitboard_t b, magic_t magic) {
	return (int)(((b&magic.mask) * magic.magic) >> (64 - magic.shift));
}

magic_t rook_magics[64];
bitboard_t rook_magic_attacks[64][4096];
bitboard_t rook_magic_blockers[64][4096];

magic_t bishop_magics[64];
bitboard_t bishop_magic_attacks[64][4096];
bitboard_t bishop_magic_blockers[64][4096];

magic_t find_magic(int sq, int bishop) {
	int (*dir)[2] = bishop ? bishop_slides : rook_slides;

	magic_t magic = {.magic = 0ull, .mask = 0ull, .shift = 0};
	bitboard_t b[4096], a[4096], *used;
	used = bishop ? bishop_magic_attacks[sq] : rook_magic_attacks[sq];

	magic.mask = create_mask(sq, dir);
	magic.shift = bitboard_count(magic.mask);

	for(int i = 0; i < (1 << magic.shift); i++) {
		b[i] = index_to_bitboard(i, magic.shift, magic.mask);
		a[i] = create_attacks(sq, b[i], dir);
	}
	for(int k = 0; k < 100000000; k++) {
		int fail = 0;
		magic.magic = bitboard_fewbits();
		if(bitboard_count((magic.mask * magic.magic) & 0xFF00000000000000ULL) < 6)
			continue;
		for(int i = 0; i < 4096; i++) used[i] = 0ULL;
		for(int i = 0; !fail && i < (1 << magic.shift); i++) {
			int j = magic_index(b[i], magic);
			if(used[j] == 0ULL) used[j] = a[i];
			else if(used[j] != a[i]) fail = 1;
		}
		if(!fail) return magic;
	}
	printf("***Failed***\n");
	return (magic_t){0};
}

int magics_generated = 0;
void generate_magics(void) {
	if (magics_generated) return;
	magics_generated = 1;
	int seed = time(NULL);
	// seed = 1745841240;
	// printf("%d\n", seed);
	srand(seed);
	// printf("rooks\n");
	for (int square = 0; square < 64; square++) {
		magic_t m = find_magic(square, 0);
		rook_magics[square] = m;
	}
	// printf("bishops\n");
	for (int square = 0; square < 64; square++) {
		magic_t m = find_magic(square, 1);
		bishop_magics[square] = m;
	}
	// printf("magics generated\n");
}

bitboard_t get_rook_attacks(int square, bitboard_t blockers) {
	return rook_magic_attacks[square][magic_index(blockers, rook_magics[square])];
}
bitboard_t get_bishop_attacks(int square, bitboard_t blockers) {
	return bishop_magic_attacks[square][magic_index(blockers, bishop_magics[square])];
}
