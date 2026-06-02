# Recommended Read
[Technical documentation on how this hexagonal chess engine works](https://www.felipelecot.com/docs/hexagonal-chess-engine)

# Hexagonal Chess Engine

## Overview
Gliński's hexagonal chess engine written in C with UCI protocol support. Plays on a 91-cell hexagonal board using cube coordinates, 128-bit bitboards, and per-ray attack tables. Can interface with any UCI-compatible chess GUI.

## Core Features
- **Move Generation**: 128-bit bitboard-based with per-ray precomputed attack tables for sliding pieces
- **Position Evaluation**: Piece-square tables built from cube-coordinate geometry + material counting
- **Search Algorithm**: Iterative deepening negamax search with alpha-beta pruning and move ordering
- **Game State Handling**: En passant, checkmate, stalemate, and insufficient material detection
- **No Castling**: Hex chess has no castling rule
- **No Opening Book**: Pure tactical play from position evaluation

## Technical Implementation
- **Bitboard Representation**: 128-bit struct (`uint64_t lo/hi`) representing the 91-cell board
- **Per-Ray Attack Tables**: Each (square, direction) pair has a 512-entry table indexed by ray occupancy. Avoids magic bitboards entirely — the hex board's long rays would require up to 2²⁴ entries per square with that approach. Total table size ≈ 9 MB, built at startup in < 1 ms.
- **Cube Coordinates**: Every cell has coordinates `(q, r, s)` with `q + r + s = 0`. Direction arithmetic is uniform across all files; `CUBE_TO_INDEX[q+5][r+5][s+5]` maps any cell to its linear index in O(1).
- **Transposition Table**: 100,000-entry hash table keyed by Zobrist hash; each entry stores evaluation, depth, node type (EXACT / LOWER / UPPER bound), and best move. Cache hits at sufficient depth skip subtree expansion entirely.
- **Iterative Deepening**: Searches depth 1, 2, … N in sequence. Each iteration warms the TT so the best move is tried first at the next depth, dramatically improving alpha-beta cutoffs at ~20% extra total work.
- **Move Ordering**: PV move from the TT is tried first; captures are scored by MVV-LVA (Most Valuable Victim − Least Valuable Attacker).
- **UCI Protocol**: Adapted for hex squares (`a1`–`l11`), file-major FEN, and no castling tokens

## Attributions
Introduction to hexagonal chess by CGP Grey:
- https://www.youtube.com/watch?v=bgR3yESAEVE

Rules and background:
- https://en.wikipedia.org/wiki/Hexagonal_chess

Hex board programming references:
- https://stackoverflow.com/questions/1838656/how-do-i-represent-a-hextile-hex-grid-in-memory
- http://www-cs-students.stanford.edu/~amitp/game-programming/grids/

Hex chess implementations referenced:
- https://github.com/theonlytechnohead/Hexchess
- https://github.com/aclap-dev/jocly

Chess engine techniques (adapted for hex):
- https://www.chessprogramming.org/
- https://github.com/bartekspitza/sophia
- https://github.com/Sopel97/FatTitz
- https://github.com/abhigyan7/chess
- https://github.com/kz04px/Baislicka
