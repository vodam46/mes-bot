// TODO: rewrite this, again
// use pthread_cond_t
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>

#include "uci.h"
#include "chess.h"
#include "engine.h"
#include "hash.h"
#include "globals.h"
#include "magic.h"

char* startfen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

uci_command_t parse_command(char* command) {
	uci_command_t cmd = {.type=cmd_unknown};
	if (!strncmp(command, "go", 2)) {
		if (!strncmp(command, "go perft", 8)) {
			cmd.type = cmd_perft;
			cmd.args.limit.depth = atoi(command+9);
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

	// debug commands
	} else if (!strncmp(command, "print", 5)) {
		cmd.type = cmd_print;
	} else if (!strncmp(command, "undo", 4)) {
		cmd.type = cmd_undo;
	}
	return cmd;
}

pthread_t time_thread,
		  search_thread;

unsigned quit = 0;
pthread_mutex_t qlock;

time_management_t timem = {.stop=1};
search_parameter_t searchm = {.stop=1};

unsigned getquit(void) {
	pthread_mutex_lock(&qlock);
	unsigned q = quit;
	pthread_mutex_unlock(&qlock);
	return q;
}

void uci_stop(void) {
	pthread_mutex_lock(&searchm.locks[2]);
	searchm.stop = 1;
	pthread_mutex_unlock(&searchm.locks[2]);
	pthread_mutex_lock(&timem.lock);
	timem.stop = 1;
	pthread_mutex_unlock(&timem.lock);
}

void uci_quit(void) {
	uci_stop();
	pthread_mutex_lock(&qlock);
	quit = 1;
	pthread_mutex_unlock(&qlock);

}

void uci_position(chessboard_t* b) {
	if (b == NULL)
		b = init_chessboard(startfen);
	pthread_mutex_lock(&searchm.locks[1]);
	if (searchm.chessboard)
		free(searchm.chessboard);
	searchm.chessboard = b;
	pthread_mutex_unlock(&searchm.locks[1]);
}

void uci_go(limit_t limit) {
	// printf("declare %llu\n", timeInMilliseconds());
	pthread_mutex_lock(&searchm.locks[0]);
	searchm.limit = limit;
	pthread_mutex_unlock(&searchm.locks[0]);

	pthread_mutex_lock(&searchm.locks[1]);
	if (searchm.chessboard == NULL)
		searchm.chessboard = init_chessboard(startfen);
	unsigned side = searchm.chessboard->side;
	pthread_mutex_unlock(&searchm.locks[1]);

	pthread_mutex_lock(&searchm.locks[2]);
	searchm.nodes_visited = 0ull;
	searchm.stop = 0;
	pthread_mutex_unlock(&searchm.locks[2]);

	if (limit.time[side] == UINT_MAX) return;
	pthread_mutex_lock(&timem.lock);
	timem.stop = 0;
	timem.start_time = timeInMilliseconds();
	timem.total_time = limit.time[side];
	timem.increment = limit.inc[side];
	pthread_mutex_unlock(&timem.lock);
}

void* time_loop(void* a) {
	while (!getquit()) {
		pthread_mutex_lock(&timem.lock);
		unsigned stop = timem.stop;
		pthread_mutex_unlock(&timem.lock);
		if (stop) continue;

		pthread_mutex_lock(&timem.lock);
		unsigned long long st = timem.start_time;
		unsigned et = timem.total_time;
		unsigned in = timem.increment == UINT_MAX ? 0 : timem.increment;
		pthread_mutex_unlock(&timem.lock);

		// printf("st: %llu\net: %u\nin: %u\n", st, et, in);

		if (et == UINT_MAX)
			continue;

		unsigned wait = et/20 + in/2;
		if (wait > et)
			wait = et-100;
		if (wait > et)
			wait = et/2;

		while (!getquit() && timeInMilliseconds() < st + wait) {
			usleep(1000);
		}
		uci_stop();
	}
	printf("time loop stopped\n");
	return a;
}

void* search_loop(void* a) {
	init_bitboard_between();
	init_attack_bitboard();
	generate_magics();
	init_piece_square_tables();
	init_hash_table();

	while (!getquit()) {
		pthread_mutex_lock(&searchm.locks[2]);
		unsigned stop = searchm.stop;
		pthread_mutex_unlock(&searchm.locks[2]);
		if (stop) continue;

		// printf("starting search\n");
		pthread_mutex_lock(&searchm.locks[1]);
		best_move_t bm = find_best_move(&searchm);
		pthread_mutex_unlock(&searchm.locks[1]);

		char string[6] = {0};
		move_to_string(bm.move, string);
		printf("bestmove %s\n", string);
		fflush(stdout);

		pthread_mutex_lock(&searchm.locks[2]);
		searchm.stop = 1;
		pthread_mutex_unlock(&searchm.locks[2]);
	}
	printf("search loop stopped\n");
	return a;
}

void input_loop(void) {
	printf("Moravský Elektrický Šachista (meš), written by Ada (@vodam)\n");
	while (1) {
		char* command = NULL;
		size_t size = 0;
		if (getline(&command, &size, stdin) < 0) {
			free(command);
			printf("bad command\n");
			uci_quit();
			return;
		}
		uci_command_t cmd = parse_command(command);
		switch(cmd.type) {
			case cmd_quit:
				uci_quit();
				printf("input loop stopped\n");
				return;

			case cmd_search:
				uci_go(cmd.args.limit);
				break;

			case cmd_position:
				uci_position(cmd.args.board);
				break;

			case cmd_stop:
				uci_stop();
				break;

			case cmd_perft:
				pthread_mutex_lock(&searchm.locks[1]);
				if (searchm.chessboard == NULL)
					searchm.chessboard = init_chessboard(startfen);
				printf("total nodes: %llu\n", perft(searchm.chessboard, cmd.args.limit.depth, 1, NULL));
				pthread_mutex_unlock(&searchm.locks[1]);
				break;

			case cmd_unknown:
				printf("unknown command %s\n", command);
				break;

			case cmd_uci:
				printf("id name meš\n");
				printf("id author Ada (@vodam)\n");
				printf("uciok\n");
				break;

			case cmd_isready:
				printf("readyok\n");
				break;

			case cmd_newgame:
				printf("clearing hash\n");
				destroy_hash_table();
				init_hash_table();
				break;

			case cmd_print:
				if (searchm.chessboard == NULL) {
					printf("(nil)\n");
					break;
				}
				print_chessboard(searchm.chessboard);
				printf("cur: %lx\nite: %lx\n", hash_key(searchm.chessboard),
						searchm.chessboard->key);
				break;

			case cmd_undo:
				undo_move(searchm.chessboard);
				print_chessboard(searchm.chessboard);
				break;
		}
		fflush(stdout);
		free(command);
	}
}

void uci(void) {
	for (int i = 0; i < 3; i++)
		if (pthread_mutex_init(&searchm.locks[i], NULL)) {
			printf("Thread lock init failure\n");
			exit(1);
		}
	if (pthread_mutex_init(&timem.lock, NULL)) {
		printf("Thread lock init failure\n");
		exit(1);
	}
	if (pthread_mutex_init(&qlock, NULL)) {
		printf("Thread lock init failure\n");
		exit(1);
	}

	int err = pthread_create(&search_thread, NULL, &search_loop, NULL);
	if (err) printf("%s\n", strerror(err));
	err = pthread_create(&time_thread, NULL, &time_loop, NULL);
	if (err) printf("%s\n", strerror(err));

	input_loop();

	pthread_join(time_thread, NULL);
	pthread_join(search_thread, NULL);

	for (int i = 0; i < 3; i++)
		pthread_mutex_destroy(&searchm.locks[i]);
	pthread_mutex_destroy(&qlock);
	pthread_mutex_destroy(&timem.lock);
}
