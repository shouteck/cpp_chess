#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string_view>

enum class Piece : uint8_t {
    EMPTY,
    WHITE_PAWN, WHITE_KNIGHT, WHITE_BISHOP, WHITE_ROOK, WHITE_QUEEN, WHITE_KING,
    BLACK_PAWN, BLACK_KNIGHT, BLACK_BISHOP, BLACK_ROOK, BLACK_QUEEN, BLACK_KING,
};

enum class Side {
    WHITE,
    BLACK,
};

enum CastlingMask {
    WHITE_KINGSIDE = 1 << 0, WHITE_QUEENSIDE = 1 << 1,
    BLACK_KINGSIDE = 1 << 2, BLACK_QUEENSIDE = 1 << 3,
};

constexpr int kBoardSize = 8;
constexpr int kSquareCount = 64;
constexpr int kWhitePawnStartRow = 6;
constexpr int kBlackPawnStartRow = 1;

constexpr std::array<Side, 13> kPieceSide = {
    Side::WHITE,
    Side::WHITE, Side::WHITE, Side::WHITE, Side::WHITE, Side::WHITE, Side::WHITE,
    Side::BLACK, Side::BLACK, Side::BLACK, Side::BLACK, Side::BLACK, Side::BLACK,
};

constexpr Side PieceSide(Piece p) noexcept {
    return kPieceSide[static_cast<std::size_t>(p)];
}

enum class PieceType {
    PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING,
};

constexpr std::array<PieceType, 13> kPieceType = {
    PieceType::PAWN,
    PieceType::PAWN, PieceType::KNIGHT, PieceType::BISHOP, PieceType::ROOK, PieceType::QUEEN, PieceType::KING,
    PieceType::PAWN, PieceType::KNIGHT, PieceType::BISHOP, PieceType::ROOK, PieceType::QUEEN, PieceType::KING,
};

constexpr PieceType GetPieceType(Piece p) noexcept {
    return kPieceType[static_cast<std::size_t>(p)];
}

struct CastleInfo {
    int rights_bit;
    int king_from, king_to;
    int empty_lo, empty_hi;
    int rook_from, rook_to;
};

constexpr std::array<CastleInfo, 4> kCastles = {{
    // WHITE_KINGSIDE:  Ke1->g1, Rh1->f1, f1..g1 empty
    {CastlingMask::WHITE_KINGSIDE, 60, 62, 61, 62, 63, 61},
    // WHITE_QUEENSIDE: Ke1->c1, Ra1->d1, b1..d1 empty
    {CastlingMask::WHITE_QUEENSIDE, 60, 58, 57, 59, 56, 59},
    // BLACK_KINGSIDE:  ke8->g8, rh8->f8, f8..g8 empty
    {CastlingMask::BLACK_KINGSIDE, 4, 6, 5, 6, 7, 5},
    // BLACK_QUEENSIDE: ke8->c8, ra8->d8, b8..d8 empty
    {CastlingMask::BLACK_QUEENSIDE, 4, 2, 1, 3, 0, 3},
}};

/*
UndoInfo is information required to restore position state that Move lacks
the key pattern here is many-to-one
if the previous state can be many, then there's no way to revert them

1. captured
there's no way to tell between a normal move or a capture move
2. castling_rights
there are 2 states where castle is allowed for a certain side
for example, KQ, K, both allows kingside castle
3. en_passant_target
there's many different en_passant_targets and they only last for a move
4. half_move_clock
because pawn move and capture move changes it to 0
*/
struct UndoInfo {
    Piece captured;
    int castling_rights;
    std::optional<int> en_passant_target;
    int halfmove_clock;
};

struct Move;

class Position {
    public:
        void SetUpStartingPosition();
        void PrintBoard() const;
        UndoInfo MakeMove(const Move& m);
        void UnmakeMove(const Move& m, const UndoInfo& undo);
        [[nodiscard]] bool FromFen(std::string_view fen);
        [[nodiscard]] constexpr Piece GetPiece(int row, int col) const noexcept {
            return board[row * kBoardSize + col];
        }
        [[nodiscard]] constexpr Piece GetPiece(int square) const noexcept {
            return board[square];
        }
        [[nodiscard]] constexpr Side GetSideToMove() const noexcept {
            return side_to_move;
        }
        [[nodiscard]] constexpr int GetCastlingRights() const noexcept {
            return castling_rights;
        }
        [[nodiscard]] constexpr std::optional<int> GetEnPassantTarget() const noexcept {
            return en_passant_target;
        }
        [[nodiscard]] constexpr int GetHalfmoveClock() const noexcept {
            return halfmove_clock;
        }
        [[nodiscard]] constexpr int GetFullmoveNumber() const noexcept {
            return fullmove_number;
        }
    private:
        std::array<Piece, kSquareCount> board;
        Side side_to_move;
        int castling_rights;
        int halfmove_clock;
        int fullmove_number;
        std::optional<int> en_passant_target;
};
