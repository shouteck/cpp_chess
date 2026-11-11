#include "movegen.h"
#include "board.h"

static constexpr std::array<std::pair<int, int>, 8> kKnightDrDc = {{
    {2, 1}, {2, -1}, {1, 2}, {1, -2}, {-2, 1}, {-2, -1}, {-1, 2}, {-1, -2},
}};

std::vector<Move> GenerateMoves(const Position& pos) {
    std::vector<Move> moves;
    Side playing_side = pos.GetSideToMove();
    auto ep = pos.GetEnPassantTarget();
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

            if (p1 == Piece::WHITE_PAWN || p1 == Piece::BLACK_PAWN) {
                int dir = (playing_side == Side::WHITE) ? 1 : -1;
                int new_i = i - 1 * dir;
                if (new_i < 0 || new_i >= kBoardSize) continue;
                int to = new_i * kBoardSize + j;
                Piece p2 = pos.GetPiece(new_i, j);
                if (p2 == Piece::EMPTY) {
                    moves.push_back(Move{from, to});
                }
                int new_j = j - 1;
                if (new_j >= 0) {
                    to = new_i * kBoardSize + new_j;
                    p2 = pos.GetPiece(new_i, new_j);
                    if (p2 != Piece::EMPTY && PieceSide(p2) != playing_side) {
                        moves.push_back(Move{from, to});
                    }
                    if (p2 == Piece::EMPTY && ep && ep.value() == to) {
                        moves.push_back(Move{from, to});
                    }
                }
                new_j = j + 1;
                if (new_j < kBoardSize) {
                    to = new_i * kBoardSize + new_j;
                    p2 = pos.GetPiece(new_i, new_j);
                    if (p2 != Piece::EMPTY && PieceSide(p2) != playing_side) {
                        moves.push_back(Move{from, to});
                    }
                    if (p2 == Piece::EMPTY && ep && ep.value() == to) {
                        moves.push_back(Move{from, to});
                    }
                }
                if ((playing_side == Side::WHITE && i == 6) || (playing_side == Side::BLACK && i == 1)) {
                    new_i = i - 2 * dir;
                    to = new_i * kBoardSize + j;
                    p2 = pos.GetPiece(new_i, j);
                    if ((pos.GetPiece(i - 1 * dir, j) == Piece::EMPTY) && (p2 == Piece::EMPTY)) {
                        moves.push_back(Move{from, to});
                    }
                }
            }

        }
    }
    return moves;
}
