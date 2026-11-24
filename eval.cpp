#include "eval.h"
#include "board.h"

int Evaluate(const Position& pos) {
    int score = 0;
    for (int i = 0; i < kSquareCount; i++) {
        Piece p = pos.GetPiece(i);
        if (p == Piece::EMPTY) continue;
        PieceType type = GetPieceType(p);
        int piece_score = 0;
        switch (type) {
            case PieceType::PAWN:
                piece_score += 1;
                break;
            case PieceType::BISHOP:
            case PieceType::KNIGHT:
                piece_score += 3;
                break;
            case PieceType::ROOK:
                piece_score += 5;
                break;
            case PieceType::QUEEN:
                piece_score += 9;
                break;
            default:
                break;
        }
        if (PieceSide(p) == Side::BLACK) piece_score *= -1;
        score += piece_score;
    }
    return score;
}
