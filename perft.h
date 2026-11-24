#pragma once

#include <cstdint>

class Position;

std::uint64_t Perft(Position& pos, int depth);
