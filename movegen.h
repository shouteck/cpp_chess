#include "board.h"
#include "move.h"
#include <vector>

std::vector<Move> GenerateMoves(const Position& pos);
bool IsSquareAttacked(const Position& pos, int sq, Side by_side);
