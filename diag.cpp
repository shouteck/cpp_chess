#include "board.h"
#include "movegen.h"
#include <cstdio>
int main() {
    Position p;
    p.FromFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    auto m = GenerateMoves(p);
    printf("count=%zu\n", m.size());
    for (auto& mv : m) printf("%d->%d kind=%d\n", mv.from, mv.to, (int)mv.kind);
}
