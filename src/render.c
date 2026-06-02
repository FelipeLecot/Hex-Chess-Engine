#include <stdio.h>
#include <string.h>
#include "render.h"
#include "board.h"
#include "coords.h"

// File letters for the 11 columns (no 'j' in Gliński's notation).
static const char FILE_NAMES[] = "abcdefghikl";

// How far (in visual rows) the first cell of each file is from the top.
// File f (centre) starts at vr=0; outermost files a and l start at vr=5.
static const int VC_DIST_FROM_TOP[11] = {5,4,3,2,1,0,1,2,3,4,5};

// Number of ranks in each file column.
static int vcMaxRank(int vc) {
    int q    = vc - HEX_RADIUS;
    int rmin = (-HEX_RADIUS > -HEX_RADIUS - q) ? -HEX_RADIUS : -HEX_RADIUS - q;
    int rmax = ( HEX_RADIUS < HEX_RADIUS  - q) ?  HEX_RADIUS :  HEX_RADIUS - q;
    return rmax - rmin + 1;
}

// Returns true and fills *out_idx if (vr, vc) is the visual centre of a cell.
static bool cellAt(int vr, int vc, int *out_idx) {
    int dist  = VC_DIST_FROM_TOP[vc];
    int delta = vr - dist;
    if (delta < 0 || delta % 2 != 0) return false;
    int rank = delta / 2 + 1;
    if (rank < 1 || rank > vcMaxRank(vc)) return false;

    int col  = vc;
    int q    = col - HEX_RADIUS;
    int rmin = (-HEX_RADIUS > -HEX_RADIUS - q) ? -HEX_RADIUS : -HEX_RADIUS - q;
    int r    = rmin + (rank - 1);
    int s    = -q - r;
    int idx  = cubeToIndex(q, r, s);
    if (idx < 0) return false;
    *out_idx = idx;
    return true;
}

static char pieceChar(Board board, int idx) {
    const char syms[12] = "PNBRQKpnbrqk";
    for (int p = 0; p < 12; p++)
        if (bbGet(*pieceBB((Board*)&board, p), idx))
            return syms[p];
    return '.';
}

// Write a square name right-justified into buf[0..2], buf[3] = ' '.
// Names are 2-3 chars ("a1".."f11"), so this always fits.
static void writeLeftLabel(char *buf, const char *name) {
    int nlen = (int)strlen(name);
    buf[0] = buf[1] = buf[2] = ' ';
    buf[3] = ' ';
    for (int i = 0; i < nlen; i++)
        buf[3 - nlen + i] = name[i];
}

// Write " <name>" into buf starting at pos.
static void writeRightLabel(char *buf, int pos, const char *name, int bufsz) {
    if (pos >= bufsz - 4) return;
    buf[pos] = ' ';
    for (int i = 0; name[i] && pos + 1 + i < bufsz - 1; i++)
        buf[pos + 1 + i] = name[i];
}

void renderBoard(Board board) {
    char buf[160];
    int NUM_VR = 26;
    int NUM_VC = 11;

    printf("\n");

    // ── file-letter header ────────────────────────────────────────────────────
    // Each file letter sits above the piece position of its column (offset +2).
    {
        memset(buf, ' ', sizeof(buf) - 1);
        buf[sizeof(buf) - 1] = '\0';
        for (int vc = 0; vc < NUM_VC; vc++) {
            int pos = 5 + vc * 6 + 2;
            if (pos < (int)sizeof(buf) - 1)
                buf[pos] = FILE_NAMES[vc];
        }
        int len = (int)sizeof(buf) - 2;
        while (len >= 0 && buf[len] == ' ') len--;
        buf[len + 1] = '\0';
        printf("    %s\n\n", buf);
    }

    // ── board rows ────────────────────────────────────────────────────────────
    for (int vr = 0; vr <= NUM_VR; vr++) {
        memset(buf, ' ', sizeof(buf) - 1);
        buf[sizeof(buf) - 1] = '\0';

        // Determine occupied column range for this visual row.
        int leftmost  = 99;
        int rightmost = -1;
        for (int vc = 0; vc < NUM_VC; vc++) {
            int unused;
            bool cur  = cellAt(vr,     vc, &unused);
            bool prev = (vr > 0) && cellAt(vr - 1, vc, &unused);
            if (cur || prev) {
                if (vc < leftmost)  leftmost  = vc;
                if (vc > rightmost) rightmost = vc;
            }
        }
        if (leftmost > rightmost) continue;

        // ── cell content ──────────────────────────────────────────────────────
        for (int vc = leftmost; vc <= rightmost; vc++) {
            int pos = 5 + vc * 6;
            int idx;
            bool is_cur  = cellAt(vr,     vc, &idx);
            bool is_prev = (vr > 0) && cellAt(vr - 1, vc, &idx);

            if (is_cur) {
                char pc = pieceChar(board, idx);
                if (pos + 5 < (int)sizeof(buf)) {
                    buf[pos + 0] = '/';
                    buf[pos + 1] = ' ';
                    buf[pos + 2] = pc;
                    buf[pos + 3] = ' ';
                    buf[pos + 4] = ' ';
                    buf[pos + 5] = '\\';
                }
            } else if (is_prev) {
                if (pos + 5 < (int)sizeof(buf)) {
                    buf[pos + 0] = '\\';
                    buf[pos + 1] = ' ';
                    buf[pos + 2] = '_';
                    buf[pos + 3] = '_';
                    buf[pos + 4] = ' ';
                    buf[pos + 5] = '/';
                }
            }
        }

        // ── inter-column bridges ──────────────────────────────────────────────
        for (int vc = leftmost; vc < rightmost; vc++) {
            int idx1;
            bool vc_has = cellAt(vr, vc, &idx1) || (vr > 0 && cellAt(vr-1, vc, &idx1));
            if (!vc_has) continue;
            int idx2;
            if (vc + 1 < NUM_VC) {
                bool next_has = cellAt(vr, vc+1, &idx2) || (vr>0 && cellAt(vr-1, vc+1, &idx2));
                if (!next_has && vc + 2 <= rightmost) {
                    int idx3;
                    bool jump_has = cellAt(vr, vc+2, &idx3) || (vr>0 && cellAt(vr-1, vc+2, &idx3));
                    if (jump_has) {
                        int bridge = 5 + vc * 6 + 6;
                        if (bridge + 4 < (int)sizeof(buf)) {
                            buf[bridge + 1] = ' ';
                            buf[bridge + 2] = '_';
                            buf[bridge + 3] = '_';
                            buf[bridge + 4] = ' ';
                        }
                    }
                }
            }
        }

        // ── rank labels ───────────────────────────────────────────────────────
        // Left: name of the leftmost CELL CENTRE on this row (e.g. "a1", "f6").
        {
            int idx;
            if (cellAt(vr, leftmost, &idx))
                writeLeftLabel(buf, CELLS[idx].name);
        }

        // Right: name of the rightmost cell centre (skip if same column as left).
        if (rightmost != leftmost) {
            int idx;
            if (cellAt(vr, rightmost, &idx)) {
                int pos = 5 + rightmost * 6 + 6;
                writeRightLabel(buf, pos, CELLS[idx].name, (int)sizeof(buf));
            }
        }

        // Trim trailing spaces and print.
        int len = (int)sizeof(buf) - 2;
        while (len >= 0 && buf[len] == ' ') len--;
        buf[len + 1] = '\0';
        printf("%s\n", buf);
    }

    printf("\nTurn: %s  Ep: %s  Half: %d  Full: %d\n",
        board.turn ? "White" : "Black",
        board.epSquare < 0 ? "-" : indexToName(board.epSquare),
        board.halfmoves, board.fullmoves);
}

void renderBitboard(Bitboard bb) {
    printf("\nBitboard (set squares):");
    bool any = false;
    for (int i = 0; i < NUM_SQUARES; i++) {
        if (bbGet(bb, i)) {
            printf(" %s", indexToName(i));
            any = true;
        }
    }
    if (!any) printf(" (empty)");
    printf("\n");
}
