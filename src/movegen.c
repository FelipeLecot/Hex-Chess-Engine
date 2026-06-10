#include <stdbool.h>
#include <string.h>
#include "movegen.h"
#include "board.h"
#include "magics.h"

// ── per-piece attack generators (sliding pieces use precomputed ray tables) ───

static Bitboard rookAttacks(int sq, Bitboard occ) {
    return rookAttacksTable(sq, occ);
}

static Bitboard bishopAttacks(int sq, Bitboard occ) {
    return bishopAttacksTable(sq, occ);
}

static Bitboard queenAttacks(int sq, Bitboard occ) {
    return bbOr(rookAttacksTable(sq, occ), bishopAttacksTable(sq, occ));
}

static Bitboard knightAttacks(int sq) {
    Bitboard a = bbZero();
    Cell *c = &CELLS[sq];
    for (int d = 0; d < 12; d++) {
        int ni = cubeToIndex(c->q + KNIGHT_DIRS[d][0],
                             c->r + KNIGHT_DIRS[d][1],
                             c->s + KNIGHT_DIRS[d][2]);
        if (ni >= 0) a = bbSet(a, ni);
    }
    return a;
}

static Bitboard kingAttacks(int sq) {
    Bitboard a = bbZero();
    Cell *c = &CELLS[sq];
    for (int d = 0; d < 12; d++) {
        int ni = cubeToIndex(c->q + KING_DIRS[d][0],
                             c->r + KING_DIRS[d][1],
                             c->s + KING_DIRS[d][2]);
        if (ni >= 0) a = bbSet(a, ni);
    }
    return a;
}

// ── full attack mask ──────────────────────────────────────────────────────────

Bitboard computeAttacks(Board board) {
    Bitboard atk = bbZero();
    Bitboard occ = board.occupancy;

    // Attack as the side NOT to move
    Bitboard queen  = board.turn ? board.queen_b  : board.queen_w;
    Bitboard rook   = board.turn ? board.rook_b   : board.rook_w;
    Bitboard bishop = board.turn ? board.bishop_b : board.bishop_w;
    Bitboard knight = board.turn ? board.knight_b : board.knight_w;
    Bitboard pawn   = board.turn ? board.pawn_b   : board.pawn_w;
    Bitboard king   = board.turn ? board.king_b   : board.king_w;

    for (Bitboard bb = queen; !bbEmpty(bb);) {
        int sq = bbLsb(bb);
        atk = bbOr(atk, queenAttacks(sq, occ));
        bb = bbClear(bb, sq);
    }
    for (Bitboard bb = rook; !bbEmpty(bb);) {
        int sq = bbLsb(bb);
        atk = bbOr(atk, rookAttacks(sq, occ));
        bb = bbClear(bb, sq);
    }
    for (Bitboard bb = bishop; !bbEmpty(bb);) {
        int sq = bbLsb(bb);
        atk = bbOr(atk, bishopAttacks(sq, occ));
        bb = bbClear(bb, sq);
    }
    for (Bitboard bb = knight; !bbEmpty(bb);) {
        int sq = bbLsb(bb);
        atk = bbOr(atk, knightAttacks(sq));
        bb = bbClear(bb, sq);
    }
    for (Bitboard bb = king; !bbEmpty(bb);) {
        int sq = bbLsb(bb);
        atk = bbOr(atk, kingAttacks(sq));
        bb = bbClear(bb, sq);
    }
    // Pawn captures in Gliński: one step in the two rook directions that flank
    // the forward direction.  White forward = (0,+1,-1); captures via (+1,0,-1)
    // and (-1,+1,0).  Black forward = (0,-1,+1); captures via (+1,-1,0) and
    // (-1,0,+1).
    // (board.turn is the side TO MOVE, so pawn = the side NOT to move.)
    static const int CAP_W[2][3] = {{ 1, 0,-1},{-1, 1, 0}};
    static const int CAP_B[2][3] = {{ 1,-1, 0},{-1, 0, 1}};
    const int (*caps)[3] = (board.turn == WHITE) ? CAP_B : CAP_W;
    for (Bitboard bb = pawn; !bbEmpty(bb);) {
        int sq = bbLsb(bb);
        Cell *c = &CELLS[sq];
        for (int d = 0; d < 2; d++) {
            int ni = cubeToIndex(c->q + caps[d][0],
                                 c->r + caps[d][1],
                                 c->s + caps[d][2]);
            if (ni >= 0) atk = bbSet(atk, ni);
        }
        bb = bbClear(bb, sq);
    }

    return atk;
}

bool isSquareAttacked(Board board, int square) {
    return bbGet(board.attacks, square);
}

// ── move legality ─────────────────────────────────────────────────────────────

// Returns true if executing `move` on a copy of `board` leaves the mover's
// king NOT in check.
static bool isLegal(Board board, Move move) {
    Board after = board;
    pushMove(&after, move);
    int kingSq = board.turn ? after.whiteKingSq : after.blackKingSq;
    // computeAttacks gives attacks of the side NOT to move.
    // after.turn has flipped; reset it to the original so "not to move" = opponent.
    after.turn = board.turn;
    Bitboard oppAtk = computeAttacks(after);
    return !bbGet(oppAtk, kingSq);
}

// ── move list helpers ─────────────────────────────────────────────────────────

static int addMove(Move moves[], int len, int from, int to, int promo, int ptype) {
    moves[len].fromSquare = from;
    moves[len].toSquare   = to;
    moves[len].promotion  = promo;
    moves[len].pieceType  = ptype;
    moves[len].validation = NOT_VALIDATED;
    moves[len].score      = 0;
    moves[len].exhausted  = false;
    return len + 1;
}

// ── pawn move generation ──────────────────────────────────────────────────────

// Gliński's pawn rules:
//   - Forward: one step in (0,+1,-1) for white, (0,-1,+1) for black.
//   - Double push only from the starting rank (r==-1 for q≤0, r==-q-1 for q>0 for white; mirrored for black).
//   - Captures: the two rook-adjacent squares that flank the forward direction:
//       white captures via (+1,0,-1) and (-1,+1,0)
//       black captures via (+1,-1,0) and (-1,0,+1)
//   - En-passant and promotion handled as in standard chess.

// Capture direction tables for each colour.
static const int PAWN_CAPS_W[2][3] = {{ 1, 0,-1},{-1, 1, 0}};
static const int PAWN_CAPS_B[2][3] = {{ 1,-1, 0},{-1, 0, 1}};


static int pawnMoves(Board *board, int sq, bool isWhite, Move moves[], int len) {
    Cell *c = &CELLS[sq];
    int ptype   = isWhite ? PAWN_W : PAWN_B;
    Bitboard enemyOcc = isWhite ? board->occupancyBlack : board->occupancyWhite;

    // Forward step: white increases r, black decreases r.
    int fdr = isWhite ? 1 : -1, fds = isWhite ? -1 : 1;
    // fdq is always 0 (pawns move along their file).

    // ── single push ──────────────────────────────────────────────────────────
    int fwd = cubeToIndex(c->q, c->r + fdr, c->s + fds);
    if (fwd >= 0 && !bbGet(board->occupancy, fwd)) {
        // Promotion: no valid square one further step ahead.
        int nextFwd = cubeToIndex(c->q, c->r + fdr*2, c->s + fds*2);
        bool isPromo = (nextFwd < 0);

        if (isPromo) {
            int promos[4] = { isWhite ? QUEEN_W  : QUEEN_B,
                              isWhite ? ROOK_W   : ROOK_B,
                              isWhite ? BISHOP_W : BISHOP_B,
                              isWhite ? KNIGHT_W : KNIGHT_B };
            for (int p = 0; p < 4; p++)
                len = addMove(moves, len, sq, fwd, promos[p], ptype);
        } else {
            len = addMove(moves, len, sq, fwd, NO_PIECE, ptype);

            // ── double push from starting rank only ───────────────────────────
            // White start: r == (q<0 ? -1 : -q-1).  Black start: r == (q>0 ? 1 : 1-q).
            // Derived from the actual starting positions: white pawns always land on
            // the row r=-1 for files q≤0, and r=-q-1 for files q>0; black is mirrored.
            bool onStart = isWhite ? (c->r == (c->q < 0 ? -1 : -c->q - 1))
                                   : (c->r == (c->q > 0 ?  1 :  1 - c->q));
            if (onStart) {
                int dbl = cubeToIndex(c->q, c->r + fdr*2, c->s + fds*2);
                if (dbl >= 0 && !bbGet(board->occupancy, dbl))
                    len = addMove(moves, len, sq, dbl, NO_PIECE, ptype);
            }
        }
    }

    // ── captures: two flanking rook-adjacent squares ──────────────────────────
    const int (*caps)[3] = isWhite ? PAWN_CAPS_W : PAWN_CAPS_B;
    int nextFwd = cubeToIndex(c->q, c->r + fdr*2, c->s + fds*2);
    bool isPromoCapture = (nextFwd < 0);

    for (int d = 0; d < 2; d++) {
        int cap = cubeToIndex(c->q + caps[d][0],
                              c->r + caps[d][1],
                              c->s + caps[d][2]);
        if (cap < 0) continue;

        bool hasEnemy = bbGet(enemyOcc, cap);
        bool isEp     = (cap == board->epSquare);
        if (!hasEnemy && !isEp) continue;

        if (isPromoCapture) {
            int promos[4] = { isWhite ? QUEEN_W  : QUEEN_B,
                              isWhite ? ROOK_W   : ROOK_B,
                              isWhite ? BISHOP_W : BISHOP_B,
                              isWhite ? KNIGHT_W : KNIGHT_B };
            for (int p = 0; p < 4; p++)
                len = addMove(moves, len, sq, cap, promos[p], ptype);
        } else {
            len = addMove(moves, len, sq, cap, NO_PIECE, ptype);
        }
    }

    return len;
}

// ── legal move generation ─────────────────────────────────────────────────────

int legalMoves(Board *board, Move moves[]) {
    board->attacks = computeAttacks(*board);
    int len = 0;
    Move pseudo[512];
    int plen = 0;

    bool white = (board->turn == WHITE);
    Bitboard friendlyOcc = white ? board->occupancyWhite : board->occupancyBlack;

    // Piece bitboards for the side to move.
    Bitboard pawns   = white ? board->pawn_w   : board->pawn_b;
    Bitboard knights = white ? board->knight_w : board->knight_b;
    Bitboard bishops = white ? board->bishop_w : board->bishop_b;
    Bitboard rooks   = white ? board->rook_w   : board->rook_b;
    Bitboard queens  = white ? board->queen_w  : board->queen_b;
    Bitboard king    = white ? board->king_w   : board->king_b;
    int ktype        = white ? KING_W   : KING_B;
    int ntype        = white ? KNIGHT_W : KNIGHT_B;
    int btype        = white ? BISHOP_W : BISHOP_B;
    int rtype        = white ? ROOK_W   : ROOK_B;
    int qtype        = white ? QUEEN_W  : QUEEN_B;

    // Pawns
    for (Bitboard bb = pawns; !bbEmpty(bb);) {
        int sq = bbLsb(bb);
        plen = pawnMoves(board, sq, white, pseudo, plen);
        bb = bbClear(bb, sq);
    }

    // Knights
    for (Bitboard bb = knights; !bbEmpty(bb);) {
        int sq = bbLsb(bb);
        Bitboard atk = bbAnd(knightAttacks(sq), bbNot(friendlyOcc));
        for (Bitboard a = atk; !bbEmpty(a);) {
            int to = bbLsb(a);
            plen = addMove(pseudo, plen, sq, to, NO_PIECE, ntype);
            a = bbClear(a, to);
        }
        bb = bbClear(bb, sq);
    }

    // Bishops
    for (Bitboard bb = bishops; !bbEmpty(bb);) {
        int sq = bbLsb(bb);
        Bitboard atk = bbAnd(bishopAttacks(sq, board->occupancy), bbNot(friendlyOcc));
        for (Bitboard a = atk; !bbEmpty(a);) {
            int to = bbLsb(a);
            plen = addMove(pseudo, plen, sq, to, NO_PIECE, btype);
            a = bbClear(a, to);
        }
        bb = bbClear(bb, sq);
    }

    // Rooks
    for (Bitboard bb = rooks; !bbEmpty(bb);) {
        int sq = bbLsb(bb);
        Bitboard atk = bbAnd(rookAttacks(sq, board->occupancy), bbNot(friendlyOcc));
        for (Bitboard a = atk; !bbEmpty(a);) {
            int to = bbLsb(a);
            plen = addMove(pseudo, plen, sq, to, NO_PIECE, rtype);
            a = bbClear(a, to);
        }
        bb = bbClear(bb, sq);
    }

    // Queens
    for (Bitboard bb = queens; !bbEmpty(bb);) {
        int sq = bbLsb(bb);
        Bitboard atk = bbAnd(queenAttacks(sq, board->occupancy), bbNot(friendlyOcc));
        for (Bitboard a = atk; !bbEmpty(a);) {
            int to = bbLsb(a);
            plen = addMove(pseudo, plen, sq, to, NO_PIECE, qtype);
            a = bbClear(a, to);
        }
        bb = bbClear(bb, sq);
    }

    // King
    for (Bitboard bb = king; !bbEmpty(bb);) {
        int sq = bbLsb(bb);
        Bitboard atk = bbAnd(kingAttacks(sq), bbNot(friendlyOcc));
        for (Bitboard a = atk; !bbEmpty(a);) {
            int to = bbLsb(a);
            plen = addMove(pseudo, plen, sq, to, NO_PIECE, ktype);
            a = bbClear(a, to);
        }
        bb = bbClear(bb, sq);
    }

    // Filter pseudo-legal moves for legality
    for (int i = 0; i < plen; i++) {
        if (isLegal(*board, pseudo[i])) {
            pseudo[i].validation = LEGAL;
            moves[len++] = pseudo[i];
        }
    }

    return len;
}
