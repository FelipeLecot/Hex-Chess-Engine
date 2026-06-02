#include "bitboards.h"

Bitboard SQUARE_BITBOARDS[NUM_SQUARES];

void initBitboards(void) {
    for (int i = 0; i < NUM_SQUARES; i++) {
        Bitboard b = {0, 0};
        if (i < 64) b.lo = 1ULL << i;
        else        b.hi = 1ULL << (i - 64);
        SQUARE_BITBOARDS[i] = b;
    }
}
