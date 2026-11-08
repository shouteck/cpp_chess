#include "board.h"
#include <iostream>

int main() {

    Position a;
    a.SetUpStartingPosition();
    a.PrintBoard();

    std::cout << "----" << "\n";

    Position b;
    b.FromFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    b.PrintBoard();

    return 0;
}
