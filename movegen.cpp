#include "movegen.h"
#include "board.h"

static constexpr std::array<std::pair<int, int>, 8> kKnightDrDc = {{
    {2, 1}, {2, -1}, {1, 2}, {1, -2}, {-2, 1}, {-2, -1}, {-1, 2}, {-1, -2},
}};

static constexpr std::array<int, 2> kPawnCaptureDc = {1, -1};

std::vector<Move> GenerateMoves(const Position& pos) {
    std::vector<Move> moves;
    moves.reserve(64);
    Side playing_side = pos.GetSideToMove();
    const auto ep = pos.GetEnPassantTarget();
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
                const int dir = (playing_side == Side::WHITE) ? 1 : -1;
                int new_i = i - dir;
                if (new_i < 0 || new_i >= kBoardSize) continue;
                int to = new_i * kBoardSize + j;
                Piece p2 = pos.GetPiece(new_i, j);
                if (p2 == Piece::EMPTY) {
                    moves.push_back(Move{from, to});
                }
                for (int dc : kPawnCaptureDc) {
                    int new_j = j + dc;
                    if (new_j >= 0 && new_j < kBoardSize) {
                        to = new_i * kBoardSize + new_j;
                        p2 = pos.GetPiece(new_i, new_j);
                        if (p2 != Piece::EMPTY && PieceSide(p2) != playing_side) {
                            moves.push_back(Move{from, to});
                        }
                        if (p2 == Piece::EMPTY && ep && *ep == to) {
                            moves.push_back(Move{from, to});
                        }
                    }
                }
                if ((playing_side == Side::WHITE && i == kWhitePawnStartRow)
                    || (playing_side == Side::BLACK && i == kBlackPawnStartRow)) {
                    new_i = i - 2 * dir;
                    to = new_i * kBoardSize + j;
                    p2 = pos.GetPiece(new_i, j);
                    if ((pos.GetPiece(i - dir, j) == Piece::EMPTY) && (p2 == Piece::EMPTY)) {
                        moves.push_back(Move{from, to});
                    }
                }
            }

        }
    }
    return moves;
}
