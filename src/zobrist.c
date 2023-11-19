#include <stdlib.h>
#include "zobrist.h"

Bitboard randomBitboard();

// https://www.chessprogramming.org/Zobrist_Hashing

Bitboard WHITE_MOVE;
Bitboard ENPASSANT[64];
Bitboard CASTLE[16];
Bitboard PIECES[12][64];

Bitboard hash(Board board) {
    Bitboard hash = 0LL;

    // Castling rights
    hash ^= CASTLE[board.castling];

    // Pieces
    for (int i = 0; i < 12; i++) {
        Bitboard bb = *(&(board.pawn_w)+i);

        while (bb) {
            int square = __builtin_ctzll(bb);
            hash ^= PIECES[i][square];
            bb &= bb -1;
        }
    }

    // En passant
    if (board.epSquare != -1) {
        hash ^= ENPASSANT[board.epSquare];
    }

    // Side to move
    if (board.turn) {
        hash ^= WHITE_MOVE;
    }

    return hash;
}

void initZobrist() {
    for (int i = 0; i < 12; i++) for (int j = 0; j < 64;j++) PIECES[i][j] = randomBitboard();
    for (int i = 0; i < 64; i++) ENPASSANT[i] = randomBitboard();
    for (int i = 0; i < 16; i++) CASTLE[i] = randomBitboard();
    WHITE_MOVE = randomBitboard();
}

Bitboard randomBitboard(void) {
    Bitboard r = 0;
    for (int i=0; i < 64; i++) {
        Bitboard tmp = (Bitboard) rand() % 2;
        r |= tmp << i;
    }
    return r;
}
