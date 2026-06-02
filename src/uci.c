#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include "board.h"
#include "fen.h"
#include "search.h"
#include "movegen.h"
#include "evaluation.h"
#include "zobrist.h"
#include "san.h"
#include "bitboards.h"
#include "coords.h"
#include "render.h"
#include "magics.h"

static const char *AUTHOR      = "Felipe Lecot";
static const char *ENGINE_NAME = "Hex Chess Engine";
static const int DEFAULT_DEPTH = 5;
static const int MAX_DEPTH     = 64;

static void printEngineInfo(void) {
    printf("id name %s\n", ENGINE_NAME);
    printf("id author %s\n", AUTHOR);
    printf("uciok\n");
}

static void parsePosition(const char *cmd, Board *board) {
    const char *p = cmd + 9; // skip "position "

    if (strncmp(p, "startpos", 8) == 0) {
        setFen(board, START_HEX_FEN);
        p += 8;
    } else if (strncmp(p, "fen", 3) == 0) {
        p += 4;
        setFen(board, p);
        // Advance past the FEN string (5 spaces = 5 sections)
        int spaces = 0;
        while (*p && spaces < 5) { if (*p++ == ' ') spaces++; }
    } else {
        return;
    }

    const char *moves = strstr(p, "moves");
    if (moves) {
        moves += 6;
        while (*moves) {
            while (*moves == ' ') moves++;
            if (!*moves) break;
            const char *end = moves;
            while (*end && *end != ' ') end++;
            char san[8] = {0};
            strncpy(san, moves, (size_t)(end - moves));
            pushSan(board, san);
            moves = end;
        }
    }
}

static void parseGo(const char *cmd, Board board) {
    int maxDepth = DEFAULT_DEPTH;
    const char *p = strstr(cmd, "depth");
    if (p) {
        p += 5;
        while (*p == ' ') p++;
        int parsed = atoi(p);
        if (parsed > 0 && parsed <= MAX_DEPTH) maxDepth = parsed;
    }

    SearchContext ctx;
    clock_t start = clock();
    int eval = 0;
    int totalNodes = 0;

    // Iterative deepening: search depth 1, 2, … maxDepth.
    // Each completed depth warms the TT so score_moves() can order the PV move
    // first at the next depth, dramatically increasing alpha-beta cutoffs.
    // The overhead is ~20% extra work (geometric series dominated by last term).
    for (int depth = 1; depth <= maxDepth; depth++) {
        eval = search(board, depth, &ctx);
        totalNodes += ctx.nodesSearched;
        double ms = (double)(clock() - start) * 1000.0 / CLOCKS_PER_SEC;
        char san[8] = {0};
        moveToSan(ctx.bestMove, san);
        printf("info depth %d time %.0f nodes %d score cp %d pv %s\n",
               depth, ms, totalNodes, eval, san);
        fflush(stdout);
    }

    char san[8] = {0};
    moveToSan(ctx.bestMove, san);
    printf("bestmove %s\n", san);
    fflush(stdout);
}

int main(void) {
    srand((unsigned)time(NULL));

    // Initialise all subsystems.
    initCoords();
    initBitboards();
    initMagics();
    initZobrist();
    initEvaluation();

    setbuf(stdin,  NULL);
    setbuf(stdout, NULL);

    char input[2000];
    Board board;
    setFen(&board, START_HEX_FEN);

    while (1) {
        memset(input, 0, sizeof(input));
        fflush(stdout);

        if (!fgets(input, (int)sizeof(input), stdin)) {
            if (feof(stdin)) break;
            continue;
        }

        if (input[0] == '\n' || input[0] == '\r') continue;

        // Strip trailing newline
        int ilen = (int)strlen(input);
        while (ilen > 0 && (input[ilen-1] == '\n' || input[ilen-1] == '\r'))
            input[--ilen] = '\0';

        if (strncmp(input, "uci", 3) == 0) {
            printEngineInfo();
        } else if (strncmp(input, "isready", 7) == 0) {
            printf("readyok\n");
        } else if (strncmp(input, "ucinewgame", 10) == 0) {
            setFen(&board, START_HEX_FEN);
        } else if (strncmp(input, "position", 8) == 0) {
            parsePosition(input, &board);
        } else if (strncmp(input, "go", 2) == 0) {
            parseGo(input, board);
        } else if (strncmp(input, "display", 7) == 0) {
            renderBoard(board);
        } else if (strncmp(input, "quit", 4) == 0) {
            break;
        }
    }

    return 0;
}
