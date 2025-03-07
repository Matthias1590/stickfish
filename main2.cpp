#include "chess.hpp"
#include <chrono>
#include <unistd.h>
#include <string.h>
#include <poll.h>

/// Parameters

// #define TEST_POS "r1b2rk1/1pp2ppp/p1p2n2/4b3/3q4/2NN4/PPPPQPPP/R1B1K2R w KQ - 2 10"
// #define TEST_POS "r1b1r1k1/1pp2ppp/p1p2n2/4Q3/3q4/2NN4/PPPP1PPP/R1B1K2R w KQ - 1 11"
// #define TEST_POS "2r1kbr1/3npp1p/p7/1p1P4/4PB2/2NK2P1/PP5P/R6R b - - 2 21"
// TODO: Analyze, figure out if this improves anything
#define TT false
#define PROFILE_CUTOFF true

#define MIN_DEPTH 5
#define MAX_DEPTH 25
#define DYNAMIC_SEARCH_TIME_MS 3000

typedef double t_score;
#define SCORE_ZERO 0
#define SCORE_MIN -1000000000
#define SCORE_MAX 1000000000

chess::Board board;

std::vector<std::string> split(std::string str, char delim) {
    std::vector<std::string> parts;
    std::string part = "";
    for (char c : str) {
        if (c == delim) {
            parts.push_back(part);
            part = "";
        } else {
            part += c;
        }
    }
    parts.push_back(part);
    return parts;
}

bool isCheck(chess::Move &move, chess::Board &board) {
	board.makeMove(move);
	bool isCheck = board.inCheck();
	board.unmakeMove(move);
	return isCheck;
}

bool isCapture(chess::Move &move, chess::Board &board) {
	return board.at(move.to()) != chess::Piece::NONE;
}

t_score pieceValue(chess::PieceType type) {
	switch (type.internal()) {
		case chess::PieceType::PAWN: return 100;
		case chess::PieceType::KNIGHT: return 320;
		case chess::PieceType::BISHOP: return 330;
		case chess::PieceType::ROOK: return 560;
		case chess::PieceType::QUEEN: return 900;
		case chess::PieceType::KING: return 20000;
		default: return 0;
	}
}

bool isQuiescenceMove(chess::Move &move, chess::Board &board) {
	return isCheck(move, board) || isCapture(move, board);
}

enum GameStage {
	OPENING,
	MIDDLEGAME,
	ENDGAME,
	COUNT,
};

// TODO: Test & analyze this function
GameStage getGameStage(chess::Board &board) {
	int pieceCount = board.pieces(chess::PieceType::KNIGHT).count()
		+ board.pieces(chess::PieceType::BISHOP).count()
		+ board.pieces(chess::PieceType::ROOK).count()
		+ board.pieces(chess::PieceType::QUEEN).count();
	int pawnCount = board.pieces(chess::PieceType::PAWN).count();

	if (pieceCount > 8 && board.fullMoveNumber() < 10) {
		return GameStage::OPENING;
	} else if (pieceCount > 4 && pawnCount > 10) {
		return GameStage::MIDDLEGAME;
	} else {
		return GameStage::ENDGAME;
	}
}

// TODO: Analyze
t_score PIECE_SQUARE_TABLES[GameStage::COUNT][6][64] = {
	[GameStage::OPENING] = {
		[0] = {
			 0,  0,  0,  0,  0,  0,  0,  0,
			50, 50, 50, 50, 50, 50, 50, 50,
			10, 10, 20, 30, 30, 20, 10, 10,
			 5,  5, 10, 25, 25, 10,  5,  5,
			 0,  0,  0, 20, 20,  0,  0,  0,
			 5, -5,-10,  0,  0,-10, -5,  5,
			 5, 10, 10,-20,-20, 10, 10,  5,
			 0,  0,  0,  0,  0,  0,  0,  0,
		},
		[1] = {
			-50,-40,-30,-30,-30,-30,-40,-50,
			-40,-20,  0,  0,  0,  0,-20,-40,
			-30,  0, 10, 15, 15, 10,  0,-30,
			-30,  5, 15, 20, 20, 15,  5,-30,
			-30,  0, 15, 20, 20, 15,  0,-30,
			-30,  5, 10, 15, 15, 10,  5,-30,
			-40,-20,  0,  5,  5,  0,-20,-40,
			-50,-40,-30,-30,-30,-30,-40,-50,
		},
		[2] = {
			-20,-10,-10,-10,-10,-10,-10,-20,
			-10,  0,  0,  0,  0,  0,  0,-10,
			-10,  0,  5, 10, 10,  5,  0,-10,
			-10,  5,  5, 10, 10,  5,  5,-10,
			-10,  0, 10, 10, 10, 10,  0,-10,
			-10, 10, 10, 10, 10, 10, 10,-10,
			-10,  5,  0,  0,  0,  0,  5,-10,
			-20,-10,-10,-10,-10,-10,-10,-20,
		},
		[3] = {
			 0,  0,  0,  0,  0,  0,  0,  0,
			 5, 10, 10, 10, 10, 10, 10,  5,
			-5,  0,  0,  0,  0,  0,  0, -5,
			-5,  0,  0,  0,  0,  0,  0, -5,
			-5,  0,  0,  0,  0,  0,  0, -5,
			-5,  0,  0,  0,  0,  0,  0, -5,
			-5,  0,  0,  0,  0,  0,  0, -5,
			 0,  0,  0,  5,  5,  0,  0,  0,
		},
		[4] = {
			-20,-10,-10, -5, -5,-10,-10,-20,
			-10,  0,  0,  0,  0,  0,  0,-10,
			-10,  0,  5,  5,  5,  5,  0,-10,
			 -5,  0,  5,  5,  5,  5,  0, -5,
			  0,  0,  5,  5,  5,  5,  0, -5,
			-10,  5,  5,  5,  5,  5,  0,-10,
			-10,  0,  5,  0,  0,  0,  0,-10,
			-20,-10,-10, -5, -5,-10,-10,-20,
		},
		[5] = {
			-30,-40,-40,-50,-50,-40,-40,-30,
			-30,-40,-40,-50,-50,-40,-40,-30,
			-30,-40,-40,-50,-50,-40,-40,-30,
			-30,-40,-40,-50,-50,-40,-40,-30,
			-20,-30,-30,-40,-40,-30,-30,-20,
			-10,-20,-20,-20,-20,-20,-20,-10,
			 20, 20,  0,  0,  0,  0, 20, 20,
			 20, 30, 10,  0,  0, 10, 30, 20,
		},
	},
	[GameStage::MIDDLEGAME] = {
		[0] = {
			 0,  0,  0,  0,  0,  0,  0,  0,
			50, 50, 50, 50, 50, 50, 50, 50,
			10, 10, 20, 30, 30, 20, 10, 10,
			 5,  5, 10, 25, 25, 10,  5,  5,
			 0,  0,  0, 20, 20,  0,  0,  0,
			 5, -5,-10,  0,  0,-10, -5,  5,
			 5, 10, 10,-20,-20, 10, 10,  5,
			 0,  0,  0,  0,  0,  0,  0,  0,
		},
		[1] = {
			-50,-40,-30,-30,-30,-30,-40,-50,
			-40,-20,  0,  0,  0,  0,-20,-40,
			-30,  0, 10, 15, 15, 10,  0,-30,
			-30,  5, 15, 20, 20, 15,  5,-30,
			-30,  0, 15, 20, 20, 15,  0,-30,
			-30,  5, 10, 15, 15, 10,  5,-30,
			-40,-20,  0,  5,  5,  0,-20,-40,
			-50,-40,-30,-30,-30,-30,-40,-50,
		},
		[2] = {
			-20,-10,-10,-10,-10,-10,-10,-20,
			-10,  0,  0,  0,  0,  0,  0,-10,
			-10,  0,  5, 10, 10,  5,  0,-10,
			-10,  5,  5, 10, 10,  5,  5,-10,
			-10,  0, 10, 10, 10, 10,  0,-10,
			-10, 10, 10, 10, 10, 10, 10,-10,
			-10,  5,  0,  0,  0,  0,  5,-10,
			-20,-10,-10,-10,-10,-10,-10,-20,
		},
		[3] = {
			 0,  0,  0,  0,  0,  0,  0,  0,
			 5, 10, 10, 10, 10, 10, 10,  5,
			-5,  0,  0,  0,  0,  0,  0, -5,
			-5,  0,  0,  0,  0,  0,  0, -5,
			-5,  0,  0,  0,  0,  0,  0, -5,
			-5,  0,  0,  0,  0,  0,  0, -5,
			-5,  0,  0,  0,  0,  0,  0, -5,
			 0,  0,  0,  5,  5,  0,  0,  0,
		},
		[4] = {
			-20,-10,-10, -5, -5,-10,-10,-20,
			-10,  0,  0,  0,  0,  0,  0,-10,
			-10,  0,  5,  5,  5,  5,  0,-10,
			 -5,  0,  5,  5,  5,  5,  0, -5,
			  0,  0,  5,  5,  5,  5,  0, -5,
			-10,  5,  5,  5,  5,  5,  0,-10,
			-10,  0,  5,  0,  0,  0,  0,-10,
			-20,-10,-10, -5, -5,-10,-10,-20,
		},
		[5] = {
			-30,-40,-40,-50,-50,-40,-40,-30,
			-30,-40,-40,-50,-50,-40,-40,-30,
			-30,-40,-40,-50,-50,-40,-40,-30,
			-30,-40,-40,-50,-50,-40,-40,-30,
			-20,-30,-30,-40,-40,-30,-30,-20,
			-10,-20,-20,-20,-20,-20,-20,-10,
			 20, 20,  0,  0,  0,  0, 20, 20,
			 20, 30, 10,  0,  0, 10, 30, 20,
		},
	},
	[GameStage::ENDGAME] = {
		[0] = {
			 0,  0,  0,  0,  0,  0,  0,  0,
			50, 50, 50, 50, 50, 50, 50, 50,
			10, 10, 20, 30, 30, 20, 10, 10,
			 5,  5, 10, 25, 25, 10,  5,  5,
			 0,  0,  0, 20, 20,  0,  0,  0,
			 5, -5,-10,  0,  0,-10, -5,  5,
			 5, 10, 10,-20,-20, 10, 10,  5,
			 0,  0,  0,  0,  0,  0,  0,  0,
		},
		[1] = {
			-50,-40,-30,-30,-30,-30,-40,-50,
			-40,-20,  0,  0,  0,  0,-20,-40,
			-30,  0, 10, 15, 15, 10,  0,-30,
			-30,  5, 15, 20, 20, 15,  5,-30,
			-30,  0, 15, 20, 20, 15,  0,-30,
			-30,  5, 10, 15, 15, 10,  5,-30,
			-40,-20,  0,  5,  5,  0,-20,-40,
			-50,-40,-30,-30,-30,-30,-40,-50,
		},
		[2] = {
			-20,-10,-10,-10,-10,-10,-10,-20,
			-10,  0,  0,  0,  0,  0,  0,-10,
			-10,  0,  5, 10, 10,  5,  0,-10,
			-10,  5,  5, 10, 10,  5,  5,-10,
			-10,  0, 10, 10, 10, 10,  0,-10,
			-10, 10, 10, 10, 10, 10, 10,-10,
			-10,  5,  0,  0,  0,  0,  5,-10,
			-20,-10,-10,-10,-10,-10,-10,-20,
		},
		[3] = {
			 0,  0,  0,  0,  0,  0,  0,  0,
			 5, 10, 10, 10, 10, 10, 10,  5,
			-5,  0,  0,  0,  0,  0,  0, -5,
			-5,  0,  0,  0,  0,  0,  0, -5,
			-5,  0,  0,  0,  0,  0,  0, -5,
			-5,  0,  0,  0,  0,  0,  0, -5,
			-5,  0,  0,  0,  0,  0,  0, -5,
			 0,  0,  0,  5,  5,  0,  0,  0,
		},
		[4] = {
			-20,-10,-10, -5, -5,-10,-10,-20,
			-10,  0,  0,  0,  0,  0,  0,-10,
			-10,  0,  5,  5,  5,  5,  0,-10,
			 -5,  0,  5,  5,  5,  5,  0, -5,
			  0,  0,  5,  5,  5,  5,  0, -5,
			-10,  5,  5,  5,  5,  5,  0,-10,
			-10,  0,  5,  0,  0,  0,  0,-10,
			-20,-10,-10, -5, -5,-10,-10,-20,
		},
		[5] = {
			-50,-40,-30,-30,-30,-30,-40,-50,
			-40,-20,  0,  5,  5,  0,-20,-40,
			-30,  0, 10, 15, 15, 10,  0,-30,
			-30,  5, 15, 25, 25, 15,  5,-30,
			-30,  0, 10, 15, 15, 10,  0,-30,
			-40,-20,  0,  5,  5,  0,-20,-40,
			-50,-40,-30,-30,-30,-30,-40,-50,
			-60,-50,-40,-40,-40,-40,-50,-60,
		},
	}
};

t_score evaluatePieceSquares(chess::Board &board) {
	t_score score = 0;
	auto stage = getGameStage(board);

	for (int i = 0; i < 64; i++) {
		auto piece = board.at(chess::Square(i));
		if (piece == chess::Piece::NONE) {
			continue;
		}

		auto type = piece.type();
		auto color = piece.color();
		int j = color == chess::Color::WHITE ? i : 63 - i;
		score += PIECE_SQUARE_TABLES[stage][type][j] * (color == chess::Color::WHITE ? 1 : -1);
	}

	return score;
}

int16_t scoreMove(chess::Move &move, chess::Board &board) {
	int16_t score = 0;
	auto pieceFrom = board.at(move.from());
	auto pieceTo = board.at(move.to());

	// Bonus for checks
	if (isCheck(move, board)) {
		score += 1000;
	}

	// Bonus for captures
	if (isCapture(move, board)) {
		score += pieceValue(pieceTo.type()) - pieceValue(pieceFrom.type()) / 2;
	}

	// Bonus for pushing pawns in the first few moves
	if (board.fullMoveNumber() < 6 && pieceFrom.type() == chess::PieceType::PAWN) {
		score += 10;
	}

	// Bonus for castling
	if (pieceFrom.type() == chess::PieceType::KING && move.typeOf() == chess::Move::CASTLING) {
		score += 1000;
	}

	// Bonus for improving piece square table
	if (pieceFrom.type() != chess::PieceType::KING) {
		auto currentScore = PIECE_SQUARE_TABLES[getGameStage(board)][(int) pieceFrom.type().internal()][move.from().index()];
		auto newScore = PIECE_SQUARE_TABLES[getGameStage(board)][(int) pieceFrom.type().internal()][move.to().index()];
		score += newScore - currentScore;
	}

	// Small penalty for rook moves because it keeps making small moves
	if (pieceFrom.type() == chess::PieceType::ROOK) {
		score -= 5;
	}

	return score;
}

void sortMoves(chess::Movelist &moves, chess::Board &board) {
	for (auto &move : moves) {
		move.setScore(scoreMove(move, board));
	}

	std::sort(moves.begin(), moves.end(), [&board](chess::Move &a, chess::Move &b) {
		return a.score() > b.score();
	});
}

chess::Movelist generateMoves(chess::Board &board) {
	chess::Movelist moveList;
	chess::movegen::legalmoves(moveList, board);
	sortMoves(moveList, board);
	return moveList;
}

// TODO: Analyze, figure out if we should add king score to this
t_score evaluateMaterial(chess::Board &board) {
	t_score whiteScore = 0;
	t_score blackScore = 0;

	whiteScore += board.pieces(chess::PieceType::PAWN, chess::Color::WHITE).count() * pieceValue(chess::PieceType::PAWN);
	blackScore += board.pieces(chess::PieceType::PAWN, chess::Color::BLACK).count() * pieceValue(chess::PieceType::PAWN);
	
	whiteScore += board.pieces(chess::PieceType::KNIGHT, chess::Color::WHITE).count() * pieceValue(chess::PieceType::KNIGHT);
	blackScore += board.pieces(chess::PieceType::KNIGHT, chess::Color::BLACK).count() * pieceValue(chess::PieceType::KNIGHT);

	whiteScore += board.pieces(chess::PieceType::BISHOP, chess::Color::WHITE).count() * pieceValue(chess::PieceType::BISHOP);
	blackScore += board.pieces(chess::PieceType::BISHOP, chess::Color::BLACK).count() * pieceValue(chess::PieceType::BISHOP);

	whiteScore += board.pieces(chess::PieceType::ROOK, chess::Color::WHITE).count() * pieceValue(chess::PieceType::ROOK);
	blackScore += board.pieces(chess::PieceType::ROOK, chess::Color::BLACK).count() * pieceValue(chess::PieceType::ROOK);

	whiteScore += board.pieces(chess::PieceType::QUEEN, chess::Color::WHITE).count() * pieceValue(chess::PieceType::QUEEN);
	blackScore += board.pieces(chess::PieceType::QUEEN, chess::Color::BLACK).count() * pieceValue(chess::PieceType::QUEEN);

	// TODO: Analyze this
	// Give a bonus for trading when up and a penalty for trading when down
	t_score whiteBonus = (whiteScore - blackScore) / 100;
	t_score blackBonus = (blackScore - whiteScore) / 100;

	whiteScore += whiteBonus;
	blackScore += blackBonus;

	return whiteScore - blackScore;
}

t_score evaluateMobility(chess::Board &board) {
	t_score score = 0;

	chess::Movelist moves;
	chess::movegen::legalmoves(moves, board);

	score += moves.size() * 10;

	return score;
}

t_score evaluate(chess::Board &board) {
	t_score score = 0;

	score += evaluateMaterial(board);
	score += evaluatePieceSquares(board);
	score += evaluateMobility(board);

	return score * (board.sideToMove() == chess::Color::WHITE ? 1 : -1);
}

enum TTFlag {
	EXACT,
	LOWERBOUND,
	UPPERBOUND,
};

struct TTEntry {
	uint64_t key;
	t_score score;
	int depth;
	TTFlag flag;
	chess::Move bestMove;
};

std::unordered_map<uint64_t, TTEntry> transpositionTable;

bool probeTT(uint64_t key, int depth, t_score alpha, t_score beta, t_score &score, chess::Move &move) {
	auto it = transpositionTable.find(key);
	if (it == transpositionTable.end()) {
		return false;
	}

	auto &entry = it->second;
	if (entry.depth < depth) {
		return false;
	}

	if (entry.flag == EXACT) {
		score = entry.score;
		move = entry.bestMove;
		return true;
	}
	if (entry.flag == LOWERBOUND && entry.score >= beta) {
		score = entry.score;
		move = entry.bestMove;
		return true;
	}
	if (entry.flag == UPPERBOUND && entry.score <= alpha) {
		score = entry.score;
		move = entry.bestMove;
		return true;
	}

	return false;
}

void storeTT(uint64_t key, int depth, t_score score, TTFlag flag, chess::Move bestMove) {
	auto it = transpositionTable.find(key);
	if (it == transpositionTable.end() || it->second.depth <= depth) {
		transpositionTable[key] = { key, score, depth, flag, bestMove };
	}
}

typedef enum {
	WAITING,
	THINKING,
	PONDERING,
	CANCELLING,
} t_state;

t_state state = WAITING;
chess::Move bestLine[MAX_DEPTH];
size_t bestLineLength = 0;

// index is the current move (hashed), for every move we can make, we store the best move for the opponent
chess::Move nextMoveMap[4096];

chess::Move expectedMove;

void cancelSearch(void) {
	std::cout << "info string cancelling search" << std::endl;
	expectedMove = nextMoveMap[bestLine[0].from().index() * 64 + bestLine[0].to().index()];
	std::cout << "info string best line: " << bestLine[0] << " " << expectedMove << std::endl;
	std::cout << "bestmove " << chess::uci::moveToUci(bestLine[0]) << std::endl;
	state = CANCELLING;
}

bool commandAvailable() {
	struct pollfd fds;
	fds.fd = 0;
	fds.events = POLLIN;
	fds.revents = 0;

	return poll(&fds, 1, 0) > 0;
}

std::string readCommand() {
	std::string command;
	std::getline(std::cin, command);
	return command;
}

t_score search(chess::Board &board);

void startPondering() {
	assert(state == THINKING);

	std::cout << "info string pondering " << expectedMove << std::endl;
	state = PONDERING;
	board.makeMove(expectedMove);
	auto score = search(board);
}

void startSearch() {
	assert(state != THINKING);

	std::cout << "info string starting search" << std::endl;
	state = THINKING;
	auto score = search(board);
	expectedMove = nextMoveMap[bestLine[0].from().index() * 64 + bestLine[0].to().index()];
	std::cout << "info string best line: " << bestLine[0] << " " << expectedMove << std::endl;
	std::cout << "bestmove " << chess::uci::moveToUci(bestLine[0]) << std::endl;
	board.makeMove(bestLine[0]);
	startPondering();
}

chess::Board rollbackBoard;

void updateState() {
	/*
	std::string input;
	std::getline(std::cin, input);

	if (input == "uci") {
		std::cout << "uciok" << std::endl;
	} else if (input == "isready") {
		std::cout << "readyok" << std::endl;
	} else if (input.starts_with("position startpos moves")) {
		std::string moves = input.substr(24);
		board = chess::Board::fromFen(std::string_view("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR"));
		std::vector<std::string> moveList = split(moves, ' ');
		for (std::string moveStr : moveList) {
			auto move = chess::uci::uciToMove(board, moveStr);
			board.makeMove(move);
		}
	} else if (input.starts_with("go ")) {
		chess::Move move;
		auto score = search(board, &move);
		std::cout << "bestmove " << chess::uci::moveToUci(move) << std::endl;

		dumpUciInfo(board, score);
	} else if (input == "quit") {
		return 0;
	} else {
		std::cout << "info string unknown command '" << input << "'" << std::endl;
	}
	*/

	switch (state) {
	case WAITING: {
		if (!commandAvailable()) {
			return;
		}

		auto command = readCommand();
		if (command == "quit") {
			exit(0);
		} else if (command.starts_with("position startpos moves")) {
			std::string moves = command.substr(24);
			board = chess::Board::fromFen(std::string_view("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR"));
			std::vector<std::string> moveList = split(moves, ' ');
			for (std::string moveStr : moveList) {
				auto move = chess::uci::uciToMove(board, moveStr);
				board.makeMove(move);
			}
		} else if (command.starts_with("go ")) {
			startSearch();
		} else {
			std::cout << "info string unknown command '" << command << "'" << std::endl;
		}
	} break;
	case PONDERING: {
		if (!commandAvailable()) {
			return;
		}

		// if we get a new position, if the move is the expected move, we can continue pondering, otherwise we need to cancel it

		auto command = readCommand();
		if (command == "quit") {
			exit(0);
		} else if (command.starts_with("position startpos moves")) {
			std::string moves = command.substr(24);
			auto localBoard = chess::Board::fromFen(std::string_view("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR"));
			std::vector<std::string> moveList = split(moves, ' ');
			for (std::string moveStr : moveList) {
				auto move = chess::uci::uciToMove(localBoard, moveStr);
				localBoard.makeMove(move);
			}
			if (moveList.size() > 0) {
				std::cout << "info string actually played move: " << moveList.back() << " expected move: " << expectedMove << std::endl;
				if (moveList.back() == chess::uci::moveToUci(expectedMove)) {
					state = THINKING;
				} else {
					cancelSearch();
					rollbackBoard = localBoard;
				}
			}
		} else if (command.starts_with("go ")) {
			startSearch();
		} else {
			std::cout << "info string unknown command '" << command << "'" << std::endl;
		}
	} break;
	case THINKING: {
		if (!commandAvailable()) {
			return;
		}

		auto command = readCommand();
		if (command == "quit") {
			exit(0);
		} else if (command == "stop") {
			cancelSearch();
		} else {
			std::cout << "info string unknown command '" << command << "'" << std::endl;
		}
	} break;
	case CANCELLING: {
		board = rollbackBoard;
		state = WAITING;
	} break;
	}
}

bool shouldCancel(void) {
	updateState();
	return state == CANCELLING;
}

#if PROFILE_CUTOFF
int totalBetaCutoffs = 0;
int firstMoveBetaCutoffs = 0;
#endif

t_score quiescence(chess::Board &board, t_score alpha, t_score beta, chess::Move prevMove) {
	if (shouldCancel()) {
		board.unmakeMove(prevMove);
		return 0;
	}

	t_score standpat = evaluate(board);
	if (standpat >= beta) {
		return beta;
	}
	if (alpha < standpat) {
		alpha = standpat;
	}

	chess::Movelist moves = generateMoves(board);
	for (auto &move : moves) {
		if (!isQuiescenceMove(move, board)) {
			continue;
		}

		board.makeMove(move);
		t_score score = -quiescence(board, -beta, -alpha, move);
		board.unmakeMove(move);

		if (score >= beta) {
			return beta;
		}
		if (score > alpha) {
			alpha = score;
		}
	}

	return alpha;
}

t_score negamax(chess::Board &board, int depth, t_score alpha, t_score beta, chess::Move *bestMove, int index = 0, chess::Move prevMove = chess::Move::NULL_MOVE) {
	if (shouldCancel()) {
		if (prevMove != chess::Move::NULL_MOVE) {
			board.unmakeMove(prevMove);
		}
		return 0;
	}

#if TT
	uint64_t key = board.hash();
	t_score ttScore;
#endif

	chess::Move bestLocalMove = chess::Move::NULL_MOVE;

#if TT
	if (probeTT(key, depth, alpha, beta, ttScore, bestLocalMove)) {
		if (bestMove) {
			*bestMove = bestLocalMove;
		}
		return ttScore;
	}
#endif

	if (depth == 0) {
		return quiescence(board, alpha, beta, prevMove);
	}

	t_score bestScore = SCORE_MIN;

	chess::Movelist moves = generateMoves(board);
	for (auto &move : moves) {
		board.makeMove(move);
		t_score score = -negamax(board, depth - 1, -beta, -alpha, bestMove + 1, index + 1, move);
		board.unmakeMove(move);

		if (score == SCORE_MAX) {
			bestScore = score;
			bestLocalMove = move;
			break;
		}

		if (score > bestScore) {
			bestScore = score;
			bestLocalMove = move;
		}

		if (score >= beta) {
			bestScore = beta;
			bestLocalMove = move;

#if PROFILE_CUTOFF
			totalBetaCutoffs++;
			if (moves[0] == move) {
				firstMoveBetaCutoffs++;
			}
#endif

			break;
		}
		if (score > alpha) {
			bestScore = score;
			bestLocalMove = move;
			alpha = score;
		}
	}

#if TT
	auto ttFlag = (bestScore <= alpha) ? UPPERBOUND : (bestScore >= beta) ? LOWERBOUND : EXACT;

	storeTT(key, depth, bestScore, ttFlag, bestLocalMove);
#endif

	if (bestMove) {
		*bestMove = bestLocalMove;
	}
	if (index == 1) {
		nextMoveMap[prevMove.from().index() * 64 + prevMove.to().index()] = bestLocalMove;
	}
	return bestScore;
}

bool getOpeningMove(chess::Board &board, chess::Move *bestMove) {
	if (board.fullMoveNumber() == 1) {
		*bestMove = chess::uci::uciToMove(board, board.sideToMove() == chess::Color::WHITE ? "e2e4" : "d7d5");
		return true;
	}

	return false;
}

uint64_t timeFunc(std::function<void()> func) {
	auto start = std::chrono::high_resolution_clock::now();
	func();
	auto end = std::chrono::high_resolution_clock::now();
	return std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
}

t_score search(chess::Board &board) {
#if PROFILE_CUTOFF
	totalBetaCutoffs = 0;
	firstMoveBetaCutoffs = 0;
#endif

	t_score savedScore;
	int depth = MIN_DEPTH;

	while (true) {
        std::cout << "info depth " << depth << std::endl;

		chess::Move line[MAX_DEPTH];
		t_score score;
		auto time = timeFunc([&]() {
			score = negamax(board, depth, SCORE_MIN, SCORE_MAX, line);
		});

		if (shouldCancel()) {
			break;
		}
		savedScore = score;
		memcpy(bestLine, line, sizeof(line));
		bestLineLength = depth;

		if (score == SCORE_MAX) {
			break;
		}

		if (depth == MAX_DEPTH || time >= DYNAMIC_SEARCH_TIME_MS) {
			break;
		}

		depth++;
	}

	return savedScore;
}

void dumpUciInfo(chess::Board &board, t_score score) {
	std::cout << "info string evaluation: " << (board.sideToMove() == chess::Color::WHITE ? 1 : -1) * score << std::endl;
	std::cout << "info string stage: ";
	switch (getGameStage(board)) {
	case GameStage::OPENING:
		std::cout << "opening";
		break;
	case GameStage::MIDDLEGAME:
		std::cout << "middlegame";
		break;
	case GameStage::ENDGAME:
		std::cout << "endgame";
		break;
	}
	std::cout << std::endl;

#if PROFILE_CUTOFF
	double efficiency = (double)firstMoveBetaCutoffs / totalBetaCutoffs;
	std::cout << "info string efficiency: " << efficiency << std::endl;
#endif
}

int main() {
#ifdef TEST_POS
	board = chess::Board::fromFen(TEST_POS);

	auto score = search(board);
	std::cout << "bestmove " << chess::uci::moveToUci(bestLine[0]) << std::endl;

	dumpUciInfo(board, score);
#else
	while (true) {
        updateState();
    }
#endif

	return 0;
}
