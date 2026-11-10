#include "board.h"
#include <climits>
#include <iostream>
#include <optional>
#include <vector>

void Position::SetUpStartingPosition() {
    side_to_move = Side::WHITE;
    castling_rights = WHITE_KINGSIDE | WHITE_QUEENSIDE | BLACK_KINGSIDE | BLACK_QUEENSIDE;
    halfmove_clock = 0;
    fullmove_number = 1;
    en_passant_target = std::nullopt;
    for (int i = 0; i < kBoardSize; i++) {
        if (i == 1 || i == 6) continue;
        for (int j = 0; j < kBoardSize; j++) {
            board[i * kBoardSize + j] = Piece::EMPTY;
        }
    }
    for (int j = 0; j < kBoardSize; j++) {
        board[1 * kBoardSize + j] = Piece::BLACK_PAWN;
        board[6 * kBoardSize + j] = Piece::WHITE_PAWN;
    }
    board[0 * kBoardSize + 0] = Piece::BLACK_ROOK;
    board[0 * kBoardSize + 1] = Piece::BLACK_KNIGHT;
    board[0 * kBoardSize + 2] = Piece::BLACK_BISHOP;
    board[0 * kBoardSize + 3] = Piece::BLACK_QUEEN;
    board[0 * kBoardSize + 4] = Piece::BLACK_KING;
    board[0 * kBoardSize + 5] = Piece::BLACK_BISHOP;
    board[0 * kBoardSize + 6] = Piece::BLACK_KNIGHT;
    board[0 * kBoardSize + 7] = Piece::BLACK_ROOK;
    board[7 * kBoardSize + 0] = Piece::WHITE_ROOK;
    board[7 * kBoardSize + 1] = Piece::WHITE_KNIGHT;
    board[7 * kBoardSize + 2] = Piece::WHITE_BISHOP;
    board[7 * kBoardSize + 3] = Piece::WHITE_QUEEN;
    board[7 * kBoardSize + 4] = Piece::WHITE_KING;
    board[7 * kBoardSize + 5] = Piece::WHITE_BISHOP;
    board[7 * kBoardSize + 6] = Piece::WHITE_KNIGHT;
    board[7 * kBoardSize + 7] = Piece::WHITE_ROOK;
}

static char PieceToCharTable(Piece piece) {

    static constexpr char kPieceChars[] = {
        '.',  // EMPTY
        'P', 'N', 'B', 'R', 'Q', 'K',        // WHITE_PAWN .. WHITE_KING
        'p', 'n', 'b', 'r', 'q', 'k',        // BLACK_PAWN .. BLACK_KING
    };

    return kPieceChars[static_cast<size_t>(piece)];
}

void Position::PrintBoard() {
    for (int i = 0; i < kBoardSize; i++) {
        for (int j = 0; j < kBoardSize; j++) {
            std::cout << PieceToCharTable(board[i * kBoardSize + j]);
            std::cout << " ";
        }
        std::cout << "\n";
    }
}

static Piece CharToPiece(char c) {
    switch (c) {
        case 'P': return Piece::WHITE_PAWN;
        case 'N': return Piece::WHITE_KNIGHT;
        case 'B': return Piece::WHITE_BISHOP;
        case 'R': return Piece::WHITE_ROOK;
        case 'Q': return Piece::WHITE_QUEEN;
        case 'K': return Piece::WHITE_KING;
        case 'p': return Piece::BLACK_PAWN;
        case 'n': return Piece::BLACK_KNIGHT;
        case 'b': return Piece::BLACK_BISHOP;
        case 'r': return Piece::BLACK_ROOK;
        case 'q': return Piece::BLACK_QUEEN;
        case 'k': return Piece::BLACK_KING;
        default:  return Piece::EMPTY;
    }
}

bool Position::FromFen(std::string_view fen) {
    Position temp;
    for (int i = 0; i < kBoardSize; i++) {
        for (int j = 0; j < kBoardSize; j++) {
            temp.board[i * kBoardSize + j] = Piece::EMPTY;
        }
    }
    std::vector<std::string_view> fields;
    std::string_view::size_type start = 0;
    while (start <= fen.size()) {
        auto pos = fen.find(' ', start);
        if (pos == std::string_view::npos) {
            fields.push_back(fen.substr(start));
            break;
        }
        fields.push_back(fen.substr(start, pos - start));
        start = pos + 1;
    }
    if (fields.size() != 6) return false;

    start = 0;
    int i = 0;
    while (start <= fields[0].size()) {
        if (i == kBoardSize) return false;
        int j = 0;
        auto pos = fields[0].find('/', start);
        std::string_view rank = fields[0].substr(start);
        if (pos != std::string_view::npos) rank = fields[0].substr(start, pos - start);
        for (char c : rank) {
            if (c >= '1' && c <= '8') {
                j += c - '0'; continue;
            }
            Piece p = CharToPiece(c);
            if (p == Piece::EMPTY || j >= kBoardSize) return false;
            temp.board[i * kBoardSize + j] = p;
            j += 1;
        }
        if (j != kBoardSize) return false;
        i += 1;
        if (pos == std::string_view::npos) break;
        start = pos + 1;
    }
    if (i != kBoardSize) return false;

    if (fields[1] != "w" && fields[1] != "b") return false;
    temp.side_to_move = (fields[1] == "w" ? Side::WHITE : Side::BLACK);

    temp.castling_rights = 0;
    for (char c : fields[2]) {
        switch (c) {
            case 'K':
                temp.castling_rights |= WHITE_KINGSIDE;
                break;
            case 'Q':
                temp.castling_rights |= WHITE_QUEENSIDE;
                break;
            case 'k':
                temp.castling_rights |= BLACK_KINGSIDE;
                break;
            case 'q':
                temp.castling_rights |= BLACK_QUEENSIDE;
                break;
            case '-':
                break;
            default:
                return false;
        }
    }

    temp.halfmove_clock = 0;
    for (char c : fields[4]) {
        int num = c - '0';
        if (num < 0 || num > 9) return false;
        if (temp.halfmove_clock > (INT_MAX - num) / 10) return false;
        temp.halfmove_clock = temp.halfmove_clock * 10 + num;
    }

    temp.fullmove_number = 0;
    for (char c : fields[5]) {
        int num = c - '0';
        if (num < 0 || num > 9) return false;
        if (temp.fullmove_number > (INT_MAX - num) / 10) return false;
        temp.fullmove_number = temp.fullmove_number * 10 + num;
    }

    if (fields[3].size() != 2 && fields[3] != "-") return false;
    if (fields[3].size() == 2) {
        char file = fields[3][0];
        if (file < 'a' || file > 'h') return false;
        char rank = fields[3][1];
        if (rank != '3' && rank != '6') return false;
        temp.en_passant_target = (kBoardSize - (fields[3][1] - '0')) * kBoardSize + (fields[3][0] - 'a');
    }

    *this = temp;
    return true;
}

Piece Position::GetPiece(int row, int col) const {
    return board[row * kBoardSize + col];
}

Side Position::GetSideToMove() const {
    return side_to_move;
}

int Position::GetCastlingRights() const {
    return castling_rights;
}

std::optional<int> Position::GetEnPassantTarget() const {
    return en_passant_target;
}

int Position::GetHalfmoveClock() const {
    return halfmove_clock;
}

int Position::GetFullmoveNumber() const {
    return fullmove_number;
}
