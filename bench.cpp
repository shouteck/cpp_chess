#include "board.h"
#include <chrono>
#include <iostream>

char PieceToCharIfElse(Piece piece) {
    switch (piece) {
        case EMPTY:        return '.';
        case WHITE_PAWN:   return 'P';
        case WHITE_KNIGHT: return 'N';
        case WHITE_BISHOP: return 'B';
        case WHITE_ROOK:   return 'R';
        case WHITE_QUEEN:  return 'Q';
        case WHITE_KING:   return 'K';
        case BLACK_PAWN:   return 'p';
        case BLACK_KNIGHT: return 'n';
        case BLACK_BISHOP: return 'b';
        case BLACK_ROOK:   return 'r';
        case BLACK_QUEEN:  return 'q';
        case BLACK_KING:   return 'k';
    }
    return '?';  // unreachable, silences warnings
}

char PieceToCharTable(Piece piece) {

    static constexpr char kPieceChars[] = {
        '.',  // EMPTY
        'P', 'N', 'B', 'R', 'Q', 'K',        // WHITE_PAWN .. WHITE_KING
        'p', 'n', 'b', 'r', 'q', 'k',        // BLACK_PAWN .. BLACK_KING
    };

    return kPieceChars[piece];
}

int main() {

    static constexpr Piece kAllPieces[] = {
        EMPTY,
        WHITE_PAWN, WHITE_KNIGHT, WHITE_BISHOP, WHITE_ROOK, WHITE_QUEEN, WHITE_KING,
        BLACK_PAWN, BLACK_KNIGHT, BLACK_BISHOP, BLACK_ROOK, BLACK_QUEEN, BLACK_KING,
    };

    auto start = std::chrono::steady_clock::now();
    int currentPieceIndex = 0;
    long long checksum = 0;
    for (int i = 0; i < 100000000; i++) {
        char symbol = PieceToCharIfElse(kAllPieces[currentPieceIndex]);
        checksum += symbol;
        currentPieceIndex = (currentPieceIndex + 1) % 13;
    }
    auto end = std::chrono::steady_clock::now();
    std::cout << "if-else elapsed: " << std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count() << " ns\n";
    std::cout << "if-else checksum: " << checksum << "\n";

    start = std::chrono::steady_clock::now();
    currentPieceIndex = 0;
    checksum = 0;
    for (int i = 0; i < 100000000; i++) {
        char symbol = PieceToCharTable(kAllPieces[currentPieceIndex]);
        checksum += symbol;
        currentPieceIndex = (currentPieceIndex + 1) % 13;
    }
    end = std::chrono::steady_clock::now();
    std::cout << "table elapsed: " << std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count() << " ns\n";
    std::cout << "table checksum: " << checksum << "\n";

    return 0;
}
