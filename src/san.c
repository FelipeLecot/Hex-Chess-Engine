#include <string.h>
#include "san.h"
#include "board.h"

static const char PROMO_CHARS[6] = { 'p', 'n', 'b', 'r', 'q', 'k' };
// Promotion chars (black piece enum offset gives the char).
// We map QUEEN_W/B -> 'q', ROOK -> 'r', BISHOP -> 'b', KNIGHT -> 'n'.

static char promoChar(int promo) {
    if (promo < 0) return '\0';
    int p = promo % 6; // collapse white/black to 0-5
    return PROMO_CHARS[p];
}

static int charToPromo(char c, int isWhite) {
    switch (c) {
        case 'q': return isWhite ? QUEEN_W  : QUEEN_B;
        case 'r': return isWhite ? ROOK_W   : ROOK_B;
        case 'b': return isWhite ? BISHOP_W : BISHOP_B;
        case 'n': return isWhite ? KNIGHT_W : KNIGHT_B;
        default:  return NO_PIECE;
    }
}

void moveToSan(Move move, char *buf) {
    const char *from = indexToName(move.fromSquare);
    const char *to   = indexToName(move.toSquare);
    int i = 0;
    while (from[i]) buf[i] = from[i], i++;
    int j = 0;
    while (to[j]) buf[i++] = to[j++];
    char pc = promoChar(move.promotion);
    if (pc) buf[i++] = pc;
    buf[i] = '\0';
}

void sanToMove(Board board, Move *move, const char *san) {
    memset(move, 0, sizeof(Move));
    move->promotion = NO_PIECE;

    // Parse from-square: file char + one or two digit rank
    char from_name[4] = {0};
    int i = 0;
    from_name[i++] = san[0]; // file letter
    from_name[i++] = san[1]; // first rank digit
    if (san[2] >= '0' && san[2] <= '9' && san[3] >= 'a' && san[3] <= 'l') {
        // Three-char from like "l11": file + two digits
        from_name[i++] = san[2];
        san += 3;
    } else {
        san += 2;
    }
    move->fromSquare = nameToIndex(from_name);

    // Parse to-square
    char to_name[4] = {0};
    int j = 0;
    to_name[j++] = san[0];
    to_name[j++] = san[1];
    if (san[2] >= '0' && san[2] <= '9') {
        to_name[j++] = san[2];
        san += 3;
    } else {
        san += 2;
    }
    move->toSquare = nameToIndex(to_name);

    // Promotion?
    if (*san) {
        move->promotion = charToPromo(*san, board.turn);
    }

    // Determine piece type
    for (int p = 0; p < 12; p++) {
        if (bbGet(*pieceBB((Board*)&board, p), move->fromSquare)) {
            move->pieceType = p;
            break;
        }
    }
}

void pushSan(Board *board, const char *san) {
    Move move;
    sanToMove(*board, &move, san);
    pushMove(board, move);
}
