#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "fen.h"
#include "board.h"
#include "zobrist.h"
#include "movegen.h"

/**
 * Hex FEN format (file-major, white-towards-low-rank):
 *
 *   <file_a>/<file_b>/.../<file_l> <w|b> <ep|-> <halfmoves> <fullmoves>
 *
 * Each file section encodes squares from rank 1 to the file's max rank.
 * Uppercase = white pieces, lowercase = black.
 * Digits = consecutive empty squares.
 *
 * Empty board:   "6/7/8/9/10/11/10/9/8/7/6 w - 0 1"
 */

static const char PIECE_CHARS[12] = { 'P','N','B','R','Q','K','p','n','b','r','q','k' };

static int charToPiece(char c) {
    for (int i = 0; i < 12; i++)
        if (PIECE_CHARS[i] == c) return i;
    return NO_PIECE;
}

static void resetBoard(Board *board) {
    // Zero out the whole struct (clears lo/hi of every Bitboard to 0).
    memset(board, 0, sizeof(Board));
    board->turn      = WHITE;
    board->epSquare  = -1;
    board->halfmoves = 0;
    board->fullmoves = 1;
}

// Index of the square at (file column col, rank rank_1based) in the board.
// col: 0=a, 1=b, ..., 5=f, ..., 10=l
static int fileRankToIndex(int col, int rank_1based) {
    int q    = col - HEX_RADIUS;
    int rmin = (-HEX_RADIUS > -HEX_RADIUS - q) ? -HEX_RADIUS : -HEX_RADIUS - q;
    int r    = rmin + (rank_1based - 1);
    int s    = -q - r;
    return cubeToIndex(q, r, s);
}

void setFen(Board *board, const char *fen) {
    resetBoard(board);

    // ── Phase 1: parse piece placement (11 file groups separated by '/') ──────
    int col  = 0;
    int rank = 1;
    const char *p = fen;

    while (*p && *p != ' ') {
        if (*p == '/') {
            col++;
            rank = 1;
            p++;
            continue;
        }
        if (isdigit(*p)) {
            // Could be '1' followed by '0' or '1' (for 10 or 11 empty squares).
            int skip = *p - '0';
            if (skip == 1 && isdigit(*(p+1))) {
                skip = 10 + (*(p+1) - '0');
                p++;
            }
            rank += skip;
            p++;
            continue;
        }
        int piece = charToPiece(*p);
        if (piece != NO_PIECE && col < 11) {
            int idx = fileRankToIndex(col, rank);
            if (idx >= 0) {
                Bitboard *bb = pieceBB(board, piece);
                *bb = bbSet(*bb, idx);
                if (piece == KING_W) board->whiteKingSq = idx;
                if (piece == KING_B) board->blackKingSq = idx;
            }
            rank++;
        }
        p++;
    }

    // ── Phase 2: side to move ─────────────────────────────────────────────────
    if (*p == ' ') p++;
    if (*p == 'b') board->turn = BLACK;
    else           board->turn = WHITE;
    while (*p && *p != ' ') p++;

    // ── Phase 3: en-passant ───────────────────────────────────────────────────
    if (*p == ' ') p++;
    if (*p == '-') {
        board->epSquare = -1;
        p++;
    } else {
        char name[4] = {0};
        int ni = 0;
        while (*p && *p != ' ' && ni < 3) name[ni++] = *p++;
        board->epSquare = nameToIndex(name);
    }

    // ── Phase 4: halfmove clock ───────────────────────────────────────────────
    if (*p == ' ') p++;
    board->halfmoves = atoi(p);
    while (*p && *p != ' ') p++;

    // ── Phase 5: fullmove counter ─────────────────────────────────────────────
    if (*p == ' ') p++;
    board->fullmoves = atoi(p);
    if (board->fullmoves < 1) board->fullmoves = 1;

    // ── Finalize ──────────────────────────────────────────────────────────────
    computeOccupancyMasks(board);
    board->hash    = computeHash(*board);
    board->attacks = computeAttacks(*board);
}
