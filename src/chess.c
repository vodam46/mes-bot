#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "chess.h"
#include "magic.h"

char piece_chars[12] = "PNBRQKpnbrqk";
piece_t chars_piece[128] = {
	['P'] = wpawn,
	['N'] = wknight,
	['B'] = wbishop,
	['R'] = wrook,
	['Q'] = wqueen,
	['K'] = wking,
	['p'] = bpawn,
	['n'] = bknight,
	['b'] = bbishop,
	['r'] = brook,
	['q'] = bqueen,
	['k'] = bking,
};

bitboard_t pawn_attacks[64];
bitboard_t pawn_moves[64][2];
bitboard_t knight_attacks[64];
bitboard_t bishop_attacks[64];
bitboard_t rook_attacks[64];
bitboard_t queen_attacks[64];
bitboard_t king_attacks[64];

unsigned knight_offsets[8][2] = {
	{-2, -1},
	{-2, +1},
	{+2, -1},
	{+2, +1},
	{-1, -2},
	{-1, +2},
	{+1, -2},
	{+1, +2},
};
#define in_bounds(n, min, max) (min <= n && n < max)
void init_attack_bitboard(void) {
	for (int rank = 0; rank < 8; rank++) {
		for (int file = 0; file < 8; file++) {
			int i = rank*8+file;
			// pawn
			bitboard_t pawn_attack_bitboard = 0ull;
			if (file > 0 && rank < 7) pawn_attack_bitboard |= 1ull << (i+7);
			if (file < 7 && rank < 7) pawn_attack_bitboard |= 1ull << (i+9);
			pawn_attacks[i] = pawn_attack_bitboard;

			bitboard_t pawn_move_bitboard = 0ull;
			if (rank < 2) pawn_move_bitboard |= 1ull << (i+16);
			if (rank < 7) pawn_move_bitboard |= 1ull << (i+8);
			pawn_moves[i][0] = pawn_move_bitboard;
			pawn_moves[i][1] = 0ull;

			// knight
			bitboard_t knight_bitboard = 0ull;
			for (int i = 0; i < 8; i++) {
				int r = knight_offsets[i][0],
					f = knight_offsets[i][1];
				if (in_bounds(rank+r, 0, 8) && in_bounds(file+f, 0, 8)) {
					knight_bitboard |= 1ull << ((rank+r)*8 + file+f);
				}
			}
			knight_attacks[i] = knight_bitboard;


			// bishop
			bitboard_t bishop_bitboard = 0ull;
			for (int offset = 1; offset < 8; offset++) {
				if (in_bounds(rank+offset, 0, 8) && in_bounds(file+offset, 0, 8))
					bishop_bitboard |= 1ull << ((rank+offset)*8 + file+offset);
				if (in_bounds(rank+offset, 0, 8) && in_bounds(file-offset, 0, 8))
					bishop_bitboard |= 1ull << ((rank+offset)*8 + file-offset);
				if (in_bounds(rank-offset, 0, 8) && in_bounds(file+offset, 0, 8))
					bishop_bitboard |= 1ull << ((rank-offset)*8 + file+offset);
				if (in_bounds(rank-offset, 0, 8) && in_bounds(file-offset, 0, 8))
					bishop_bitboard |= 1ull << ((rank-offset)*8 + file-offset);
			}
			bishop_attacks[i] = bishop_bitboard;

			// rook
			bitboard_t rook_bitboard = 0ull;
			for (int offset = 1; offset < 8; offset++) {
				if (in_bounds(rank+offset, 0, 8))
					rook_bitboard |= 1ull << ((rank+offset)*8 + file);
				if (in_bounds(rank-offset, 0, 8))
					rook_bitboard |= 1ull << ((rank-offset)*8 + file);
				if (in_bounds(file+offset, 0, 8))
					rook_bitboard |= 1ull << (rank*8 + file+offset);
				if (in_bounds(file-offset, 0, 8))
					rook_bitboard |= 1ull << (rank*8 + file-offset);
			}
			rook_attacks[i] = rook_bitboard;

			// queen
			// do we even need this?
			queen_attacks[i] = bishop_bitboard | rook_bitboard;

			// king
			bitboard_t king_bitboard = 0ull;
			for (int r = -1; r <= 1; r++) {
				for (int f = -1; f <= 1; f++) {
					if ((r || f) && in_bounds(rank+r, 0, 8) && in_bounds(file+f, 0, 8)) {
						king_bitboard |= 1ull << ((rank+r)*8 + file+f);
					}
				}
			}
			king_attacks[i] = king_bitboard;
		}
	}
}
#undef in_bounds

// TODO: implement everything thats needed

void print_chessboard(chessboard_t* b) {
	printf("%s:\n", b->side ? "black" : "white");
	for (int rank = 7; rank >= 0; rank--) {
		for (int file = 0; file < 8; file++) {
			piece_t p = piece_at(b, (rank<<3) + file);
			printf("%c", p == empty ? '.' : piece_chars[p]);
		}
		printf("\n");
	}
}

chessboard_t* init_chessboard(char fenstring[]) {
	chessboard_t* b = calloc(1,sizeof(chessboard_t));
	for (int i = 0; i < 12; i++)
		b->pieces[i] = 0ULL;

	if (fenstring == NULL || !strcmp(fenstring, "")) return b;
	const char len = strlen(fenstring);

	char* fen = malloc(len*sizeof(char));
	strncpy(fen, fenstring, len > 100 ? 100 : len);

	// initialize the board
	char* board = strtok(fen, " ");
	int boardlen = strlen(board);
	int rank = 7;
	int file = 0;
	for (int i = 0; i < boardlen; i++) {
		char c = board[i];
		if (c == '/') {
			rank--;
			file = 0;
		} else if ('1' <= c && c <= '8') {
			file += c - '1' + 1;
		} else {
			piece_t p = chars_piece[(int)c];
			b->pieces[p] |= 1ull << (rank*8+file);
			file++;
		}
	}

	// active color
	char* active = strtok(NULL, " ");
	b->side = (active[0] == 'b');

	//castling availability
	char* castling = strtok(NULL, " ");
	char castle[4] = "KQkq";
	if (castling[0] != '-') {
		for (int c = 0, i = 0; c < 4; c++) {
			if (castling[i] == castle[c]) {
				b->castling_rights |= 1u << c;
				i++;
			}
		}
	}

	// en passant target square
	char* enpassant = strtok(NULL, " ");
	if (enpassant[0] == '-') b->en_passant_square = -1;
	else b->en_passant_square = string_to_square(enpassant);

	// halfmove clock
	char* halfmove = strtok(NULL, " ");
	for (unsigned long i = 0; i < strlen(halfmove); i++) {
		b->fiftymove *= 10;
		b->fiftymove += halfmove[i] - '0';
	}

	// fullmove clock
	char* fullmove = strtok(NULL, " ");
	b->fullmove = 0u;
	for (unsigned long i = 0; i < strlen(fullmove); i++) {
		b->fullmove *= 10;
		b->fullmove += fullmove[i] - '0';
	}

	free(fen);
	return b;
}

bitboard_t pieces_color(chessboard_t* b, int color) {
	bitboard_t r = 0ULL;
	for (int i = color*6; i < color*6+6; i++) {
		r |= b->pieces[i];
	}
	return r;
}

piece_t piece_at(chessboard_t* b, unsigned pos) {
	for (int i = 0; i < 12; i++) {
		if (bitboard_contains(b->pieces[i], pos)) {
			return i;
		}
	}

	return empty;
}

int piece_side(piece_t p) {
	if (p == empty) return -1;
	return p >= 6;
}

void square_to_string(unsigned char square, char string[2]) {
	string[0] = (square&7) + 'a';
	string[1] = (((square>>3)&7)) + '1';
}

unsigned char string_to_square(char square[2]) {
	return (square[0] - 'a') | ((square[1] - '1')<<3);
}

move_t string_to_move(char s[5]) {
	unsigned char from = string_to_square(s);
	unsigned char to   = string_to_square(s+2);
	unsigned char flags = 0;
	char p = s[4];
	if (p != 0) {
		flags |= 0x8;
		if (p == 'n') flags |= 0x0;
		if (p == 'b') flags |= 0x1;
		if (p == 'r') flags |= 0x2;
		if (p == 'q') flags |= 0x3;
	}
	return flags | (to<<6) | from;
}

void move_to_string(move_t m, char s[5]) {
	square_to_string(m, s);
	square_to_string(m>>6, s+2);
	unsigned char flags = m >> 12;
	if (flags & 0x8) {
		flags &= 0x3;
		if (flags == 0) s[4] = 'n';
		if (flags == 1) s[4] = 'b';
		if (flags == 2) s[4] = 'r';
		if (flags == 3) s[4] = 'q';
	}
}

void print_moves(moves_t m) {
	printf("%d\n", m.num_moves);
	for (int i = 0; i < m.num_moves; i++) {
		char string[6] = {0};
		move_to_string(m.moves[i], string);
		printf("%s %x\n", string, m.moves[i]>>12);
	}
}

void print_meta_move(meta_move_t* m) {
	if (m == NULL) return;
	print_meta_move(m->next);
	char string[6] = {0};
	move_to_string(m->move, string);
	printf("%s ", string);
	// printf("%s %x\n", string, m->move>>12);
}
void print_game_history(chessboard_t* b) {
	print_meta_move(b->moves);
	printf("\n");
}

// assumes move is legal
int play_move(chessboard_t* b, move_t m) {
	unsigned from = m&0x3f;
	piece_t pick_up = piece_at(b, from);
	piece_t put_down = pick_up;
	if (piece_side(pick_up) != b->side) return 0;

	unsigned to   = (m>>6)&0x3f;
	if (piece_side(piece_at(b, to)) == b->side) return 0;
	unsigned flags = (m>>12)&0xf;
	piece_t capture = empty;
	int oldfifty = b->fiftymove;

	if (flags&0x8) {
		if (pick_up != wpawn + 6*b->side) return 0;
		unsigned char promo = flags&0x3;
		if (promo == 0) put_down = wknight + 6*b->side;
		if (promo == 1) put_down = wbishop + 6*b->side;
		if (promo == 2) put_down = wrook   + 6*b->side;
		if (promo == 3) put_down = wqueen  + 6*b->side;
	}
	if (flags&0x4) {
		int capture_square = to;
		if (!(flags&0x8) && flags&1) {
			if (b->side) {
				capture_square += 8;
			} else {
				capture_square -= 8;
			}
		}
		capture = piece_at(b, capture_square);
		if (flags == 0x5 && capture != bpawn - 6*b->side) return 0;
		if (piece_side(capture) != !b->side) return 0;
		b->pieces[capture] &= ~(1ull << capture_square);
	}

	unsigned char oldcr = b->castling_rights;
	// int flip = b->side ? 56 : 0;
	if (flags == 0x3) { // queen castle = 0x3
						// move rook three spaces right
		// if (!bitboard_contains(b->pieces[wrook+6*b->side], flip)) return 0;
		b->pieces[wrook + 6*b->side] ^= b->side ? flip_horizontal(0x9ull) : 0x9ull;
		b->castling_rights &= 0x3 << (2 * !b->side);
	}
	if (flags == 0x2) { // king castle = 0x2
							   // move rook two spaces left
		// if (!bitboard_contains(b->pieces[wrook+6*b->side], 7^flip)) return 0;
		b->pieces[wrook + 6*b->side] ^= b->side ? flip_horizontal(0xa0ull) : 0xa0ull;
		b->castling_rights &= 0x3 << (2 * !b->side);
	}

	if (pick_up == wking + 6*b->side) {
		b->castling_rights &= ~(3<<(b->side*2));
	}

	if (pick_up == wrook + 6*b->side) {
		unsigned flip = 56*b->side;
		if (from == flip) {
			b->castling_rights &= ~(1<<(1+b->side*2));
		}
		if (from == (7 ^ flip)) {
			b->castling_rights &= ~(1<<b->side*2);
		}
	}
	if (capture == brook - 6*b->side) {
		unsigned flip = 56*!b->side;
		if (to == flip) {
			b->castling_rights &= ~(1<<(1+(!b->side)*2));
		}
		if (to == (7 ^ flip)) {
			b->castling_rights &= ~(1<<(!b->side)*2);
		}
	}
	if (pick_up == wpawn + 6*b->side || flags&0x4)
		b->fiftymove = 0;
	else
		b->fiftymove++;

	int oldep = b->en_passant_square;
	b->en_passant_square = -1;
	if (flags == 0x1) b->en_passant_square = (to+from) / 2;

	b->pieces[pick_up] &= ~(1ull << from);
	b->pieces[put_down] |= 1ull << to;
	b->side = !b->side;

	meta_move_t* mm = calloc(1, sizeof(meta_move_t));
	mm->fiftymove = oldfifty;
	mm->move = m;
	mm->piece = put_down;
	mm->capture = capture;
	mm->enpassant = oldep;
	mm->castling = oldcr;
	mm->next = b->moves;
	b->moves = mm;

	return 1;
}

/*
 * ***BE EXTREMELY CAREFUL WHEN ADDING STUF HERE***
 * you dont want another annoying mistake
 * promotions
 * enpassant
 * castling
 */
int undo_move(chessboard_t* b) {
	meta_move_t* mm = b->moves;
	if (mm == NULL) return 0;

	move_t m = mm->move;
	piece_t pick_up = mm->piece;
	piece_t put_down = pick_up;
	piece_t capture = mm->capture;

	unsigned from = (m>>6)&0x3f;
	unsigned to = m&0x3f;
	unsigned flags = (m>>12)&0xf;

	if (flags&0x8) put_down = bpawn - 6*b->side;
	else if (piece_at(b, from) != pick_up) return 0;
	if (flags&0x4) {
		int capture_square = from;
		if (!(flags&0x8) && flags&1) {
			if (capture != wpawn + 6*b->side) return 0;

			if (b->side) capture_square -= 8;
			else capture_square += 8;
		}
		if (piece_side(capture) != b->side) return 0;
		b->pieces[capture] |= 1ull << capture_square;
	}
	if (flags == 0x3) {
		b->pieces[brook - 6*b->side] ^= b->side ? 0x9ull : flip_horizontal(0x9ull);
	}
	if (flags == 0x2) {
		b->pieces[brook - 6*b->side] ^= b->side ? 0xa0ull : flip_horizontal(0xa0ull);
	}

	b->fiftymove = mm->fiftymove;
	b->castling_rights = mm->castling;
	b->en_passant_square = mm->enpassant;
	b->pieces[pick_up] &= ~(1ull << from);
	b->pieces[put_down] |= 1ull << to;
	b->side = !b->side;

	b->moves = mm->next;
	free(mm);
	return 1;
}



/*
 *
 * HERE STARTS MOVE GENERATION
 * abandon all hope, all ye who enter
 *
 */
void append_move(moves_t* moves, move_t move) {
	if (moves->num_moves >= moves->arr_len) {
		moves->arr_len *= 2;
		moves->moves = realloc(moves->moves, moves->arr_len * sizeof(move_t));
	}
	moves->moves[moves->num_moves++] = move;
}

void append_all_options(moves_t* moves, int from, bitboard_t options, unsigned
		char flags) {
	while (options) {
		int to = bitboard_lowest(options);
		options &= ~(1ull<<to);
		append_move(moves, (flags<<12) | (to<<6) | from);
	}
}

bitboard_t potential_moves(chessboard_t* b, bitboard_t blockers,
		int position, int is_attack) {
	piece_t p = piece_at(b, position);
	switch (p) {
		case wpawn:
			if (is_attack) return pawn_attacks[position];
			return pawn_moves[position][piece_at(b, position+8) != empty];
		case bpawn:
			if (is_attack) return flip_horizontal(pawn_attacks[position^56]);
			return flip_horizontal(
					pawn_moves[position^56][piece_at(b, position-8) != empty]
					);

		case wknight: case bknight:
			return knight_attacks[position];
		case wbishop: case bbishop:
			return get_bishop_attacks(position, blockers);
		case wrook: case brook:
			return get_rook_attacks(position, blockers);
		case wqueen: case bqueen:
			return get_bishop_attacks(position, blockers)
				| get_rook_attacks(position, blockers);
		case wking: case bking:
			return king_attacks[position];
		default:
			return 0ull;
	}
}

bitboard_t get_attacked_squares(chessboard_t* b) {
	bitboard_t kingless = (pieces_color(b, b->side) | pieces_color(b, !b->side))
		& ~b->pieces[wking + 6*b->side];

	bitboard_t attacked_squares = 0ull;
	bitboard_t enemy = pieces_color(b, !b->side);
	while (enemy) {
		int position = bitboard_lowest(enemy);
		enemy &= ~(1ull<<position);
		attacked_squares |= potential_moves(b, kingless, position, 1);
	}
	return attacked_squares;
}

moves_t generate_moves(chessboard_t* b) {
	moves_t moves = {.num_moves = 0, .arr_len = 40};
	moves.moves = malloc(moves.arr_len*sizeof(move_t));

	bitboard_t our   = pieces_color(b, b->side);
	bitboard_t other = pieces_color(b, !b->side);
	bitboard_t all = our | other;
	bitboard_t attacked_squares = get_attacked_squares(b);
	bitboard_t capture_mask = 0xffffffffffffffffull;
	bitboard_t push_mask = 0xffffffffffffffffull;
	bitboard_t enpassant = b->en_passant_square != -1 ? 1ull << b->en_passant_square : 0ull;
	bitboard_t enpassant_pawn = b->side ? enpassant << 8 : enpassant >> 8;

	int king = bitboard_lowest(b->pieces[wking + 6*b->side]);
	// TODO: fix this - shouldnt ever happen
	// enpassant pins, probably
	if (king == 64) {
		printf(".");
		return moves;
	}
	bitboard_t options = king_attacks[king] & ~(attacked_squares | our);
	bitboard_t attacks = options & other;
	bitboard_t movement = options & ~other;
	append_all_options(&moves, king, attacks, 0x4);
	append_all_options(&moves, king, movement, 0x0);

	bitboard_t checkers = 0ull;
	if (b->side) {
		checkers |= flip_horizontal(pawn_attacks[king^56]) & b->pieces[wpawn];
	} else {
		checkers |= pawn_attacks[king] & b->pieces[bpawn];
	}
	checkers |= knight_attacks[king] & b->pieces[bknight - 6*b->side];
	checkers |= get_rook_attacks(king, all) & (b->pieces[brook - 6*b->side] | b->pieces[bqueen - 6*b->side]);
	checkers |= get_bishop_attacks(king, all) & (b->pieces[bbishop - 6*b->side] | b->pieces[bqueen - 6*b->side]);
	int num_checkers = bitboard_count(checkers);
	if (num_checkers > 1) return moves;
	if (num_checkers == 1) {
		capture_mask = checkers;

		int position = bitboard_lowest(checkers);
		piece_t piece = piece_at(b, position);

		if (piece == bpawn - 6*b->side) {
			push_mask = 0x0ull;
			if (position == bitboard_lowest(enpassant_pawn)) {
				capture_mask |= enpassant;
			}
		}
		// TODO: write it more cleanly
		else if (piece == (bknight - 6*b->side) || piece == (bking - 6*b->side))
			push_mask = 0x0ull;
		else push_mask = bitboard_between(king, position);
	} else {
		unsigned char castling = b->castling_rights;
		castling >>= 2*b->side;
		castling &= 0x3;
		int flip = 56*b->side;
		// queenside
		if (castling&0x2) {
			if (!(bitboard_between(king, flip) & all)
				&& !(bitboard_between(king, 1^flip) & attacked_squares)) {
				append_move(&moves, (0x3<<12) | ((2^flip)<<6) | king);
			}
		}

		// kignside
		if (castling&0x1) {
			if (!(bitboard_between(king, 7^flip) & (all | attacked_squares))) {
				append_move(&moves, (0x2<<12) | ((6^flip)<<6) | king);
			}
		}
	}


	bitboard_t pinned_pieces = 0ull;

	bitboard_t diag = b->pieces[bbishop - 6*b->side] | b->pieces[bqueen - 6*b->side];
	bitboard_t orth = b->pieces[brook - 6*b->side] | b->pieces[bqueen - 6*b->side];

	bitboard_t potential_pinned = our
		& ~(b->pieces[wking + 6*b->side])
		& (get_bishop_attacks(king, all & ~enpassant_pawn) | get_rook_attacks(king, all & ~enpassant_pawn));

	while (potential_pinned) {
		int pos = bitboard_lowest(potential_pinned);
		potential_pinned &= ~(1ull<<pos);

		piece_t piece = piece_at(b, pos);
		bitboard_t diag_pin = get_bishop_attacks(king, all & ~(1ull<<pos)) & diag & ~checkers;
		bitboard_t orth_pin = get_rook_attacks(  king, all & ~(1ull<<pos)) & orth & ~checkers;
		if (diag_pin) {
			pinned_pieces |= 1ull << pos;
			bitboard_t mask = bitboard_between(king, bitboard_lowest(diag_pin))
				| diag_pin;

			if (piece == wknight + 6*b->side || piece == wrook + 6*b->side) {}
			else if (piece == wpawn + 6*b->side) {
				int is_promoting = b->side ? pos <= 15 : pos >= 48;
				if (is_promoting) {
					bitboard_t options = potential_moves(b, all, pos, 1)
						& other
						& (push_mask | capture_mask)
						& ~our
						& mask;
					// should only be one bit - cant be pinned by multiple
					// bishops at once to only one king
					int to = bitboard_lowest(options);
					move_t move = ((0x8 | 0x4)<<12) | (to<<6) | pos;
					for (int i = 0; i < 4; i++)
						append_move(&moves, move | (i<<12));
				} else {
					append_all_options(
							&moves, pos,
							potential_moves(b, all, pos, 1)
							& other
							& (push_mask | capture_mask)
							& ~our
							& mask,
							0x4
							);
					append_all_options(
							&moves, pos,
							potential_moves(b, all, pos, 1)
							& enpassant
							& (push_mask | capture_mask)
							& ~our
							& mask,
							0x5
							);
				}
			} else {
				append_all_options(&moves, pos,
						potential_moves(b, all, pos, 0)
						& other
						& (push_mask | capture_mask)
						& ~our
						& mask,
						0x4
						);
				append_all_options(&moves, pos,
						potential_moves(b, all, pos, 0)
						& ~other
						& (push_mask | capture_mask)
						& ~our
						& mask,
						0x0
						);
			}
		}
		if (orth_pin) {
			pinned_pieces |= 1ull << pos;
			bitboard_t mask = bitboard_between(king, bitboard_lowest(orth_pin))
				| orth_pin;
			if (piece == wknight + 6*b->side || piece == wbishop + 6*b->side) {}
			else if (piece == wpawn + 6*b->side) {
				bitboard_t options = potential_moves(b, all, pos, 0)
										& push_mask
										& ~all
										& mask;
				while (options) {
					int to = bitboard_lowest(options);
					options &= ~(1ull<<to);
					append_move(&moves, ((abs(pos - to) == 16)<<12) | (to<<6) | pos);
				}
			} else {
				append_all_options(&moves, pos,
						potential_moves(b, all, pos, 1)
						& other
						& (push_mask | capture_mask)
						& ~our
						& mask,
						0x4
						);
				append_all_options(&moves, pos,
						potential_moves(b, all, pos, 0)
						& ~other
						& (push_mask | capture_mask)
						& ~our
						& mask,
						0x0
						);
			}
		}
		if (!orth_pin && !diag_pin) {
			bitboard_t enpassant_pin = 0ull;
			if (piece == wpawn + 6*b->side) {
				enpassant_pin = get_rook_attacks(king, (all | enpassant) & ~(1ull<<pos | enpassant_pawn)) & orth & ~checkers;
			}

			if (enpassant_pin) {
				pinned_pieces |= 1ull << pos;
				// can do basically anything except enpassant
				bitboard_t captures = potential_moves(b, all, pos, 1)
					& other
					& (push_mask | capture_mask)
					& ~our;
				bitboard_t movement = potential_moves(b, all, pos, 0)
					& (push_mask | capture_mask)
					& ~all;
				append_all_options(&moves, pos, captures, 0x4);
				append_all_options(&moves, pos, movement, 0x0);
			}
		}
	}

	// TODO: do this more optimaly
	bitboard_t movers = our & ~(b->pieces[wking + 6*b->side] | pinned_pieces);
	while (movers) {
		int from = bitboard_lowest(movers);
		movers &= ~(1ull<<from);

		piece_t p = piece_at(b, from);
		bitboard_t captures;
		bitboard_t movement;
		if (p == wpawn + 6*b->side) {
			captures = potential_moves(b, all, from, 1)
				& other
				& (push_mask | capture_mask)
				& ~our;
			movement = potential_moves(b, all, from, 0)
				& (push_mask | capture_mask)
				& ~all;
			int is_promoting = b->side ? from <= 15 : from >= 48;
			if (is_promoting) {
				while (captures) {
					int to = bitboard_lowest(captures);
					captures &= ~(1ull<<to);
					move_t move = ((0x8 | 0x4)<<12) | (to<<6) | from;
					for (int i = 0; i < 4; i++)
						append_move(&moves, move | (i<<12));
				}
				while (movement) {
					int to = bitboard_lowest(movement);
					movement &= ~(1ull<<to);
					move_t move = (0x8<<12) | (to<<6) | from;
					for (int i = 0; i < 4; i++)
						append_move(&moves, move | (i<<12));
				}
			} else {
				append_all_options(&moves, from, potential_moves(b, all, from, 1)
						& enpassant & (push_mask | capture_mask) & ~our, 0x5);
				append_all_options(&moves, from, captures, 0x4);
				while (movement) {
					int to = bitboard_lowest(movement);
					movement &= ~(1ull<<to);
					append_move(&moves, ((abs(from-to)==16)<<12) | (to<<6) | from);
				}
			}

			// append_all_options(&moves, from, movement, 0);
		} else {
			bitboard_t options = potential_moves(b, all, from, 0)
				& (push_mask | (capture_mask & ~enpassant))
				& ~our;
			append_all_options(&moves, from, options & other, 0x4);
			append_all_options(&moves, from, options & ~other, 0);
		}
	}

	return moves;
}

unsigned long long perft(chessboard_t* b, int depth, int print, unsigned long long* total_visited) {
	++*total_visited;
	unsigned long long r = 0;
	if (depth == 0) return 1;
	moves_t moves = generate_moves(b);
	if (depth == 1) {
		*total_visited += (unsigned long long)moves.num_moves;
		free(moves.moves);
		return (unsigned long long)moves.num_moves;
	}


	for (int i = 0; i < moves.num_moves; i++) {
		if (print) {
			char string[6] = {0};
			move_to_string(moves.moves[i], string);
			if (print >= 2)
				print_chessboard(b);
			printf("%s: ", string);
			fflush(stdout);
		}

		play_move(b, moves.moves[i]);
		// if (!play_move(b, moves.moves[i]))
		// 	continue;
		unsigned long long n = perft(b, depth-1, 0, total_visited);
		undo_move(b);

		if (print) {
			printf("%lld\n", n);
			if (print >= 2) {
				print_chessboard(b);
				printf("\n\n");
			}
		}

		r += n;
	}
	free(moves.moves);
	return r;
}

// TODO: threefold repetition
game_result_t game_result(chessboard_t* b) {
	int is_draw = 1;
	for (int i = 0; i < 12; i++) {
		if (i != wking && i != bking && b->pieces[i]) {
			is_draw = 0;
			break;
		}
	}
	if (is_draw) return draw;

	// TODO: is there a way to do this without generating all the moves??
	moves_t moves = generate_moves(b);
	free(moves.moves);

	if (moves.num_moves == 0) {
		bitboard_t attacked_squares = get_attacked_squares(b);
		if (b->pieces[wking + 6*b->side] & attacked_squares)
			return checkmate;
		return draw;
	}
	if (b->fiftymove >= 50) return draw;
	return ongoing;
}
