#ifndef MAGICS_H
#define MAGICS_H

#include <stdint.h>
#include "typedefs.h"
#include "bitboards.h"
#include "coords.h"

// Per-ray precomputed attack tables for sliding pieces (rook and bishop).
//
// Why not per-square magic bitboards?
// The centre square (f6) has 6 rook rays of length 5, giving 4 relevant
// blocker bits each = 24 bits total.  A per-square table would need
// 2^24 = 16 M entries × 16 bytes = 256 MB for that one square alone.
//
// Per-ray tables are feasible: each ray has at most 9 blocker squares
// (length-10 ray, endpoint excluded since it never blocks further travel),
// so 2^9 = 512 entries per (square, direction) pair.
// Total: 91 × 12 × 512 × 16 bytes ≈ 9 MB, filled by initMagics() in < 1 ms.
//
// Lookup is O(ray_length) for pattern extraction then O(1) table read —
// the same number of iterations as on-the-fly, but without the cube-
// coordinate arithmetic and bounds-checking on each step.

#define MAX_RAY_LEN      10   // longest possible ray on the 91-cell board
#define MAX_RAY_PATTERNS 512  // 2^(MAX_RAY_LEN - 1)

// Ordered list of squares on one sliding-piece ray.
typedef struct {
    int squares[MAX_RAY_LEN]; // linear indices, nearest-first from the source
    int len;                   // number of squares (0 if the ray leaves the board immediately)
} Ray;

extern Ray ROOK_RAYS[NUM_SQUARES][6];
extern Ray BISHOP_RAYS[NUM_SQUARES][6];

// ROOK_RAY_ATK[sq][d][pat]   — rook on sq, direction d, occupancy pattern pat
// BISHOP_RAY_ATK[sq][d][pat] — bishop on sq, direction d, occupancy pattern pat
//
// pat encodes the occupancy of ray squares 0 .. len-2 (the endpoint is excluded
// because it is always attacked regardless of whether it is occupied).
extern Bitboard ROOK_RAY_ATK[NUM_SQUARES][6][MAX_RAY_PATTERNS];
extern Bitboard BISHOP_RAY_ATK[NUM_SQUARES][6][MAX_RAY_PATTERNS];

// Fill all ray and attack tables.  Must be called after initCoords().
void initMagics(void);

// ── inline lookup helpers ─────────────────────────────────────────────────────

// Pack the occupancy of ray squares 0..len-2 into a uint16_t index.
static inline uint16_t rayPattern(Bitboard occ, const Ray *ray) {
    uint16_t pat = 0;
    int n = ray->len - 1; // exclude endpoint
    for (int i = 0; i < n; i++)
        if (bbGet(occ, ray->squares[i]))
            pat |= (uint16_t)(1u << i);
    return pat;
}

static inline Bitboard rookAttacksTable(int sq, Bitboard occ) {
    Bitboard atk = bbZero();
    for (int d = 0; d < 6; d++)
        atk = bbOr(atk, ROOK_RAY_ATK[sq][d][rayPattern(occ, &ROOK_RAYS[sq][d])]);
    return atk;
}

static inline Bitboard bishopAttacksTable(int sq, Bitboard occ) {
    Bitboard atk = bbZero();
    for (int d = 0; d < 6; d++)
        atk = bbOr(atk, BISHOP_RAY_ATK[sq][d][rayPattern(occ, &BISHOP_RAYS[sq][d])]);
    return atk;
}

#endif
