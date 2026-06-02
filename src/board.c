#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "board.h"
#include "zobrist.h"

// ── helpers ───────────────────────────────────────────────────────────────────

Bitboard *pieceBB(Board *board, int pieceType) {
    return &board->pawn_w + pieceType;
}

void computeOccupancyMasks(Board *board) {
    board->occupancyWhite = bbOr(bbOr(bbOr(bbOr(bbOr(
        board->pawn_w, board->knight_w), board->bishop_w),
        board->rook_w), board->queen_w), board->king_w);

    board->occupancyBlack = bbOr(bbOr(bbOr(bbOr(bbOr(
        board->pawn_b, board->knight_b), board->bishop_b),
        board->rook_b), board->queen_b), board->king_b);

    board->occupancy = bbOr(board->occupancyWhite, board->occupancyBlack);
}

// ── capture helper ────────────────────────────────────────────────────────────

// Remove any opponent piece on `sq`. Piece loop starts at the opponent's pawn.
static void removeCapturedPiece(Board *board, int sq) {
    int start = board->turn ? PAWN_B : PAWN_W;
    for (int i = 0; i < 6; i++) {
        Bitboard *bb = pieceBB(board, start + i);
        if (bbGet(*bb, sq)) {
            board->hash ^= PIECES[start + i][sq];
            *bb = bbClear(*bb, sq);
            return;
        }
    }
}

// ── en-passant move ───────────────────────────────────────────────────────────

static void makeEnPassantMove(Board *board, Move move) {
    // The pawn we are capturing sits on a different square than toSquare.
    // In hex chess, white pawns move "north" (s decreases) and black "south".
    // The captured pawn is on move.toSquare shifted one step backward.
    // We store epSquare as the destination of the moving pawn, and the
    // captured pawn is found by tracing one step in the attacker's backward direction.
    //
    // Hex ep: the captured pawn lives one step behind the toSquare (behind the
    // mover's direction). "North" for white is rook direction 0 (dq=1,dr=-1,ds=0)
    // viewed from the captured pawn's side. We find the captured pawn by scanning
    // opponent pawns adjacent to the ep square.
    Cell *to = &CELLS[move.toSquare];

    // The captured pawn is on epCapturedSq: the opponent pawn adjacent to toSquare
    // that moved two squares last turn. It was stored when the double-push happened.
    // Rather than re-derive direction here, we walk all 6 rook neighbors of toSquare
    // and remove the first opponent pawn we find.
    int captureSq = -1;
    Bitboard *opPawns = board->turn ? &board->pawn_b : &board->pawn_w;
    for (int d = 0; d < 6 && captureSq == -1; d++) {
        int nq = to->q + ROOK_DIRS[d][0];
        int nr = to->r + ROOK_DIRS[d][1];
        int ns = to->s + ROOK_DIRS[d][2];
        int ni = cubeToIndex(nq, nr, ns);
        if (ni >= 0 && bbGet(*opPawns, ni)) {
            captureSq = ni;
        }
    }

    if (captureSq >= 0) {
        int opPawnType = board->turn ? PAWN_B : PAWN_W;
        board->hash ^= PIECES[opPawnType][captureSq];
        *opPawns = bbClear(*opPawns, captureSq);
    }

    int myPawnType = board->turn ? PAWN_W : PAWN_B;
    Bitboard *myPawns = board->turn ? &board->pawn_w : &board->pawn_b;

    board->hash ^= PIECES[myPawnType][move.fromSquare];
    board->hash ^= PIECES[myPawnType][move.toSquare];
    board->hash ^= EN_PASSANT[board->epSquare];

    *myPawns = bbClear(*myPawns, move.fromSquare);
    *myPawns = bbSet(*myPawns, move.toSquare);

    board->epSquare = -1;
    board->turn ^= 1;
    board->hash ^= WHITE_TO_MOVE;
    computeOccupancyMasks(board);
}

// ── main move executor ────────────────────────────────────────────────────────

void pushMove(Board *board, Move move) {
    bool isEp = (move.toSquare == board->epSquare)
             && (move.pieceType == PAWN_W || move.pieceType == PAWN_B);

    if (isEp) {
        makeEnPassantMove(board, move);
        return;
    }

    // XOR out ep square if any
    if (board->epSquare >= 0) {
        board->hash ^= EN_PASSANT[board->epSquare];
        board->epSquare = -1;
    }

    board->hash ^= PIECES[move.pieceType][move.fromSquare];
    board->hash ^= WHITE_TO_MOVE;

    // Lift the piece
    Bitboard *moved = pieceBB(board, move.pieceType);
    *moved = bbClear(*moved, move.fromSquare);

    // Detect pawn double-push for en passant
    if (move.pieceType == PAWN_W || move.pieceType == PAWN_B) {
        Cell *from = &CELLS[move.fromSquare];
        Cell *to   = &CELLS[move.toSquare];
        // A double-push in hex moves 2 steps in the same rook direction.
        // We record the ep square only when from and to differ by exactly 2 rook steps.
        int dq = to->q - from->q, dr = to->r - from->r, ds = to->s - from->s;
        bool doublePush = false;
        for (int d = 0; d < 6; d++) {
            if (dq == ROOK_DIRS[d][0]*2 && dr == ROOK_DIRS[d][1]*2 && ds == ROOK_DIRS[d][2]*2) {
                doublePush = true;
                board->epSquare = cubeToIndex(from->q + ROOK_DIRS[d][0],
                                              from->r + ROOK_DIRS[d][1],
                                              from->s + ROOK_DIRS[d][2]);
                break;
            }
        }
        if (doublePush && board->epSquare >= 0) {
            board->hash ^= EN_PASSANT[board->epSquare];
        }
    }

    // Remove any captured opponent piece
    removeCapturedPiece(board, move.toSquare);

    // Place the piece (or the promoted piece)
    if (move.promotion < 0) {
        *moved = bbSet(*moved, move.toSquare);
        board->hash ^= PIECES[move.pieceType][move.toSquare];

        if (move.pieceType == KING_W) board->whiteKingSq = move.toSquare;
        if (move.pieceType == KING_B) board->blackKingSq = move.toSquare;
    } else {
        Bitboard *promoBB = pieceBB(board, move.promotion);
        *promoBB = bbSet(*promoBB, move.toSquare);
        board->hash ^= PIECES[move.promotion][move.toSquare];
    }

    board->turn ^= 1;
    computeOccupancyMasks(board);
}

// ── insufficient material ─────────────────────────────────────────────────────

static bool insufficientMaterial(Board board) {
    // If any rook, queen, or pawn exists, there's enough material.
    if (!bbEmpty(board.pawn_w)  || !bbEmpty(board.pawn_b))  return false;
    if (!bbEmpty(board.rook_w)  || !bbEmpty(board.rook_b))  return false;
    if (!bbEmpty(board.queen_w) || !bbEmpty(board.queen_b)) return false;
    // K vs K, KN vs K, or KB vs K are draws.
    int minors = bbPopcount(board.knight_w) + bbPopcount(board.knight_b)
               + bbPopcount(board.bishop_w) + bbPopcount(board.bishop_b);
    return minors <= 1;
}

// ── game result ───────────────────────────────────────────────────────────────

int result(Board board, Move legal[], int count) {
    if (insufficientMaterial(board)) return DRAW;
    if (count > 0) return UN_DETERMINED;

    // No legal moves: check if the king is in check
    Bitboard kingSq = board.turn ? SQUARE_BITBOARDS[board.whiteKingSq]
                                 : SQUARE_BITBOARDS[board.blackKingSq];
    bool inCheck = !bbEmpty(bbAnd(kingSq, board.attacks));
    if (inCheck) return board.turn ? BLACK_WIN : WHITE_WIN;
    return DRAW; // stalemate
}

// ── debug display ─────────────────────────────────────────────────────────────

void printBoard(Board board) {
    const char *symbols = "PNBRQKpnbrqk";
    printf("\n");
    for (int i = 0; i < NUM_SQUARES; i++) {
        char c = '.';
        for (int p = 0; p < 12; p++) {
            if (bbGet(*pieceBB(&board, p), i)) {
                c = symbols[p];
                break;
            }
        }
        printf("%s:%c  ", indexToName(i), c);
        if ((i + 1) % 11 == 0) printf("\n");
    }
    printf("\nTurn: %s\n", board.turn ? "White" : "Black");
    printf("Ep: %s\n", board.epSquare < 0 ? "-" : indexToName(board.epSquare));
    printf("\n");
}
