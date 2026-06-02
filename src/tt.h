#ifndef TT_H
#define TT_H

#include <stdint.h>
#include "typedefs.h"

typedef struct {
    uint64_t zobrist;
    int eval;
    int nodeType;
    int depth;
    Move move;
} TTEntry;

enum NODE_TYPE { EXACT, LOWER, UPPER };

// Power-of-2 size enables bitmask indexing (faster than modulo).
// 1<<17 = 131,072 entries; at ~52 bytes each ≈ 7 MB — fits in typical L3 cache.
// Larger sizes cause cache-miss penalties that outweigh collision savings.
#define TT_SIZE (1 << 17)
#define TT_MASK (TT_SIZE - 1)

extern TTEntry TT_TABLE[TT_SIZE];

TTEntry getTTEntry(uint64_t zobrist);
void addTTEntry(Board board, int eval, Move move, int depth, int beta, int alpha);

#endif
