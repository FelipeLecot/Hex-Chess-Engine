#include <stdlib.h>
#include "zobrist.h"
#include "board.h"

uint64_t PIECES[12][NUM_SQUARES];
uint64_t EN_PASSANT[NUM_SQUARES];
uint64_t WHITE_TO_MOVE;

static uint64_t randU64(void) {
    // Mix four rand() calls (each gives at least 15 bits) to fill 64 bits.
    return ((uint64_t)rand() << 48)
         ^ ((uint64_t)rand() << 32)
         ^ ((uint64_t)rand() << 16)
         ^  (uint64_t)rand();
}

void initZobrist(void) {
    for (int p = 0; p < 12; p++)
        for (int s = 0; s < NUM_SQUARES; s++)
            PIECES[p][s] = randU64();
    for (int s = 0; s < NUM_SQUARES; s++)
        EN_PASSANT[s] = randU64();
    WHITE_TO_MOVE = randU64();
}

uint64_t computeHash(Board board) {
    uint64_t h = 0;
    for (int p = 0; p < 12; p++) {
        Bitboard bb = *pieceBB(&board, p);
        for (; !bbEmpty(bb);) {
            int sq = bbLsb(bb);
            h ^= PIECES[p][sq];
            bb = bbClear(bb, sq);
        }
    }
    if (board.epSquare >= 0) h ^= EN_PASSANT[board.epSquare];
    if (board.turn == WHITE) h ^= WHITE_TO_MOVE;
    return h;
}
