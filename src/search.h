#ifndef SEARCH_H
#define SEARCH_H

#include "typedefs.h"

typedef struct {
    int  nodesSearched;
    Move bestMove;
    int  maxDepth;
} SearchContext;

// Runs alpha-beta search to the given depth.
// Fills ctx->bestMove and ctx->nodesSearched. Returns evaluation (white-positive).
int search(Board board, int depth, SearchContext *ctx);

#endif
