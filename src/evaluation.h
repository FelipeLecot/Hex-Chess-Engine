#ifndef EVALUATION_H
#define EVALUATION_H

#include "typedefs.h"

#define MAX_EVAL  1000000
#define MIN_EVAL (-1000000)

void initEvaluation(void);
int evaluate(Board board, int gameResult);

#endif
