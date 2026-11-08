#include "board.h"
#include <iostream>

void Position::SetUpStartingPosition() {
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            board[i][j] = EMPTY;
        }
    }
}

static char PieceToCharTable(Piece piece) {

    static constexpr char kPieceChars[] = {
        '.',  // EMPTY
        'P', 'N', 'B', 'R', 'Q', 'K',        // WHITE_PAWN .. WHITE_KING
        'p', 'n', 'b', 'r', 'q', 'k',        // BLACK_PAWN .. BLACK_KING
    };

    return kPieceChars[piece];
}

void Position::PrintBoard() {
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            std::cout << PieceToCharTable(board[i][j]);
            std::cout << " ";
        }
        std::cout << "\n";
    }
}
