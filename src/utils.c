#include <stdio.h>
#include "utils.h"
#include "san.h"

void printMoves(Move moves[], int length) {
    printf("Moves (%d):\n", length);
    char buf[8];
    for (int i = 0; i < length; i++) {
        moveToSan(moves[i], buf);
        printf("  %s\n", buf);
    }
}
