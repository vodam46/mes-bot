/*
 * TODO:
 * asserts (ifdef debug)
 * debug info
 * remove some of the return 0; from play_move and undo_move?
 * some more optimizations?
 *	- rewrite some of the code so its branchless?
 *	- definitely lots of ways to improve generate_moves (but its correct right now)
 * UCI protocol
 * gui to engine
	 * go (infinite | ponder | nodes <x> | mate <x> | movetime <x>)
 * engine to gui
	 * info
	 * option
 * global init() and quit() functions?
	 * name, author, version of the engine
 * improve move gen
 * quiescence search
 * more eval
 * 3-fold repetition detection
 * better time management
 * uci testing
 * stop the delay on the search start
 * actual normal bools instead of this unsigned bullshit
 * fix all the undefined behavior and bad things (see ../Makefile)
 * instead of piece count, remove unused tt entries based on irreversible moves made
 * change move_to_string, so its normal
 * some better replacement scheme?
 */

#include <string.h>

#include "uci.h"

#include "test.h"


int main(int argc, char** argv) {

	if (argc > 1 && !strcmp(argv[1], "--test")) {test_all(); return 0;}

	uci();

	return 0;
}
