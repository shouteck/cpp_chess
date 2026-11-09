#include "board.h"
#include <chrono>
#include <iostream>

char PieceToCharIfElse(Piece piece) {
    switch (piece) {
        case Piece::EMPTY:        return '.';
        case Piece::WHITE_PAWN:   return 'P';
        case Piece::WHITE_KNIGHT: return 'N';
        case Piece::WHITE_BISHOP: return 'B';
        case Piece::WHITE_ROOK:   return 'R';
        case Piece::WHITE_QUEEN:  return 'Q';
        case Piece::WHITE_KING:   return 'K';
        case Piece::BLACK_PAWN:   return 'p';
        case Piece::BLACK_KNIGHT: return 'n';
        case Piece::BLACK_BISHOP: return 'b';
        case Piece::BLACK_ROOK:   return 'r';
        case Piece::BLACK_QUEEN:  return 'q';
        case Piece::BLACK_KING:   return 'k';
    }
    return '?';  // unreachable, silences warnings
}

char PieceToCharTable(Piece piece) {

    static constexpr char kPieceChars[] = {
        '.',  // EMPTY
        'P', 'N', 'B', 'R', 'Q', 'K',        // WHITE_PAWN .. WHITE_KING
        'p', 'n', 'b', 'r', 'q', 'k',        // BLACK_PAWN .. BLACK_KING
    };

    return kPieceChars[static_cast<size_t>(piece)];
}

int main() {

    static constexpr Piece kAllPieces[] = {
        Piece::EMPTY,
        Piece::WHITE_PAWN, Piece::WHITE_KNIGHT, Piece::WHITE_BISHOP, Piece::WHITE_ROOK, Piece::WHITE_QUEEN, Piece::WHITE_KING,
        Piece::BLACK_PAWN, Piece::BLACK_KNIGHT, Piece::BLACK_BISHOP, Piece::BLACK_ROOK, Piece::BLACK_QUEEN, Piece::BLACK_KING,
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
