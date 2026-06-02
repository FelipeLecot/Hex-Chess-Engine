#include "tt.h"

TTEntry TT_TABLE[TT_SIZE]; // 131,072 entries ≈ 7 MB

TTEntry getTTEntry(uint64_t zobrist) {
    return TT_TABLE[zobrist & TT_MASK];
}

void addTTEntry(Board board, int eval, Move move, int depth, int beta, int alpha) {
    TTEntry entry;
    entry.eval    = eval;
    entry.move    = move;
    entry.depth   = depth;
    entry.zobrist = board.hash;

    if (eval <= alpha)      entry.nodeType = UPPER;
    else if (eval >= beta)  entry.nodeType = LOWER;
    else                    entry.nodeType = EXACT;

    TT_TABLE[entry.zobrist & TT_MASK] = entry;
}
