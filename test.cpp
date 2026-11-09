#include "board.h"
#include <iostream>
#include <string>

static int g_failures = 0;

static void Check(bool condition, const std::string& name) {
    if (condition) {
        std::cout << "  ok   " << name << "\n";
    } else {
        std::cout << "  FAIL " << name << "\n";
        g_failures++;
    }
}

static void TestValidFens() {
    std::cout << "== valid FENs accepted ==\n";
    const char* cases[] = {
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
        "rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPP1PPP/RNBQKBNR w KQkq e6 0 2",
        "rnbqkbnr/ppp1pppp/8/3p4/8/8/PPPPPPPP/RNBQKBNR w KQkq d6 0 2",
        "8/8/8/8/8/8/8/K6k w - - 0 1",
        "r3k2r/8/8/8/8/8/8/R3K2R b KQkq - 99 42",
        "4k3/8/8/8/8/8/4P3/4K3 b - - 12 34",
        "8/8/8/8/8/8/3P4/8 w - - 0 1",
        "8/8/8/8/8/8/8/K6k b Kq - 7 9",
        "8/8/8/8/8/8/8/K6k w k - 0 1",
    };
    for (const char* fen : cases) {
        Position p;
        Check(p.FromFen(fen), fen);
    }
}

static void TestStartposValues() {
    std::cout << "== startpos field values ==\n";
    Position p;
    Check(p.FromFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"), "parse");
    Check(p.GetPiece(0, 0) == BLACK_ROOK, "a8 = black rook");
    Check(p.GetPiece(0, 4) == BLACK_KING, "e8 = black king");
    Check(p.GetPiece(1, 4) == BLACK_PAWN, "e7 = black pawn");
    Check(p.GetPiece(6, 4) == WHITE_PAWN, "e2 = white pawn");
    Check(p.GetPiece(7, 0) == WHITE_ROOK, "a1 = white rook");
    Check(p.GetPiece(7, 4) == WHITE_KING, "e1 = white king");
    Check(p.GetPiece(4, 4) == EMPTY, "e4 empty");
    Check(p.GetSideToMove() == WHITE, "white to move");
    Check(p.GetCastlingRights() == 15, "all castling rights");
    Check(p.GetEnPassantTarget() == -1, "no ep target");
    Check(p.GetHalfmoveClock() == 0, "halfmove 0");
    Check(p.GetFullmoveNumber() == 1, "fullmove 1");
}

static void TestMidgameValues() {
    std::cout << "== midgame field values ==\n";
    Position p;
    Check(p.FromFen("rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPP1PPP/RNBQKBNR w KQkq e6 0 2"), "parse 1.e4 e5");
    Check(p.GetPiece(3, 4) == BLACK_PAWN, "e5 = black pawn");
    Check(p.GetPiece(4, 4) == WHITE_PAWN, "e4 = white pawn");
    Check(p.GetPiece(6, 4) == EMPTY, "e2 vacated");
    Check(p.GetPiece(1, 4) == EMPTY, "e7 vacated");
    Check(p.GetEnPassantTarget() == 20, "ep e6 = index 20");
    Check(p.GetFullmoveNumber() == 2, "fullmove 2");
    Check(p.GetSideToMove() == WHITE, "white to move");

    Position q;
    Check(q.FromFen("rnbqkbnr/ppp1pppp/8/3p4/8/8/PPPPPPPP/RNBQKBNR w KQkq d6 0 2"), "parse d5");
    Check(q.GetPiece(3, 3) == BLACK_PAWN, "d5 = black pawn");
    Check(q.GetEnPassantTarget() == 19, "ep d6 = index 19 (non-e file)");
}

static void TestCastlingAndCounters() {
    std::cout << "== castling subsets and counters ==\n";
    Position p;
    Check(p.FromFen("8/8/8/8/8/8/8/K6k b Kq - 7 9"), "parse Kq");
    Check(p.GetCastlingRights() == (WHITE_KINGSIDE | BLACK_QUEENSIDE), "Kq -> 9");
    Check(p.GetSideToMove() == BLACK, "black to move");
    Check(p.GetHalfmoveClock() == 7, "halfmove 7");
    Check(p.GetFullmoveNumber() == 9, "fullmove 9");

    Position q;
    Check(q.FromFen("r3k2r/8/8/8/8/8/8/R3K2R b KQkq - 99 42"), "parse rooks");
    Check(q.GetCastlingRights() == 15, "KQkq -> 15");
    Check(q.GetHalfmoveClock() == 99, "halfmove 99");
    Check(q.GetFullmoveNumber() == 42, "fullmove 42");
    Check(q.GetPiece(0, 0) == BLACK_ROOK && q.GetPiece(0, 7) == BLACK_ROOK, "black rooks");
    Check(q.GetPiece(0, 3) == EMPTY, "b8 empty in r3k2r");

    Position r;
    Check(r.FromFen("8/8/8/8/8/8/8/K6k w - - 0 1"), "parse bare kings");
    Check(r.GetCastlingRights() == 0, "'-' castling -> 0");
    Check(r.GetEnPassantTarget() == -1, "'-' ep -> -1");
    Check(r.GetPiece(7, 0) == WHITE_KING && r.GetPiece(7, 7) == BLACK_KING, "bare kings");
}

static void TestInvalidFens() {
    std::cout << "== invalid FENs rejected ==\n";
    const char* cases[] = {
        "",
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0",
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 extra",
        "8/8/8/8/8/8/8 w - - 0 1",
        "8/8/8/8/8/8/8/8/8 w - - 0 1",
        "ppppppp/8/8/8/8/8/8/8 w - - 0 1",
        "ppppppppp/8/8/8/8/8/8/8 w - - 0 1",
        "9/8/8/8/8/8/8/8 w - - 0 1",
        "0/8/8/8/8/8/8/8 w - - 0 1",
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPX/RNBQKBNR w KQkq - 0 1",
        "8/8/8/8/8/8/8/K6k x - - 0 1",
        "8/8/8/8/8/8/8/K6k w X - 0 1",
        "8/8/8/8/8/8/8/K6k w - zz 0 1",
        "8/8/8/8/8/8/8/K6k w - - x 1",
        "8/8/8/8/8/8/8/K6k w - - 0 x",
        "8/8/8/8/8/8/8/K6k w - - 1a 1",
        "8/8/8/8/8/8/8/K6k w - - 0 1a",
    };
    for (const char* fen : cases) {
        Position p;
        Check(!p.FromFen(fen), fen);
    }
}

static void TestAtomicity() {
    std::cout << "== failed parse leaves position untouched ==\n";
    Position p;
    p.SetUpStartingPosition();

    // Fails LATE — after board, side, castling, ep, halfmove all parsed
    bool ok = p.FromFen("8/8/8/8/8/8/8/K6k w - - 0 x");
    Check(!ok, "late failure returns false");
    Check(p.GetPiece(0, 0) == BLACK_ROOK, "board row 0 intact");
    Check(p.GetPiece(7, 4) == WHITE_KING, "board row 7 intact");
    Check(p.GetPiece(6, 4) == WHITE_PAWN, "pawns intact");
    Check(p.GetSideToMove() == WHITE, "side intact");
    Check(p.GetCastlingRights() == 15, "castling intact");
    Check(p.GetEnPassantTarget() == -1, "ep intact");
    Check(p.GetHalfmoveClock() == 0 && p.GetFullmoveNumber() == 1, "counters intact");
}

static void TestReuse() {
    std::cout << "== position reuse ==\n";
    Position p;
    Check(p.FromFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"), "first parse");
    Check(p.FromFen("8/8/8/8/8/8/8/K6k b - - 5 6"), "second parse");
    Check(p.GetSideToMove() == BLACK, "second parse wins");
    Check(p.GetPiece(0, 0) == EMPTY, "old board gone");
    Check(p.GetPiece(7, 0) == WHITE_KING, "new board present");
    Check(p.GetHalfmoveClock() == 5 && p.GetFullmoveNumber() == 6, "new counters");
}

int main() {
    TestValidFens();
    TestStartposValues();
    TestMidgameValues();
    TestCastlingAndCounters();
    TestInvalidFens();
    TestAtomicity();
    TestReuse();

    if (g_failures == 0) {
        std::cout << "\nALL TESTS PASSED\n";
        return 0;
    }
    std::cout << "\n" << g_failures << " FAILURE(S)\n";
    return 1;
}
