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
		unsigned depth;
		chessboard_t* board;
		char* position;
		limit_t limit;
	} args;
} uci_command_t;

typedef struct time_management {
	unsigned long long start_time;
	unsigned total_time;
	unsigned increment;
	unsigned finished;
} time_management_t;

typedef struct search_parameter {
	limit_t limit;
	chessboard_t* chessboard;
	unsigned keep_running;
	unsigned stop;
	pthread_rwlock_t locks[4];
	unsigned multithreaded;
} search_parameter_t;

uci_command_t parse_command(char* command);
void uci_loop(void);
