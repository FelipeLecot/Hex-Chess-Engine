#include <string.h>
#include "search.h"
#include "evaluation.h"
#include "board.h"
#include "movegen.h"
#include "tt.h"
#include "evaluatemove.h"

#define smax(a, b) ((a) > (b) ? (a) : (b))
#define smin(a, b) ((a) < (b) ? (a) : (b))

static int alphabeta(Board board, int depth, int alpha, int beta, SearchContext *ctx);

int search(Board board, int depth, SearchContext *ctx) {
    memset(&ctx->bestMove, 0, sizeof(Move));
    ctx->nodesSearched = 0;
    ctx->maxDepth = depth;
    return alphabeta(board, depth, MIN_EVAL, MAX_EVAL, ctx);
}

static int alphabeta(Board board, int depth, int alpha, int beta, SearchContext *ctx) {
    ctx->nodesSearched++;
    int origAlpha = alpha;

    TTEntry entry = getTTEntry(board.hash);
    if (entry.zobrist == board.hash && entry.depth >= depth) {
        if (entry.nodeType == EXACT) {
            if (depth == ctx->maxDepth) ctx->bestMove = entry.move;
            return entry.eval;
        } else if (entry.nodeType == LOWER) {
            alpha = smax(alpha, entry.eval);
        } else if (entry.nodeType == UPPER) {
            beta  = smin(beta,  entry.eval);
        }
        if (alpha >= beta) {
            if (depth == ctx->maxDepth) ctx->bestMove = entry.move;
            return entry.eval;
        }
    }

    Move moves[512];
    int cmoves = legalMoves(&board, moves);

    int res = result(board, moves, cmoves);
    if (res != UN_DETERMINED) {
        int eval = evaluate(board, res);
        if (res != DRAW)
            eval += (ctx->maxDepth - depth) * (board.turn ? 1 : -1);
        return eval * (board.turn ? 1 : -1);
    }
    if (depth == 0) {
        return evaluate(board, UN_DETERMINED) * (board.turn ? 1 : -1);
    }

    score_moves(board, entry, moves, cmoves);

    int eval = MIN_EVAL;
    Move bestMove;
    memset(&bestMove, 0, sizeof(Move));
    int nextMove;

    while ((nextMove = select_move(moves, cmoves)) != -1) {
        Board child = board;
        pushMove(&child, moves[nextMove]);
        int childEval = -alphabeta(child, depth - 1, -beta, -alpha, ctx);

        if (childEval > eval) {
            eval = childEval;
            bestMove = moves[nextMove];
            if (depth == ctx->maxDepth) ctx->bestMove = bestMove;
        }

        alpha = smax(alpha, childEval);
        if (alpha >= beta) break;
    }

    addTTEntry(board, eval, bestMove, depth, beta, origAlpha);
    return eval;
}
