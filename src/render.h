#ifndef RENDER_H
#define RENDER_H

#include <stdio.h>
#include "typedefs.h"

// Render the board with pieces to stdout.
void renderBoard(Board board);

// Render a raw bitboard (which squares are set) to stdout.
void renderBitboard(Bitboard bb);

#endif
