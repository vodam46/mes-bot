#pragma once

#include <pthread.h>

#include "chess.h"
typedef enum command_type {
	cmd_quit,
	cmd_stop,
	cmd_position,
	cmd_perft,
	cmd_search,
	cmd_unknown,
	cmd_uci,
	cmd_isready,
	cmd_newgame,

	// debug commands
	cmd_print,
	cmd_undo,
} command_type_t;

// TODO: proper infinite
typedef struct limit {
	unsigned time[2];
	unsigned inc[2];
	unsigned depth;
	// int ponder;	// do i need this?
} limit_t;

typedef struct uci_command {
	command_type_t type;
	union {
		chessboard_t* board;
		limit_t limit;
	} args;
} uci_command_t;

typedef struct time_management {
	unsigned long long start_time;
	unsigned total_time;
	unsigned increment;
	unsigned stop;
	pthread_mutex_t lock;
} time_management_t;

typedef struct search_parameter {
	limit_t limit;
	chessboard_t* chessboard;
	unsigned stop;
	pthread_mutex_t locks[3];

	unsigned multithreaded;
	unsigned long long nodes_visited;
} search_parameter_t;

uci_command_t parse_command(char* command);
void uci(void);
