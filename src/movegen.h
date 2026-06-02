#ifndef MOVEGEN_H
#define MOVEGEN_H

#include "typedefs.h"

// Generates all legal moves for the side to move.
// Returns the count of legal moves written to `moves`.
int legalMoves(Board *board, Move moves[]);

// Returns whether `square` is attacked by any piece of the side NOT to move.
bool isSquareAttacked(Board board, int square);

// Computes the full attack mask of the side NOT to move
// (used for checks, pins, ep legality, and board.attacks).
Bitboard computeAttacks(Board board);

#endif
