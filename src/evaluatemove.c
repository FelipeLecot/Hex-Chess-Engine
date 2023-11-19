#include "evaluatemove.h"
#include "bitboards.h"

const int VICTIM_PIECES[] = {
    100 * 16,
    320 * 16,
    330 * 16,
    500 * 16,
    900 * 16,
};
const int ATTACKING_PIECES[] = {
    100,
    200,
    300,
    400,
    500,
    600,
    100, 200, 300, 400, 500, 600,
};

const int MAX_MOVE_SCORE = 1000;
const int PAWN_EXCHANGE_VALUE = (100 * 16) - 100; // Max value victim piece - min value attacking piece

bool both_moves_are_equal(Move a, Move b);

void score_moves(Board board, TTEntry entry, Move moves[], int cmoves) {
    Move pvMove;
    if (board.hash == entry.zobrist && entry.nodeType < UPPER) {
        pvMove = entry.move;
    }

    for (int i = 0; i < cmoves; i++) {
        Move* move = &moves[i];

        if (both_moves_are_equal(pvMove, *move)) {
            move->score = MAX_MOVE_SCORE;
        } else {
            Bitboard enemyOcc = board.turn ? board.occupancyBlack : board.occupancyWhite;
            bool isCapture = enemyOcc & SQUARE_BITBOARDS[move->toSquare];
            bool isEnPassant = board.epSquare == move->toSquare;

            if (isEnPassant) {
                moves[i].score = PAWN_EXCHANGE_VALUE;
            } else if (isCapture) {
                // Find type of captured piece
                int capturedPiece;
                Bitboard toSquare = SQUARE_BITBOARDS[moves[i].toSquare];
                Bitboard* bb = board.turn ? &board.pawn_b : &board.pawn_w;
                for (int i = 0; i < 5; i++) {
                    if (toSquare & *bb) {
                        capturedPiece = i;
                        break;
                    }
                    bb++;
                }

                move->score = VICTIM_PIECES[capturedPiece] - ATTACKING_PIECES[move->pieceType];
            }
        }
    }
}

int select_move(Move moves[], int cmoves) {
    int bestScore = 0;
    int indx = -1;

    for (int i = 0; i < cmoves; i++) {
        if (!moves[i].exhausted && moves[i].score >= bestScore) {
            bestScore = moves[i].score;
            indx = i;
        }
    }

    if (indx != -1) {
        moves[indx].exhausted = true;
    }

    return indx;
}

bool both_moves_are_equal(Move a, Move b) {
    return a.fromSquare == b.fromSquare && a.toSquare == b.toSquare && a.promotion == b.promotion;
}
