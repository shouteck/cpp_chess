#pragma once

#include <array>
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

class Position {
    public:
        void SetUpStartingPosition();
        void PrintBoard();
        [[nodiscard]] bool FromFen(std::string_view fen);
        [[nodiscard]] Piece GetPiece(int row, int col) const;
        [[nodiscard]] Side GetSideToMove() const;
        [[nodiscard]] int GetCastlingRights() const;
        [[nodiscard]] std::optional<int> GetEnPassantTarget() const;
        [[nodiscard]] int GetHalfmoveClock() const;
        [[nodiscard]] int GetFullmoveNumber() const;
    private:
        static constexpr int kBoardSize = 8;
        static constexpr int kSquareCount = 64;
        std::array<Piece, kSquareCount> board;
        Side side_to_move;
        int castling_rights;
        int halfmove_clock;
        int fullmove_number;
        std::optional<int> en_passant_target;
};
