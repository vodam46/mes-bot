#include <stdio.h>
#include <stdlib.h>

#include "bitboard.h"

void print_bitboard(bitboard_t b) {
	unsigned char* p = (unsigned char*)&b;
	for (int i = 7; i >= 0; i--) {
		unsigned char byte = p[i];
		for (int j = 7; j >= 0; j--, byte >>= 1)
			printf("%d", byte&1);
		printf("\n");
	}
	printf("\n");
}

bitboard_t flip_horizontal(bitboard_t b) {
	// TODO: do it more optimized?
	bitboard_t r = b;
	unsigned char* p = (unsigned char*)&r;
	for (int i = 0; i < 4; i++) {
		unsigned char t = p[i];
		p[i] = p[7-i];
		p[7-i] = t;
	}
	return r;
}

int bitboard_contains(bitboard_t b, int i) {
	return (b>>i)&1;
}

int bitboard_lowest(bitboard_t b) {
#ifdef __GNUC__
	return __builtin_ctzll(b);
#endif
	bitboard_t upper = b & (b-1);
	return bitboard_count((b ^ upper) - 1);
}

int bitboard_count(bitboard_t b) {
#ifdef __GNUC__
	return __builtin_popcountll(b);
#endif
	int c;
	for (c = 0; b; c++, b &= b-1);
	return c;
}

bitboard_t random_bitboard(void) {
	bitboard_t u1, u2, u3, u4;
	u1 = (bitboard_t)(random()) & 0xFFFF; u2 = (bitboard_t)(random()) & 0xFFFF;
	u3 = (bitboard_t)(random()) & 0xFFFF; u4 = (bitboard_t)(random()) & 0xFFFF;
	return u1 | (u2 << 16) | (u3 << 32) | (u4 << 48);
}

bitboard_t bitboard_fewbits() {
	return random_bitboard() & random_bitboard() & random_bitboard();
}

bitboard_t bitboard_betweens[64][64];
void init_bitboard_between(void) {
	for (int s = 0; s < 64; s++) {
		for (int e = 0; e < 64; e++) {
			int fr = s>>3, ff = s&7;
			int tr = e>>3, tf = e&7;
			int ro = (tr>fr) - (tr<fr);
			int fo = (tf>ff) - (tf<ff);

			int dr = tr - fr;
			int df = tf - ff;

			bitboard_t b = 0x0ull;
			if (dr == 0 || df == 0 || abs(dr) == abs(df)) {
				for (int r = fr+ro, f = ff+fo;
						r != tr || f != tf;
						r += ro, f += fo) {
					b |= 1ull << (r*8+f);
				}
			}
			bitboard_betweens[s][e] = b;
		}
	}
}

bitboard_t bitboard_between(int start, int end) {
	return bitboard_betweens[start][end];
}
