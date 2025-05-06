import chess
import sys
fen = sys.argv[2] if len(sys.argv) > 2 else chess.STARTING_FEN

def perft(board: chess.Board, depth: int, do_print: bool = True) -> int:
    if depth == 1:
        return len(list(board.legal_moves))

    moves = 0
    for move in board.legal_moves:
        if do_print:
            print(move.uci(), end=": ", flush=True)
        board.push(move)
        n = perft(board, depth-1, False)
        board.pop()
        if do_print:
            print(n, flush=True)
        moves += n
    return moves

print(perft(chess.Board(fen), int(sys.argv[1]) if len(sys.argv) > 1 else 5))
