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

class Position {
    public:
        void SetUpStartingPosition();
        void PrintBoard() const;
        [[nodiscard]] bool FromFen(std::string_view fen);
        [[nodiscard]] constexpr Piece GetPiece(int row, int col) const noexcept {
            return board[row * kBoardSize + col];
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
