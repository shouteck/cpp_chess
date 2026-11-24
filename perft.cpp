#include "perft.h"
#include "movegen.h"
#include "board.h"

std::uint64_t Perft(Position& pos, int depth) {
    if (depth == 0) return 1;
    std::uint64_t nodes = 0;
    for (const auto& m : GenerateLegalMoves(pos)) {
        UndoInfo ui = pos.MakeMove(m);
        nodes += Perft(pos, depth - 1);
        pos.UnmakeMove(m, ui);
    }
    return nodes;
}
