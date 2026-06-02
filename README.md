# Recommended Read
[Technical documentation on how this hexagonal chess engine works](https://www.felipelecot.com/docs/hexagonal-chess-engine)

# Chess Engine

## Overview
Chess engine written in C with UCI protocol support. Features bitboard-based move generation, alpha-beta search with transposition tables, and piece-square table evaluation. Can interface with any UCI-compatible chess GUI.

## Core Features
- **Move Generation**: Bitboard-based with magic numbers for sliding pieces (bishops and rooks)
- **Position Evaluation**: Piece-square table evaluation with material counting
- **Search Algorithm**: Iterative deepening negamax search with alpha-beta pruning and move ordering
- **Game State Handling**: Full support for castling, en passant, checkmate, stalemate, and insufficient material
- **No Opening Book**: Pure tactical play from position evaluation

## Technical Implementation
- **Bitboard Representation**: 64-bit integers represent board state for efficient operations
- **Magic Bitboards**: Pre-computed magic numbers enable fast sliding piece attack generation
- **Transposition Table**: 100,000-entry hash table keyed by Zobrist hash; each entry stores evaluation, depth, node type (EXACT / LOWER / UPPER bound), and best move. Cache hits at sufficient depth skip subtree expansion entirely.
- **Iterative Deepening**: Searches depth 1, 2, … N in sequence. Each iteration warms the TT so the best move is tried first at the next depth, dramatically improving alpha-beta cutoffs at ~20% extra total work.
- **Move Ordering**: PV move from the TT is tried first; captures are scored by MVV-LVA (Most Valuable Victim − Least Valuable Attacker). Together these produce the cutoffs that make iterative deepening effective.
- **UCI Protocol**: Standard chess interface for GUI compatibility

## Attributions
Heavily based on the following documentation and code:
- https://www.chessprogramming.org/
- https://github.com/bartekspitza/sophia
- https://github.com/Sopel97/FatTitz
- https://github.com/abhigyan7/chess
- https://github.com/kz04px/Baislicka
