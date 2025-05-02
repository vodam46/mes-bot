/*
* TODO:
* asserts (ifdef debug)
* remove some of the return 0; from play_move and undo_move?
* bitboard_between - no calculation, only lookup
* some more optimizations?
*	- rewrite some of the code so its branchless?
* transposition tables -> iterative deepening
* UCI protocol
*	gui to engine
	*	uci -> uciok
	*	position (startpos | fen) (moves ...) -> -nothing-
	*	isready -> readyok
	*	ucinewgame
	*	go (infinite | ponder | wtime <x> | btime <x> | winc <x> | binc <x>
	*	| depth <x> | nodex <x> | mate <x> | movetime <x>)
	*	stop -> bestmove
	*	ponderhit	?
	*	quit -> duh
*	engine to gui
	* id (name <x> | author <x>)
	* uciok
	* readyok
	* bestmove
	* copyprotection
	* registration
	* info
	* option
*/

#include <stdio.h>
#include <string.h>
#include <pthread.h>

#include "bitboard.h"
#include "chess.h"
#include "magic.h"
#include "engine.h"
#include "uci.h"

#include "test.h"


int main(int argc, char** argv) {
	init_bitboard_between();
	init_attack_bitboard();
	generate_magics();
	init_piece_square_tables();
	if (argc > 1 && !strcmp(argv[1], "--test")) {test_all(); return 0;}

	uci_loop();

	return 0;
}
