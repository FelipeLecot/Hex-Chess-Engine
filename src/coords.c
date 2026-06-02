#include <stdio.h>
#include <string.h>
#include "coords.h"

Cell CELLS[NUM_SQUARES];
int CUBE_TO_INDEX[11][11][11];

// File letters, skipping 'j' as Gliński does. Column index = q + 5.
static const char FILE_LETTERS[11] = {
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'k', 'l'
};

// The 6 orthogonal hex directions (shared cell edges).
const int ROOK_DIRS[6][3] = {
    { 1, -1,  0}, { 1,  0, -1}, { 0,  1, -1},
    {-1,  1,  0}, {-1,  0,  1}, { 0, -1,  1}
};

// The 6 diagonal hex directions (shared cell vertices); each is the sum of
// two adjacent rook directions.
const int BISHOP_DIRS[6][3] = {
    { 1,  1, -2}, { 2, -1, -1}, { 1, -2,  1},
    {-1, -1,  2}, {-2,  1,  1}, {-1,  2, -1}
};

// King steps one cell in any of the 12 rook+bishop directions.
const int KING_DIRS[12][3] = {
    { 1, -1,  0}, { 1,  0, -1}, { 0,  1, -1},
    {-1,  1,  0}, {-1,  0,  1}, { 0, -1,  1},
    { 1,  1, -2}, { 2, -1, -1}, { 1, -2,  1},
    {-1, -1,  2}, {-2,  1,  1}, {-1,  2, -1}
};

// The 12 hex-knight leaps: every permutation of (±1, ±2, ±3) that sums to 0.
const int KNIGHT_DIRS[12][3] = {
    { 1,  2, -3}, { 2,  1, -3}, { 3, -1, -2}, { 3, -2, -1},
    { 2, -3,  1}, { 1, -3,  2}, {-1, -2,  3}, {-2, -1,  3},
    {-3,  1,  2}, {-3,  2,  1}, {-2,  3, -1}, {-1,  3, -2}
};

// Smallest valid r for a given q (used to number ranks from 1).
static int minR(int q) {
    int m = -HEX_RADIUS;
    if (-HEX_RADIUS - q > m) m = -HEX_RADIUS - q;
    return m;
}

void initCoords(void) {
    for (int a = 0; a < 11; a++)
        for (int b = 0; b < 11; b++)
            for (int c = 0; c < 11; c++)
                CUBE_TO_INDEX[a][b][c] = -1;

    int index = 0;
    // File-major order: q ascending (a..l), then rank ascending within the file.
    for (int q = -HEX_RADIUS; q <= HEX_RADIUS; q++) {
        int rmin = minR(q);
        for (int r = rmin; r <= HEX_RADIUS; r++) {
            int s = -q - r;
            if (s < -HEX_RADIUS || s > HEX_RADIUS) continue;

            int rank = r - rmin + 1;
            char letter = FILE_LETTERS[q + HEX_RADIUS];

            Cell *cell = &CELLS[index];
            cell->q = q;
            cell->r = r;
            cell->s = s;
            snprintf(cell->name, sizeof(cell->name), "%c%d", letter, rank);

            CUBE_TO_INDEX[q + HEX_RADIUS][r + HEX_RADIUS][s + HEX_RADIUS] = index;
            index++;
        }
    }
    // index now equals NUM_SQUARES (91); enforced indirectly by callers/tests.
}

int cubeToIndex(int q, int r, int s) {
    if (q < -HEX_RADIUS || q > HEX_RADIUS) return -1;
    if (r < -HEX_RADIUS || r > HEX_RADIUS) return -1;
    if (s < -HEX_RADIUS || s > HEX_RADIUS) return -1;
    if (q + r + s != 0) return -1;
    return CUBE_TO_INDEX[q + HEX_RADIUS][r + HEX_RADIUS][s + HEX_RADIUS];
}

int nameToIndex(const char *name) {
    if (!name) return -1;
    for (int i = 0; i < NUM_SQUARES; i++) {
        if (strncmp(CELLS[i].name, name, sizeof(CELLS[i].name)) == 0)
            return i;
    }
    return -1;
}

const char *indexToName(int index) {
    if (!isValidIndex(index)) return "??";
    return CELLS[index].name;
}
