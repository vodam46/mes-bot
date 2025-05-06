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

inline bitboard_t flip_horizontal(bitboard_t b) {
#ifdef __GNUC__
	return __builtin_bswap64(b);
#else
	// TODO: do it more optimized?
	// b = (b&0x00000000ffffffffull)<<32 | (b&0xffffffff00000000ull)>>32;
	// b = (b&0x0000ffff0000ffffull)<<16 | (b&0xffff0000ffff0000ull)>>16;
	// b = (b&0x00ff00ff00ff00ffull)<<8  | (b&0xff00ff00ff00ff00ull)>>8;
	// return b;
	unsigned char* p = (unsigned char*)&b;
	for (int i = 0; i < 4; i++) {
		unsigned char t = p[i];
		p[i] = p[7-i];
		p[7-i] = t;
	}
	return b;
#endif
}

inline int bitboard_contains(bitboard_t b, int i) {
	return (b>>i)&1;
}

inline int bitboard_lowest(bitboard_t b) {
#ifdef __GNUC__
	return __builtin_ctzll(b);
#else
	bitboard_t upper = b & (b-1);
	return bitboard_count((b ^ upper) - 1);
#endif
}

inline int bitboard_count(bitboard_t b) {
#ifdef __GNUC__
	return __builtin_popcountll(b);
#else
	int c;
	for (c = 0; b; c++, b &= b-1);
	return c;
#endif
}

inline int bitboard_poplsb(bitboard_t* b) {
	int bit = bitboard_lowest(*b);
	*b &= ~(1ull<<bit);
	return bit;
}

bitboard_t random_bitboard(void) {
	bitboard_t u = 0ull;
	for (int i = 0; i < 4; i++)
		u |= (bitboard_t)(random()&0xffff) << (i*16);
	return u;
}

bitboard_t bitboard_fewbits(void) {
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

inline bitboard_t bitboard_between(int start, int end) {
	return bitboard_betweens[start][end];
}
