#pragma once

#include <cstdint>
#include <string_view>

enum class Piece : uint8_t {
    EMPTY,
    WHITE_PAWN, WHITE_KNIGHT, WHITE_BISHOP, WHITE_ROOK, WHITE_QUEEN, WHITE_KING,
    BLACK_PAWN, BLACK_KNIGHT, BLACK_BISHOP, BLACK_ROOK, BLACK_QUEEN, BLACK_KING,
};

enum Side {
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
        [[nodiscard]] int GetEnPassantTarget() const;
        [[nodiscard]] int GetHalfmoveClock() const;
        [[nodiscard]] int GetFullmoveNumber() const;
    private:
        Piece board[8][8];
        Side side_to_move;
        int castling_rights;
        int halfmove_clock;
        int fullmove_number;
        int en_passant_target;
};
