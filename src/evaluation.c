#include "evaluation.h"
#include "board.h"
#include "coords.h"
#include <stdlib.h>

// Piece material values (centipawns). Black values are negated.
static const int PIECE_VALUES[12] = {
    100, 320, 330, 500, 900, 20000,   // white
   -100,-320,-330,-500,-900,-20000    // black
};

// Piece-square tables: PST_W[pt][sq] for white, PST_B[pt][sq] for black.
// White bonuses are positive (good for white), black bonuses are negative
// (good for black). Built by initEvaluation() from cube coordinates.
static int PST_W[6][NUM_SQUARES];
static int PST_B[6][NUM_SQUARES];

// Knight bonus by hex distance from center (0..5).
static const int KNIGHT_DIST[6] = { 30, 20, 10, 0, -15, -30 };

// Bishop bonus by hex distance from center.
static const int BISHOP_DIST[6] = { 20, 12, 6, 0, -5, -10 };

// Queen bonus by hex distance from center (small — don't encourage early development).
static const int QUEEN_DIST[6] = { 10, 5, 2, 0, 0, 0 };

// King safety: prefer edges (no game-phase detection yet).
static const int KING_DIST[6] = { -40, -20, -5, 5, 15, 20 };

void initEvaluation(void) {
    for (int sq = 0; sq < NUM_SQUARES; sq++) {
        int q = CELLS[sq].q;
        int r = CELLS[sq].r;
        int s = CELLS[sq].s;
        int dist = (abs(q) + abs(r) + abs(s)) / 2;

        // Pawn: reward advancement (r increases toward black's side) + central file.
        PST_W[0][sq] = r * 8 + (HEX_RADIUS - abs(q)) * 3;

        // Knight: strong central bonus, penalise rim.
        PST_W[1][sq] = KNIGHT_DIST[dist];

        // Bishop: moderate central bonus.
        PST_W[2][sq] = BISHOP_DIST[dist];

        // Rook: small advancement + small central-file bonus.
        PST_W[3][sq] = r * 2 + (HEX_RADIUS - abs(q)) * 2;

        // Queen: small central bonus.
        PST_W[4][sq] = QUEEN_DIST[dist];

        // King: prefer edges for safety.
        PST_W[5][sq] = KING_DIST[dist];
    }

    // Black tables: 180° point reflection through center maps (q,r,s) -> (-q,-r,-s).
    // Values are negated so the sign convention matches PIECE_VALUES (black = negative).
    for (int pt = 0; pt < 6; pt++) {
        for (int sq = 0; sq < NUM_SQUARES; sq++) {
            int msq = cubeToIndex(-CELLS[sq].q, -CELLS[sq].r, -CELLS[sq].s);
            PST_B[pt][sq] = -PST_W[pt][msq];
        }
    }
}

int evaluate(Board board, int gameResult) {
    if (gameResult == WHITE_WIN) return  MAX_EVAL;
    if (gameResult == BLACK_WIN) return  MIN_EVAL;
    if (gameResult == DRAW)      return  0;

    int score = 0;
    for (int p = 0; p < 6; p++) {
        Bitboard wb = *pieceBB(&board, p);
        while (!bbEmpty(wb)) {
            int sq = bbLsb(wb);
            score += PIECE_VALUES[p] + PST_W[p][sq];
            wb = bbClear(wb, sq);
        }
        Bitboard bb = *pieceBB(&board, p + 6);
        while (!bbEmpty(bb)) {
            int sq = bbLsb(bb);
            score += PIECE_VALUES[p + 6] + PST_B[p][sq];
            bb = bbClear(bb, sq);
        }
    }
    return score;
}
