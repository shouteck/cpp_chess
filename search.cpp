#include "board.h"
#include "eval.h"
#include "movegen.h"
#include "search.h"
#include <optional>

static int Minimax(Position& pos, int depth) {
    if (depth == 0) {
        return Evaluate(pos);
    }
    Side mover = pos.GetSideToMove();
    Side enemy = Enemy(mover);
    const bool maximizing = (mover == Side::WHITE);
    int best_score = maximizing ? std::numeric_limits<int>::lowest()
                                : std::numeric_limits<int>::max();
    auto moves = GenerateLegalMoves(pos);
    if (moves.empty()) {
        std::optional<int> king_square = FindKing(pos, mover);
        if (king_square && IsSquareAttacked(pos, *king_square, enemy)) {
            if (maximizing) return std::numeric_limits<int>::lowest();
            else return std::numeric_limits<int>::max();
        }
        return 0;
    }
    for (const auto& m : moves) {
        UndoInfo ui = pos.MakeMove(m);
        if (maximizing) {
            best_score = std::max(best_score, Minimax(pos, depth - 1));
        }
        else {
            best_score = std::min(best_score, Minimax(pos, depth - 1));
        }
        pos.UnmakeMove(m, ui);
    }
    return best_score;
}

std::optional<Move> FindBestMove(Position &pos, int depth) {
    std::optional<Move> best_move = std::nullopt;
    Side mover = pos.GetSideToMove();
    const bool maximizing = (mover == Side::WHITE);
    int best_score = maximizing ? std::numeric_limits<int>::lowest()
                                : std::numeric_limits<int>::max();
    for (const auto& m : GenerateLegalMoves(pos)) {
        UndoInfo ui = pos.MakeMove(m);
        int score = Minimax(pos, depth - 1);
        if (maximizing) {
            if (!best_move || score > best_score) {
                best_move = m; best_score = score;
            }
        }
        else {
            if (!best_move || score < best_score) {
                best_move = m; best_score = score;
            }
        }
        pos.UnmakeMove(m, ui);
    }
    return best_move;
}
