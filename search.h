#pragma once

#include "move.h"
#include <optional>

std::optional<Move>FindBestMove(Position& pos, int depth);
