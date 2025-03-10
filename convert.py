import chess
import chess.pgn

moves = "e2e4 d7d5 f1b5 c8d7 b5d3 d5e4 d3e4 d7c6 e4c6 b8c6 d1h5 g8f6 h5b5 a8b8 b1c3 a7a6 b5f5 c6d4 f5d3 e7e5 g1f3 d4f3 d3f3 f8b4 c3e2 e8g8 f3b3 b4d6 f2f3 e5e4 f3e4 f6e4 b3f3 f8e8 d2d3 e4g5 f3g4 h7h6 e1d1 d8f6 d1e1 f6e6 g4e6 e8e6 e1d1 b8e8 e2g3 d6g3 h2g3 e6e2 h1g1 e2e5 c1f4 e5c5 d1d2 g5e6 a1e1 g8f8 f4e3 c5b5 d2c1 c7c5 g1f1 e6d4 e3d4 c5d4 e1e8 f8e8 f1f4 b5g5 g3g4 h6h5 g4h5 g5g2 f4d4 g2h2 d4b4 b7b5 b4g4 e8f8 g4g5 f7f6 g5c5 h2h4 c5c6 h4h5 c6a6 f6f5 a6b6 f5f4 c1d2 h5f5 b6b8 f8f7 d2c3 f4f3 b8b7 f7f6 b7b6 f6g5 b6b7 g7g6 b7h7 f3f2 h7h1 f2f1q h1f1 f5f1 c3b4 f1f2 b4b3 g5f5 c2c4 b5c4 d3c4 f5e6 b3a3 e6d6 a3b3 d6c5 b3c3 g6g5 b2b4 c5c6 a2a3 g5g4 c3d3 g4g3 b4b5 c6c5 d3e3 f2f7 b5b6 c5b6 e3d2 g3g2 d2c2 g2g1q c2b2 g1e3 b2b1 e3e2"

# convert to pgn
board = chess.Board()

for move in moves.split():
	board.push_uci(move)

game = chess.pgn.Game.from_board(board)

print(game)
