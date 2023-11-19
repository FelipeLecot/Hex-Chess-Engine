#include <stdlib.h>
#include "fen.h"
#include "board.h"
#include "zobrist.h"
#include "utils.h"
#include "movegen.h"

// https://www.chessprogramming.org/Forsyth-Edwards_Notation

Bitboard* pieceBitboard(Board* board, int pieceType) {
    return (Bitboard*) board + pieceType;
}

void reset(Board* board) {
    Bitboard* bb = (Bitboard*) (board);
    for (int i = 0; i < 12; i++) {
        *bb &= 0;
        bb++;
    }
    board->turn = WHITE;
    board->castling = 0;
    board->epSquare = -1;
}

Bitboard computeAttacks(Board board) {
    Bitboard attackMask = 0;

    Bitboard king = board.turn ? board.king_b : board.king_w;
    Bitboard queen = board.turn ? board.queen_b : board.queen_w;
    Bitboard rook = board.turn ? board.rook_b : board.rook_w;
    Bitboard bishop = board.turn ? board.bishop_b : board.bishop_w;
    Bitboard knight = board.turn ? board.knight_b : board.knight_w;
    Bitboard pawn = board.turn ? board.pawn_b : board.pawn_w;

    while (queen) {
        int sq = __builtin_ctzl(queen);
        Bitboard attacks = getBishopAttacks(sq, board.occupancy);
        attacks |= getRookAttacks(sq, board.occupancy); // Queen attacks are bishop + rook attacks
        attackMask |= attacks;
        queen &= queen - 1;
    }
    while (bishop) {
        int sq = __builtin_ctzl(bishop);
        Bitboard attacks = getBishopAttacks(sq, board.occupancy);
        attackMask |= attacks;
        bishop &= bishop - 1;
    }
    while (rook) {
        int sq = __builtin_ctzl(rook);
        Bitboard attacks = getRookAttacks(sq, board.occupancy);
        attackMask |= attacks;
        rook &= rook - 1;
    }
    while (knight) {
        int sq = __builtin_ctzl(knight);
        attackMask |= KNIGHT_MOVEMENT[sq];
        knight &= knight - 1;
    }
    while (pawn) {
        int sq = __builtin_ctzl(pawn);

        if (board.turn) {
            attackMask |= PAWN_B_ATTACKS_EAST[sq];
            attackMask |= PAWN_B_ATTACKS_WEST[sq];
        } else {
            attackMask |= PAWN_W_ATTACKS_EAST[sq];
            attackMask |= PAWN_W_ATTACKS_WEST[sq];
        }
        pawn &= pawn - 1;
    }
    while (king) {
        int sq = __builtin_ctzll(king);
        attackMask |= KING_MOVEMENT[sq];
        king &= king - 1;
    }

    return attackMask;
}

void setFen(Board* board, char* fen) {
    reset(board);

    int piece = -1;
	int rank = 7;
    int file = 0;
    int spacesCounter = 0;
    int turn;
    char enpassantFile;
    char enpassantRank;
	while (*fen) {
        piece = -1;

		switch (*fen) {
            case 'b': 
                piece = BISHOP_B; 
                turn = BLACK;
                enpassantFile = *fen;
                break;
            case 'w':
                turn = WHITE;
                break;
            case 'p': piece = PAWN_B; break;
            case 'n': piece = KNIGHT_B; break;
            case 'r': piece = ROOK_B; break;
            case 'q': piece = QUEEN_B; break;
            case 'k': piece = KING_B; break;
            case 'P': piece = PAWN_W; break;
            case 'N': piece = KNIGHT_W; break;
            case 'B': piece = BISHOP_W; break;
            case 'R': piece = ROOK_W; break;
            case 'Q': piece = QUEEN_W; break;
            case 'K': piece = KING_W; break;

            case 'a':
            case 'c':
            case 'd':
            case 'e':
            case 'f':
            case 'g':
            case 'h': 
                enpassantFile = *fen; 
                break;

            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
                enpassantRank = *fen;
                file += atoi(fen);
                break;

            case '/':
                rank--;
                file = 0;
                fen++;
                continue;
            case ' ':
                spacesCounter++;
                enpassantFile = '0';
                enpassantRank = '0';
                fen++;
                continue;
            case '-':
                fen++;
                continue;
        }

        if (spacesCounter == 0) { // Piece postion
            if (piece != -1 && rank >= 0) {
                int square = ((rank+1) * 8) - file - 1;
                file++;

                Bitboard* bb = pieceBitboard(board, piece);
                *bb |= 1LL << square;
            }
        } else if (spacesCounter == 1) { // Who's turn is it?
            board->turn = turn;
        } else if (spacesCounter == 2) { // Castle rights
            if (piece == KING_W) board->castling |= 1;
            else if (piece == QUEEN_W) board->castling |= 1 << 1;
            else if (piece == KING_B) board->castling |= 1 << 2;
            else if (piece == QUEEN_B) board->castling |= 1 << 3;
        } else if (spacesCounter == 3) { // En passant attack
            if (enpassantRank != '0' && enpassantFile != '0') {
                int file = 'h' - enpassantFile;
                int rank = enpassantRank - '1';
                board->epSquare = 8 * rank + file;
            }
        }
		
		fen++;
	}

    // Initialize king squares
    for (int i = 0; i < 64; i++) {
        if (getBit(board->king_w, i)) {
            board->whiteKingSq = i;
        }
        if (getBit(board->king_b, i)) {
            board->blackKingSq = i;
        }
    }

    computeOccupancyMasks(board);
    board->hash = hash(*board);
    board->attacks = computeAttacks(*board);
}