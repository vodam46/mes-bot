#pragma once

#include <stdint.h>

#include "chess.h"
#include "engine.h"

typedef enum node_type {
	node_empty,
	node_searching,
	node_exact,
	node_upper,
	node_lower
} node_type_t;

typedef struct hash_element {
	uint64_t key;
	unsigned depth;
	best_move_t best_move;
	node_type_t type;
} hash_element_t;

typedef struct hash_table {
	hash_element_t* elements;
	unsigned size;
	unsigned num_elements;
} hash_table_t;

void init_hash_table(void);
void destroy_hash_table(void);

uint64_t hash_key(chessboard_t* b);
hash_element_t* hash_index(uint64_t key);

extern uint64_t polyglot_random_array[781];
