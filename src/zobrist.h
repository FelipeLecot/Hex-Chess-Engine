#ifndef ZOBRIST_H
#define ZOBRIST_H

#include "typedefs.h"

// Random 64-bit keys for each piece on each square.
extern uint64_t PIECES[12][NUM_SQUARES];
// Random key for each possible en-passant square.
extern uint64_t EN_PASSANT[NUM_SQUARES];
// Toggled when it's white's turn.
extern uint64_t WHITE_TO_MOVE;

void initZobrist(void);

// Full hash computation from scratch (used during FEN loading).
uint64_t computeHash(Board board);

#endif
