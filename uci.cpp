// UCI adapter: text-protocol glue between a chess GUI (lichess-bot, Arena,
// CuteChess) and the engine. Uses only the engine's public API — no engine
// internals, no engine changes. Protocol: https://wbec-ridderkerk.nl/html/UCIProtocol.html

#include "board.h"
#include "movegen.h"
#include "search.h"

#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

static std::string SquareToString(int sq) {
    std::string s;
    s += static_cast<char>('a' + sq % 8);
    s += static_cast<char>('8' - sq / 8);
    return s;
}

static int StringToSquare(const std::string& s) {
    if (s.size() != 2) return -1;
    int file = s[0] - 'a';
    int rank = s[1] - '1';
    if (file < 0 || file > 7 || rank < 0 || rank > 7) return -1;
    return (7 - rank) * 8 + file;
}

static std::string MoveToString(const Move& m) {
    std::string s = SquareToString(m.from) + SquareToString(m.to);
    if (m.promo != Piece::EMPTY) {
        switch (GetPieceType(m.promo)) {
            case PieceType::QUEEN:  s += 'q'; break;
            case PieceType::ROOK:   s += 'r'; break;
            case PieceType::BISHOP: s += 'b'; break;
            case PieceType::KNIGHT: s += 'n'; break;
            default: break;
        }
    }
    return s;
}

// Finds the legal move matching a UCI string ("e2e4", "e7e8q") and plays it.
static bool ApplyUciMove(Position& pos, const std::string& uci_move) {
    int from = StringToSquare(uci_move.substr(0, 2));
    int to = StringToSquare(uci_move.substr(2, 2));
    if (from < 0 || to < 0) return false;
    for (const Move& m : GenerateLegalMoves(pos)) {
        if (m.from != from || m.to != to) continue;
        if (uci_move.size() == 5) {
            if (m.promo == Piece::EMPTY) continue;
            PieceType want;
            switch (uci_move[4]) {
                case 'q': want = PieceType::QUEEN; break;
                case 'r': want = PieceType::ROOK; break;
                case 'b': want = PieceType::BISHOP; break;
                case 'n': want = PieceType::KNIGHT; break;
                default: continue;
            }
            if (GetPieceType(m.promo) != want) continue;
        }
        UndoInfo ui = pos.MakeMove(m);
        (void)ui;
        return true;
    }
    return false;
}

static void HandlePosition(Position& pos, std::istringstream& iss) {
    std::vector<std::string> tok;
    for (std::string t; iss >> t;) tok.push_back(t);
    std::size_t i = 0;
    if (i < tok.size() && tok[i] == "startpos") {
        pos.SetUpStartingPosition();
        i++;
    } else if (i < tok.size() && tok[i] == "fen") {
        i++;
        std::string fen;
        for (int n = 0; n < 6 && i < tok.size() && tok[i] != "moves"; n++, i++) {
            if (!fen.empty()) fen += ' ';
            fen += tok[i];
        }
        bool ok = pos.FromFen(fen);
        (void)ok;
    }
    if (i < tok.size() && tok[i] == "moves") {
        for (i++; i < tok.size(); i++) ApplyUciMove(pos, tok[i]);
    }
}

int main() {
    constexpr int kDefaultDepth = 4;
    Position pos;
    pos.SetUpStartingPosition();

    std::string line;
    while (std::getline(std::cin, line)) {
        std::istringstream iss(line);
        std::string cmd;
        iss >> cmd;

        if (cmd == "uci") {
            std::cout << "id name cpp_chess\nid author shouteck\nuciok\n" << std::flush;
        } else if (cmd == "isready") {
            std::cout << "readyok\n" << std::flush;
        } else if (cmd == "ucinewgame") {
            pos.SetUpStartingPosition();
        } else if (cmd == "position") {
            HandlePosition(pos, iss);
        } else if (cmd == "go") {
            int depth = kDefaultDepth;
            std::string tok;
            while (iss >> tok) {
                if (tok == "depth") iss >> depth;
            }
            auto best = FindBestMove(pos, depth);
            std::cout << "bestmove " << (best ? MoveToString(*best) : "0000") << "\n" << std::flush;
        } else if (cmd == "quit") {
            return 0;
        }
        // "stop" and other commands ignored: single-threaded, can't abort a
        // running search — bestmove arrives when the search completes.
    }
    return 0;
}
