// TODO: fen parsing tests
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "bitboard.h"
#include "chess.h"
#include "test.h"
#include "engine.h"
#include "hash.h"
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
		.num_results = 6	// 9
	},
	{
		.fen="r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
		.results={48, 2039, 97862, 4085603, 193690690, 8031647685},
		.start_depth = 1,
		.num_results = 6	// 6
	},
	{
		.fen="8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1 ",
		.results={14, 191, 2812, 43238, 674624, 11030083, 178633661, 3009794393},
		.start_depth=1,
		.num_results=8,
	},
	{
		.fen="r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1",
		.results={6, 264, 9467, 422333, 15833292, 706045033},
		.start_depth=1,
		.num_results=6,
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
		.num_results=6	// 9
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

	int maximum_perft = 7;

	int len;
	unsigned long long total_visited = 0;
	long long start = timeInMilliseconds();
	len = sizeof(perft_results) / sizeof(perft_results[0]);
	for (int i = 0; i < len; i++) {
		struct perft_results p = perft_results[i];
		printf("%s\n", p.fen);
		chessboard_t* b = init_chessboard(p.fen);
		chessboard_t* compare = init_chessboard(p.fen);
		for (int depth = p.start_depth; depth <= (maximum_perft < p.num_results ? maximum_perft : p.num_results); depth++) {
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

int play_out_position(char* fen, game_result_t expect) {
	chessboard_t* b = init_chessboard(fen);
	while (game_result(b) == ongoing) {
		best_move_t move = best_move(b);
		play_move(b, move.move);
	}
	game_result_t result = game_result(b);
	if (result != checkmate) {
		printf("%d\n", result);
		printf("%d\n", b->fullmove);
		print_chessboard(b);
	}
	free(b);
	return result == expect;
}
struct engine_puzzle {
	char* fen;
	char* best_move;
	unsigned char flags;
};
struct engine_puzzle engine_puzzles[] = {
	{.fen="1k1r4/pp1b1R2/3q2pp/4p3/2B5/4Q3/PPP2B2/2K5 b - -",
		.best_move="d6d1", .flags=0},
	{.fen="3r1k2/4npp1/1ppr3p/p6P/P2PPPP1/1NR5/5K2/2R5 w - -",
		.best_move="d4d5", .flags=0},
	{.fen="2q1rr1k/3bbnnp/p2p1pp1/2pPp3/PpP1P1P1/1P2BNNP/2BQ1PRK/7R b - -",
		.best_move="f6f5", .flags=0},
	{.fen="rnbqkb1r/p3pppp/1p6/2ppP3/3N4/2P5/PPP1QPPP/R1B1KB1R w KQkq -",
		.best_move="e5e6", .flags=0},
	{.fen="r1b2rk1/2q1b1pp/p2ppn2/1p6/3QP3/1BN1B3/PPP3PP/R4RK1 w - -",
		.best_move="b3e8", .flags=0},
	{.fen="2r3k1/pppR1pp1/4p3/4P1P1/5P2/1P4K1/P1P5/8 w - -",
		.best_move="g5g6", .flags=0},
	{.fen="1nk1r1r1/pp2n1pp/4p3/q2pPp1N/b1pP1P2/B1P2R2/2P1B1PP/R2Q2K1 w - -",
		.best_move="h5f6", .flags=0},
	{.fen="4b3/p3kp2/6p1/3pP2p/2pP1P2/4K1P1/P3N2P/8 w - -",
		.best_move="f4f5", .flags=0},
	// {.fen="2kr1bnr/pbpq4/2n1pp2/3p3p/3P1P1B/2N2N1Q/PPP3PP/2KR1B1R w - -",
	// 	.best_move=f5},
	// {.fen="3rr1k1/pp3pp1/1qn2np1/8/3p4/PP1R1P2/2P1NQPP/R1B3K1 b - -",
	// 	.best_move=Ne5},
	// {.fen="2r1nrk1/p2q1ppp/bp1p4/n1pPp3/P1P1P3/2PBB1N1/4QPPP/R4RK1 w - -",
	// 	.best_move=f4},
	// {.fen="r3r1k1/ppqb1ppp/8/4p1NQ/8/2P5/PP3PPP/R3R1K1 b - -",
	// 	.best_move=Bf5},
	// {.fen="r2q1rk1/4bppp/p2p4/2pP4/3pP3/3Q4/PP1B1PPP/R3R1K1 w - -",
	// 	.best_move=b4},
	// {.fen="rnb2r1k/pp2p2p/2pp2p1/q2P1p2/8/1Pb2NP1/PB2PPBP/R2Q1RK1 w - -",
	// 	.best_move=Qd2 Qe1},
	// {.fen="2r3k1/1p2q1pp/2b1pr2/p1pp4/6Q1/1P1PP1R1/P1PN2PP/5RK1 w - -",
	// 	.best_move=Qxg7+},
	// {.fen="r1bqkb1r/4npp1/p1p4p/1p1pP1B1/8/1B6/PPPN1PPP/R2Q1RK1 w kq -",
	// 	.best_move=Ne4},
	// {.fen="r2q1rk1/1ppnbppp/p2p1nb1/3Pp3/2P1P1P1/2N2N1P/PPB1QP2/R1B2RK1 b - -",
	// 	.best_move=h5},
	// {.fen="r1bq1rk1/pp2ppbp/2np2p1/2n5/P3PP2/N1P2N2/1PB3PP/R1B1QRK1 b - -",
	// 	.best_move=Nb3},
	// {.fen="3rr3/2pq2pk/p2p1pnp/8/2QBPP2/1P6/P5PP/4RRK1 b - -",
	// 	.best_move=Rxe4},
	// {.fen="r4k2/pb2bp1r/1p1qp2p/3pNp2/3P1P2/2N3P1/PPP1Q2P/2KRR3 w - -",
	// 	.best_move=g4},
	// {.fen="3rn2k/ppb2rpp/2ppqp2/5N2/2P1P3/1P5Q/PB3PPP/3RR1K1 w - -",
	// 	.best_move=Nh6},
	// {.fen="2r2rk1/1bqnbpp1/1p1ppn1p/pP6/N1P1P3/P2B1N1P/1B2QPP1/R2R2K1 b - -",
	// 	.best_move=Bxe4},
	// {.fen="r1bqk2r/pp2bppp/2p5/3pP3/P2Q1P2/2N1B3/1PP3PP/R4RK1 b kq -",
	// 	.best_move=f6},
	// {.fen="r2qnrnk/p2b2b1/1p1p2pp/2pPpp2/1PP1P3/PRNBB3/3QNPPP/5RK1 w - -",
	// 	.best_move=f4},
};
void test_engine(void) {
	char* checkmates[] = {
		"4k3/8/8/8/8/8/8/3RK2R w K - 0 1",
		"4k3/8/8/8/8/8/8/3RKR2 w - - 0 1",
		"4k3/8/8/8/8/8/8/R3K2R w KQ - 0 1",
		"4k3/8/8/8/8/8/8/R3KR2 w Q - 0 1",
		"4k3/8/8/8/8/8/8/3QKQ2 w - - 0 1",
		"4k3/8/8/8/8/8/8/QQ2K3 w - - 0 1",
		"4k3/8/8/8/8/8/8/Q3K3 w - - 0 1",
	};
	for (unsigned i = 0; i < sizeof(checkmates) / sizeof(checkmates[0]); i++) {
		printf("%s\n", checkmates[i]);
		assert(play_out_position(checkmates[i], checkmate));
	}
	for (unsigned i = 0; i < sizeof(engine_puzzles)/sizeof(engine_puzzles[0]); i++) {
		printf("%s\n%s\n", engine_puzzles[i].fen, engine_puzzles[i].best_move);
		chessboard_t* b = init_chessboard(engine_puzzles[i].fen);
		move_t engine = best_move(b).move;
		free(b);
		char string[6];
		move_to_string(engine, string);
		printf("%s\n", string);
		move_t best   = string_to_move(engine_puzzles[i].best_move) | engine_puzzles[i].flags<<12;
		assert(engine == best);
	}

}


void test_all(void) {
	init_bitboard_between();
	init_attack_bitboard();
	generate_magics();
	init_piece_square_tables();
	init_hash_table();

	test_bitboard();
	test_chess();
	test_magic();
	test_perft();
	test_engine();

	if (tests_failed) printf("\033[31m❌");
	else printf("\033[32m✅");
	printf("passed %d out of %d tests\n", tests_run-tests_failed, tests_run);
	printf("\033[0m");

	destroy_hash_table();
}
