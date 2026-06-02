#include <stdbool.h>
#include "evaluatemove.h"
#include "board.h"
#include "bitboards.h"

static const int VICTIM_VALUES[6] = {
    100*16, 320*16, 330*16, 500*16, 900*16, 20000*16
};
static const int ATTACKER_VALUES[12] = {
    100, 320, 330, 500, 900, 20000,
    100, 320, 330, 500, 900, 20000
};
static const int MAX_MOVE_SCORE   = 1000;
static const int PAWN_EXCHANGE    = (100*16) - 100;

static bool movesEqual(Move a, Move b) {
    return a.fromSquare == b.fromSquare
        && a.toSquare   == b.toSquare
        && a.promotion  == b.promotion;
}

void score_moves(Board board, TTEntry entry, Move moves[], int cmoves) {
    Move pvMove;
    bool hasPV = false;
    if (entry.zobrist == board.hash && entry.nodeType < UPPER) {
        pvMove = entry.move;
        hasPV  = true;
    }

    Bitboard enemyOcc = board.turn ? board.occupancyBlack : board.occupancyWhite;

    for (int i = 0; i < cmoves; i++) {
        Move *m = &moves[i];

        if (hasPV && movesEqual(pvMove, *m)) {
            m->score = MAX_MOVE_SCORE;
            continue;
        }

        bool isEp      = (board.epSquare == m->toSquare);
        bool isCapture = bbGet(enemyOcc, m->toSquare);

        if (isEp) {
            m->score = PAWN_EXCHANGE;
        } else if (isCapture) {
            // Find captured piece type
            int capPiece = 0;
            int opStart  = board.turn ? PAWN_B : PAWN_W;
            for (int p = 0; p < 6; p++) {
                if (bbGet(*pieceBB((Board*)&board, opStart + p), m->toSquare)) {
                    capPiece = p;
                    break;
                }
            }
            m->score = VICTIM_VALUES[capPiece] - ATTACKER_VALUES[m->pieceType];
        } else {
            m->score = 0;
        }
    }
}

int select_move(Move moves[], int cmoves) {
    int best = -1;
    int bestScore = -1;
    for (int i = 0; i < cmoves; i++) {
        if (!moves[i].exhausted && moves[i].score > bestScore) {
            bestScore = moves[i].score;
            best = i;
        }
    }
    // Fall back to any un-exhausted move if none scored > -1
    if (best == -1) {
        for (int i = 0; i < cmoves; i++) {
            if (!moves[i].exhausted) { best = i; break; }
        }
    }
    if (best >= 0) moves[best].exhausted = true;
    return best;
}
