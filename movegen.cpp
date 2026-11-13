#include "movegen.h"
#include "board.h"
#include "move.h"

static constexpr std::array<std::pair<int, int>, 8> kKnightDrDc = {{
    {2, 1}, {2, -1}, {1, 2}, {1, -2}, {-2, 1}, {-2, -1}, {-1, 2}, {-1, -2},
}};

static constexpr std::array<int, 2> kPawnCaptureDc = {1, -1};

static constexpr std::array<std::pair<int, int>, 4> kBishopDrDc = {{
   {-1, 1}, {-1, -1}, {1, 1}, {1, -1},
}};

static constexpr std::array<std::pair<int, int>, 4> kRookDrDc = {{
   {0, 1}, {0, -1}, {1, 0}, {-1, 0},
}};

static constexpr std::array<std::pair<int, int>, 8> kKingDrDc = {{
    {-1, -1}, {-1, 0}, {-1, 1}, { 0, -1}, { 0, 1}, { 1, -1}, { 1, 0}, { 1, 1}
}};

static constexpr bool SquaresAreEmpty(const Position& pos, int lo, int hi) {
    for (int sq = lo; sq <= hi; sq++)
        if (pos.GetPiece(sq) != Piece::EMPTY) return false;
    return true;
}

static constexpr std::array<Piece, 4> kWhitePromoPieces = { Piece::WHITE_KNIGHT, Piece::WHITE_BISHOP,
    Piece::WHITE_ROOK, Piece::WHITE_QUEEN, };
static constexpr std::array<Piece, 4> kBlackPromoPieces = { Piece::BLACK_KNIGHT, Piece::BLACK_BISHOP,
    Piece::BLACK_ROOK, Piece::BLACK_QUEEN, };

template <std::size_t N>
static void GenerateSlidingMoves(const Position& pos, int from, int i, int j,
    Side playing_side, const std::array<std::pair<int, int>, N>& directions, std::vector<Move>& moves) {
    for (const auto& [dr, dc] : directions) {
        int new_i = i; int new_j = j;
        while (true) {
            new_i += dr; new_j += dc;
            if (new_i < 0 || new_i >= kBoardSize || new_j < 0 || new_j >= kBoardSize) break;
            int to = new_i * kBoardSize + new_j;
            Piece p2 = pos.GetPiece(to);
            if (p2 == Piece::EMPTY) {
                moves.push_back(Move{from, to});
            }
            else {
                if (playing_side != PieceSide(p2)) moves.push_back(Move{from, to});
                break;
            }
        }
    }
}

std::vector<Move> GenerateMoves(const Position& pos) {
    std::vector<Move> moves;
    moves.reserve(64);
    const Side playing_side = pos.GetSideToMove();
    const auto ep = pos.GetEnPassantTarget();
    for (int i = 0; i < kBoardSize; i++) {
        for (int j = 0; j < kBoardSize; j++) {
            const int from = i * kBoardSize + j;
            const Piece p1 = pos.GetPiece(from);
            if (p1 == Piece::EMPTY || PieceSide(p1) != playing_side) continue;

            const auto type = GetPieceType(p1);

            switch (type) {
                case PieceType::KNIGHT: {
                    for (const auto& [dr, dc] : kKnightDrDc) {
                        const int new_i = i + dr; const int new_j = j + dc;
                        if (new_i < 0 || new_i >= kBoardSize || new_j < 0 || new_j >= kBoardSize) continue;
                        int to = new_i * kBoardSize + new_j;
                        Piece p2 = pos.GetPiece(to);
                        if (p2 != Piece::EMPTY && PieceSide(p2) == playing_side) continue;
                        moves.push_back(Move{from, to});
                    }
                    break;
                }
                case PieceType::PAWN: {
                    const int dir = (playing_side == Side::WHITE) ? 1 : -1;
                    const int promo_rank = (playing_side == Side::WHITE) ? 0 : kBoardSize - 1;
                    const auto& promo_pieces = (playing_side == Side::WHITE)
                            ? kWhitePromoPieces : kBlackPromoPieces;
                    const int new_i = i - dir;
                    auto emit = [&](int to, MoveKind kind = MoveKind::NORMAL) {
                        if (new_i == promo_rank) {
                            for (Piece promo : promo_pieces) {
                                moves.push_back(Move{from, to, promo});
                            }
                        }
                        else {
                            moves.push_back(Move{from, to, Piece::EMPTY, kind});
                        }
                    };
                    if (new_i < 0 || new_i >= kBoardSize) continue;
                    int to = new_i * kBoardSize + j;
                    Piece p2 = pos.GetPiece(to);
                    if (p2 == Piece::EMPTY) {
                        emit(to);
                    }
                    for (int dc : kPawnCaptureDc) {
                        const int new_j = j + dc;
                        if (new_j >= 0 && new_j < kBoardSize) {
                            to = new_i * kBoardSize + new_j;
                            p2 = pos.GetPiece(to);
                            if (p2 != Piece::EMPTY && PieceSide(p2) != playing_side) {
                                emit(to);
                            }
                            else if (p2 == Piece::EMPTY && ep && *ep == to) {
                                emit(to, MoveKind::EN_PASSANT);
                            }
                        }
                    }
                    if ((playing_side == Side::WHITE && i == kWhitePawnStartRow)
                        || (playing_side == Side::BLACK && i == kBlackPawnStartRow)) {
                        int dbl_i = i - 2 * dir;
                        to = dbl_i * kBoardSize + j;
                        p2 = pos.GetPiece(to);
                        if ((pos.GetPiece(from - kBoardSize * dir) == Piece::EMPTY) && (p2 == Piece::EMPTY)) {
                            moves.push_back(Move{from, to, Piece::EMPTY, MoveKind::DOUBLE_PUSH});
                        }
                    }
                }
                break;
                case PieceType::BISHOP: {
                    GenerateSlidingMoves(pos, from, i, j, playing_side, kBishopDrDc, moves);
                    break;
                }
                case PieceType::ROOK: {
                    GenerateSlidingMoves(pos, from, i, j, playing_side, kRookDrDc, moves);
                    break;
                }
                case PieceType::QUEEN: {
                    GenerateSlidingMoves(pos, from, i, j, playing_side, kBishopDrDc, moves);
                    GenerateSlidingMoves(pos, from, i, j, playing_side, kRookDrDc, moves);
                    break;
                }
                case PieceType::KING: {
                    for (const auto& [dr, dc] : kKingDrDc) {
                        const int new_i = i + dr; const int new_j = j + dc;
                        if (new_i < 0 || new_i >= kBoardSize || new_j < 0 || new_j >= kBoardSize) continue;
                        int to = new_i * kBoardSize + new_j;
                        Piece p2 = pos.GetPiece(to);
                        if (p2 == Piece::EMPTY || PieceSide(p2) != playing_side) {
                            moves.push_back(Move{from, to});
                        }
                    }
                    const int rights = pos.GetCastlingRights();
                    for (const CastleInfo& c : kCastles) {
                        if (from != c.king_from) continue;
                        if (!(rights & c.rights_bit)) continue;
                        if (!SquaresAreEmpty(pos, c.empty_lo, c.empty_hi)) continue;
                        moves.push_back(Move{from, c.king_to, Piece::EMPTY,
                            c.king_to > c.king_from ? MoveKind::CASTLE_KINGSIDE : MoveKind::CASTLE_QUEENSIDE});
                    }
                    break;
                }
                default:
                    break;
            }
        }
    }
    return moves;
}
