#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "chess.h"
#include "hash.h"
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

void print_chessboard(chessboard_t* b) {
	printf("%s %u ", b->side ? "black" : "white", b->ply);
	char castling[4] = "KQkq";
	for (int i = 0; i < 4; i++)
		if ((b->castling_rights>>i)&1)
			printf("%c", castling[i]);
	if (b->en_passant_square != -1)
		printf("ep: %d", b->en_passant_square);
	printf("\n");
	for (int rank = 7; rank >= 0; rank--) {
		for (int file = 0; file < 8; file++) {
			piece_t p = piece_at(b, (rank<<3) + file);
			printf("%c", p == pempty ? '.' : piece_chars[p]);
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

	char* fen = calloc(len+1, sizeof(char));
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

	b->key = hash_key(b);

	// halfmove clock
	char* halfmove = strtok(NULL, " ");
	if (halfmove == NULL) {
		free(fen);
		return b;
	}
	for (unsigned long i = 0; i < strlen(halfmove); i++) {
		b->fiftymove *= 10;
		b->fiftymove += halfmove[i] - '0';
	}

	// fullmove clock
	char* fullmove = strtok(NULL, " ");
	b->ply = 0u;
	for (unsigned long i = 0; i < strlen(fullmove); i++) {
		b->ply *= 10;
		b->ply += fullmove[i] - '0';
	}

	free(fen);
	return b;
}

inline bitboard_t pieces_color(chessboard_t* b, int color) {
	bitboard_t r = 0ULL;
	for (int i = color*6; i < color*6+6; i++) {
		r |= b->pieces[i];
	}
	return r;
}

inline piece_t piece_at(chessboard_t* b, unsigned pos) {
	for (int i = 0; i < 12; i++) {
		if (bitboard_contains(b->pieces[i], pos)) {
			return i;
		}
	}

	return pempty;
}

inline int piece_side(piece_t p) {
	return p == pempty ? -1 : (p >= bpawn);
	// if (p == pempty) return -1;
	// return p >= bpawn;
}

void square_to_string(unsigned char square, char string[2]) {
	string[0] = (square&7) + 'a';
	string[1] = (((square>>3)&7)) + '1';
}

inline unsigned char string_to_square(char square[2]) {
	return (square[0] - 'a') | ((square[1] - '1')<<3);
}

inline move_t string_to_move(char s[5]) {
	unsigned char from = string_to_square(s);
	unsigned char to   = string_to_square(s+2);
	unsigned char flags = 0;
	char p = s[4];
	if (p != 0) {
		if (p == 'n') flags |= 0x8;
		if (p == 'b') flags |= 0x9;
		if (p == 'r') flags |= 0xa;
		if (p == 'q') flags |= 0xb;
	}
	return (flags<<12) | (to<<6) | from;
}

inline move_t string_to_move_flags(chessboard_t* b, char s[5]) {
	move_t move = string_to_move(s);
	unsigned from = move&0x3f;
	unsigned to = (move>>6)&0x3f;
	piece_t pick_up = piece_at(b, from);
	piece_t capture = piece_at(b, to);
	if (capture != pempty) {
		move |= 0x4<<12;
	}
	if (move>>12) {
		return move;
	}

	unsigned diff = from > to ? from - to : to - from;
	int enpassant = b->en_passant_square == -1
		? -1
		: b->en_passant_square + (b->side ? 8 : -8);
	if (pick_up == wpawn + 6*b->side) {
		if (enpassant != -1 && capture == pempty && to == (unsigned)enpassant)
			move |= 0x5<<12;
		if (diff == 16)
			move |= 0x1<<12;
	}
	if ((pick_up == wking + 6*b->side) && diff == 2) {
		move |= 0x2<<12;
		move |= ((to&0x7) == 2)<<12;
	}

	return move;
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

inline unsigned can_ep(chessboard_t* b, int side) {
	bitboard_t ep_mask = 1ull<<b->en_passant_square;
	if (side) ep_mask <<= 8;
	else ep_mask >>= 8;

	ep_mask = ((ep_mask & (~0x8080808080808080ull))<<1)
		| ((ep_mask & (~0x0101010101010101ull))>>1);

	return !!(b->pieces[wpawn + 6*side] & ep_mask);
}

// assumes move is legal
int play_move(chessboard_t* b, move_t m) {
	int from = m&0x3f;
	piece_t pick_up = piece_at(b, from);
	piece_t put_down = pick_up;
	if (piece_side(pick_up) != b->side) return 0;

	int to   = (m>>6)&0x3f;
	if (piece_side(piece_at(b, to)) == b->side) return 0;
	int flags = (m>>12)&0xf;
	piece_t capture = pempty;
	int oldfifty = b->fiftymove;
	uint64_t key_update = zobrist_update_piece(pick_up, from)
		^ zobrist_update_piece(put_down, to);

	if (flags&0x8) {
		if (pick_up != wpawn + 6*b->side) return 0;
		unsigned char promo = flags&0x3;
		if (promo > 3) return 0;

		key_update ^= zobrist_update_piece(put_down, to);
		put_down = wknight + 6*b->side + promo;
		key_update ^= zobrist_update_piece(put_down, to);
	}

	if (flags&0x4) {
		int capture_square = to;
		if (flags == 0x5) {
			if (b->side) {
				capture_square += 8;
			} else {
				capture_square -= 8;
			}
		}
		capture = piece_at(b, capture_square);
		if (flags == 0x5 && capture != bpawn - 6*b->side) return 0;
		if (piece_side(capture) != !b->side) return 0;
		key_update ^= zobrist_update_piece(capture, capture_square);
		b->pieces[capture] &= ~(1ull << capture_square);
	}

	unsigned char oldcr = b->castling_rights;

	int flip = b->side ? 56 : 0;
	if (flags == 0x3) { // queen castle = 0x3
						// move rook three spaces right
		if (!bitboard_contains(b->pieces[wrook+6*b->side], flip)) return 0;
		b->pieces[wrook + 6*b->side] ^= b->side ? flip_horizontal(0x9ull) : 0x9ull;
		key_update ^= zobrist_update_piece(wrook+6*b->side, flip);
		key_update ^= zobrist_update_piece(wrook+6*b->side, 3^flip);
	}
	if (flags == 0x2) { // king castle = 0x2
						// move rook two spaces left
		if (!bitboard_contains(b->pieces[wrook+6*b->side], 7^flip)) return 0;
		b->pieces[wrook + 6*b->side] ^= b->side ? flip_horizontal(0xa0ull) : 0xa0ull;
		key_update ^= zobrist_update_piece(wrook+6*b->side, 5^flip);
		key_update ^= zobrist_update_piece(wrook+6*b->side, 7^flip);
	}

	if (pick_up == wking + 6*b->side) {
		b->castling_rights &= ~(3<<(b->side*2));
	}

	if (pick_up == wrook + 6*b->side) {
		if (from == flip) {
			b->castling_rights &= ~(1<<(1+b->side*2));
		}
		if (from == (7 ^ flip)) {
			b->castling_rights &= ~(1<<b->side*2);
		}
	}
	if (capture == brook - 6*b->side) {
		int flip = 56*!b->side;
		if (to == flip) {
			b->castling_rights &= ~(1<<(1+(!b->side)*2));
		}
		if (to == (7 ^ flip)) {
			b->castling_rights &= ~(1<<(!b->side)*2);
		}
	}

	unsigned char cdiff = oldcr ^ b->castling_rights;
	for (int i = 0; i < 4; i++)
		if ((cdiff>>i)&1)
			key_update ^= polyglot_random_array[768+i];

	b->fiftymove = (pick_up == wpawn + 6*b->side || flags&0x4) ? 0 : b->fiftymove+1;

	int oldep = b->en_passant_square;
	if (oldep != -1) {
		key_update ^= polyglot_random_array[772 + oldep%8];
	}
	b->en_passant_square = -1;
	if (flags == 0x1) {
		b->en_passant_square = (from+to)/2;
		if (can_ep(b, !b->side)) {
			key_update ^= polyglot_random_array[772 + b->en_passant_square%8];
		} else {
			b->en_passant_square = -1;
		}
	}

	key_update ^= polyglot_random_array[780];

	b->pieces[pick_up] &= ~(1ull << from);
	b->pieces[put_down] |= 1ull << to;
	b->ply++;
	b->side = !b->side;
	b->key ^= key_update;

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
 * zobrist hash
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
	uint64_t key_update = zobrist_update_piece(pick_up, from)
		^ zobrist_update_piece(put_down, to);

	if (flags&0x8) {
		key_update ^= zobrist_update_piece(put_down, to);
		put_down = bpawn - 6*b->side;
		key_update ^= zobrist_update_piece(put_down, to);
	}
	else if (piece_at(b, from) != pick_up) return 0;
	int flip = b->side ? 0 : 56;
	if (flags&0x4) {
		int capture_square = from;
		if (!(flags&0x8) && flags&1) {
			if (capture != wpawn + 6*b->side) return 0;

			if (b->side) capture_square -= 8;
			else capture_square += 8;
		}
		if (piece_side(capture) != b->side) return 0;
		key_update ^= zobrist_update_piece(capture, capture_square);
		b->pieces[capture] |= 1ull << capture_square;
	}

	if (flags == 0x3) {
		b->pieces[brook - 6*b->side] ^= b->side ? 0x9ull : flip_horizontal(0x9ull);
		key_update ^= zobrist_update_piece(brook - 6*b->side, 0^flip);
		key_update ^= zobrist_update_piece(brook - 6*b->side, 3^flip);
	}
	if (flags == 0x2) {
		b->pieces[brook - 6*b->side] ^= b->side ? 0xa0ull : flip_horizontal(0xa0ull);
		key_update ^= zobrist_update_piece(brook - 6*b->side, 5^flip);
		key_update ^= zobrist_update_piece(brook - 6*b->side, 7^flip);
	}

	unsigned char cdiff = mm->castling ^ b->castling_rights;
	for (int i = 0; i < 4; i++)
		if ((cdiff>>i)&1)
			key_update ^= polyglot_random_array[768+i];

	key_update ^= polyglot_random_array[780];

	b->fiftymove = mm->fiftymove;
	b->castling_rights = mm->castling;
	if (b->en_passant_square != -1) {
		key_update ^= polyglot_random_array[772 + (b->en_passant_square%8)];
	}
	b->en_passant_square = mm->enpassant;
	if (b->en_passant_square != -1) {
		key_update ^= polyglot_random_array[772 + (b->en_passant_square%8)];
	}
	b->pieces[pick_up] &= ~(1ull << from);
	b->pieces[put_down] |= 1ull << to;
	b->ply--;
	b->side = !b->side;
	b->key ^= key_update;

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
inline void append_move(moves_t* moves, move_t move) {
	if (moves->num_moves >= moves->arr_len) {
		moves->arr_len *= 2;
		moves->moves = realloc(moves->moves, moves->arr_len * sizeof(move_t));
	}
	moves->moves[moves->num_moves++] = move;
}

inline void append_all_options(moves_t* moves, int from, bitboard_t options, unsigned
		char flags) {
	while (options) {
		append_move(moves, (flags<<12) | (bitboard_poplsb(&options)<<6) | from);
	}
}

bitboard_t potential_moves(chessboard_t* b, bitboard_t blockers,
		piece_t piece_type,
		int position, int is_attack) {
	switch (piece_type) {
		case wpawn:
			if (is_attack) return pawn_attacks[position];
			return pawn_moves[position][piece_at(b, position+8) != pempty];
		case bpawn:
			if (is_attack) return flip_horizontal(pawn_attacks[position^56]);
			return flip_horizontal(
					pawn_moves[position^56][piece_at(b, position-8) != pempty]
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

inline bitboard_t get_attacked_squares(chessboard_t* b) {
	bitboard_t kingless = (pieces_color(b, b->side) | pieces_color(b, !b->side))
		& ~b->pieces[wking + 6*b->side];

	bitboard_t attacked_squares = 0ull;
	for (int i = bpawn-6*b->side; i <= bking-6*b->side; i++) {
		bitboard_t enemy = b->pieces[i];
		while (enemy) {
			int position = bitboard_poplsb(&enemy);
			attacked_squares |= potential_moves(b, kingless, i, position, 1);
		}
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

	// TODO: fix this - shouldnt ever happen
	if (!b->pieces[wking + 6*b->side]) {
		print_chessboard(b);
		return moves;
	}
	int king = bitboard_lowest(b->pieces[wking + 6*b->side]);
	bitboard_t options = king_attacks[king] & ~(attacked_squares | our);
	append_all_options(&moves, king, options & other, 0x4);
	append_all_options(&moves, king, options & ~other, 0x0);

	bitboard_t checkers = 0ull;

	checkers |= b->side
		? flip_horizontal(pawn_attacks[king^56]) & b->pieces[wpawn]
		: pawn_attacks[king] & b->pieces[bpawn];

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
		else if (piece == (bknight - 6*b->side) || piece == (bking - 6*b->side))
			push_mask = 0x0ull;
		else push_mask = bitboard_between(king, position);
	} else {
		unsigned char castling = b->castling_rights;
		castling >>= 2*b->side;
		castling &= 0x3;
		int flip = 56*b->side;
		// queenside
		if (castling&0x2 && !(bitboard_between(king, flip) & all)
				&& !(bitboard_between(king, 1^flip) & attacked_squares)) {
			append_move(&moves, (0x3<<12) | ((2^flip)<<6) | king);
		}

		// kignside
		if (castling&0x1 && !(bitboard_between(king, 7^flip) & (all | attacked_squares))) {
			append_move(&moves, (0x2<<12) | ((6^flip)<<6) | king);
		}
	}


	bitboard_t pinned_pieces = 0ull;

	bitboard_t diag = b->pieces[bbishop - 6*b->side] | b->pieces[bqueen - 6*b->side];
	bitboard_t orth = b->pieces[brook - 6*b->side] | b->pieces[bqueen - 6*b->side];

	bitboard_t potential_pinned = our
		& (get_bishop_attacks(king, all) | get_rook_attacks(king, all & ~enpassant_pawn));

	while (potential_pinned) {
		int pos = bitboard_poplsb(&potential_pinned);
		piece_t piece = piece_at(b, pos);

		bitboard_t diag_pin = get_bishop_attacks(king, all & ~(1ull<<pos)) & diag & ~checkers;
		if (diag_pin) {
			pinned_pieces |= 1ull << pos;
			bitboard_t mask = bitboard_between(king, bitboard_lowest(diag_pin))
				| diag_pin;

			if (piece == wknight + 6*b->side || piece == wrook + 6*b->side) {}
			else if (piece == wpawn + 6*b->side) {
				unsigned is_promoting = b->side ? pos <= 15 : pos >= 48;
				bitboard_t all_captures = potential_moves(b, all, wpawn + 6*b->side, pos, 1)
					& (push_mask | capture_mask)
					& mask;
				// should only be one bit - cant be pinned by multiple
				// bishops at once to only one king
				bitboard_t captures = all_captures & other;
				bitboard_t enpascap = all_captures & enpassant;
				move_t move = (0x4<<12) | pos;
				if (captures) {
					int to = bitboard_lowest(captures);
					if (is_promoting) {
						move |= (0x8<<12) | (to<<6);
						for (int i = 0; i < 4; i++)
							append_move(&moves, (i<<12) | move);
					} else
						append_move(&moves, move | (to<<6));
				}
				if (enpascap)
					append_move(&moves, (0x5<<12) | (b->en_passant_square<<6) | pos);

			} else {
				append_all_options(&moves, pos,
						potential_moves(b, all, piece, pos, 0)
						& other
						& (push_mask | capture_mask)
						& mask,
						0x4
						);
				append_all_options(&moves, pos,
						potential_moves(b, all, piece, pos, 0)
						& ~all
						& (push_mask | capture_mask)
						& mask,
						0x0
						);
			}
			continue;
		}
		bitboard_t orth_pin = get_rook_attacks(king, all & ~(1ull<<pos)) & orth & ~checkers;
		 if (orth_pin) {
			pinned_pieces |= 1ull << pos;
			bitboard_t mask = bitboard_between(king, bitboard_lowest(orth_pin))
				| orth_pin;
			if (piece == wknight + 6*b->side || piece == wbishop + 6*b->side) {}
			else if (piece == wpawn + 6*b->side) {
				bitboard_t options = potential_moves(b, all, wpawn + 6*b->side, pos, 0)
										& push_mask
										& ~all
										& mask;
				while (options) {
					int to = bitboard_poplsb(&options);
					append_move(&moves, ((abs(pos - to) == 16)<<12) | (to<<6) | pos);
				}
			} else {
				append_all_options(&moves, pos,
						potential_moves(b, all, piece, pos, 1)
						& other
						& (push_mask | capture_mask)
						& mask,
						0x4
						);
				append_all_options(&moves, pos,
						potential_moves(b, all, piece, pos, 0)
						& ~all
						& (push_mask | capture_mask)
						& mask,
						0x0
						);
			}
			continue;
		 }
		 if (piece == wpawn + 6*b->side && get_rook_attacks(king, all ^ (1ull<<pos | enpassant_pawn | enpassant)) & orth & ~checkers) {
			pinned_pieces |= 1ull << pos;
			// can do basically anything except enpassant
			// special ep mask?
			append_all_options(&moves, pos,
					potential_moves(b, all, wpawn + 6*b->side, pos, 1)
						& other
						& (push_mask | capture_mask),
					0x4);
			append_all_options(&moves, pos,
					potential_moves(b, all, wpawn + 6*b->side, pos, 0)
						& (push_mask | capture_mask)
						& ~all,
					0x0);
		 }
	}

	// TODO: make this cleaner
	// also do this for knights too
	bitboard_t pawns = b->pieces[wpawn + 6*b->side] & ~pinned_pieces;
	if (pawns) {
		if (b->side) {
			bitboard_t advance = (pawns >> 8) & ~all;
			bitboard_t double_move = ((advance & 0xff0000000000ull) >> 8) & ~all & push_mask;
			advance &= push_mask;
			while (advance) {
				int to = bitboard_poplsb(&advance);
				if (to < 8)
					for (int i = 0; i < 4; i++)
						append_move(&moves, ((0x8|i)<<12) | (to<<6) | (to+8));
				else append_move(&moves, (to<<6) | (to+8));
			}
			while (double_move) {
				int to = bitboard_poplsb(&double_move);
				append_move(&moves, (0x1<<12) | (to<<6) | (to+16));
			}

			bitboard_t pos_left  = ((pawns & ~0x8080808080808080) >> 7) & capture_mask;
			bitboard_t pos_right = ((pawns & ~0x0101010101010101) >> 9) & capture_mask;
			bitboard_t left_capture = pos_left & other;
			bitboard_t right_capture = pos_right & other;
			while (left_capture) {
				int to = bitboard_poplsb(&left_capture);
				if (to < 8)
					for (int i = 0; i < 4; i++)
						append_move(&moves, ((0x8|0x4|i)<<12) | (to<<6) | (to+7));
				else append_move(&moves, (0x4<<12) | (to<<6) | (to+7));
			}
			while (right_capture) {
				int to = bitboard_poplsb(&right_capture);
				if (to < 8)
					for (int i = 0; i < 4; i++)
						append_move(&moves, ((0x8|0x4|i)<<12) | (to<<6) | (to+9));
				else append_move(&moves, (0x4<<12) | (to<<6) | (to+9));
			}
			if (pos_left & enpassant)
				append_move(&moves, (0x5<<12) | (b->en_passant_square<<6) | (b->en_passant_square+7));
			if (pos_right & enpassant)
				append_move(&moves, (0x5<<12) | (b->en_passant_square<<6) | (b->en_passant_square+9));
		} else {
			bitboard_t advance = (pawns << 8) & ~all;
			bitboard_t double_move = ((advance & 0xff0000ull) << 8) & ~all & push_mask;
			advance &= push_mask;
			while (advance) {
				int to = bitboard_poplsb(&advance);
				if (to > 55)
					for (int i = 0; i < 4; i++)
						append_move(&moves, ((0x8|i)<<12) | (to<<6) | (to-8));
				else append_move(&moves, (to<<6) | (to-8));
			}
			while (double_move) {
				int to = bitboard_poplsb(&double_move);
				append_move(&moves, (0x1<<12) | (to<<6) | (to-16));
			}

			bitboard_t pos_left  = ((pawns & ~0x0101010101010101) << 7) & capture_mask;
			bitboard_t pos_right = ((pawns & ~0x8080808080808080) << 9) & capture_mask;
			bitboard_t left_capture = pos_left & other;
			bitboard_t right_capture = pos_right & other;
			while (left_capture) {
				int to = bitboard_poplsb(&left_capture);
				if (to > 55)
					for (int i = 0; i < 4; i++)
						append_move(&moves, ((0x8|0x4|i)<<12) | (to<<6) | (to-7));
				else append_move(&moves, (0x4<<12) | (to<<6) | (to-7));
			}
			while (right_capture) {
				int to = bitboard_poplsb(&right_capture);
				if (to > 55)
					for (int i = 0; i < 4; i++)
						append_move(&moves, ((0x8|0x4|i)<<12) | (to<<6) | (to-9));
				else append_move(&moves, (0x4<<12) | (to<<6) | (to-9));
			}
			if (pos_left & enpassant)
				append_move(&moves, (0x5<<12) | (b->en_passant_square<<6) | (b->en_passant_square-7));
			if (pos_right & enpassant)
				append_move(&moves, (0x5<<12) | (b->en_passant_square<<6) | (b->en_passant_square-9));
		}
	}
	for (int p = wknight+6*b->side; p < wking+6*b->side; p++) {
		bitboard_t pieces = b->pieces[p] & ~pinned_pieces;
		while (pieces) {
			int from = bitboard_poplsb(&pieces);
			bitboard_t options = potential_moves(b, all, p, from, 0)
				& (push_mask | (capture_mask & ~enpassant));
			append_all_options(&moves, from, options & other, 0x4);
			append_all_options(&moves, from, options & ~all, 0);
		}
	}

	return moves;
}

unsigned long long perft(chessboard_t* b, int depth, int print, unsigned long long* total_visited) {
	if (total_visited)
		++*total_visited;
	if (depth == 0) return 1;
	if (b->key != hash_key(b)) {
		printf(".");
		b->key = hash_key(b);
	}

	unsigned long long r = 0;
	moves_t moves = generate_moves(b);
	if (depth == 1) {
		if (total_visited)
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

inline unsigned draw_material(chessboard_t* b) {
	for (int i = 0; i < 12; i++) {
		if (i != wking && i != bking && b->pieces[i]) {
			return 0;
		}
	}
	return 1;
}

// TODO: threefold repetition
inline game_result_t game_result(chessboard_t* b) {
	if (draw_material(b))
		return draw;

	// TODO: is there a way to do this without generating all the moves??
	moves_t moves = generate_moves(b);
	free(moves.moves);

	if (moves.num_moves == 0) {
		bitboard_t attacked_squares = get_attacked_squares(b);
		if (b->pieces[wking + 6*b->side] & attacked_squares)
			return checkmate;
		return draw;
	}
	if (b->fiftymove >= 100) return draw;
	return ongoing;
}
