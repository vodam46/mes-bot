// TODO: stop using rwlock for everything
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>

#include "globals.h"
#include "chess.h"
#include "engine.h"
#include "hash.h"
#include "magic.h"
#include "uci.h"


char* startfen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

uci_command_t parse_command(char* command) {
	uci_command_t cmd = {.type=cmd_unknown};
	if (!strncmp(command, "go", 2)) {
		if (!strncmp(command, "go perft", 8)) {
			cmd.type = cmd_perft;
			cmd.args.depth = atoi(command+9);
		} else {
			cmd.type = cmd_search;
			cmd.args.limit = (limit_t){
				.time = {UINT_MAX, UINT_MAX},
				.inc = {UINT_MAX, UINT_MAX},
				.depth = UINT_MAX
			};
			char* limit = strtok(command+3, " ");
			while (limit != NULL) {
				if (!strncmp(limit, "wtime", 5)) {
					cmd.args.limit.time[0] = atoi(strtok(NULL, " "));
				} else if (!strncmp(limit, "winc", 4)) {
					cmd.args.limit.inc[0] = atoi(strtok(NULL, " "));
				} else if (!strncmp(limit, "btime", 5)) {
					cmd.args.limit.time[1] = atoi(strtok(NULL, " "));
				} else if (!strncmp(limit, "binc", 4)) {
					cmd.args.limit.inc[1] = atoi(strtok(NULL, " "));
				} else if (!strncmp(limit, "depth", 5)) {
					cmd.args.limit.depth = atoi(strtok(NULL, " "));
				}
				limit = strtok(NULL, " ");
			}
		}

	} else if (!strncmp(command, "position", 8)) {
		cmd.type = cmd_position;
		if (!strncmp(command, "position startpos", 17)) {
			cmd.args.board = init_chessboard(startfen);
		} else if (!strncmp(command, "position fen", 12)){
			cmd.args.board = init_chessboard(command + 12);
		}

		char* moves_start = strstr(command, "moves");
		if (moves_start) {
			char* move = strtok(moves_start+6, " ");
			while (move != NULL) {
				move_t m = string_to_move_flags(cmd.args.board, move);
				if (!play_move(cmd.args.board, m)) {
					printf("%s = %c %c -> %c %c (%x)\n",
							move,
							(m&0x7)+'a', ((m>>3)&0x7)+'1',
							((m>>6)&0x7)+'a', ((m>>9)&0x7)+'1', m>>12);
					print_chessboard(cmd.args.board);
					fflush(stdout);
				}
				move = strtok(NULL, " ");
			}
		}

	} else if (!strncmp(command, "stop", 4)) {
		cmd.type = cmd_stop;
	} else if (!strncmp(command, "quit", 4)) {
		cmd.type = cmd_quit;
	} else if (!strncmp(command, "uci", 3)) {
		if (!strncmp(command, "ucinewgame", 10)) {
			cmd.type = cmd_newgame;
		} else {
			cmd.type = cmd_uci;
		}
	} else if (!strncmp(command, "isready", 7)) {
		cmd.type = cmd_isready;
	}
	return cmd;
}



pthread_t search_thread;
pthread_t time_thread;
pthread_t input_thread;

time_management_t time_management;
pthread_rwlock_t timelock;
search_parameter_t* search = NULL;

void* time_loop(void* a) {
	while (1) {
		pthread_rwlock_rdlock(&search->locks[3]);
		unsigned loop = search->keep_running;
		pthread_rwlock_unlock(&search->locks[3]);
		if (!loop) return a;

		pthread_rwlock_rdlock(&timelock);
		time_management_t time = time_management;
		pthread_rwlock_unlock(&timelock);
		if (time.finished) continue;

		while (!time.finished && timeInMilliseconds() - time.start_time < time.total_time/20) {
			usleep(10);
			pthread_rwlock_rdlock(&timelock);
			time = time_management;
			pthread_rwlock_unlock(&timelock);
		}

		pthread_rwlock_wrlock(&search->locks[3]);
		search->stop = 1;
		pthread_rwlock_unlock(&search->locks[3]);

		pthread_rwlock_wrlock(&timelock);
		time_management.finished = 1;
		pthread_rwlock_unlock(&timelock);
	}
}

void* search_loop(void* a) {
	init_bitboard_between();
	init_attack_bitboard();
	generate_magics();
	init_piece_square_tables();
	init_hash_table();

	while (1) {
		pthread_rwlock_rdlock(&search->locks[3]);
		unsigned loop = search->keep_running;
		pthread_rwlock_unlock(&search->locks[3]);
		if (!loop) break;

		pthread_rwlock_rdlock(&search->locks[3]);
		unsigned stop = search->stop;
		pthread_rwlock_unlock(&search->locks[3]);
		if (stop) continue;

		pthread_rwlock_wrlock(&search->locks[1]);
		best_move_t bm = find_best_move(search);
		pthread_rwlock_unlock(&search->locks[1]);

		pthread_rwlock_wrlock(&search->locks[3]);
		search->stop = 1;
		pthread_rwlock_unlock(&search->locks[3]);

		char string[6] = {0};
		move_to_string(bm.move, string);
		printf("bestmove %s\n", string);
		fflush(stdout);
	}
	printf("search thread stopped\n");
	destroy_hash_table();
	return a;
}

void input_loop(void) {
	printf("Moravský Elektrický Šachista (meš), written by Ada (@vodam)\n");
	while (1) {
		char* command = NULL;
		size_t size = 0;
		ssize_t chars = getline(&command, &size, stdin);
		if (chars < 0) {
			free(command);
			exit(0);
		}
		uci_command_t cmd = parse_command(command);
		// rewrite as switch case?
		if (cmd.type == cmd_unknown) {
			printf("Unknown command [%.*s]\n", (int)chars - (command[chars-1]=='\n'), command);
		}
		free(command);

		if (cmd.type == cmd_uci) {
			printf("id name meš\n");
			printf("id author Ada (@vodam)\n");
			printf("uciok\n");
		}

		if (cmd.type == cmd_isready) {
			printf("readyok\n");
		}

		if (cmd.type == cmd_stop || cmd.type == cmd_quit) {
			pthread_rwlock_wrlock(&search->locks[3]);
			search->stop = 1;
			pthread_rwlock_unlock(&search->locks[3]);

			pthread_rwlock_wrlock(&timelock);
			time_management.finished = 1;
			pthread_rwlock_unlock(&timelock);
		}

		if (cmd.type == cmd_quit) {
			printf("quitting\n");
			pthread_rwlock_wrlock(&search->locks[2]);
			search->keep_running = 0;
			pthread_rwlock_unlock(&search->locks[2]);
			return;
		}

		if (cmd.type == cmd_position) {
			pthread_rwlock_wrlock(&search->locks[1]);
			if (search->chessboard) {
				free(search->chessboard);
				search->chessboard = NULL;
			}
			search->chessboard = cmd.args.board;
			pthread_rwlock_unlock(&search->locks[1]);
		}

		if (cmd.type == cmd_perft) {
			pthread_rwlock_wrlock(&search->locks[1]);
			if (search->chessboard == NULL) {
				search->chessboard = init_chessboard(startfen);
			}
			unsigned long long total = 0;
			printf("total nodes: %llu\n", perft(search->chessboard, cmd.args.depth, 1, &total));
			printf("total visited: %llu\n", total);
			pthread_rwlock_unlock(&search->locks[1]);
		}

		if (cmd.type == cmd_search) {
			printf("declare %llu\n", timeInMilliseconds());
			pthread_rwlock_wrlock(&search->locks[1]);
			if (search->chessboard == NULL)
				search->chessboard = init_chessboard(startfen);
			unsigned side = search->chessboard->side;
			pthread_rwlock_unlock(&search->locks[1]);

			pthread_rwlock_wrlock(&search->locks[3]);
			search->stop = 0;
			pthread_rwlock_unlock(&search->locks[3]);

			pthread_rwlock_wrlock(&timelock);
			time_management.finished = 0;
			time_management.start_time = timeInMilliseconds();
			time_management.total_time = cmd.args.limit.time[side];
			time_management.increment = cmd.args.limit.inc[side];
			pthread_rwlock_unlock(&timelock);

			pthread_rwlock_wrlock(&search->locks[0]);
			search->limit = cmd.args.limit;
			pthread_rwlock_unlock(&search->locks[0]);
		}

		if (cmd.type == cmd_newgame) {
			printf("clearing the hash table\n");
			destroy_hash_table();
			init_hash_table();
		}

		fflush(stdout);

		// sleep(1);		// give some time between commands for the search thread to go
						// (maybe not the best idea, uci is meant to be responsive)
						// also its probably not needed, the thread should wait
						// until input
	}
}

void uci_loop(void) {
	search = calloc(1, sizeof(search_parameter_t));
	search->stop = 1;
	search->keep_running = 1;
	search->multithreaded = 1;
	time_management.finished = 1;
	for (int i = 0; i < 4; i++) {
		if (pthread_rwlock_init(&search->locks[i], NULL)) {
			printf("Thread lock init failure\n");
			exit(1);
		}
	}
	if (pthread_rwlock_init(&timelock, NULL)) {
		printf("Thread lock init failure\n");
		exit(1);
	}

	int error;

	error = pthread_create(&search_thread, NULL, &search_loop, NULL);
	if (error) printf("%s\n", strerror(error));
	error = pthread_create(&time_thread, NULL, &time_loop, NULL);
	if (error) printf("%s\n", strerror(error));

	input_loop();

	pthread_join(time_thread, NULL);
	pthread_join(search_thread, NULL);

	for (int i = 0; i < 4; i++) {
		pthread_rwlock_destroy(&search->locks[i]);
	}
	pthread_rwlock_destroy(&timelock);
	free(search);
}
