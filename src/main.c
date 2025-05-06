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
	 * id (name <x> | author <x>)
	 * info
	 * option
 * global init() and quit() functions?
	 * have some sort of globals.h
	 * name, author, version of the engine
 * fix hash table eating memory - rewrite old entries
	 * always replace?
 * improve move gen
 * quiescence search
 * more eval
 * 3-fold repetition detection
 * incremental update zorbist hash keys (and everything, honestly)
 * better time management
 * builtins for clang - https://clang.llvm.org/docs/LanguageExtensions.html
	 * #ifdef __clang__
 * uci testing
 * stop the delay on the search start
 * actual normal bools instead of this unsigned bullshit
 * fix all the undefined behavior and bad things (see ../Makefile)
 * instead of piece count, remove unused tt entries based on irreversible moves made
 * change move_to_string, so its normal
 */

#include <string.h>

#include "uci.h"

#include "test.h"


int main(int argc, char** argv) {

	if (argc > 1 && !strcmp(argv[1], "--test")) {test_all(); return 0;}

	uci_loop();

	return 0;
}
