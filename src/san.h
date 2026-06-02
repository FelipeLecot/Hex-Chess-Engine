#ifndef SAN_H
#define SAN_H

#include "typedefs.h"

// Convert move to hex long algebraic notation, e.g. "f6g7" or "a1b2q".
// `buf` must be at least 8 bytes.
void moveToSan(Move move, char *buf);

// Parse hex long algebraic notation into a Move.
void sanToMove(Board board, Move *move, const char *san);

// Parse and execute a move given as hex SAN.
void pushSan(Board *board, const char *san);

#endif
