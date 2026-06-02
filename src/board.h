#ifndef BOARD_H
#define BOARD_H

#include "typedefs.h"
#include "bitboards.h"
#include "coords.h"

// ── occupancy ────────────────────────────────────────────────────────────────
void computeOccupancyMasks(Board *board);

// ── board operations ─────────────────────────────────────────────────────────
void pushMove(Board *board, Move move);

// ── game result ──────────────────────────────────────────────────────────────
// Returns UN_DETERMINED, WHITE_WIN, BLACK_WIN, or DRAW.
int result(Board board, Move legal[], int count);

// ── debug display ─────────────────────────────────────────────────────────────
void printBoard(Board board);

// ── piece-bitboard access (0-based piece enum offset from pawn_w) ─────────────
// Returns a pointer to the bitboard for piece i in board.
Bitboard *pieceBB(Board *board, int pieceType);

#endif
