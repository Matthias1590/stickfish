#include "chess.hpp"
#include <iostream>
#include <fstream>
#include <chrono>
#include <stdint.h>

#define MIN_DEPTH 5
#define MAX_DEPTH 20

int PIECE_SQUARE_TABLE_PAWN[] = {
    0, 0, 0, 0, 0, 0, 0, 0,
    50, 50, 50, 50, 50, 50, 50, 50,
    10, 10, 20, 30, 30, 20, 10, 10,
    5, 5, 10, 25, 25, 10, 5, 5,
    5, 0, 0, 20, 20, 0, 0, 5,
    5, -5, -10, 0, 0, -10, -5, 5,
    5, 10, 10, -20, -20, 10, 10, 5,
    0, 0, 0, 0, 0, 0, 0, 0
};

int PIECE_SQUARE_TABLE_KNIGHT[] = {
    -50, -40, -30, -30, -30, -30, -40, -50,
    -40, -20, 0, 0, 0, 0, -20, -40,
    -30, 0, 10, 15, 15, 10, 0, -30,
    -30, 5, 15, 20, 20, 15, 5, -30,
    -30, 0, 15, 20, 20, 15, 0, -30,
    -30, 5, 10, 15, 15, 10, 5, -30,
    -40, -20, 0, 5, 5, 0, -20, -40,
    -50, -40, -30, -30, -30, -30, -40, -50
};

int PIECE_SQUARE_TABLE_BISHOP[] = {
    -20, -10, -10, -10, -10, -10, -10, -20,
    -10, 0, 0, 0, 0, 0, 0, -10,
    -10, 0, 5, 10, 10, 5, 0, -10,
    -10, 5, 5, 10, 10, 5, 5, -10,
    -10, 0, 10, 10, 10, 10, 0, -10,
    -10, 10, 10, 10, 10, 10, 10, -10,
    -10, 5, 0, 0, 0, 0, 5, -10,
    -20, -10, -10, -10, -10, -10, -10, -20
};

int PIECE_SQUARE_TABLE_ROOK[] = {
    0, 0, 0, 0, 0, 0, 0, 0,
    5, 10, 10, 10, 10, 10, 10, 5,
    -5, 0, 0, 0, 0, 0, 0, -5,
    -5, 0, 0, 0, 0, 0, 0, -5,
    -5, 0, 0, 0, 0, 0, 0, -5,
    -5, 0, 0, 0, 0, 0, 0, -5,
    -5, 0, 0, 0, 0, 0, 0, -5,
    0, 0, 0, 5, 5, 0, 0, 0
};

int PIECE_SQUARE_TABLE_QUEEN[] = {
    -20, -10, -10, -5, -5, -10, -10, -20,
    -10, 0, 0, 0, 0, 0, 0, -10,
    -10, 0, 5, 5, 5, 5, 0, -10,
    -5, 0, 5, 5, 5, 5, 0, -5,
    0, 0, 5, 5, 5, 5, 0, -5,
    -10, 5, 5, 5, 5, 5, 0, -10,
    -10, 0, 5, 0, 0, 0, 0, -10,
    -20, -10, -10, -5, -5, -10, -10, -20
};

int PIECE_SQUARE_TABLE_KING_MIDGAME[] = {
    -30, -40, -40, -50, -50, -40, -40, -30,
    -30, -40, -40, -50, -50, -40, -40, -30,
    -30, -40, -40, -50, -50, -40, -40, -30,
    -30, -40, -40, -50, -50, -40, -40, -30,
    -20, -30, -30, -40, -40, -30, -30, -20,
    -10, -20, -20, -20, -20, -20, -20, -10,
    20, 20,   0,   0,   0,   0,  20,  20,
    20, 30,  10,   0,   0,  10,  30,  20,
};

int evalPieceSquares(chess::Board &board, chess::Color color) {
    int score = 0;

    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
			auto p = board.at(chess::Square(chess::Rank(y), chess::File(x)));
            if (p == p.NONE) {
                continue;
            }
            if (p.color() != color) {
                continue;
            }

            int index = y * 8 + x;
            switch (p.type().internal()) {
            case chess::PieceType::underlying::PAWN:
                score += color == chess::Color::WHITE ? PIECE_SQUARE_TABLE_PAWN[index] : PIECE_SQUARE_TABLE_PAWN[63 - index];
                break;
            case chess::PieceType::underlying::KNIGHT:
                score += color == chess::Color::WHITE ? PIECE_SQUARE_TABLE_KNIGHT[index] : PIECE_SQUARE_TABLE_KNIGHT[63 - index];
                break;
            case chess::PieceType::underlying::BISHOP:
                score += color == chess::Color::WHITE ? PIECE_SQUARE_TABLE_BISHOP[index] : PIECE_SQUARE_TABLE_BISHOP[63 - index];
                break;
            case chess::PieceType::underlying::ROOK:
                score += color == chess::Color::WHITE ? PIECE_SQUARE_TABLE_ROOK[index] : PIECE_SQUARE_TABLE_ROOK[63 - index];
                break;
            case chess::PieceType::underlying::QUEEN:
                score += color == chess::Color::WHITE ? PIECE_SQUARE_TABLE_QUEEN[index] : PIECE_SQUARE_TABLE_QUEEN[63 - index];
                break;
            case chess::PieceType::underlying::KING:
                score += color == chess::Color::WHITE ? PIECE_SQUARE_TABLE_KING_MIDGAME[index] : PIECE_SQUARE_TABLE_KING_MIDGAME[63 - index];
                break;
            }
        }
    }

    return score;
}

int pieceValue(chess::PieceType type) {
    switch (type.internal()) {
    case chess::PieceType::PAWN:
        return 100;
    case chess::PieceType::KNIGHT:
        return 300;
    case chess::PieceType::BISHOP:
        return 320;
    case chess::PieceType::ROOK:
        return 530;
    case chess::PieceType::QUEEN:
        return 950;
    case chess::PieceType::KING:
        return 20000;
    default:
        return 0;
    }
}

int evalMat(chess::Board &board, chess::Color color) {
	int score = 0;

	score += board.pieces(chess::PieceType::PAWN, color).count() * pieceValue(chess::PieceType::PAWN);
	score += board.pieces(chess::PieceType::KNIGHT, color).count() * pieceValue(chess::PieceType::KNIGHT);
	score += board.pieces(chess::PieceType::BISHOP, color).count() * pieceValue(chess::PieceType::BISHOP);
	score += board.pieces(chess::PieceType::ROOK, color).count() * pieceValue(chess::PieceType::ROOK);
	score += board.pieces(chess::PieceType::QUEEN, color).count() * pieceValue(chess::PieceType::QUEEN);
	score += board.pieces(chess::PieceType::KING, color).count() * pieceValue(chess::PieceType::KING);

	return score;
}

int evalKingSafety(chess::Board &board, chess::Color color) {
	// todo: optimize using bitboards

    int score = 0;
    int dir = color == chess::Color::WHITE ? -1 : 1;

    auto kingIndex = board.pieces(chess::PieceType::KING, color).lsb();
	auto kingX = kingIndex % 8;
	auto kingY = kingIndex / 8;
    for (int x = kingX; x < kingX + 2; x++) {
        if (x < 0 || x >= 8) {
            continue;
        }
        auto pawnFound = false;
        for (int y = kingY; y >= 0 && y < 8; y += dir) {
            auto p = board.at(chess::Square(chess::Rank(y), chess::File(x)));
            if (p == p.NONE) {
				continue;
			}
            if (p.color() == color && p.type().internal() == chess::PieceType::PAWN) {
                pawnFound = true;
                break;
            }
        }
        if (!pawnFound) {
            score -= 10;
        }
    }

    return score;
}

int evalMobility(chess::Board &board, chess::Color color) {
    int score = 0;

    chess::Movelist moveList;
    chess::movegen::legalmoves(moveList, board);
    score += moveList.size();

    return score;
}

int evalOpenFiles(chess::Board &board, chess::Color color) {
    int score = 0;

    auto pawns = board.pieces(chess::PieceType::PAWN);
    for (int x = 0; x < 8; x++) {
        auto fileMask = 0x0101010101010101ULL << x;

        if (pawns & fileMask) {
            continue;
        }

        // count rooks and queens on open files
        auto rooks = board.pieces(chess::PieceType::ROOK, color) & fileMask;
        auto queens = board.pieces(chess::PieceType::QUEEN, color) & fileMask;

        score += rooks.count() * 20;
        score += queens.count() * 10;

        if (score > 0) {
            // if blocked by knights or bishops, subtract score
            if ((board.pieces(chess::PieceType::KNIGHT) | board.pieces(chess::PieceType::BISHOP)) & fileMask) {
                score -= 5;
            }
        }
    }

    return score;
}

int evalDoublePawns(chess::Board &board, chess::Color color) {
    int score = 0;

    auto pawns = board.pieces(chess::PieceType::PAWN, color);
    for (int x = 0; x < 8; x++) {
        auto fileMask = 0x0101010101010101ULL << x;
        auto filePawns = pawns & fileMask;
        if (filePawns.count() > 1) {
            score -= 10;
        }
    }

    return score;
}

int evaluate(chess::Board &board) {
	double score = 0;

	auto gameOver = board.isGameOver();
	switch (gameOver.second)
	{
	case chess::GameResult::DRAW:
		return 0;
	case chess::GameResult::LOSE:
		return board.sideToMove() == chess::Color::WHITE ? INT64_MIN : INT64_MAX;
	case chess::GameResult::WIN:
		return board.sideToMove() == chess::Color::WHITE ? INT64_MAX : INT64_MIN;
	default:
		break;
	}

    // todo: king safety doesnt matter nearly as much in endgame
    #define KING_SAFETY_WEIGHT 4
	score += evalKingSafety(board, chess::Color::WHITE) * KING_SAFETY_WEIGHT;
	score -= evalKingSafety(board, chess::Color::BLACK) * KING_SAFETY_WEIGHT;
    // std::cout << "King safety: " << evalKingSafety(board, chess::Color::WHITE) * KING_SAFETY_WEIGHT << std::endl;

    #define MATERIAL_WEIGHT 0.1
	score += evalMat(board, chess::Color::WHITE) * MATERIAL_WEIGHT;
	score -= evalMat(board, chess::Color::BLACK) * MATERIAL_WEIGHT;
    // std::cout << "Material: " << evalMat(board, chess::Color::WHITE) * MATERIAL_WEIGHT << std::endl;

    #define PIECE_SQUARE_WEIGHT 1
	score += evalPieceSquares(board, chess::Color::WHITE) * PIECE_SQUARE_WEIGHT;
	score -= evalPieceSquares(board, chess::Color::BLACK) * PIECE_SQUARE_WEIGHT;
    // std::cout << "Piece squares: " << evalPieceSquares(board, chess::Color::WHITE) * PIECE_SQUARE_WEIGHT << std::endl;

    #define MOBILITY_WEIGHT 4
    score += evalMobility(board, chess::Color::WHITE) * MOBILITY_WEIGHT;
    score -= evalMobility(board, chess::Color::BLACK) * MOBILITY_WEIGHT;
    // std::cout << "Mobility: " << evalMobility(board, chess::Color::WHITE) * MOBILITY_WEIGHT << std::endl;

    #define OPEN_FILES_WEIGHT 10
    score += evalOpenFiles(board, chess::Color::WHITE) * OPEN_FILES_WEIGHT;
    score -= evalOpenFiles(board, chess::Color::BLACK) * OPEN_FILES_WEIGHT;
    // std::cout << "Open files: " << evalOpenFiles(board, chess::Color::WHITE) * OPEN_FILES_WEIGHT << std::endl;

    #define DOUBLED_PAWN_WEIGHT 10
    score += evalDoublePawns(board, chess::Color::WHITE) * DOUBLED_PAWN_WEIGHT;
    score -= evalDoublePawns(board, chess::Color::BLACK) * DOUBLED_PAWN_WEIGHT;
    // std::cout << "Doubled pawns: " << evalDoublePawns(board, chess::Color::WHITE) * DOUBLED_PAWN_WEIGHT << std::endl;

	return score;
}

int scoreMove(chess::Board &board, chess::Move move) {
    int score = 0;

    // capture is good
    score += pieceValue(board.at(move.to()).type()) * 5;

    // castling is good, removing castling rights is bad
    if (board.castlingRights().has(board.sideToMove())) {
        // if this is a castle move, its good
        if (move.typeOf() == chess::Move::CASTLING) {
            score += 1000;
        } else if (board.at(move.from()).type() == chess::PieceType::KING) {
            score -= 1000;
        }
    }

    return score;
}

void sortMoves(chess::Board &board, chess::Movelist &moveList) {
    for (auto &move : moveList) {
        move.setScore(scoreMove(board, move));
    }

    std::sort(moveList.begin(), moveList.end(), [](const chess::Move &a, const chess::Move &b) {
        return a.score() > b.score();
    });
}

int64_t mini(chess::Board &board, int depth, int64_t alpha, int64_t beta, chess::Move *bestMove);
int64_t maxi(chess::Board &board, int depth, int64_t alpha, int64_t beta, chess::Move *bestMove);

int64_t search(chess::Board &board, int depth, chess::Move *bestMove) {
    if (bestMove) {
        auto legalMoves = chess::Movelist();
        chess::movegen::legalmoves(legalMoves, board);
        if (legalMoves.size() > 0) {
            *bestMove = legalMoves[0];
        }
    }
    return board.sideToMove() == chess::Color::WHITE 
        ? maxi(board, depth, INT64_MIN, INT64_MAX, bestMove)
        : mini(board, depth, INT64_MIN, INT64_MAX, bestMove);
}

int64_t maxi(chess::Board &board, int depth, int64_t alpha, int64_t beta, chess::Move *bestMove) {
    if (depth <= -2 || (depth <= 0 && !board.inCheck())) {
        return evaluate(board);
    }

    chess::Movelist moveList;
    chess::movegen::legalmoves(moveList, board);
    sortMoves(board, moveList);
    int64_t bestScore = INT64_MIN;

    for (auto move : moveList) {
		// if depth < -1, only consider captures
		if (depth < -1 && board.at(move.to()) == chess::Piece::NONE) {
			continue;
		}

        board.makeMove(move);
        int64_t score = mini(board, depth - 1, alpha, beta, nullptr);
        board.unmakeMove(move);
        
        if (score > bestScore) {
            bestScore = score;
			if (bestMove) *bestMove = move;
            alpha = std::max(alpha, score);
        }
        if (alpha >= beta) {
            break; // Beta cutoff
        }
    }
    return bestScore;
}

int64_t mini(chess::Board &board, int depth, int64_t alpha, int64_t beta, chess::Move *bestMove) {
    if (depth <= -2 || (depth <= 0 && !board.inCheck())) {
        return evaluate(board);
    }

    chess::Movelist moveList;
    chess::movegen::legalmoves(moveList, board);
    sortMoves(board, moveList);
    int64_t bestScore = INT64_MAX;

    for (auto move : moveList) {
		// if depth < -1, only consider captures
		if (depth < -1 && board.at(move.to()) == chess::Piece::NONE) {
			continue;
		}

        board.makeMove(move);
        int64_t score = maxi(board, depth - 1, alpha, beta, nullptr);
        board.unmakeMove(move);
        
        if (score < bestScore) {
            bestScore = score;
			if (bestMove) *bestMove = move;
            beta = std::min(beta, score);
        }
        if (alpha >= beta) {
            break; // Alpha cutoff
        }
    }
    return bestScore;
}

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

std::unordered_map<std::string, chess::Move> openingBook;

bool tryFindOpening(chess::Board &board, chess::Move *bestMove) {
    auto fen = board.getFen();
    fen = fen.substr(0, fen.find(' ') + 2);

    auto it = openingBook.find(fen);
    if (it == openingBook.end()) {
        return false;
    }

    *bestMove = it->second;
    return true;
}

int searchDynamic(chess::Board &board, chess::Move *bestMove) {
    if (tryFindOpening(board, bestMove)) {
        return 0;
    }

	int depth = MIN_DEPTH;
	while (true) {
		std::cout << "info depth " << depth << std::endl;
		auto startTime = std::chrono::high_resolution_clock::now();
		auto score = search(board, depth, bestMove);
		auto elapsedMillis = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - startTime).count();
		if (elapsedMillis > 1000 || depth >= MAX_DEPTH) {
			return score;
		}
		depth++;
	}
}

void loadOpenings() {
    std::ifstream file("openings.txt");
    if (!file.is_open()) {
        std::cerr << "Failed to open openings.txt" << std::endl;
        return;
    }

    std::string fenLine;
    std::string moveLine;
    
    // Read FEN and move lines in pairs
    while (std::getline(file, fenLine) && std::getline(file, moveLine)) {
        // Skip empty lines
        if (fenLine.empty() || moveLine.empty()) {
            continue;
        }
        
        try {
            auto fen = fenLine;
            // The FEN in file is partial, needs to have complete format for the chess library
            if (fen.find(' ') == fen.size() - 1) {
                // If FEN ends with a space, add the missing parts
                fen += "KQkq - 0 1";
            }
            
            auto board = chess::Board::fromFen(fen);
            auto move = chess::uci::uciToMove(board, moveLine);
            
            // Store with the partial FEN as key to match the lookup in tryFindOpening
            std::string lookupFen = fenLine;
            openingBook[lookupFen] = move;
        }
        catch (const std::exception& e) {
            std::cerr << "Error loading opening: " << e.what() << std::endl;
            std::cerr << "FEN: " << fenLine << ", Move: " << moveLine << std::endl;
        }
    }
    
    std::cout << "Loaded " << openingBook.size() << " openings" << std::endl;
}

int main() {
    auto b1 = chess::Board::fromFen(std::string_view("1r2r1k1/1pp2pp1/p2b1q1p/6n1/6Q1/3P4/PPP1N1PP/R1BK3R w - - 2 21"));

    // // Print current score
    // std::cout << "Score: " << search(b1, 0, nullptr) << std::endl;

    // // Move h2h4
    // auto m = chess::uci::uciToMove(b1, "h2h4");
    // b1.makeMove(m);

    // // Print new score
    // std::cout << "Score: " << search(b1, 0, nullptr) << std::endl;

    // return 1;

    chess::Move bestMove;
    auto score = searchDynamic(b1, &bestMove);
    std::cout << "Score: " << score << std::endl;
    std::cout << "Best move: " << bestMove.from() << bestMove.to() << std::endl;

    return 0;

    loadOpenings();

	auto board = chess::Board::fromFen(std::string_view("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR"));

	// std::cout << board << std::endl;

	// chess::Move bestMove;
	// auto score = search(board, 8, &bestMove);

	// std::cout << "Score: " << score << std::endl;
	// std::cout << "Best move: " << bestMove.from() << bestMove.to() << std::endl;

	while (true) {
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
			chess::Move bestMove;
			auto score = searchDynamic(board, &bestMove);
			std::cout << "bestmove " << chess::uci::moveToUci(bestMove) << std::endl;
		}
	}

	return 0;
}
