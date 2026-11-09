#include "board.h"
#include <iostream>
#include <vector>

void Position::SetUpStartingPosition() {
    side_to_move = WHITE;
    castling_rights = WHITE_KINGSIDE | WHITE_QUEENSIDE | BLACK_KINGSIDE | BLACK_QUEENSIDE;
    halfmove_clock = 0;
    fullmove_number = 1;
    en_passant_target = -1;
    for (int i = 0; i < 8; i++) {
        if (i == 1 || i == 6) continue;
        for (int j = 0; j < 8; j++) {
            board[i][j] = EMPTY;
        }
    }
    for (int j = 0; j < 8; j++) {
        board[1][j] = BLACK_PAWN;
        board[6][j] = WHITE_PAWN;
    }
    board[0][0] = BLACK_ROOK;
    board[0][1] = BLACK_KNIGHT;
    board[0][2] = BLACK_BISHOP;
    board[0][3] = BLACK_QUEEN;
    board[0][4] = BLACK_KING;
    board[0][5] = BLACK_BISHOP;
    board[0][6] = BLACK_KNIGHT;
    board[0][7] = BLACK_ROOK;
    board[7][0] = WHITE_ROOK;
    board[7][1] = WHITE_KNIGHT;
    board[7][2] = WHITE_BISHOP;
    board[7][3] = WHITE_QUEEN;
    board[7][4] = WHITE_KING;
    board[7][5] = WHITE_BISHOP;
    board[7][6] = WHITE_KNIGHT;
    board[7][7] = WHITE_ROOK;
}

static char PieceToCharTable(Piece piece) {

    static constexpr char kPieceChars[] = {
        '.',  // EMPTY
        'P', 'N', 'B', 'R', 'Q', 'K',        // WHITE_PAWN .. WHITE_KING
        'p', 'n', 'b', 'r', 'q', 'k',        // BLACK_PAWN .. BLACK_KING
    };

    return kPieceChars[piece];
}

void Position::PrintBoard() {
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            std::cout << PieceToCharTable(board[i][j]);
            std::cout << " ";
        }
        std::cout << "\n";
    }
}

static Piece CharToPiece(char c) {
    switch (c) {
        case 'P': return WHITE_PAWN;
        case 'N': return WHITE_KNIGHT;
        case 'B': return WHITE_BISHOP;
        case 'R': return WHITE_ROOK;
        case 'Q': return WHITE_QUEEN;
        case 'K': return WHITE_KING;
        case 'p': return BLACK_PAWN;
        case 'n': return BLACK_KNIGHT;
        case 'b': return BLACK_BISHOP;
        case 'r': return BLACK_ROOK;
        case 'q': return BLACK_QUEEN;
        case 'k': return BLACK_KING;
        default:  return EMPTY;
    }
}

bool Position::FromFen(std::string_view fen) {
    Position temp;
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            temp.board[i][j] = EMPTY;
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
        if (i == 8) return false;
        int j = 0;
        auto pos = fields[0].find('/', start);
        std::string_view rank = fields[0].substr(start);
        if (pos != std::string_view::npos) rank = fields[0].substr(start, pos - start);
        for (char c : rank) {
            if (c >= '1' && c <= '8') {
                j += c - '0'; continue;
            }
            Piece p = CharToPiece(c);
            if (p == EMPTY || j >= 8) return false;
            temp.board[i][j] = p;
            j += 1;
        }
        if (j != 8) return false;
        i += 1;
        if (pos == std::string_view::npos) break;
        start = pos + 1;
    }
    if (i != 8) return false;

    if (fields[1] != "w" && fields[1] != "b") return false;
    temp.side_to_move = (fields[1] == "w" ? WHITE : BLACK);

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
        temp.halfmove_clock *= 10;
        int num = c - '0';
        if (num < 0 || num > 9) return false;
        temp.halfmove_clock += num;
    }

   temp.fullmove_number = 0;
    for (char c : fields[5]) {
        temp.fullmove_number *= 10;
        int num = c - '0';
        if (num < 0 || num > 9) return false;
        temp.fullmove_number += num;
    }

    temp.en_passant_target = -1;
    if (fields[3].size() != 2 && fields[3] != "-") return false;
    if (fields[3].size() == 2) {
        char file = fields[3][0];
        if (file < 'a' || file > 'h') return false;
        char rank = fields[3][1];
        if (rank != '3' && rank != '6') return false;
        temp.en_passant_target = (8 - (fields[3][1] - '0')) * 8 + (fields[3][0] - 'a');
    }

    *this = temp;
    return true;
}

Piece Position::GetPiece(int row, int col) const {
    return board[row][col];
}

Side Position::GetSideToMove() const {
    return side_to_move;
}

int Position::GetCastlingRights() const {
    return castling_rights;
}

int Position::GetEnPassantTarget() const {
    return en_passant_target;
}

int Position::GetHalfmoveClock() const {
    return halfmove_clock;
}

int Position::GetFullmoveNumber() const {
    return fullmove_number;
}
