#include "board.h"
#include <iostream>

void Position::SetUpStartingPosition() {
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            board[i][j] = 0;
        }
    }
}

void Position::PrintBoard() {
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            std::cout << board[i][j];
            std::cout << " ";
        }
        std::cout << "\n";
    }
}
