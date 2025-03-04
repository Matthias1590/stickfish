 #include "chess.hpp"
#include <iostream>
#include <fstream>
#include <chrono>
#include <stdint.h>
#include <stdlib.h>

// #define DEBUG

typedef double t_score;
#define SCORE_MIN -922337203685477.0
#define SCORE_MAX 922337203685477.0

#define MIN_DEPTH 5
#define MAX_DEPTH 20

t_score PIECE_SQUARE_TABLE_PAWN[] = {
    0, 0, 0, 0, 0, 0, 0, 0,
    100, 100, 100, 100, 100, 100, 100, 100,
    30, 30, 50, 60, 60, 50, 30, 30,
    5, 5, 10, 25, 25, 10, 5, 5,
    5, 0, 0, 20, 20, 0, 0, 5,
    5, -5, -10, 0, 0, -10, -5, 5,
    5, 10, 10, -20, -20, 10, 10, 5,
    0, 0, 0, 0, 0, 0, 0, 0
};

t_score PIECE_SQUARE_TABLE_KNIGHT[] = {
    -50, -40, -30, -30, -30, -30, -40, -50,
    -40, -20, 0, 0, 0, 0, -20, -40,
    -30, 0, 10, 15, 15, 10, 0, -30,
    -30, 5, 15, 20, 20, 15, 5, -30,
    -30, 0, 15, 20, 20, 15, 0, -30,
    -30, 5, 10, 15, 15, 10, 5, -30,
    -40, -20, 0, 5, 5, 0, -20, -40,
    -50, -40, -30, -30, -30, -30, -40, -50
};

t_score PIECE_SQUARE_TABLE_BISHOP[] = {
    -20, -10, -10, -10, -10, -10, -10, -20,
    -10, 0, 0, 0, 0, 0, 0, -10,
    -10, 0, 5, 10, 10, 5, 0, -10,
    -10, 5, 5, 10, 10, 5, 5, -10,
    -10, 0, 10, 10, 10, 10, 0, -10,
    -10, 10, 10, 10, 10, 10, 10, -10,
    -10, 5, 0, 0, 0, 0, 5, -10,
    -20, -10, -10, -10, -10, -10, -10, -20
};

t_score PIECE_SQUARE_TABLE_ROOK[] = {
    0, 0, 0, 0, 0, 0, 0, 0,
    5, 10, 10, 10, 10, 10, 10, 5,
    -5, 0, 0, 0, 0, 0, 0, -5,
    -5, 0, 0, 0, 0, 0, 0, -5,
    -5, 0, 0, 0, 0, 0, 0, -5,
    -5, 0, 0, 0, 0, 0, 0, -5,
    -5, 0, 0, 0, 0, 0, 0, -5,
    0, 0, 0, 5, 5, 0, 0, 0
};

t_score PIECE_SQUARE_TABLE_QUEEN[] = {
    -20, -10, -10, -5, -5, -10, -10, -20,
    -10, 0, 0, 0, 0, 0, 0, -10,
    -10, 0, 5, 5, 5, 5, 0, -10,
    -5, 0, 5, 5, 5, 5, 0, -5,
    0, 0, 5, 5, 5, 5, 0, -5,
    -10, 5, 5, 5, 5, 5, 0, -10,
    -10, 0, 5, 0, 0, 0, 0, -10,
    -20, -10, -10, -5, -5, -10, -10, -20
};

t_score PIECE_SQUARE_TABLE_KING_MIDGAME[] = {
    -30, -40, -40, -50, -50, -40, -40, -30,
    -30, -40, -40, -50, -50, -40, -40, -30,
    -30, -40, -40, -50, -50, -40, -40, -30,
    -30, -40, -40, -50, -50, -40, -40, -30,
    -20, -30, -30, -40, -40, -30, -30, -20,
    -10, -20, -20, -20, -20, -20, -20, -10,
    20, 20,   0,   0,   0,   0,  20,  20,
    20, 30,  10,   0,   0,  10,  30,  20,
};

t_score getPieceSquareValue(chess::Board &board, chess::Square square, chess::PieceType type) {
    auto piece = board.at(square);
    if (piece == chess::Piece::NONE) {
        return 0;
    }

    int index = square.index();
    switch (type.internal()) {
    case chess::PieceType::underlying::PAWN:
        return PIECE_SQUARE_TABLE_PAWN[index];
    case chess::PieceType::underlying::KNIGHT:
        return PIECE_SQUARE_TABLE_KNIGHT[index];
    case chess::PieceType::underlying::BISHOP:
        return PIECE_SQUARE_TABLE_BISHOP[index];
    case chess::PieceType::underlying::ROOK:
        return PIECE_SQUARE_TABLE_ROOK[index];
    case chess::PieceType::underlying::QUEEN:
        return PIECE_SQUARE_TABLE_QUEEN[index];
    case chess::PieceType::underlying::KING:
        return PIECE_SQUARE_TABLE_KING_MIDGAME[index];
    default:
        return 0;
    }
}

t_score evalPieceSquares(chess::Board &board, chess::Color color) {
    t_score score = 0;

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

t_score pieceValue(chess::PieceType type) {
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

t_score evalMat(chess::Board &board, chess::Color color) {
    t_score score = 0;

    score += board.pieces(chess::PieceType::PAWN, color).count() * pieceValue(chess::PieceType::PAWN);
    score += board.pieces(chess::PieceType::KNIGHT, color).count() * pieceValue(chess::PieceType::KNIGHT);
    score += board.pieces(chess::PieceType::BISHOP, color).count() * pieceValue(chess::PieceType::BISHOP);
    score += board.pieces(chess::PieceType::ROOK, color).count() * pieceValue(chess::PieceType::ROOK);
    score += board.pieces(chess::PieceType::QUEEN, color).count() * pieceValue(chess::PieceType::QUEEN);
    score += board.pieces(chess::PieceType::KING, color).count() * pieceValue(chess::PieceType::KING);

    return score;
}

t_score evalKingSafety(chess::Board &board, chess::Color color) {
    t_score score = 0;

    // todo: king is safer if theres minor pieces closeby
    auto kingSquare = board.kingSq(color);
    auto ourPawns = board.pieces(chess::PieceType::PAWN, color);

    uint64_t fileA = 0x8080808080808080ULL;
    auto kingX = kingSquare.index() % 8;
    uint64_t kingFiles = fileA >> kingX;
    if (kingX > 0) {
        kingFiles |= fileA >> (kingX - 1);
    }
    if (kingX < 7) {
        kingFiles |= fileA >> (kingX + 1);
    }
    auto kingY = kingSquare.index() / 8;
    kingFiles <<= kingY * 8;

    auto blockingPawns = ourPawns & kingFiles;
    score += blockingPawns.count();

    return score;
}

t_score evalMobility(chess::Board &board, chess::Color color) {
    t_score score = 0;
    bool nullMove = false;

    if (board.sideToMove() != color) {
        nullMove = true;
        board.makeNullMove();
    }
    chess::Movelist moveList;
    chess::movegen::legalmoves(moveList, board);
    score += moveList.size();
    if (nullMove) {
        board.unmakeNullMove();
    }

    return score;
}

t_score evalOpenFiles(chess::Board &board, chess::Color color) {
    t_score score = 0;

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

t_score evalDoublePawns(chess::Board &board, chess::Color color) {
    t_score score = 0;

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

t_score evalDevelopment(chess::Board &board, chess::Color color) {
    t_score score = 0;

    auto backRankY = color == chess::Color::WHITE ? 0 : 7;
    auto backRankMask = 0xFFULL << (backRankY * 8);
    auto backRankPieces = board.pieces(chess::PieceType::ROOK, color) | board.pieces(chess::PieceType::KNIGHT, color) | board.pieces(chess::PieceType::BISHOP, color) | board.pieces(chess::PieceType::QUEEN, color);

    auto backRank = backRankPieces & backRankMask;
    score -= backRank.count();

    return score;
}

class EvalResult {
private:
    double _score;
    std::vector<std::tuple<std::string, double>> _info;

    EvalResult(double score)
        : _score(score)
    { }

public:
    EvalResult()
        : _score(0)
    { }

    static EvalResult Win(chess::Color forColor, int depth) {
        if (forColor == chess::Color::WHITE) {
            return EvalResult(SCORE_MAX + depth);
        }
        return EvalResult(SCORE_MIN - depth);
    }

    bool isCheckmate(chess::Color forColor) {
        if (forColor == chess::Color::WHITE) {
            return _score >= SCORE_MAX;
        }
        return _score <= SCORE_MIN;
    }

    void add(std::string reason, double value) {
        #ifdef DEBUG
        _info.push_back(std::tuple(reason + " for white", value));
        #endif
        _score += value;
    }

    void sub(std::string reason, double value) {
        #ifdef DEBUG
        _info.push_back(std::tuple(reason + " for black", value));
        #endif
        _score -= value;
    }

    t_score getScore() {
        return _score;
    }

    void dump() {
        #ifdef DEBUG
        for (auto pair : _info) {
            auto first = std::get<0>(pair);
            auto second = std::get<1>(pair);
            std::cout << first << ": " << second << std::endl;
        }
        #endif
    }

    void dumpDiff(EvalResult &other) {
        #ifdef DEBUG
        // print info where score changed
        for (auto pair : _info) {
            auto first = std::get<0>(pair);
            for (auto pair2 : other._info) {
                auto first2 = std::get<0>(pair2);
                if (first == first2) {
                    auto second = std::get<1>(pair);
                    auto second2 = std::get<1>(pair2);
                    if (second != second2) {
                        std::cout << first << ": " << second << " -> " << second2 << "  (diff " << (second2 - second) << ")" << std::endl;
                    }
                }
            }
        }
        #endif
    }
};

EvalResult evaluate(chess::Board &board) {
    EvalResult score;

    auto gameOver = board.isGameOver();
    switch (gameOver.second)
    {
    case chess::GameResult::DRAW:
        return score;
    case chess::GameResult::LOSE:
        return EvalResult::Win(board.sideToMove() == chess::Color::WHITE ? chess::Color::BLACK : chess::Color::WHITE, 0);
    case chess::GameResult::WIN:
        return EvalResult::Win(board.sideToMove(), 0);
    default:
        break;
    }

    // this
    // #define DEVELOPMENT_WEIGHT 30
    // score.add("Development", evalDevelopment(board, chess::Color::WHITE) * DEVELOPMENT_WEIGHT);
    // score.sub("Development", evalDevelopment(board, chess::Color::BLACK) * DEVELOPMENT_WEIGHT);

    // this
    // todo: king safety doesnt matter nearly as much in endgame
    #define KING_SAFETY_WEIGHT 4
    score.add("King safety", evalKingSafety(board, chess::Color::WHITE) * KING_SAFETY_WEIGHT);
    score.sub("King safety", evalKingSafety(board, chess::Color::BLACK) * KING_SAFETY_WEIGHT);

    // todo: make being castled good, no castle rights is bad, right now it moves the king up to give the rooks a lot more moves
    // but this is bad, it should castle instead to connect the rooks

    #define MATERIAL_WEIGHT 2
    score.add("Material", evalMat(board, chess::Color::WHITE) * MATERIAL_WEIGHT);
    score.sub("Material", evalMat(board, chess::Color::BLACK) * MATERIAL_WEIGHT);

    // this
    #define PIECE_SQUARE_WEIGHT 0.9
    score.add("Piece squares", evalPieceSquares(board, chess::Color::WHITE) * PIECE_SQUARE_WEIGHT);
    score.sub("Piece squares", evalPieceSquares(board, chess::Color::BLACK) * PIECE_SQUARE_WEIGHT);

    // this
    #define MOBILITY_WEIGHT 5
    score.add("Mobility", evalMobility(board, chess::Color::WHITE) * MOBILITY_WEIGHT);
    score.sub("Mobility", evalMobility(board, chess::Color::BLACK) * MOBILITY_WEIGHT);

    // #define OPEN_FILES_WEIGHT 10
    // score.add("Open files", evalOpenFiles(board, chess::Color::WHITE) * OPEN_FILES_WEIGHT);
    // score.sub("Open files", evalOpenFiles(board, chess::Color::BLACK) * OPEN_FILES_WEIGHT);

    // #define DOUBLED_PAWN_WEIGHT 10
    // score.add("Doubled pawns", evalDoublePawns(board, chess::Color::WHITE) * DOUBLED_PAWN_WEIGHT);
    // score.sub("Doubled pawns", evalDoublePawns(board, chess::Color::BLACK) * DOUBLED_PAWN_WEIGHT);

    return score;
}

int scoreMove(chess::Board &board, chess::Move move) {
    int score = 0;
    auto pieceType = board.at(move.from()).type();

    // checks are good
    board.makeMove(move);
    if (board.inCheck()) {
        score += 1000;
    }
    board.unmakeMove(move);

    // capture is good
    score += pieceValue(board.at(move.to()).type()) * 5;

    // castling is good, removing castling rights is bad
    if (board.castlingRights().has(board.sideToMove())) {
        // if this is a castle move, its good
        if (move.typeOf() == chess::Move::CASTLING) {
            score += 1000;
        } else if (pieceType == chess::PieceType::KING) {
            score -= 1000;
        }
    }

    // improving piece square table is good
    auto currentScore = getPieceSquareValue(board, move.from(), pieceType);
    auto newScore = getPieceSquareValue(board, move.to(), pieceType);
    score += newScore - currentScore;

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

EvalResult mini(chess::Board &board, int depth, t_score alpha, t_score beta, chess::Move *bestMove);
EvalResult maxi(chess::Board &board, int depth, t_score alpha, t_score beta, chess::Move *bestMove);

EvalResult search(chess::Board &board, int depth, chess::Move *bestMove) {
    if (bestMove) {
        auto legalMoves = chess::Movelist();
        chess::movegen::legalmoves(legalMoves, board);
        if (legalMoves.size() > 0) {
            *bestMove = legalMoves[0];
        }
    }
    return board.sideToMove() == chess::Color::WHITE
        ? maxi(board, depth, SCORE_MIN, SCORE_MAX, bestMove)
        : mini(board, depth, SCORE_MIN, SCORE_MAX, bestMove);
}

EvalResult maxi(chess::Board &board, int depth, t_score alpha, t_score beta, chess::Move *bestMove) {
    if (depth <= 0) {
        EvalResult standpat = evaluate(board);
        if (standpat.getScore() >= beta) return standpat;
        alpha = std::max(alpha, standpat.getScore());
    }

    chess::Movelist moveList;
    chess::movegen::legalmoves(moveList, board);
    sortMoves(board, moveList);
    EvalResult bestScore = EvalResult::Win(chess::Color::BLACK, depth);

    #ifdef DEBUG
    if (bestMove) {
        for (auto move : moveList) {
            std::cout << move << " - " << move.score() << std::endl;
        }
    }
    #endif

    for (auto move : moveList) {
        auto skip = depth <= 0;

        // if depth <= 0, only consider captures
        if (skip && board.at(move.to()) != chess::Piece::NONE) {
            skip = false;
        }

        board.makeMove(move);

        // if depth <= 0, also consider checks
        if (skip && board.inCheck()) {
            skip = false;
        }

        if (skip) {
            board.unmakeMove(move);
            continue;
        }

        EvalResult score = mini(board, depth - 1, alpha, beta, nullptr);
        board.unmakeMove(move);

        if (score.isCheckmate(chess::Color::WHITE)) {
            if (bestMove) *bestMove = move;
            return score;
        }

        if (score.getScore() > bestScore.getScore()) {
            bestScore = score;
            if (bestMove) *bestMove = move;
            alpha = std::max(alpha, score.getScore());
        }
        if (alpha >= beta) {
            break; // Beta cutoff
        }
    }

    #ifdef DEBUG
    if (bestMove) {
        std::cout << "Best move: " << *bestMove << " - " << bestMove->score() << std::endl;
    }
    #endif

    return bestScore;
}

EvalResult mini(chess::Board &board, int depth, t_score alpha, t_score beta, chess::Move *bestMove) {
    if (depth <= 0) {
        EvalResult standpat = evaluate(board);
        if (standpat.getScore() <= alpha) return standpat;
        beta = std::min(beta, standpat.getScore());
    }

    chess::Movelist moveList;
    chess::movegen::legalmoves(moveList, board);
    sortMoves(board, moveList);
    EvalResult bestScore = EvalResult::Win(chess::Color::WHITE, depth);

    for (auto move : moveList) {
        auto skip = depth <= 0;

        // if depth <= 0, only consider captures
        if (skip && board.at(move.to()) != chess::Piece::NONE) {
            skip = false;
        }

        board.makeMove(move);

        // if depth <= 0, also consider checks
        if (skip && board.inCheck()) {
            skip = false;
        }

        if (skip) {
            board.unmakeMove(move);
            continue;
        }

        EvalResult score = maxi(board, depth - 1, alpha, beta, nullptr);
        board.unmakeMove(move);

        if (score.isCheckmate(chess::Color::BLACK)) {
            if (bestMove) *bestMove = move;
            return score;
        }

        if (score.getScore() < bestScore.getScore()) {
            bestScore = score;
            if (bestMove) *bestMove = move;
            beta = std::min(beta, score.getScore());
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

EvalResult searchDynamic(chess::Board &board, chess::Move *bestMove) {
    if (tryFindOpening(board, bestMove)) {
        return EvalResult();
    }

    int depth = MIN_DEPTH;
    while (true) {
        std::cout << "info depth " << depth << std::endl;
        auto startTime = std::chrono::high_resolution_clock::now();
        auto score = search(board, depth, bestMove);
        if (score.isCheckmate(board.sideToMove())) {
            return score;
        }
        auto elapsedMillis = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - startTime).count();
        if (elapsedMillis > 2000 || depth >= MAX_DEPTH) {
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

    std::cout << "info string loaded " << openingBook.size() << " openings" << std::endl;
}

int main() {
// #define TEST_EVAL

#ifdef TEST_EVAL
    auto b1 = chess::Board::fromFen(std::string_view("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR"));

    std::cout << "Standard eval white " << evalDevelopment(b1, chess::Color::WHITE) << std::endl;
    std::cout << "Standard eval black " << evalDevelopment(b1, chess::Color::BLACK) << std::endl;

    auto move = chess::uci::uciToMove(b1, "b1c3");
    b1.makeMove(move);

    std::cout << "Standard eval white " << evalDevelopment(b1, chess::Color::WHITE) << std::endl;
    std::cout << "Standard eval black " << evalDevelopment(b1, chess::Color::BLACK) << std::endl;

    move = chess::uci::uciToMove(b1, "b8c6");
    b1.makeMove(move);

    std::cout << "Standard eval white " << evalDevelopment(b1, chess::Color::WHITE) << std::endl;
    std::cout << "Standard eval black " << evalDevelopment(b1, chess::Color::BLACK) << std::endl;

    return 1;
#endif

// #define TEST

#ifdef TEST
    // rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR
    auto b1 = chess::Board::fromFen(std::string_view("rn1qkb1r/pp2pppp/2p5/8/2pP2n1/2N1P3/PP3PPP/R1BQKB1R b KQkq - 0 7"));

    std::cout << b1 << std::endl;

    // Print current score
    auto eval = evaluate(b1);
    std::cout << "Current score: " << eval.getScore() << std::endl;
    // std::cout << "Move: " << mv << std::endl;
    auto eval1 = eval;
    eval.dump();
    std::cout << std::endl;

    // Find best move
    chess::Move mv;
    eval = search(b1, 3, &mv);

    // Play move
    std::cout << "After " << mv << std::endl;
    b1.makeMove(mv);
    std::cout << std::endl;

    // eval = evaluate(b1);
    std::cout << "New score: " << eval.getScore() << std::endl;
    // std::cout << "Move: " << mv << std::endl;
    eval.dump();
    std::cout << std::endl;
    auto eval2 = eval;

    eval1.dumpDiff(eval2);

    // get best move for white after black move
    eval = search(b1, 3, &mv);
    b1.makeMove(mv);
    
    // Print new score
    std::cout << "Score after white plays " << mv << std::endl;
    eval = evaluate(b1);
    std::cout << "Score: " << eval.getScore() << std::endl;
    eval.dump();
    std::cout << std::endl;

    // // Move h2h4
    // auto m = chess::uci::uciToMove(b1, "h2h4");
    // b1.makeMove(m);

    // // Print new score
    // eval = searchDynamic(b1, &mv);
    // std::cout << "Score: " << eval.getScore() << std::endl;
    // std::cout << "Move: " << mv << std::endl;
    // eval.dump();
    // std::cout << std::endl;

    return 1;
#endif

//    chess::Move bestMove;
//    auto score = searchDynamic(b1, &bestMove);
//    std::cout << "Score: " << score << std::endl;
//    std::cout << "Best move: " << bestMove.from() << bestMove.to() << std::endl;

//    return 0;

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
        } else if (input == "quit") {
            return 0;
        }
    }

    return 0;
}

// TODOS:
// - Figure out if development eval is correct or inverted
// - Figure out why the engine never castles
