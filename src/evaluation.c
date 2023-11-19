#include "evaluation.h"
#include "board.h"

// https://www.chessprogramming.org/Piece-Square_Tables

int PIECE_HEAD_VALUES[] = {
    100,
    320,
    330,
    500,
    900,
    2000,
    -100,
    -320,
    -330,
    -500,
    -900,
    -2000};

int PAWN_W_POS[] = {
    0, 0, 0, 0, 0, 0, 0, 0,
    5, 10, 10, -20, -20, 10, 10, 5,
    5, -5, -10, 0, 0, -10, -5, 5,
    0, 0, 0, 20, 20, 0, 0, 0,
    5, 5, 10, 25, 25, 10, 5, 5,
    10, 10, 20, 30, 30, 20, 10, 10,
    50, 50, 50, 50, 50, 50, 50, 50,
    0, 0, 0, 0, 0, 0, 0, 0};
int KNIGHT_W_POS[] = {
    -50, -40, -30, -30, -30, -30, -40, -50,
    -40, -20, 0, 5, 5, 0, -20, -40,
    -30, 5, 10, 15, 15, 10, 5, -30,
    -30, 0, 15, 20, 20, 15, 0, -30,
    -30, 5, 15, 20, 20, 15, 5, -30,
    -30, 0, 10, 15, 15, 10, 0, -30,
    -40, -20, 0, 0, 0, 0, -20, -40,
    -50, -40, -30, -30, -30, -30, -40, -50};
int BISHOP_W_POS[] = {

    -20, -10, -10, -10, -10, -10, -10, -20,
    -10, 5, 0, 0, 0, 0, 5, -10,
    -10, 10, 10, 10, 10, 10, 10, -10,
    -10, 0, 10, 10, 10, 10, 0, -10,
    -10, 5, 5, 10, 10, 5, 5, -10,
    -10, 0, 5, 10, 10, 5, 0, -10,
    -10, 0, 0, 0, 0, 0, 0, -10,
    -20, -10, -10, -10, -10, -10, -10, -20};
int ROOK_W_POS[] = {
    0,
    0,
    5,
    10,
    10,
    5,
    0,
    0,
    -5,
    0,
    0,
    0,
    0,
    0,
    0,
    -5,
    -5,
    0,
    0,
    0,
    0,
    0,
    0,
    -5,
    -5,
    0,
    0,
    0,
    0,
    0,
    0,
    -5,
    -5,
    0,
    0,
    0,
    0,
    0,
    0,
    -5,
    -5,
    0,
    0,
    0,
    0,
    0,
    0,
    -5,
    5,
    10,
    10,
    10,
    10,
    10,
    10,
    5,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
};
int QUEEN_W_POS[] = {
    -20,
    -10,
    -10,
    -5,
    -5,
    -10,
    -10,
    -20 - 10,
    0,
    5,
    0,
    0,
    0,
    0,
    -10,
    -10,
    5,
    5,
    5,
    5,
    5,
    0,
    -10,
    0,
    0,
    5,
    5,
    5,
    5,
    0,
    -5,
    -5,
    0,
    5,
    5,
    5,
    5,
    0,
    -5,
    -10,
    0,
    5,
    5,
    5,
    5,
    0,
    -10,
    -10,
    0,
    0,
    0,
    0,
    0,
    0,
    -10,
    -20,
    -10,
    -10,
    -5,
    -5,
    -10,
    -10,
    -20,
};
int KING_W_POS[] = {
    20,
    30,
    10,
    0,
    0,
    10,
    30,
    20,
    20,
    20,
    0,
    0,
    0,
    0,
    20,
    20,
    -10,
    -20,
    -20,
    -20,
    -20,
    -20,
    -20,
    -10,
    -20,
    -30,
    -30,
    -40,
    -40,
    -30,
    -30,
    -20,
    -30,
    -40,
    -40,
    -50,
    -50,
    -40,
    -40,
    -30,
    -30,
    -40,
    -40,
    -50,
    -50,
    -40,
    -40,
    -30,
    -30,
    -40,
    -40,
    -50,
    -50,
    -40,
    -40,
    -30,
    -30,
    -40,
    -40,
    -50,
    -50,
    -40,
    -40,
    -30,
};

int KNIGHT_B_POS[64];
int PAWN_B_POS[64];
int BISHOP_B_POS[64];
int ROOK_B_POS[64];
int QUEEN_B_POS[64];
int KING_B_POS[64];

int MIN_EVAL = -1000000;
int MAX_EVAL = 1000000;

void initEvaluation(void)
{
    for (int i = 0; i < 64; i++)
    { // Black evaluation is reverse of white
        PAWN_B_POS[i] = PAWN_W_POS[63 - i];
        KNIGHT_B_POS[i] = KNIGHT_W_POS[63 - i];
        BISHOP_B_POS[i] = BISHOP_W_POS[63 - i];
        ROOK_B_POS[i] = ROOK_W_POS[63 - i];
        QUEEN_B_POS[i] = QUEEN_W_POS[63 - i];
        KING_B_POS[i] = KING_W_POS[63 - i];
    }
}

int evaluate(Board board, int result)
{
    if (result == DRAW)
        return 0;
    else if (result == WHITE_WIN)
        return MAX_EVAL;
    else if (result == BLACK_WIN)
        return MIN_EVAL;

    int eval = 0;

    while (board.pawn_w)
    {
        int sq = __builtin_ctzll(board.pawn_w);
        eval += PAWN_W_POS[sq];
        eval += PIECE_HEAD_VALUES[PAWN_W];
        board.pawn_w &= board.pawn_w - 1;
    }
    while (board.knight_w)
    {
        int sq = __builtin_ctzll(board.knight_w);
        eval += KNIGHT_W_POS[sq];
        eval += PIECE_HEAD_VALUES[KNIGHT_W];
        board.knight_w &= board.knight_w - 1;
    }
    while (board.bishop_w)
    {
        int sq = __builtin_ctzll(board.bishop_w);
        eval += BISHOP_W_POS[sq];
        eval += PIECE_HEAD_VALUES[BISHOP_W];
        board.bishop_w &= board.bishop_w - 1;
    }
    while (board.rook_w)
    {
        int sq = __builtin_ctzll(board.rook_w);
        eval += ROOK_W_POS[sq];
        eval += PIECE_HEAD_VALUES[ROOK_W];
        board.rook_w &= board.rook_w - 1;
    }
    while (board.queen_w)
    {
        int sq = __builtin_ctzll(board.queen_w);
        eval += QUEEN_W_POS[sq];
        eval += PIECE_HEAD_VALUES[QUEEN_W];
        board.queen_w &= board.queen_w - 1;
    }
    while (board.pawn_b)
    {
        int sq = __builtin_ctzll(board.pawn_b);
        eval -= PAWN_B_POS[sq];
        eval += PIECE_HEAD_VALUES[PAWN_B];
        board.pawn_b &= board.pawn_b - 1;
    }
    while (board.knight_b)
    {
        int sq = __builtin_ctzll(board.knight_b);
        eval -= KNIGHT_B_POS[sq];
        eval += PIECE_HEAD_VALUES[KNIGHT_B];
        board.knight_b &= board.knight_b - 1;
    }
    while (board.bishop_b)
    {
        int sq = __builtin_ctzll(board.bishop_b);
        eval -= BISHOP_B_POS[sq];
        eval += PIECE_HEAD_VALUES[BISHOP_B];
        board.bishop_b &= board.bishop_b - 1;
    }
    while (board.rook_b)
    {
        int sq = __builtin_ctzll(board.rook_b);
        eval -= ROOK_B_POS[sq];
        eval += PIECE_HEAD_VALUES[ROOK_B];
        board.rook_b &= board.rook_b - 1;
    }
    while (board.queen_b)
    {
        int sq = __builtin_ctzll(board.queen_b);
        eval -= QUEEN_B_POS[sq];
        eval += PIECE_HEAD_VALUES[QUEEN_B];
        board.queen_b &= board.queen_b - 1;
    }

    // King square bonuses
    eval += KING_W_POS[board.whiteKingSq];
    eval -= KING_B_POS[board.blackKingSq];

    return eval;
}