#include <stdbool.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include "board.h"
#include "movegen.h"
#include "zobrist.h"
#include "utils.h"

#define TWO_RANKS 16

char SQUARES[64][3] = {
    "h1", "g1", "f1", "e1", "d1", "c1", "b1", "a1",
    "h2", "g2", "f2", "e2", "d2", "c2", "b2", "a2",
    "h3", "g3", "f3", "e3", "d3", "c3", "b3", "a3",
    "h4", "g4", "f4", "e4", "d4", "c4", "b4", "a4",
    "h5", "g5", "f5", "e5", "d5", "c5", "b5", "a5",
    "h6", "g6", "f6", "e6", "d6", "c6", "b6", "a6",
    "h7", "g7", "f7", "e7", "d7", "c7", "b7", "a7",
    "h8", "g8", "f8", "e8", "d8", "c8", "b8", "a8"
};

char CASTLING_RIGHTS[4][2] = { "K", "Q", "k", "q" };

int ALL_CASTLE_W = 0b0011;
int ALL_CASTLE_B = 0b1100;

bool isInsufficientMaterial(Board board) {
    if (board.pawn_w || board.pawn_b || board.rook_w || board.rook_b || board.queen_w || board.queen_b) {
        return false;
    }

    int knights = 0;
    int bishops = 0;
    Bitboard knightsBB = board.knight_w | board.knight_b;
    Bitboard bishopsBB = board.bishop_w | board.bishop_b;
    while (knightsBB) {
        knightsBB &= knightsBB - 1;
        knights++;
    }
    while (bishopsBB) {
        bishopsBB &= bishopsBB - 1;
        bishops++;
    }

    if (!bishops && knights) return true;       // KN or KNN vs K
    if (!knights && bishops == 1) return true;  // KB vs K

    return false;
}

/**
 * The game result
*/
int result(Board board, Move legal[], int length) {
    if (isInsufficientMaterial(board)) {
        return DRAW;
    }

    bool noLegalMoves = length == 0;

    if (noLegalMoves) {
        Bitboard kingInCheck = SQUARE_BITBOARDS[board.turn ? board.whiteKingSq : board.blackKingSq] && board.attacks;
        if (kingInCheck) {
            return board.turn ? BLACK_WIN : WHITE_WIN;
        }

        return DRAW;
    }

    return UN_DETERMINED;
}

void computeOccupancyMasks(Board* board) {
    board->occupancyWhite = board->pawn_w | board->knight_w | board->bishop_w | board->rook_w | board->queen_w | board->king_w;
    board->occupancyBlack = board->pawn_b | board->knight_b | board->bishop_b | board->rook_b | board->queen_b | board->king_b;
    board->occupancy = board->occupancyBlack | board->occupancyWhite;
}

void makeEnPassantMove(Board* board, Move move) {
    int capturedSquare = board->epSquare + (board->turn ? -8 : 8);

    board->hash ^= EN_PASSANT[board->epSquare];
    board->hash ^= PIECES[move.pieceType][move.fromSquare];
    board->hash ^= PIECES[move.pieceType][move.toSquare];
    board->hash ^= PIECES[board->turn ? PAWN_B : PAWN_W][capturedSquare];
    board->hash ^= WHITE_TO_MOVE;

    Bitboard* friendlyPawns = board->turn ? &(board->pawn_w) : &(board->pawn_b);
    Bitboard* opponentPawns = board->turn ? &(board->pawn_b) : &(board->pawn_w);

    // Capture the pawn
    *opponentPawns = toggleBit(*opponentPawns, capturedSquare);

    // Move the pawn that takes
    *friendlyPawns = toggleBit(*friendlyPawns, move.fromSquare);
    *friendlyPawns = setBit(*friendlyPawns, board->epSquare);

    board->epSquare = -1;
    board->turn = board->turn ? 0 : 1;
    computeOccupancyMasks(board);
}

void makeCastleMove(Board* board, Move move) {
    if (move.castle == K) {
        board->hash ^= PIECES[KING_W][E1];
        board->hash ^= PIECES[KING_W][G1];
        board->hash ^= PIECES[ROOK_W][H1];
        board->hash ^= PIECES[ROOK_W][F1];
        board->king_w = board->king_w >> 2;
        board->rook_w = toggleBit(board->rook_w, H1);
        board->rook_w = setBit(board->rook_w, F1);
        board->whiteKingSq = G1;
    } else if (move.castle == Q) {
        board->hash ^= PIECES[KING_W][E1];
        board->hash ^= PIECES[KING_W][C1];
        board->hash ^= PIECES[ROOK_W][A1];
        board->hash ^= PIECES[ROOK_W][D1];
        board->king_w = board->king_w << 2;
        board->rook_w = toggleBit(board->rook_w, A1);
        board->rook_w = setBit(board->rook_w, D1);
        board->whiteKingSq = C1;
    } else if (move.castle == k) {
        board->hash ^= PIECES[KING_B][E8];
        board->hash ^= PIECES[KING_B][G8];
        board->hash ^= PIECES[ROOK_B][H8];
        board->hash ^= PIECES[ROOK_B][F8];
        board->king_b = board->king_b >> 2;
        board->rook_b = toggleBit(board->rook_b, H8);
        board->rook_b = setBit(board->rook_b, F8);
        board->blackKingSq = G8;
    } else if (move.castle == q) {
        board->hash ^= PIECES[KING_B][E8];
        board->hash ^= PIECES[KING_B][C8];
        board->hash ^= PIECES[ROOK_B][A8];
        board->hash ^= PIECES[ROOK_B][D8];
        board->king_b = board->king_b << 2;
        board->rook_b = toggleBit(board->rook_b, A8);
        board->rook_b = setBit(board->rook_b, D8);
        board->blackKingSq = C8;
    }

    if (board->epSquare != -1) {
        board->hash ^= EN_PASSANT[board->epSquare];
    }

    board->hash ^= WHITE_TO_MOVE;
    board->hash ^= CASTLING[board->castling];

    // Update castling rights
    board->castling &= board->turn ? ALL_CASTLE_B : ALL_CASTLE_W;
    board->hash ^= CASTLING[board->castling];

    computeOccupancyMasks(board);
    board->epSquare = -1;
    board->turn = board->turn ? 0 : 1;
}

void pushMove(Board* board, Move move) {
    bool isEnPassantMove = move.toSquare==board->epSquare && (move.pieceType == PAWN_W || move.pieceType == PAWN_B);
    if (isEnPassantMove) {
        makeEnPassantMove(board, move);
        return;
    } 
    
    if (move.castle) {
        makeCastleMove(board, move); 
        return;
    }

    // Update hash
    board->hash ^= PIECES[move.pieceType][move.fromSquare];
    board->hash ^= WHITE_TO_MOVE;
    board->hash ^= CASTLING[board->castling]; // XOR out old castling rights
    if (board->epSquare != -1) {              // XOR out potential ep square
        board->hash ^= EN_PASSANT[board->epSquare];
    }

    // Set potential ep-square
    board->epSquare = -1;
    bool starterPawnMoved = (move.pieceType == PAWN_W && (move.fromSquare > A1 && move.fromSquare < H3)) ||
                            (move.pieceType == PAWN_B && (move.fromSquare > A6 && move.fromSquare < H8));
    if (starterPawnMoved) {
        int distanceCovered = abs(move.fromSquare - move.toSquare);
        if (distanceCovered == TWO_RANKS) {
            board->epSquare = board->turn ? move.fromSquare + 8 : move.fromSquare - 8;
        }
    }

    // Update hash with the new ep square
    if (board->epSquare != -1) {
        board->hash ^= EN_PASSANT[board->epSquare];
    }

    // Make move
    Bitboard friendlyRooks = board->turn ? board->rook_w : board->rook_b;
    Bitboard* pieceThatMoved = (&board->pawn_w + move.pieceType);

    // Update castling rights
    if (move.pieceType == KING_W || move.pieceType == KING_B) {
        board->castling &= board->turn ? ALL_CASTLE_B : ALL_CASTLE_W;
    } else if (*pieceThatMoved == friendlyRooks) {
        if (board->turn && move.fromSquare == A1) board->castling &= 0b1101;
        if (board->turn && move.fromSquare == H1) board->castling &= 0b1110;
        if (!board->turn && move.fromSquare == A8) board->castling &= 0b0111;
        if (!board->turn && move.fromSquare == H8) board->castling &= 0b1011;
    }

    // "Lift up the piece"
    *pieceThatMoved = toggleBit(*pieceThatMoved, move.fromSquare);

    // If not promotion set the piece square directly
    if (move.promotion <= 0) {
        *pieceThatMoved = setBit(*pieceThatMoved, move.toSquare);

        if (*pieceThatMoved == board->king_w) {
            board->whiteKingSq = move.toSquare;
        } else if (*pieceThatMoved == board->king_b) {
            board->blackKingSq = move.toSquare;
        }

        board->hash ^= PIECES[move.pieceType][move.toSquare];
    } else {
        Bitboard* targetMask = &(board->pawn_w) + move.promotion;
        *targetMask = setBit(*targetMask, move.toSquare);
        board->hash ^= PIECES[move.promotion][move.toSquare];
    }

    Bitboard* bb = board->turn ? &(board->pawn_b) : &(board->pawn_w);
    Bitboard opponentRooks = board->turn ? board->rook_b : board->rook_w;
    for (int i = 0; i < 5; i++) {

        if (getBit(*bb, move.toSquare)) {

            // Update hash
            board->hash ^= PIECES[(board->turn ? 6 : 0) + i][move.toSquare];

            // Update castling rights if rooks are captured
            if (*bb == opponentRooks) {
                if (board->turn && move.toSquare == H8) board->castling &= 0b1011;
                if (board->turn && move.toSquare == A8) board->castling &= 0b0111;
                if (!board->turn && move.toSquare == A1) board->castling &= 0b1101;
                if (!board->turn && move.toSquare == H1) board->castling &= 0b1110;
            }

            // Remove captured piece
            *bb = toggleBit(*bb, move.toSquare);
        }
        ++bb;
    }

    // XOR in new castling rights
    board->hash ^= CASTLING[board->castling];

    // Toggle turn
    board->turn = board->turn ? 0 : 1;
    computeOccupancyMasks(board);
}

void printBoard(Board board) {
    char pieceSymbols[] = "PNBRQKpnbrqk";
    printf("\n");
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {

            int loc = 63 - ((y*8) + x);
            bool printed = false;

            for (int i = 0; i < 12; i++) {
                Bitboard* bb = ((Bitboard*) &board) + i;                    

                if (getBit(*bb, loc)) {
                    printf("%c", pieceSymbols[i]);
                    printed = true;
                    break;
                }
            }

            if (! printed) putchar('.');
            printf(" ");
        }
        printf("\n");
    }

    printf("Turn: %s\n", board.turn ? "White" : "Black");
    printf("Castling: ");
    for (int i = 0; i < 4; i++) {
        if ((board.castling & (1 << i)) >> i) {
            putchar(CASTLING_RIGHTS[i][0]);
        }
    }
    printf("\nEp square: %s\n", board.epSquare == -1 ? "None" : &SQUARES[board.epSquare][0]);
    printf("\n");
}
