#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "board.h"
#include "fen.h"
#include "search.h"
#include "movegen.h"
#include "evaluation.h"
#include "zobrist.h"
#include "san.h"
#include "bitboards.h"

void parsePosition(char *command, Board *board);
void getBestMove(Board board);
void printEngineInfo(void);

char *AUTHOR = "Felipe Lecot";
char *ENGINE_NAME = "Basic Chess Engine";

int main(void) {
    // Init engine
    initBitboards();
    initZobrist();
    initMoveGeneration();
    initEvaluation();
    // Init engine

    setbuf(stdin, NULL);
    setbuf(stdout, NULL);

    int bufferSize = 2000;
    char input[bufferSize];

    Board board = {};
    while (1)
    {
        memset(input, 0, sizeof(input));
        fflush(stdout);

        if (!fgets(input, bufferSize, stdin))
            continue;

        if (input[0] == '\n')
            continue;

        if (strncmp(input, "isready", 7) == 0) {
            printf("readyok\n");
            continue;
        }

        else if (strncmp(input, "position", 8) == 0)
            parsePosition(input, &board);

        else if (strncmp(input, "ucinewgame", 10) == 0)
            parsePosition("position startpos", &board);

        else if (strncmp(input, "go", 2) == 0)
            getBestMove(board);

        else if (strncmp(input, "quit", 4) == 0)
            break;

        else if (strncmp(input, "uci", 3) == 0)
            printEngineInfo();
    }

    return 0;
}

void parsePosition(char *command, Board *board) {
    command += 9;
    char *current_char = command;

    setFen(board, START_FEN);

    current_char = strstr(command, "fen");
    if (current_char != NULL)
    {
        current_char += 4;
        setFen(board, current_char);
    }

    char *san = strstr(command, "moves");

    if (san != NULL)
    {
        san += 6;

        while (*san)
        {
            pushSan(board, san);
            while (*san && *san != ' ')
                san++;
            san++;
        }
    }
}

void getBestMove(Board board) {
    int depth = 7; // Change for custom depth
    int nodesSearched = 0;

    clock_t start = clock();
    int eval = search(board, depth);
    clock_t end = clock();
    double time_spent = (double)(end - start) * 1000 / CLOCKS_PER_SEC;

    printf("Info:\n Depth: %d\n Time %.0f\n Nodes %d\n Score %d\n", depth, time_spent, SEARCH_NODES_SEARCHED, eval);

    char san[6];
    memset(san, 0, sizeof(san));
    moveToSan(SEARCH_BEST_MOVE, san);
    printf("bestmove %s\n", san);
}