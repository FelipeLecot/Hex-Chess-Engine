#ifndef TYPEDEFS_H
#define TYPEDEFS_H

#include <stdint.h>
#include <stdbool.h>

#define BLACK 0
#define WHITE 1

// Gliński's hexagonal chess: a hexagon of radius 5 -> 91 cells.
#define NUM_SQUARES 91
#define HEX_RADIUS 5

/**
 * A bitboard for the 91-cell hex board.
 * Bits 0..63 live in `lo`, bits 64..90 in `hi` (27 bits used).
 * The whole engine uses this single 128-bit type; there is no 64-bit board.
 */
typedef struct {
    uint64_t lo;
    uint64_t hi;
} Bitboard;

/** A single board cell, addressed by cube coordinates (q + r + s == 0). */
typedef struct {
    int q, r, s;
    char name[4]; // algebraic name, e.g. "f6", "l11"
} Cell;

typedef struct {
    Bitboard pawn_w;
    Bitboard knight_w;
    Bitboard bishop_w;
    Bitboard rook_w;
    Bitboard queen_w;
    Bitboard king_w;
    Bitboard pawn_b;
    Bitboard knight_b;
    Bitboard bishop_b;
    Bitboard rook_b;
    Bitboard queen_b;
    Bitboard king_b;
    int turn;          // WHITE or BLACK
    int epSquare;      // en passant target square, or -1 (no castling in hex chess)
    int halfmoves;
    int fullmoves;
    int whiteKingSq;
    int blackKingSq;
    Bitboard occupancy;
    Bitboard occupancyWhite;
    Bitboard occupancyBlack;
    uint64_t hash;     // Zobrist hash (64 bits is plenty)
    Bitboard attacks;  // attack mask of the side NOT to move
} Board;

typedef struct {
    int fromSquare;
    int toSquare;
    int promotion;   // promoted piece type, or -1
    int validation;  // NOT_VALIDATED / LEGAL / ILLEGAL
    int pieceType;
    int score;
    bool exhausted;
} Move;

enum PIECES {
    PAWN_W, KNIGHT_W, BISHOP_W, ROOK_W, QUEEN_W, KING_W,
    PAWN_B, KNIGHT_B, BISHOP_B, ROOK_B, QUEEN_B, KING_B,
    NO_PIECE = -1
};

enum MOVE_VALIDATION { NOT_VALIDATED, LEGAL, ILLEGAL };

enum GAME_RESULT { UN_DETERMINED, WHITE_WIN, BLACK_WIN, DRAW };

#endif
