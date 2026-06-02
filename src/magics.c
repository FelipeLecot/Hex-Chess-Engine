#include "magics.h"

Ray      ROOK_RAYS[NUM_SQUARES][6];
Ray      BISHOP_RAYS[NUM_SQUARES][6];
Bitboard ROOK_RAY_ATK[NUM_SQUARES][6][MAX_RAY_PATTERNS];
Bitboard BISHOP_RAY_ATK[NUM_SQUARES][6][MAX_RAY_PATTERNS];

// Walk from sq in cube direction (dq,dr,ds) and record each reachable square.
static void buildRay(Ray *ray, int sq, int dq, int dr, int ds) {
    ray->len = 0;
    Cell *c = &CELLS[sq];
    int q = c->q + dq, r = c->r + dr, s = c->s + ds;
    while (ray->len < MAX_RAY_LEN) {
        int ni = cubeToIndex(q, r, s);
        if (ni < 0) break;
        ray->squares[ray->len++] = ni;
        q += dq; r += dr; s += ds;
    }
}

// Fill one ray's attack table.
// pat encodes occupancy of squares 0..len-2; the endpoint (len-1) is always
// attacked so it is excluded from the pattern, halving the table size.
static void fillTable(Bitboard table[MAX_RAY_PATTERNS], const Ray *ray) {
    int n    = ray->len;
    int npat = (n <= 1) ? 1 : (1 << (n - 1));
    for (int pat = 0; pat < npat; pat++) {
        Bitboard atk = bbZero();
        for (int i = 0; i < n; i++) {
            atk = bbSet(atk, ray->squares[i]);
            // Stop after the first occupied non-endpoint square (blocker is included).
            if (i < n - 1 && ((pat >> i) & 1)) break;
        }
        table[pat] = atk;
    }
}

void initMagics(void) {
    for (int sq = 0; sq < NUM_SQUARES; sq++) {
        for (int d = 0; d < 6; d++) {
            buildRay(&ROOK_RAYS[sq][d],
                     sq, ROOK_DIRS[d][0], ROOK_DIRS[d][1], ROOK_DIRS[d][2]);
            fillTable(ROOK_RAY_ATK[sq][d], &ROOK_RAYS[sq][d]);

            buildRay(&BISHOP_RAYS[sq][d],
                     sq, BISHOP_DIRS[d][0], BISHOP_DIRS[d][1], BISHOP_DIRS[d][2]);
            fillTable(BISHOP_RAY_ATK[sq][d], &BISHOP_RAYS[sq][d]);
        }
    }
}
