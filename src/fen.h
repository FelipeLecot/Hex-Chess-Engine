#ifndef FEN_H
#define FEN_H

#include "typedefs.h"

// Gliński starting position in hex FEN:
//   ranks listed innermost (rank 1 of each file) first, separated by '/'.
//   Within each rank group, squares are file a..l.
// Format: <pieces-per-file-groups> <w|b> <ep|-> <halfmoves> <fullmoves>
// Example startpos uses the canonical Gliński piece placement.
#define START_HEX_FEN "6/P5p/RP4pr/N1P3p1n/Q2P2p2q/BBB1P1p1bbb/K2P2p2k/N1P3p1n/RP4pr/P5p/6 w - 0 1"

void setFen(Board *board, const char *fen);

// Serialise *board into a NUL-terminated FEN string written to buf.
// buf must be at least 256 bytes.
void getFen(Board board, char *buf);

#endif