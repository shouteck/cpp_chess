#include "board.h"
#include "eval.h"
#include "movegen.h"
#include "perft.h"
#include "search.h"
#include <iostream>
#include <optional>
#include <string>
#include <vector>

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
        "4k3/8/8/8/3pP3/8/8/4K3 b - e3 0 2",
        "8/8/8/8/8/8/8/K6k w - - 2147483647 1",
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
    Check(p.GetPiece(0, 0) == Piece::BLACK_ROOK, "a8 = black rook");
    Check(p.GetPiece(0, 4) == Piece::BLACK_KING, "e8 = black king");
    Check(p.GetPiece(1, 4) == Piece::BLACK_PAWN, "e7 = black pawn");
    Check(p.GetPiece(6, 4) == Piece::WHITE_PAWN, "e2 = white pawn");
    Check(p.GetPiece(7, 0) == Piece::WHITE_ROOK, "a1 = white rook");
    Check(p.GetPiece(7, 4) == Piece::WHITE_KING, "e1 = white king");
    Check(p.GetPiece(4, 4) == Piece::EMPTY, "e4 empty");
    Check(p.GetSideToMove() == Side::WHITE, "white to move");
    Check(p.GetCastlingRights() == 15, "all castling rights");
    Check(p.GetEnPassantTarget() == std::nullopt, "no ep target");
    Check(p.GetHalfmoveClock() == 0, "halfmove 0");
    Check(p.GetFullmoveNumber() == 1, "fullmove 1");
}

static void TestMidgameValues() {
    std::cout << "== midgame field values ==\n";
    Position p;
    Check(p.FromFen("rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPP1PPP/RNBQKBNR w KQkq e6 0 2"), "parse 1.e4 e5");
    Check(p.GetPiece(3, 4) == Piece::BLACK_PAWN, "e5 = black pawn");
    Check(p.GetPiece(4, 4) == Piece::WHITE_PAWN, "e4 = white pawn");
    Check(p.GetPiece(6, 4) == Piece::EMPTY, "e2 vacated");
    Check(p.GetPiece(1, 4) == Piece::EMPTY, "e7 vacated");
    Check(p.GetEnPassantTarget() == 20, "ep e6 = index 20");
    Check(p.GetFullmoveNumber() == 2, "fullmove 2");
    Check(p.GetSideToMove() == Side::WHITE, "white to move");

    Position q;
    Check(q.FromFen("rnbqkbnr/ppp1pppp/8/3p4/8/8/PPPPPPPP/RNBQKBNR w KQkq d6 0 2"), "parse d5");
    Check(q.GetPiece(3, 3) == Piece::BLACK_PAWN, "d5 = black pawn");
    Check(q.GetEnPassantTarget() == 19, "ep d6 = index 19 (non-e file)");

    Position r;
    Check(r.FromFen("4k3/8/8/8/3pP3/8/8/4K3 b - e3 0 2"), "parse rank-3 ep");
    Check(r.GetSideToMove() == Side::BLACK, "black to move");
    Check(r.GetEnPassantTarget() == 44, "ep e3 = index 44 (rank 3)");
}

static void TestCastlingAndCounters() {
    std::cout << "== castling subsets and counters ==\n";
    Position p;
    Check(p.FromFen("8/8/8/8/8/8/8/K6k b Kq - 7 9"), "parse Kq");
    Check(p.GetCastlingRights() == (WHITE_KINGSIDE | BLACK_QUEENSIDE), "Kq -> 9");
    Check(p.GetSideToMove() == Side::BLACK, "black to move");
    Check(p.GetHalfmoveClock() == 7, "halfmove 7");
    Check(p.GetFullmoveNumber() == 9, "fullmove 9");

    Position q;
    Check(q.FromFen("r3k2r/8/8/8/8/8/8/R3K2R b KQkq - 99 42"), "parse rooks");
    Check(q.GetCastlingRights() == 15, "KQkq -> 15");
    Check(q.GetHalfmoveClock() == 99, "halfmove 99");
    Check(q.GetFullmoveNumber() == 42, "fullmove 42");
    Check(q.GetPiece(0, 0) == Piece::BLACK_ROOK && q.GetPiece(0, 7) == Piece::BLACK_ROOK, "black rooks");
    Check(q.GetPiece(0, 3) == Piece::EMPTY, "b8 empty in r3k2r");

    Position r;
    Check(r.FromFen("8/8/8/8/8/8/8/K6k w - - 0 1"), "parse bare kings");
    Check(r.GetCastlingRights() == 0, "'-' castling -> 0");
    Check(r.GetEnPassantTarget() == std::nullopt, "'-' ep -> -1");
    Check(r.GetPiece(7, 0) == Piece::WHITE_KING && r.GetPiece(7, 7) == Piece::BLACK_KING, "bare kings");
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
        "8/8/8/8/8/8/8/K6k w - - -1 1",
        "8/8/8/8/8/8/8/K6k w - - 99999999999 1",
        "8/8/8/8/8/8/8/K6k w - - 0 99999999999",
        "8/8/8/8/8/8/8/K6k w - - 2147483648 1",
        "8/8/8/8/8/8/8/K6k w - e 0 1",
        "8/8/8/8/8/8/8/K6k w - e6x 0 1",
        "8/8/8/8/8/8/8/K6k w - z6 0 1",
        "8/8/8/8/8/8/8/K6k w - 33 0 1",
        "8/8/8/8/8/8/8/K6k w - e4 0 1",
        "8/8/8/8/8/8/8/K6k w - a1 0 1",
        "8/8/8/8/8/8/8/K6k w - h8 0 1",
        "8/8/8/8/8/8/8/K6k w  Kq - 0 1",
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 ",
        "6p/8/8/8/8/8/8/K6k w - - 0 1",
        "8/8/8/8/8/8/8/8/ w - - 0 1",
        "/8/8/8/8/8/8/8/K6k w - - 0 1",
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
    Check(p.GetPiece(0, 0) == Piece::BLACK_ROOK, "board row 0 intact");
    Check(p.GetPiece(7, 4) == Piece::WHITE_KING, "board row 7 intact");
    Check(p.GetPiece(6, 4) == Piece::WHITE_PAWN, "pawns intact");
    Check(p.GetSideToMove() == Side::WHITE, "side intact");
    Check(p.GetCastlingRights() == 15, "castling intact");
    Check(p.GetEnPassantTarget() == std::nullopt, "ep intact");
    Check(p.GetHalfmoveClock() == 0 && p.GetFullmoveNumber() == 1, "counters intact");
}

static void TestReuse() {
    std::cout << "== position reuse ==\n";
    Position p;
    Check(p.FromFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"), "first parse");
    Check(p.FromFen("8/8/8/8/8/8/8/K6k b - - 5 6"), "second parse");
    Check(p.GetSideToMove() == Side::BLACK, "second parse wins");
    Check(p.GetPiece(0, 0) == Piece::EMPTY, "old board gone");
    Check(p.GetPiece(7, 0) == Piece::WHITE_KING, "new board present");
    Check(p.GetHalfmoveClock() == 5 && p.GetFullmoveNumber() == 6, "new counters");
}

static bool ContainsMove(const std::vector<Move>& moves, int from, int to) {
    for (const Move& m : moves) {
        if (m.from == from && m.to == to) return true;
    }
    return false;
}

static int CountFrom(const std::vector<Move>& moves, int from) {
    int count = 0;
    for (const Move& m : moves) {
        if (m.from == from) count++;
    }
    return count;
}

static bool ContainsPromoMove(const std::vector<Move>& moves, int from, int to, Piece promo) {
    for (const Move& m : moves) {
        if (m.from == from && m.to == to && m.promo == promo) return true;
    }
    return false;
}

static bool ContainsKindMove(const std::vector<Move>& moves, int from, int to, MoveKind kind) {
    for (const Move& m : moves) {
        if (m.from == from && m.to == to && m.kind == kind) return true;
    }
    return false;
}

static void TestKnightMoves() {
    std::cout << "== knight movegen ==\n";

    Position p;
    Check(p.FromFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"), "parse startpos");
    std::vector<Move> moves = GenerateMoves(p);
    Check(moves.size() == 20, "startpos has 20 pseudo-legal moves");
    Check(CountFrom(moves, 57) == 2, "Nb1 has 2 moves");
    Check(CountFrom(moves, 62) == 2, "Ng1 has 2 moves");
    Check(ContainsMove(moves, 57, 40), "Nb1 -> a3 (57->40)");
    Check(ContainsMove(moves, 57, 42), "Nb1 -> c3 (57->42)");
    Check(ContainsMove(moves, 62, 45), "Ng1 -> f3 (62->45)");
    Check(ContainsMove(moves, 62, 47), "Ng1 -> h3 (62->47)");

    Position q;
    Check(q.FromFen("4k3/8/8/8/4N3/8/8/4K3 w - - 0 1"), "parse center knight");
    std::vector<Move> qmoves = GenerateMoves(q);
    Check(CountFrom(qmoves, 36) == 8, "center knight (e4) has all 8 moves");
    Check(ContainsMove(qmoves, 36, 19), "Ne4 -> c5 (36->19)");
    Check(ContainsMove(qmoves, 36, 53), "Ne4 -> f2 (36->53)");
    Check(CountFrom(qmoves, 60) == 5, "corner-edge king (e1) has 5 moves");
    Check(ContainsMove(qmoves, 60, 51), "Ke1 -> d2 (60->51)");
    Check(ContainsMove(qmoves, 60, 53), "Ke1 -> f2 (60->53)");
}

static void TestPawnMoves() {
    std::cout << "== pawn movegen ==\n";

    Position p;
    Check(p.FromFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"), "parse startpos");
    std::vector<Move> moves = GenerateMoves(p);
    int pawn_moves = 0;
    for (int sq = 48; sq <= 55; sq++) pawn_moves += CountFrom(moves, sq);
    Check(pawn_moves == 16, "startpos: 8 white pawns x 2 moves");
    Check(ContainsMove(moves, 48, 40), "Pa2 -> a3 (48->40)");
    Check(ContainsMove(moves, 48, 32), "Pa2 -> a4 (48->32)");
    Check(ContainsMove(moves, 55, 39), "Ph2 -> h4 (55->39)");

    Position cap;
    Check(cap.FromFen("4k3/8/8/3p4/4P3/8/8/4K3 w - - 0 1"), "parse capture pos");
    std::vector<Move> cmoves = GenerateMoves(cap);
    Check(CountFrom(cmoves, 36) == 2, "Pe4: push + capture d5");
    Check(ContainsMove(cmoves, 36, 28), "Pe4 -> e5 (36->28)");
    Check(ContainsMove(cmoves, 36, 27), "Pe4xd5 (36->27)");
    Check(!ContainsMove(cmoves, 36, 29), "no capture on empty f5 (36->29)");

    Position blk;
    Check(blk.FromFen("4k3/8/8/8/8/4p3/4P3/4K3 w - - 0 1"), "parse blocked pos");
    std::vector<Move> bmoves = GenerateMoves(blk);
    Check(CountFrom(bmoves, 52) == 0, "blocked pawn has no moves");
    Check(CountFrom(bmoves, 60) == 4, "Ke1 has 4 moves (e2 occupied)");

    Position ep;
    Check(ep.FromFen("4k3/8/8/3pP3/8/8/8/4K3 w - d6 0 2"), "parse ep pos");
    std::vector<Move> emoves = GenerateMoves(ep);
    Check(CountFrom(emoves, 28) == 2, "Pe5: push + ep capture");
    Check(ContainsMove(emoves, 28, 20), "Pe5 -> e6 (28->20)");
    Check(ContainsMove(emoves, 28, 19), "Pe5xd6 ep (28->19)");

    std::cout << "== black pawn movegen ==\n";

    Position bstart;
    Check(bstart.FromFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR b KQkq - 0 1"), "parse black startpos");
    std::vector<Move> bs = GenerateMoves(bstart);
    Check(bs.size() == 20, "black startpos has 20 pseudo-legal moves");
    int bpawn_moves = 0;
    for (int sq = 8; sq <= 15; sq++) bpawn_moves += CountFrom(bs, sq);
    Check(bpawn_moves == 16, "black startpos: 8 pawns x 2 moves");
    Check(ContainsMove(bs, 8, 16), "Pa7 -> a6 (8->16)");
    Check(ContainsMove(bs, 8, 24), "Pa7 -> a5 (8->24)");
    Check(ContainsMove(bs, 15, 31), "Ph7 -> h5 (15->31)");

    Position bcap;
    Check(bcap.FromFen("4k3/8/8/4p3/3P4/8/8/4K3 b - - 0 1"), "parse black capture pos");
    std::vector<Move> bcm = GenerateMoves(bcap);
    Check(CountFrom(bcm, 28) == 2, "Pe5: push + capture d4");
    Check(ContainsMove(bcm, 28, 36), "Pe5 -> e4 (28->36)");
    Check(ContainsMove(bcm, 28, 35), "Pe5xd4 (28->35)");
    Check(!ContainsMove(bcm, 28, 37), "no capture on empty f4 (28->37)");

    Position bblk;
    Check(bblk.FromFen("4k3/4p3/4P3/8/8/8/8/4K3 b - - 0 1"), "parse black blocked pos");
    std::vector<Move> bbm = GenerateMoves(bblk);
    Check(CountFrom(bbm, 12) == 0, "blocked black pawn has no moves");
    Check(CountFrom(bbm, 4) == 4, "ke8 has 4 moves (e7 occupied)");

    Position bep;
    Check(bep.FromFen("4k3/8/8/8/3Pp3/8/8/4K3 b - d3 0 2"), "parse black ep pos");
    std::vector<Move> bem = GenerateMoves(bep);
    Check(CountFrom(bem, 36) == 2, "Pe4: push + ep capture");
    Check(ContainsMove(bem, 36, 44), "Pe4 -> e3 (36->44)");
    Check(ContainsMove(bem, 36, 43), "Pe4xd3 ep (36->43)");

    Position edge;
    Check(edge.FromFen("4k3/8/8/8/7P/8/8/4K3 w - - 0 1"), "parse h-file edge pos");
    std::vector<Move> edge_moves = GenerateMoves(edge);
    Check(CountFrom(edge_moves, 39) == 1, "h4 pawn has only the push");
    Check(ContainsMove(edge_moves, 39, 31), "Ph4 -> h5 (39->31)");
    Check(!ContainsMove(edge_moves, 39, 32), "no file-wrap capture onto a4 (39->32)");

    Position edge_a;
    Check(edge_a.FromFen("4k3/8/8/8/P7/8/8/4K3 w - - 0 1"), "parse a-file edge pos");
    std::vector<Move> edge_a_moves = GenerateMoves(edge_a);
    Check(CountFrom(edge_a_moves, 32) == 1, "a4 pawn has only the push");
    Check(ContainsMove(edge_a_moves, 32, 24), "Pa4 -> a5 (32->24)");
    Check(!ContainsMove(edge_a_moves, 32, 23), "no file-wrap capture onto h6 (32->23)");
}

static void TestPromotionMoves() {
    std::cout << "== promotion movegen ==\n";

    Position p;
    Check(p.FromFen("k7/4P3/8/8/8/8/8/4K3 w - - 0 1"), "parse white promo pos");
    std::vector<Move> moves = GenerateMoves(p);
    Check(CountFrom(moves, 12) == 4, "Pe7 -> e8 yields 4 promo moves");
    Check(ContainsPromoMove(moves, 12, 4, Piece::WHITE_QUEEN), "e8=Q (12->4)");
    Check(ContainsPromoMove(moves, 12, 4, Piece::WHITE_KNIGHT), "e8=N (12->4)");
    Check(!ContainsPromoMove(moves, 12, 4, Piece::EMPTY), "promo never EMPTY");

    Position cap;
    Check(cap.FromFen("k3r3/3P4/8/8/8/8/8/4K3 w - - 0 1"), "parse promo capture pos");
    std::vector<Move> cm = GenerateMoves(cap);
    Check(CountFrom(cm, 11) == 8, "Pd7: 4 push-promos + 4 capture-promos");
    Check(ContainsPromoMove(cm, 11, 4, Piece::WHITE_QUEEN), "d7xe8=Q (11->4)");
    Check(ContainsPromoMove(cm, 11, 3, Piece::WHITE_ROOK), "d8=R push-promo (11->3)");

    Position b;
    Check(b.FromFen("4k3/8/8/8/8/8/4p3/K7 b - - 0 1"), "parse black promo pos");
    std::vector<Move> bm = GenerateMoves(b);
    Check(CountFrom(bm, 52) == 4, "pe2 -> e1 yields 4 promo moves");
    Check(ContainsPromoMove(bm, 52, 60, Piece::BLACK_QUEEN), "e1=q (52->60)");
    Check(ContainsPromoMove(bm, 52, 60, Piece::BLACK_BISHOP), "e1=b (52->60)");
}

static void TestCastleMoves() {
    std::cout << "== castle movegen ==\n";

    Position open;
    Check(open.FromFen("r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1"), "parse open castle pos");
    std::vector<Move> m = GenerateMoves(open);
    Check(ContainsKindMove(m, 60, 62, MoveKind::CASTLE_KINGSIDE), "O-O: e1->g1 (60->62)");
    Check(ContainsKindMove(m, 60, 58, MoveKind::CASTLE_QUEENSIDE), "O-O-O: e1->c1 (60->58)");

    Position bopen;
    Check(bopen.FromFen("r3k2r/8/8/8/8/8/8/R3K2R b KQkq - 0 1"), "parse black open castle pos");
    std::vector<Move> bm = GenerateMoves(bopen);
    Check(ContainsKindMove(bm, 4, 6, MoveKind::CASTLE_KINGSIDE), "O-O: e8->g8 (4->6)");
    Check(ContainsKindMove(bm, 4, 2, MoveKind::CASTLE_QUEENSIDE), "O-O-O: e8->c8 (4->2)");

    Position partial;
    Check(partial.FromFen("r3k2r/8/8/8/8/8/8/R3K2R w K - 0 1"), "parse K-only rights");
    std::vector<Move> pm = GenerateMoves(partial);
    Check(ContainsKindMove(pm, 60, 62, MoveKind::CASTLE_KINGSIDE), "K right: kingside available");
    Check(!ContainsKindMove(pm, 60, 58, MoveKind::CASTLE_QUEENSIDE), "K right: queenside absent");

    Position blocked;
    Check(blocked.FromFen("r3k2r/8/8/8/8/8/8/R3K1NR w KQkq - 0 1"), "parse blocked castle pos");
    std::vector<Move> km = GenerateMoves(blocked);
    Check(!ContainsKindMove(km, 60, 62, MoveKind::CASTLE_KINGSIDE), "g1 occupied: no kingside");
    Check(ContainsKindMove(km, 60, 58, MoveKind::CASTLE_QUEENSIDE), "g1 occupied: queenside still available");

    Position wrongside;
    Check(wrongside.FromFen("r3k2r/8/8/8/8/8/8/R3K2R b KQkq - 0 1"), "black cannot use white rights");
    std::vector<Move> wm = GenerateMoves(wrongside);
    Check(!ContainsKindMove(wm, 60, 62, MoveKind::CASTLE_KINGSIDE), "black turn: no white castle");
}

static void TestMakeMove() {
    std::cout << "== make move ==\n";

    // double push: relocate, ep target = skipped square, halfmove reset
    Position p;
    Check(p.FromFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"), "parse startpos");
    (void)p.MakeMove(Move{52, 36, Piece::EMPTY, MoveKind::DOUBLE_PUSH});  // e2->e4
    Check(p.GetPiece(36) == Piece::WHITE_PAWN && p.GetPiece(52) == Piece::EMPTY, "e2->e4 relocates");
    Check(p.GetEnPassantTarget() == std::optional<int>(44), "double push sets ep=e3 (44)");
    Check(p.GetHalfmoveClock() == 0 && p.GetFullmoveNumber() == 1, "pawn push: halfmove 0, fullmove 1");
    Check(p.GetSideToMove() == Side::BLACK, "side flips to black");

    // black double push: ep + fullmove increment
    Position bp;
    Check(bp.FromFen("4k3/3p4/8/8/8/8/8/4K3 b - - 0 1"), "parse black pawn pos");
    (void)bp.MakeMove(Move{11, 27, Piece::EMPTY, MoveKind::DOUBLE_PUSH});  // d7->d5
    Check(bp.GetEnPassantTarget() == std::optional<int>(19), "black push sets ep=d6 (19)");
    Check(bp.GetFullmoveNumber() == 2 && bp.GetSideToMove() == Side::WHITE, "fullmove++ after black");

    // normal move: clears ep, halfmove +1
    Position nm;
    Check(nm.FromFen("4k3/8/8/3p4/8/8/4P3/4K3 w - d6 0 1"), "parse pos with ep");
    (void)nm.MakeMove(Move{52, 44});  // e2->e3
    Check(!nm.GetEnPassantTarget().has_value(), "normal move clears ep target");

    Position nk;
    Check(nk.FromFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"), "parse startpos 2");
    (void)nk.MakeMove(Move{62, 45});  // Ng1->f3
    Check(nk.GetPiece(45) == Piece::WHITE_KNIGHT && nk.GetPiece(62) == Piece::EMPTY, "Ng1->f3 relocates");
    Check(nk.GetHalfmoveClock() == 1, "knight move: halfmove +1");

    // capture: victim snapshotted, halfmove reset
    Position cap;
    Check(cap.FromFen("4k3/8/8/3p4/8/8/8/4K2Q w - - 5 1"), "parse capture pos");
    UndoInfo capu = cap.MakeMove(Move{63, 27});  // Qh1 captures d5
    Check(cap.GetPiece(27) == Piece::WHITE_QUEEN && capu.captured == Piece::BLACK_PAWN, "capture: victim recorded");
    Check(cap.GetHalfmoveClock() == 0, "capture resets halfmove");

    // en passant capture: victim behind 'to' removed
    Position epp;
    Check(epp.FromFen("4k3/8/8/3pP3/8/8/8/4K3 w - d6 0 2"), "parse ep capture pos");
    UndoInfo epu = epp.MakeMove(Move{28, 19, Piece::EMPTY, MoveKind::EN_PASSANT});  // e5xd6 e.p.
    Check(epp.GetPiece(19) == Piece::WHITE_PAWN, "ep: capturer lands on d6");
    Check(epp.GetPiece(27) == Piece::EMPTY, "ep: victim removed from d5");
    Check(epu.captured == Piece::BLACK_PAWN, "ep: undo.captured = victim pawn");
    Check(!epp.GetEnPassantTarget().has_value(), "ep move clears ep target");

    // castling: king + rook both relocate
    Position wk;
    Check(wk.FromFen("r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1"), "parse castle pos");
    (void)wk.MakeMove(Move{60, 62, Piece::EMPTY, MoveKind::CASTLE_KINGSIDE});
    Check(wk.GetPiece(62) == Piece::WHITE_KING && wk.GetPiece(61) == Piece::WHITE_ROOK, "O-O: K on g1, R on f1");
    Check(wk.GetPiece(60) == Piece::EMPTY && wk.GetPiece(63) == Piece::EMPTY, "O-O: e1,h1 vacated");
    Check(wk.GetCastlingRights() == (BLACK_KINGSIDE | BLACK_QUEENSIDE), "O-O: white rights cleared");

    Position wq;
    Check(wq.FromFen("r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1"), "parse castle pos 2");
    (void)wq.MakeMove(Move{60, 58, Piece::EMPTY, MoveKind::CASTLE_QUEENSIDE});
    Check(wq.GetPiece(58) == Piece::WHITE_KING && wq.GetPiece(59) == Piece::WHITE_ROOK, "O-O-O: K on c1, R on d1");
    Check(wq.GetPiece(60) == Piece::EMPTY && wq.GetPiece(56) == Piece::EMPTY, "O-O-O: e1,a1 vacated");

    Position bk;
    Check(bk.FromFen("r3k2r/8/8/8/8/8/8/R3K2R b KQkq - 0 1"), "parse black castle pos");
    (void)bk.MakeMove(Move{4, 6, Piece::EMPTY, MoveKind::CASTLE_KINGSIDE});
    Check(bk.GetPiece(6) == Piece::BLACK_KING && bk.GetPiece(5) == Piece::BLACK_ROOK, "black O-O: k on g8, r on f8");
    Check(bk.GetCastlingRights() == (WHITE_KINGSIDE | WHITE_QUEENSIDE), "black O-O: black rights cleared");

    // rights decay: king move kills both, rook move kills one, rook capture kills theirs
    Position km;
    Check(km.FromFen("r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1"), "parse rights pos");
    (void)km.MakeMove(Move{60, 59});  // Ke1->d1
    Check(km.GetCastlingRights() == (BLACK_KINGSIDE | BLACK_QUEENSIDE), "king move: both white bits die");

    Position rm;
    Check(rm.FromFen("r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1"), "parse rook pos");
    (void)rm.MakeMove(Move{63, 55});  // Rh1->h2
    Check(rm.GetCastlingRights() == (WHITE_QUEENSIDE | BLACK_KINGSIDE | BLACK_QUEENSIDE), "rook h1: only K dies");

    Position rc;
    Check(rc.FromFen("r3k3/8/8/8/8/8/8/4K2R w kq - 0 1"), "parse rook-capture pos");
    (void)rc.MakeMove(Move{63, 7});  // Rxh8
    Check(rc.GetCastlingRights() == BLACK_QUEENSIDE, "capture h8: black k dies, q survives");

    // promotion: promo piece lands, pawn gone
    Position pr;
    Check(pr.FromFen("4k3/4P3/8/8/8/8/8/4K3 w - - 0 1"), "parse promo pos");
    (void)pr.MakeMove(Move{12, 4, Piece::WHITE_QUEEN});  // e7->e8=Q
    Check(pr.GetPiece(4) == Piece::WHITE_QUEEN && pr.GetPiece(12) == Piece::EMPTY, "promotion: queen lands on e8");
}

static void CheckPositionsEqual(const Position& a, const Position& b, const std::string& name) {
    bool board_eq = true;
    for (int sq = 0; sq < kSquareCount; sq++)
        if (a.GetPiece(sq) != b.GetPiece(sq)) board_eq = false;
    Check(board_eq, name + ": board");
    Check(a.GetSideToMove() == b.GetSideToMove(), name + ": side");
    Check(a.GetCastlingRights() == b.GetCastlingRights(), name + ": rights");
    Check(a.GetEnPassantTarget() == b.GetEnPassantTarget(), name + ": ep");
    Check(a.GetHalfmoveClock() == b.GetHalfmoveClock(), name + ": halfmove");
    Check(a.GetFullmoveNumber() == b.GetFullmoveNumber(), name + ": fullmove");
}

static void RoundTrip(const char* fen, const Move& m, const std::string& name) {
    Position p;
    if (!p.FromFen(fen)) { Check(false, name + ": parse failed"); return; }
    Position before = p;
    UndoInfo u = p.MakeMove(m);
    p.UnmakeMove(m, u);
    CheckPositionsEqual(before, p, name);
}

static void TestUnmakeMove() {
    std::cout << "== unmake move round-trip ==\n";

    RoundTrip("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
        Move{62, 45}, "knight move");
    RoundTrip("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
        Move{52, 36, Piece::EMPTY, MoveKind::DOUBLE_PUSH}, "double push");
    RoundTrip("4k3/8/8/3p4/8/8/8/4K2Q w - - 5 1",
        Move{63, 27}, "queen capture");
    RoundTrip("4k3/8/8/3pP3/8/8/8/4K3 w - d6 0 2",
        Move{28, 19, Piece::EMPTY, MoveKind::EN_PASSANT}, "ep capture");
    RoundTrip("4k3/8/3p4/8/8/8/8/4K3 b - - 0 1",
        Move{19, 27}, "black move (fullmove)");
    RoundTrip("r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1",
        Move{60, 62, Piece::EMPTY, MoveKind::CASTLE_KINGSIDE}, "white O-O");
    RoundTrip("r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1",
        Move{60, 58, Piece::EMPTY, MoveKind::CASTLE_QUEENSIDE}, "white O-O-O");
    RoundTrip("r3k2r/8/8/8/8/8/8/R3K2R b KQkq - 0 1",
        Move{4, 6, Piece::EMPTY, MoveKind::CASTLE_KINGSIDE}, "black O-O");
    RoundTrip("4k3/4P3/8/8/8/8/8/4K3 w - - 0 1",
        Move{12, 4, Piece::WHITE_QUEEN}, "promotion");
    RoundTrip("4k3/8/8/8/8/8/8/4K2R w kq - 0 1",
        Move{63, 7}, "rook capture kills right");
    RoundTrip("4k3/8/8/3p4/8/8/4P3/4K3 w - d6 0 1",
        Move{52, 44}, "move clears ep (ep restore)");
}

static void TestSquareAttacked() {
    std::cout << "== square attacked ==\n";
    {
        Position p;
        Check(p.FromFen("4k3/8/8/8/8/8/B7/R3K2R w - - 0 1"), "parse bishop pos");
        Check(IsSquareAttacked(p, 57, Side::WHITE), "bishop a2 hits b1");
        Check(IsSquareAttacked(p, 41, Side::WHITE), "bishop a2 hits b3");
        Check(IsSquareAttacked(p, 20, Side::WHITE), "bishop a2 hits e6 mid-ray");
        Check(IsSquareAttacked(p, 6, Side::WHITE), "bishop a2 hits g8 ray end");
        Check(!IsSquareAttacked(p, 19, Side::WHITE), "bishop a2 misses d6 off-ray");
        Check(!IsSquareAttacked(p, 40, Side::WHITE), "bishop a2 blocks rook a1 ray");
        Check(!IsSquareAttacked(p, 41, Side::BLACK), "b3 not attacked by black");
    }
    {
        Position p;
        Check(p.FromFen("4k3/8/8/8/2P5/8/B7/R3K2R w - - 0 1"), "parse blocked bishop pos");
        Check(IsSquareAttacked(p, 41, Side::WHITE), "b3 still hit before blocker");
        Check(!IsSquareAttacked(p, 20, Side::WHITE), "pawn c4 blocks e6");
        Check(!IsSquareAttacked(p, 6, Side::WHITE), "pawn c4 blocks g8");
    }
    {
        Position p;
        Check(p.FromFen("4k3/8/8/8/8/8/8/4K2Q w - - 0 1"), "parse queen pos");
        Check(IsSquareAttacked(p, 7, Side::WHITE), "queen h1 hits h8 file end");
        Check(IsSquareAttacked(p, 62, Side::WHITE), "queen h1 hits g1");
        Check(IsSquareAttacked(p, 36, Side::WHITE), "queen h1 hits e4 diagonal");
        Check(IsSquareAttacked(p, 0, Side::WHITE), "queen h1 hits a8 diagonal end");
        Check(IsSquareAttacked(p, 61, Side::WHITE), "queen h1 hits f1");
        Check(!IsSquareAttacked(p, 58, Side::WHITE), "own king e1 blocks c1");
    }
    {
        Position p;
        Check(p.FromFen("4k3/8/8/8/8/8/8/r3K3 w - - 0 1"), "parse black rook pos");
        Check(IsSquareAttacked(p, 59, Side::BLACK), "black rook a1 hits d1");
        Check(IsSquareAttacked(p, 0, Side::BLACK), "black rook a1 hits a8");
        Check(!IsSquareAttacked(p, 58, Side::WHITE), "c1 not attacked by white");
        Check(!IsSquareAttacked(p, 0, Side::WHITE), "a8 not attacked by white");
    }
    {
        Position p;
        Check(p.FromFen("4k3/8/8/4P3/4p3/8/8/4K3 w - - 0 1"), "parse pawn pos");
        Check(IsSquareAttacked(p, 19, Side::WHITE), "white e5 pawn hits d6");
        Check(IsSquareAttacked(p, 21, Side::WHITE), "white e5 pawn hits f6");
        Check(!IsSquareAttacked(p, 19, Side::BLACK), "d6 not attacked by black");
        Check(IsSquareAttacked(p, 43, Side::BLACK), "black e4 pawn hits d3");
        Check(IsSquareAttacked(p, 45, Side::BLACK), "black e4 pawn hits f3");
        Check(!IsSquareAttacked(p, 36, Side::WHITE), "white pawn cannot attack backwards");
        Check(!IsSquareAttacked(p, 28, Side::BLACK), "black pawn cannot attack backwards");
    }
    {
        Position p;
        Check(p.FromFen("4k3/1P6/8/8/8/8/8/4K3 w - - 0 1"), "parse edge pawn pos");
        Check(IsSquareAttacked(p, 0, Side::WHITE), "white b7 pawn hits a8");
        Check(IsSquareAttacked(p, 2, Side::WHITE), "white b7 pawn hits c8");
    }
    {
        Position p;
        Check(p.FromFen("4k3/8/8/8/4N3/8/8/4K3 w - - 0 1"), "parse knight pos");
        Check(IsSquareAttacked(p, 19, Side::WHITE), "knight e4 hits d6");
        Check(IsSquareAttacked(p, 30, Side::WHITE), "knight e4 hits g5");
        Check(IsSquareAttacked(p, 51, Side::WHITE), "knight e4 hits d2");
        Check(!IsSquareAttacked(p, 28, Side::WHITE), "knight e4 misses e5");
        Check(!IsSquareAttacked(p, 19, Side::BLACK), "d6 not attacked by black");
    }
    {
        Position p;
        Check(p.FromFen("4k3/8/8/8/8/8/8/N3K3 w - - 0 1"), "parse corner knight pos");
        Check(IsSquareAttacked(p, 41, Side::WHITE), "knight a1 hits b3");
        Check(IsSquareAttacked(p, 50, Side::WHITE), "knight a1 hits c2");
        Check(!IsSquareAttacked(p, 48, Side::WHITE), "knight a1 misses a2");
    }
    {
        Position p;
        Check(p.FromFen("8/8/8/8/3k4/8/8/4K3 w - - 0 1"), "parse enemy king pos");
        Check(IsSquareAttacked(p, 36, Side::BLACK), "black king d4 hits e4");
        Check(IsSquareAttacked(p, 43, Side::BLACK), "black king d4 hits d3");
        Check(!IsSquareAttacked(p, 19, Side::BLACK), "d6 out of king reach");
    }
    {
        Position p;
        Check(p.FromFen("8/8/8/8/8/8/8/4Kk2 w - - 0 1"), "parse adjacent kings pos");
        Check(IsSquareAttacked(p, 60, Side::BLACK), "adjacent black king hits e1");
        Check(IsSquareAttacked(p, 61, Side::WHITE), "white king hits f1");
        Check(!IsSquareAttacked(p, 62, Side::WHITE), "g1 out of white king reach");
    }
    {
        Position p;
        Check(p.FromFen("4k3/8/8/8/8/8/8/4K3 w - - 0 1"), "parse bare kings pos");
        Check(!IsSquareAttacked(p, 36, Side::WHITE), "e4 attacked by no one (white)");
        Check(!IsSquareAttacked(p, 36, Side::BLACK), "e4 attacked by no one (black)");
    }
}

static void TestLegalMoves() {
    std::cout << "== legal moves ==\n";
    auto has = [](const std::vector<Move>& v, int from, int to,
                  MoveKind kind = MoveKind::NORMAL) {
        for (const auto& mv : v)
            if (mv.from == from && mv.to == to && mv.kind == kind) return true;
        return false;
    };
    {
        Position p;
        Check(p.FromFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"), "parse startpos");
        Check(GenerateLegalMoves(p).size() == 20, "startpos: all 20 pseudo-legal are legal");
    }
    {
        Position p;
        Check(p.FromFen("4r2k/8/8/8/8/8/4N3/4K3 w - - 0 1"), "parse pinned knight pos");
        auto legal = GenerateLegalMoves(p);
        Check(legal.size() == 4, "pinned knight: only king moves survive");
        Check(!has(legal, 52, 35), "pinned Ne2 cannot go d4");
        Check(has(legal, 60, 59), "Kd1 still legal");
    }
    {
        Position p;
        Check(p.FromFen("3r3k/8/8/8/8/8/8/4K3 w - - 0 1"), "parse king step pos");
        auto legal = GenerateLegalMoves(p);
        Check(!has(legal, 60, 59), "Kd1 rejected: d-file covered by rook");
        Check(!has(legal, 60, 51), "Kd2 rejected: still on d-file");
        Check(has(legal, 60, 61), "Kf1 legal");
        Check(has(legal, 60, 52), "Ke2 legal");
        Check(legal.size() == 3, "3 of 5 king moves legal");
    }
    {
        Position p;
        Check(p.FromFen("4r2k/8/8/8/8/8/8/4KB2 w - - 0 1"), "parse in-check pos");
        auto legal = GenerateLegalMoves(p);
        Check(has(legal, 61, 52), "Be2 blocks the check");
        Check(!has(legal, 61, 54), "Bg2 does not block: illegal");
        Check(!has(legal, 60, 52), "Ke2 stays on e-file: illegal");
        Check(has(legal, 60, 59), "Kd1 evades");
        Check(legal.size() == 4, "in check: 4 legal moves");
    }
    {
        Position p;
        Check(p.FromFen("7k/8/8/KPp4r/8/8/8/8 w - c6 0 1"), "parse ep pin pos");
        auto legal = GenerateLegalMoves(p);
        Check(!has(legal, 25, 18, MoveKind::EN_PASSANT), "ep exposes king to rook: illegal");
        Check(has(legal, 25, 17), "b6 push still legal");
    }
    {
        Position p;
        Check(p.FromFen("5r1k/8/8/8/8/8/8/R3K2R w KQ - 0 1"), "parse castle-through-check pos");
        auto legal = GenerateLegalMoves(p);
        Check(!has(legal, 60, 62, MoveKind::CASTLE_KINGSIDE), "O-O rejected: f1 attacked");
        Check(has(legal, 60, 58, MoveKind::CASTLE_QUEENSIDE), "O-O-O still legal");
    }
    {
        Position p;
        Check(p.FromFen("4r2k/8/8/8/8/8/8/R3K2R w KQ - 0 1"), "parse castle-out-of-check pos");
        auto legal = GenerateLegalMoves(p);
        Check(!has(legal, 60, 62, MoveKind::CASTLE_KINGSIDE), "O-O rejected: e1 attacked");
        Check(!has(legal, 60, 58, MoveKind::CASTLE_QUEENSIDE), "O-O-O rejected: e1 attacked");
    }
    {
        Position p;
        Check(p.FromFen("6rk/8/8/8/8/8/8/R3K2R w KQ - 0 1"), "parse castle-into-check pos");
        auto legal = GenerateLegalMoves(p);
        Check(!has(legal, 60, 62, MoveKind::CASTLE_KINGSIDE), "O-O rejected: g1 attacked");
        Check(has(legal, 60, 58, MoveKind::CASTLE_QUEENSIDE), "O-O-O still legal");
    }
    {
        Position p;
        Check(p.FromFen("1r5k/8/8/8/8/8/8/R3K2R w KQ - 0 1"), "parse b1-attacked pos");
        auto legal = GenerateLegalMoves(p);
        Check(has(legal, 60, 58, MoveKind::CASTLE_QUEENSIDE), "O-O-O legal: b1 attack irrelevant");
        Check(has(legal, 60, 62, MoveKind::CASTLE_KINGSIDE), "O-O legal too");
    }
}

static void TestPerft() {
    std::cout << "== perft ==\n";
    struct Case { const char* fen; int depth; std::uint64_t expected; };
    const Case cases[] = {
        // startpos
        {"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", 1, 20},
        {"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", 2, 400},
        {"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", 3, 8902},
        {"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", 4, 197281},
        // Kiwipete: castling, ep, promotions, pins
        {"r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1", 1, 48},
        {"r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1", 2, 2039},
        {"r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1", 3, 97862},
        // position 3: en passant edge cases
        {"8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1", 1, 14},
        {"8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1", 2, 191},
        {"8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1", 3, 2812},
        {"8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1", 4, 43238},
        // position 4: promotion-heavy
        {"r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1", 1, 6},
        {"r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1", 2, 264},
        {"r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1", 3, 9467},
        {"r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1", 4, 422333},
        // position 5
        {"rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8", 1, 44},
        {"rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8", 2, 1486},
        {"rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8", 3, 62379},
        // position 6
        {"r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10", 1, 46},
        {"r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10", 2, 2079},
        {"r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10", 3, 89890},
    };
    for (const auto& c : cases) {
        Position p;
        Check(p.FromFen(c.fen), "parse perft position");
        Check(Perft(p, c.depth) == c.expected,
            "perft depth " + std::to_string(c.depth) + " = " + std::to_string(c.expected));
    }
}

static void TestEvaluate() {
    std::cout << "== evaluate (material) ==\n";
    struct Case { const char* fen; int expected; };
    const Case cases[] = {
        // balanced material
        {"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", 0},
        // eval is white-absolute: identical position, black to move, same score
        {"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR b KQkq - 0 1", 0},
        // bare kings: every empty square must score nothing
        {"4k3/8/8/8/8/8/8/4K3 w - - 0 1", 0},
        // lone white pawn, both sides to move
        {"4k3/8/8/8/8/8/4P3/4K3 w - - 0 1", 1},
        {"4k3/8/8/8/8/8/4P3/4K3 b - - 0 1", 1},
        // black material while white to move: sign follows the piece, not the turn
        {"4k3/4p3/8/8/8/8/8/4K3 w - - 0 1", -1},
        // white up a queen
        {"rnb1kbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", 9},
        // black up a rook (white a1 removed)
        {"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/1NBQKBNR w Kkq - 0 1", -5},
    };
    for (const auto& c : cases) {
        Position p;
        Check(p.FromFen(c.fen), "parse eval position");
        Check(Evaluate(p) == c.expected,
            "eval = " + std::to_string(c.expected) + " : " + c.fen);
    }
}

static void TestSearch() {
    std::cout << "== search ==\n";
    // free queen: Qh1 must capture the hanging Qd5 (h1=63, d5=27)
    {
        Position p;
        Check(p.FromFen("4k3/8/8/3q4/8/8/8/4K2Q w - - 0 1"), "parse free-queen pos");
        auto best = FindBestMove(p, 2);
        Check(best.has_value(), "finds a move");
        Check(best && best->from == 63 && best->to == 27, "captures the hanging queen");
    }
    // mate in 1 available: the chosen move must actually deliver mate
    {
        Position p;
        Check(p.FromFen("k7/2K5/8/8/8/8/8/7Q w - - 0 1"), "parse mate-in-1 pos");
        auto best = FindBestMove(p, 2);
        Check(best.has_value(), "finds a move");
        if (best) {
            UndoInfo ui = p.MakeMove(*best);
            (void)ui;
            auto replies = GenerateLegalMoves(p);
            auto bk = FindKing(p, Side::BLACK);
            Check(replies.empty() && bk && IsSquareAttacked(p, *bk, Side::WHITE),
                  "chosen move delivers mate");
        }
    }
    // stalemate (a8 boxed by Qb6, not in check): no legal move to return
    {
        Position p;
        Check(p.FromFen("k7/8/1Q6/8/8/8/8/K7 b - - 0 1"), "parse stalemate pos");
        Check(!FindBestMove(p, 2).has_value(), "stalemate returns nullopt");
    }
    // checkmate (Qb7 defended by Kc6): no legal move to return
    {
        Position p;
        Check(p.FromFen("k7/1Q6/2K5/8/8/8/8/8 b - - 0 1"), "parse mated pos");
        Check(!FindBestMove(p, 2).has_value(), "mated side returns nullopt");
    }
    // ordinary position: returns some move
    {
        Position p;
        Check(p.FromFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"), "parse startpos");
        Check(FindBestMove(p, 2).has_value(), "startpos yields a move");
    }
}

int main() {
    TestValidFens();
    TestStartposValues();
    TestMidgameValues();
    TestCastlingAndCounters();
    TestInvalidFens();
    TestAtomicity();
    TestReuse();
    TestKnightMoves();
    TestPawnMoves();
    TestPromotionMoves();
    TestCastleMoves();
    TestMakeMove();
    TestUnmakeMove();
    TestSquareAttacked();
    TestLegalMoves();
    TestPerft();
    TestEvaluate();
    TestSearch();

    if (g_failures == 0) {
        std::cout << "\nALL TESTS PASSED\n";
        return 0;
    }
    std::cout << "\n" << g_failures << " FAILURE(S)\n";
    return 1;
}
