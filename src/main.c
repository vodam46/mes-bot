/*
* TODO:
* asserts (ifdef debug)
* remove some of the return 0; from play_move and undo_move?
* bitboard_between - no calculation, only lookup
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "bitboard.h"
#include "chess.h"
#include "magic.h"
#include "engine.h"

#include "test.h"

char* startfen =
 "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
 // "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1";
 // "rnbqkbnr/pp1ppppp/8/2p5/4P3/8/PPPP1PPP/RNBQKBNR w KQkq c6 0 2";
 // "rnbqkbnr/pp1ppppp/8/2p5/4P3/5N2/PPPP1PPP/RNBQKB1R b KQkq - 1 2";

int main(int argc, char** argv) {

	// char str[] ="- This, a sample string.";
	// char * pch;
	// printf ("Splitting string \"%s\" into tokens:\n",str);
	// pch = strtok (str," ,.-");
	// while (pch != NULL)
	// {
	// 	printf ("%s\n",pch);
	// 	pch = strtok (NULL, " ,.-");
	// }
	// return 0;


	init_attack_bitboard();
	generate_magics();
	init_piece_square_tables();
	if (argc > 1 && !strcmp(argv[1], "--test")) {test_all(); return 0;}

	srand(time(NULL));
	chessboard_t* b = init_chessboard(startfen);

	print_chessboard(b);
	printf("The bot shall now play against itself!\n\n");
	while (game_result(b) == ongoing) {
		best_move_t move = find_best_move(b);
		play_move(b, move.move);
		print_chessboard(b);
		printf("%d\n\n", move.score);
	}
	print_chessboard(b);
	printf("%d\n", game_result(b));
	print_game_history(b);
	free(b);

	return 0;
}
