#include "board.h"
#include <iostream>
#include <sstream>
#include <string>
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

void Position::FromFen(const std::string& fen) {
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            board[i][j] = EMPTY;
        }
    }
    std::istringstream fenStream(fen);
    std::string field;
    std::vector<std::string> fields;
    while (std::getline(fenStream, field, ' ')) {
        fields.push_back(field);
    }
    std::istringstream rankStream(fields[0]);
    std::string rank;
    int i = 0;
    while (std::getline(rankStream, rank, '/')) {
        int j = 0;
        for (int k = 0; k < rank.size(); k++) {
            if (rank[k] >= '1' && rank[k] <= '8') {
                j += rank[k] - '0'; continue;
            }
            board[i][j] = CharToPiece(rank[k]);
            j += 1;
        }
        i += 1;
    }
    side_to_move = (fields[1] == "w" ? WHITE : BLACK);
    castling_rights = 0;
    for (int i = 0; i < fields[2].size(); i++) {
        switch (fields[2][i]) {
            case 'K':
                castling_rights |= WHITE_KINGSIDE;
                break;
            case 'Q':
                castling_rights |= WHITE_QUEENSIDE;
                break;
            case 'k':
                castling_rights |= BLACK_KINGSIDE;
                break;
            case 'q':
                castling_rights |= BLACK_QUEENSIDE;
                break;
        }
    }
    halfmove_clock = 0;
    for (int i = 0; i < fields[4].size(); i++) {
        halfmove_clock *= 10;
        halfmove_clock += fields[4][i] - '0';
    }
    fullmove_number = 0;
    for (int i = 0; i < fields[5].size(); i++) {
        fullmove_number *= 10;
        fullmove_number += fields[5][i] - '0';
    }
    en_passant_target = -1;
    if (fields[3].size() > 1) {
        en_passant_target = (8 - (fields[3][1] - '0')) * 8 + (fields[3][0] - 'a');
    }
}
