#include "movegen.h"
#include "board.h"

static constexpr std::array<std::pair<int, int>, 8> kKnightDrDc = {{
    {2, 1}, {2, -1}, {1, 2}, {1, -2}, {-2, 1}, {-2, -1}, {-1, 2}, {-1, -2},
}};

std::vector<Move> GenerateMoves(const Position& pos) {
    std::vector<Move> moves;
    Side playing_side = pos.GetSideToMove();
    for (int i = 0; i < kBoardSize; i++) {
        for (int j = 0; j < kBoardSize; j++) {
            int from = i * kBoardSize + j;
            Piece p1 = pos.GetPiece(i, j);
            if (p1 == Piece::EMPTY || PieceSide(p1) != playing_side) continue;
            if (p1 == Piece::WHITE_KNIGHT || p1 == Piece::BLACK_KNIGHT) {
                for (const auto& [dr, dc] : kKnightDrDc) {
                    int new_i = i + dr; int new_j = j + dc;
                    if (new_i < 0 || new_i >= kBoardSize) continue;
                    if (new_j < 0 || new_j >= kBoardSize) continue;
                    int to = new_i * kBoardSize + new_j;
                    Piece p2 = pos.GetPiece(new_i, new_j);
                    if (p2 != Piece::EMPTY && PieceSide(p2) == playing_side) continue;
                    moves.push_back(Move{from, to});
                }
            }
        }
    }
    return moves;
}
