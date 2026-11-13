#pragma once

#include "board.h"

struct Move {
    int from;
    int to;
    Piece promo = Piece::EMPTY;
};
