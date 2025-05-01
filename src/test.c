// TODO: fen parsing tests
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "bitboard.h"
#include "chess.h"
#include "test.h"
#include "magic.h"

#define assert(b) do {tests_run++; if (!(b)) { \
	tests_failed++;\
	printf("\033[31mfailed line %d: %s\n\033[0m", __LINE__, #b);\
}} while (0);
int tests_run = 0;
int tests_failed = 0;

void test_bitboard(void) {
	// TODO: write more tests
	assert(flip_horizontal(0ULL) == 0ULL);
	assert(flip_horizontal(1ULL) == 0x100000000000000ULL);
	assert(bitboard_contains(1ULL, 0));
	assert(bitboard_contains(0xff00ull, 8));
	for (int i = 0; i < 64; i++) {
		assert(!bitboard_contains(0ull, i));
		assert(bitboard_contains(0xffffffffffffffffull, i));
	}
	for (int i = 0; i < 64; i++)
		assert(bitboard_lowest(1ull<<i) == i);
	assert(bitboard_lowest(0ULL) == 64);
}

char* testfen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
char* onlykings = "k7/8/8/8/8/8/8/K7 w - - 0 1";

// TODO: more!
struct perft_results {
	char* fen;
	unsigned long long results[10];
	int num_results;
	int start_depth;
};
struct perft_results perft_results[] = {
	{
		.fen="rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
		.results={20, 400, 8902, 197281, 4865609, 119060324, 3195901860, 84998978956, 2439530234167},
		.start_depth = 1,
		.num_results = 7	// 7
	},
	{
		.fen="r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
		.results={48, 2039, 97862, 4085603, 193690690, 803164768},
		.start_depth = 1,
		.num_results = 6	// 6
	},
	{
		.fen="8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1 ",
		.results={14, 191, 2812, 43238, 674624, 11030083, 178633661, 3009794393},
		.start_depth=1,
		.num_results=6,
	},
	{
		.fen="r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1",
		.results={6, 264, 9467, 422333, 15833292, 706045033},
		.start_depth=1,
		.num_results=5,
	},
	{
		.fen="rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8",
		.results={44, 1486, 62379, 2103487, 89941194},
		.start_depth=1,
		.num_results=5
	},
	{
		.fen="r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10",
		.results={1, 46, 2079, 89890, 3894594, 164075551, 6923051137 ,
			287177884746, 11923589843526, 490154852788714},
		.start_depth=0,
		.num_results=5	// 6
	},
	{
		.fen="r6r/1b2k1bq/8/8/7B/8/8/R3K2R b KQ - 3 2",
		.results={8},
		.start_depth = 1,
		.num_results=1,
	},
	{
		.fen="8/8/8/2k5/2pP4/8/B7/4K3 b - d3 0 3",
		.results={8},
		.start_depth = 1,
		.num_results=1,
	},
	{
		.fen="r1bqkbnr/pppppppp/n7/8/8/P7/1PPPPPPP/RNBQKBNR w KQkq - 2 2",
		.results={19},
		.start_depth = 1,
		.num_results=1,
	},
	{
		.fen="r3k2r/p1pp1pb1/bn2Qnp1/2qPN3/1p2P3/2N5/PPPBBPPP/R3K2R b KQkq - 3 2",
		.results={5},
		.start_depth = 1,
		.num_results=1,
	},
	{
		.fen="2kr3r/p1ppqpb1/bn2Qnp1/3PN3/1p2P3/2N5/PPPBBPPP/R3K2R b KQ - 3 2",
		.results={44},
		.start_depth = 1,
		.num_results=1,
	},
	{
		.fen="rnb2k1r/pp1Pbppp/2p5/q7/2B5/8/PPPQNnPP/RNB1K2R w KQ - 3 9",
		.results={39},
		.start_depth = 1,
		.num_results=1,
	},
	{
		.fen="2r5/3pk3/8/2P5/8/2K5/8/8 w - - 5 4",
		.results={9},
		.start_depth = 1,
		.num_results=1,
	},
	{
		.fen="rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8",
		.results={62379},
		.start_depth = 3,
		.num_results=3,
	},
	{
		.fen="r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10",
		.results={89890},
		.start_depth = 3,
		.num_results=3,
	},
	{
		.fen="3k4/3p4/8/K1P4r/8/8/8/8 b - - 0 1",
		.results={1134888},
		.start_depth = 6,
		.num_results=6,
	},
	{
		.fen="8/8/4k3/8/2p5/8/B2P2K1/8 w - - 0 1",
		.results={1015133},
		.start_depth = 6,
		.num_results=6,
	},
	{
		.fen="8/8/1k6/2b5/2pP4/8/5K2/8 b - d3 0 1",
		.results={1440467},
		.start_depth = 6,
		.num_results=6,
	},
	{
		.fen="5k2/8/8/8/8/8/8/4K2R w K - 0 1",
		.results={661072},
		.start_depth = 6,
		.num_results=6,
	},
	{
		.fen="3k4/8/8/8/8/8/8/R3K3 w Q - 0 1",
		.results={803711},
		.start_depth = 6,
		.num_results=6,
	},
	{
		.fen="r3k2r/1b4bq/8/8/8/8/7B/R3K2R w KQkq - 0 1",
		.results={1274206},
		.start_depth = 4,
		.num_results=4,
	},
	{
		.fen="r3k2r/8/3Q4/8/8/5q2/8/R3K2R b KQkq - 0 1",
		.results={1720476},
		.start_depth = 4,
		.num_results=4,
	},
	{
		.fen="2K2r2/4P3/8/8/8/8/8/3k4 w - - 0 1",
		.results={3821001},
		.start_depth = 6,
		.num_results=6,
	},
	{
		.fen="8/8/1P2K3/8/2n5/1q6/8/5k2 b - - 0 1",
		.results={1004658},
		.start_depth = 5,
		.num_results=5,
	},
	{
		.fen="4k3/1P6/8/8/8/8/K7/8 w - - 0 1",
		.results={217342},
		.start_depth = 6,
		.num_results=6,
	},
	{
		.fen="8/P1k5/K7/8/8/8/8/8 w - - 0 1",
		.results={92683},
		.start_depth = 6,
		.num_results=6,
	},
	{
		.fen="K1k5/8/P7/8/8/8/8/8 w - - 0 1",
		.results={2217},
		.start_depth = 6,
		.num_results=6,
	},
	{
		.fen="8/k1P5/8/1K6/8/8/8/8 w - - 0 1",
		.results={567584},
		.start_depth = 7,
		.num_results=7,
	},
	{
		.fen="8/8/2k5/5q2/5n2/8/5K2/8 b - - 0 1",
		.results={23527},
		.start_depth = 4,
		.num_results=4,
	}
};


bitboard_t rmask(int sq) {
	bitboard_t result = 0ULL;
	int rk = sq/8, fl = sq%8, r, f;
	for(r = rk+1; r <= 6; r++) result |= (1ULL << (fl + r*8));
	for(r = rk-1; r >= 1; r--) result |= (1ULL << (fl + r*8));
	for(f = fl+1; f <= 6; f++) result |= (1ULL << (f + rk*8));
	for(f = fl-1; f >= 1; f--) result |= (1ULL << (f + rk*8));
	return result;
}

bitboard_t bmask(int sq) {
	bitboard_t result = 0ULL;
	int rk = sq/8, fl = sq%8, r, f;
	for(r=rk+1, f=fl+1; r<=6 && f<=6; r++, f++) result |= (1ULL << (f + r*8));
	for(r=rk+1, f=fl-1; r<=6 && f>=1; r++, f--) result |= (1ULL << (f + r*8));
	for(r=rk-1, f=fl+1; r>=1 && f<=6; r--, f++) result |= (1ULL << (f + r*8));
	for(r=rk-1, f=fl-1; r>=1 && f>=1; r--, f--) result |= (1ULL << (f + r*8));
	return result;
}

bitboard_t ratt(int sq, bitboard_t block) {
	bitboard_t result = 0ULL;
	int rk = sq/8, fl = sq%8, r, f;
	for(r = rk+1; r <= 7; r++) {
		result |= (1ULL << (fl + r*8));
		if(block & (1ULL << (fl + r*8))) break;
	}
	for(r = rk-1; r >= 0; r--) {
		result |= (1ULL << (fl + r*8));
		if(block & (1ULL << (fl + r*8))) break;
	}
	for(f = fl+1; f <= 7; f++) {
		result |= (1ULL << (f + rk*8));
		if(block & (1ULL << (f + rk*8))) break;
	}
	for(f = fl-1; f >= 0; f--) {
		result |= (1ULL << (f + rk*8));
		if(block & (1ULL << (f + rk*8))) break;
	}
	return result;
}

bitboard_t batt(int sq, bitboard_t block) {
	bitboard_t result = 0ULL;
	int rk = sq/8, fl = sq%8, r, f;
	for(r = rk+1, f = fl+1; r <= 7 && f <= 7; r++, f++) {
		result |= (1ULL << (f + r*8));
		if(block & (1ULL << (f + r * 8))) break;
	}
	for(r = rk+1, f = fl-1; r <= 7 && f >= 0; r++, f--) {
		result |= (1ULL << (f + r*8));
		if(block & (1ULL << (f + r * 8))) break;
	}
	for(r = rk-1, f = fl+1; r >= 0 && f <= 7; r--, f++) {
		result |= (1ULL << (f + r*8));
		if(block & (1ULL << (f + r * 8))) break;
	}
	for(r = rk-1, f = fl-1; r >= 0 && f >= 0; r--, f--) {
		result |= (1ULL << (f + r*8));
		if(block & (1ULL << (f + r * 8))) break;
	}
	return result;
}

void test_chess(void) {
	generate_magics();
	chessboard_t* b;

	b = init_chessboard("");
	assert(!b->side);
	for (int i = 0; i<12; i++)
		assert(b->pieces[i] == 0ULL);
	// TODO: more asserts for a null chessboard?
	free(b);

	b = init_chessboard(testfen);
	assert(!b->side);
	bitboard_t positions[6] = {
		0xff00ull,	// pawns
		0x42ull,	// knight
		0x24ull,	// bishop
		0x81ull,	// rook
		0x08ull,	// queen
		0x10ull,	// king
	};
	for (int i = 0; i < 12; i++)
		assert(b->pieces[i] == (i<6 ? positions[i] : flip_horizontal(positions[i-6])));

	assert(piece_at(b, string_to_square("e1")) == wking);
	assert(piece_at(b, string_to_square("d1")) == wqueen);
	assert(bitboard_lowest(b->pieces[wking]) == 4);

	assert(pieces_color(b, 0) == 0xffffull);
	assert(pieces_color(b, b->side) == 0xffffull);
	assert(pieces_color(b, 1) == flip_horizontal(0xffffull));

	assert(string_to_square("a1") == 0);
	for (int i = 0; i < 64; i++) {
		char string[2];
		square_to_string(i, string);
		assert(string_to_square(string) == i);
	}
	for (int from = 0; from < 64; from++)
		for (int to = 0; to < 64; to++) {
			char string[6] = {0};
			move_to_string((to<<6) | from, string);
			assert(string_to_move(string) == ((to<<6) | from));
		}

	moves_t m = generate_moves(b);
	// print_chessboard(b);
	// print_moves(m);
	assert(m.num_moves == 20);
	free(m.moves);

	assert(string_to_square("a1") == 0);
	assert(string_to_square("e2") == 12);

	assert(play_move(b, string_to_move("e2e4") | 1<<12));
	assert(b->side);
	assert(!bitboard_contains(b->pieces[wpawn], string_to_square("e2")));
	assert(bitboard_contains(b->pieces[wpawn], string_to_square("e4")));
	assert(b->pieces[wpawn] == 0x1000ef00ull);
	for (int i=1; i < 12; i++) {
		assert(b->pieces[i] == (i<6 ? positions[i] : flip_horizontal(positions[i-6])));
	}

	assert(play_move(b, string_to_move("d7d5") | 1<<12));
	assert(!b->side);

	assert(play_move(b, string_to_move("e4d5") | 0x4<<12));
	assert(b->side);

	assert(play_move(b, string_to_move("d8d5") | 0x4<<12));
	assert(!b->side);

	assert(b->pieces[wpawn] == 0xef00ull);
	assert(b->pieces[bpawn] == flip_horizontal(0xf700ull));
	assert(b->pieces[bqueen] == 1ull << (4*8+3));
	for (int i = 1; i < 12; i++) {
		if (i != bpawn && i != bqueen)
			assert(b->pieces[i] == (i<6 ? positions[i] : flip_horizontal(positions[i-6])));
	}

	assert(undo_move(b));
	assert(undo_move(b));
	assert(undo_move(b));
	assert(undo_move(b));
	for (int i = 0; i < 12; i++) {
		assert(b->pieces[i] == (i<6 ? positions[i] : flip_horizontal(positions[i-6])));
	}

	free(b);

	b = init_chessboard(onlykings);
	for (int i = 0; i < 12; i++)
		assert(i == wking || i == bking || b->pieces[i] == 0ull);

	moves_t moves = generate_moves(b);
	assert(moves.num_moves == 3);
	assert(game_result(b) == draw);
	free(moves.moves);
	free(b);

	b = init_chessboard("4k3/8/8/8/8/8/8/4KR2 b - - 0 1");
	moves = generate_moves(b);
	assert(moves.num_moves == 3);
	free(moves.moves);
	free(b);

	b = init_chessboard("4k3/8/8/8/8/8/8/3KR3 b - - 0 1");
	moves = generate_moves(b);
	assert(moves.num_moves == 4);
	free(moves.moves);
	free(b);

	b = init_chessboard(testfen);
	play_move(b, string_to_move("c2c3"));
	moves = generate_moves(b);
	// print_moves(moves);
	free(moves.moves);
	free(b);

	b = init_chessboard("rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 10 1");
	assert(b->en_passant_square == string_to_square("e3"));
	assert(b->fiftymove == 10);
	free(b);


	b = init_chessboard("4k3/8/6n1/4R3/8/8/8/4K3 b - - 0 1");
	moves = generate_moves(b);
	// print_chessboard(b);
	// print_moves(moves);
	assert(moves.num_moves == 6);
	free(moves.moves);
	free(b);

	b = init_chessboard("4k3/4n3/8/4R3/8/8/8/4K3 b - - 0 1");
	moves = generate_moves(b);
	// print_chessboard(b);
	// print_moves(moves);
	assert(moves.num_moves == 4);
	free(moves.moves);
	free(b);

	b = init_chessboard("4k3/4r3/8/4R3/8/8/8/4K3 b - - 0 1");
	moves = generate_moves(b);
	// print_chessboard(b);
	// print_moves(moves);
	assert(moves.num_moves == 6);
	free(moves.moves);
	free(b);

	b = init_chessboard(testfen);
	assert(game_result(b) == ongoing);
	play_move(b, string_to_move("f2f3"));
	assert(game_result(b) == ongoing);
	play_move(b, string_to_move("e7e6"));
	assert(game_result(b) == ongoing);
	play_move(b, string_to_move("g2g4"));
	assert(game_result(b) == ongoing);
	play_move(b, string_to_move("d8h4"));
	assert(game_result(b) == checkmate);
	free(b);
}

void test_magic(void) {
	generate_magics();
	for (int square = 0; square < 64; square++) {
		assert(create_mask(square, rook_slides) == rmask(square));

		assert(create_mask(square, bishop_slides) == bmask(square));

		magic_t r = rook_magics[square];
		for (int i = 0; i < (1ll << r.shift); i++) {
			bitboard_t blockers = index_to_bitboard(i, r.shift, r.mask);
			assert(create_attacks(square, blockers, rook_slides) == ratt(square, blockers));
			assert(get_rook_attacks(square, blockers) == create_attacks(square, blockers, rook_slides));

		}
		magic_t b = bishop_magics[square];
		for (int i = 0; i < (1ll << b.shift); i++) {
			bitboard_t blockers = index_to_bitboard(i, b.shift, b.mask);
			assert(create_attacks(square, blockers, bishop_slides) == batt(square, blockers));
			assert(get_bishop_attacks(square, blockers) == create_attacks(square, blockers, bishop_slides));
		}
	}
}

long long timeInMilliseconds(void) {
    struct timeval tv;

    gettimeofday(&tv,NULL);
    return (((long long)tv.tv_sec)*1000)+(tv.tv_usec/1000);
}

void test_perft(void) {

	// printf("\n\n----\n\n");
	// chessboard_t* b = init_chessboard(
	// 		"8/8/K4R2/1Ppp3r/8/6k1/4P1P1/8 w - c6 0 4"
	// );
	// print_moves(generate_moves(b));
	// unsigned long long t;
	// printf("%llu\n", perft(b, 2, 1, &t));
	// free(b);
	// return;


	int len;
	unsigned long long total_visited = 0;
	long long start = timeInMilliseconds();
	len = sizeof(perft_results) / sizeof(perft_results[0]);
	for (int i = 0; i < len; i++) {
		struct perft_results p = perft_results[i];
		printf("%s\n", p.fen);
		chessboard_t* b = init_chessboard(p.fen);
		chessboard_t* compare = init_chessboard(p.fen);
		for (int depth = p.start_depth; depth <= p.num_results; depth++) {
			unsigned long long pe = perft(b, depth, 1, &total_visited);
			long long stop = timeInMilliseconds();
			printf("%d - %llu (%llu)\n", depth, pe, p.results[depth - p.start_depth]);
			assert(pe == p.results[depth - p.start_depth])
			long long time_taken = stop - start;
			printf("total time: %lld\n", time_taken);
			if (time_taken > 0)
				printf("nps: ~%llu\n", (total_visited*1000)/time_taken);
			printf("\n");
		}
		printf("total_visited: %llu\n\n", total_visited);
		free(b);
		free(compare);
	}

}

void test_engine(void) {

}


void test_all(void) {
	test_bitboard();
	test_chess();
	test_magic();
	// test_perft();
	test_engine();

	if (tests_failed) printf("\033[31m❌");
	else printf("\033[32m✅");
	printf("passed %d out of %d tests\n", tests_run-tests_failed, tests_run);
	printf("\033[0m");

}
