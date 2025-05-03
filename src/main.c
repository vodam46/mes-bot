/*
* TODO:
* asserts (ifdef debug)
* debug info
* remove some of the return 0; from play_move and undo_move?
* some more optimizations?
*	- rewrite some of the code so its branchless?
*	- definitely lots of ways to improve generate_moves (but its correct right now)
* transposition tables -> iterative deepening
* UCI protocol
*	gui to engine
	*	ucinewgame	- do i really need this?
	*	go (infinite | ponder | nodes <x> | mate <x> | movetime <x>)
*	engine to gui
	* id (name <x> | author <x>)
	* info
	* option
*
* global init() and quit() functions?
* have some sort of globals.h
	* name, and author of the engine
*/

#include <string.h>

#include "uci.h"

#include "test.h"


int main(int argc, char** argv) {

	if (argc > 1 && !strcmp(argv[1], "--test")) {test_all(); return 0;}

	uci_loop();

	return 0;
}
