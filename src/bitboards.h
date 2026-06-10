#ifndef BITBOARDS_H
#define BITBOARDS_H

#include "typedefs.h"

// Single-bit boards: SQUARE_BITBOARDS[i] has exactly bit i set.
extern Bitboard SQUARE_BITBOARDS[NUM_SQUARES];

// Bits 64..90 occupy the low 27 bits of `hi`.
#define HEX_HIGH_MASK ((1ULL << (NUM_SQUARES - 64)) - 1)

void initBitboards(void);

static inline Bitboard bbZero(void) {
    Bitboard b = {0, 0};
    return b;
}

static inline bool bbEmpty(Bitboard b) {
    return b.lo == 0 && b.hi == 0;
}

static inline bool bbEqual(Bitboard a, Bitboard b) {
    return a.lo == b.lo && a.hi == b.hi;
}

static inline bool bbGet(Bitboard b, int sq) {
    return sq < 64 ? (b.lo >> sq) & 1ULL
                   : (b.hi >> (sq - 64)) & 1ULL;
}

static inline Bitboard bbSet(Bitboard b, int sq) {
    if (sq < 64) b.lo |= (1ULL << sq);
    else         b.hi |= (1ULL << (sq - 64));
    return b;
}

static inline Bitboard bbClear(Bitboard b, int sq) {
    if (sq < 64) b.lo &= ~(1ULL << sq);
    else         b.hi &= ~(1ULL << (sq - 64));
    return b;
}

static inline Bitboard bbAnd(Bitboard a, Bitboard b) {
    Bitboard r = {a.lo & b.lo, a.hi & b.hi};
    return r;
}

static inline Bitboard bbOr(Bitboard a, Bitboard b) {
    Bitboard r = {a.lo | b.lo, a.hi | b.hi};
    return r;
}

static inline Bitboard bbXor(Bitboard a, Bitboard b) {
    Bitboard r = {a.lo ^ b.lo, a.hi ^ b.hi};
    return r;
}

static inline Bitboard bbNot(Bitboard a) {
    Bitboard r = {~a.lo, (~a.hi) & HEX_HIGH_MASK};
    return r;
}

static inline int bbPopcount(Bitboard b) {
    return __builtin_popcountll(b.lo) + __builtin_popcountll(b.hi);
}

// Index of the least-significant set bit. Caller must guarantee b is non-empty.
static inline int bbLsb(Bitboard b) {
    if (b.lo) return __builtin_ctzll(b.lo);
    return 64 + __builtin_ctzll(b.hi);
}

#endif
