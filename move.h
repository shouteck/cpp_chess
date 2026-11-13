#pragma once

#include "board.h"

enum class MoveKind {
    NORMAL, EN_PASSANT, CASTLE_KINGSIDE, CASTLE_QUEENSIDE,
};

struct Move {
    int from;
    int to;
    Piece promo = Piece::EMPTY;
    MoveKind kind = MoveKind::NORMAL;
};
