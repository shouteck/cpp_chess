#include "board.h"
#include <iostream>

int main() {

    Position a;
    a.SetUpStartingPosition();
    a.PrintBoard();

    std::cout << "----" << "\n";

    Position b;
    if (!b.FromFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1")) {
        std::cout << "invalid FEN\n";
        return 1;
    }
    b.PrintBoard();

    return 0;
}
