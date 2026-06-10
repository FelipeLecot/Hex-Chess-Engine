# Recommended Read
[Technical documentation on how this hexagonal chess engine works](https://www.felipelecot.com/docs/hexagonal-chess-engine)

# Hexagonal Chess Engine

## Overview
Gliński's hexagonal chess engine written in C with UCI protocol support. Plays on a 91-cell hexagonal board using cube coordinates, 128-bit bitboards, and per-ray attack tables.


## How to run

Standard chess GUIs (Cute Chess, Arena, etc.) use rectangular board coordinates and will not work with this engine. Interact with it directly via the UCI protocol in the terminal.

**Build**
```bash
make
./hex_chess_engine
```

The engine reads UCI commands from stdin and writes responses to stdout. A typical session:

```
uci
# id name Hex Chess Engine
# id author Felipe Lecot
# uciok

isready
# readyok

ucinewgame
position startpos
display
# prints the hex board in ASCII

go depth 5
# info depth 1 time 0 nodes 42 score cp 0 pv <move>
# info depth 2 ...
# bestmove <move>

# Play your move, then ask the engine for its reply.
position startpos moves <your-move>
go depth 5
# bestmove <engine-reply>

position startpos moves <your-move> <engine-reply>
go depth 5
# bestmove <your-next-move>

# You can either grow the move list or use the fen command to get the current position and execute moves on that state.

position startpos moves f4f5
fen
# 6/P5p/RP4pr/N1P3p1n/Q2P2p2q/BBB1P1p1bbb/K2P2p2k/N1P3p1n/RP4pr/P5p/6 b - 0 1

position fen 6/P5p/RP4pr/N1P3p1n/Q2P2p2q/BBB1P1p1bbb/K2P2p2k/N1P3p1n/RP4pr/P5p/6 b - 0 1
go depth 5

quit
```

### Supported commands

| Command | Description | Type |
|---|---|---|
| `uci` | Identify the engine | UCI |
| `isready` | Confirm engine is ready | UCI |
| `ucinewgame` | Reset to the starting position | UCI |
| `position startpos [moves m1 m2 ...]` | Set position from start, with optional move list | UCI |
| `position fen <fen> [moves ...]` | Set position from a FEN string | UCI |
| `go [depth N]` | Search to depth N (default 5, max 64) | UCI |
| `fen` | Print the current position as a FEN string | Custom |
| `display` | Print the current board | Custom |
| `quit` | Exit | UCI |

Moves use hex square notation `a1`–`l11` (e.g. `f4f5` moves the piece on f4 to f5).

## Implementation
- **Bitboard Move Generation**: 128-bit struct (`uint64_t lo/hi`) for the 91-cell board. Per-ray precomputed attack tables for sliding pieces — each (square, direction) pair has a 512-entry table indexed by ray occupancy. Avoids magic bitboards entirely (the hex board's long rays would require up to 2²⁴ entries per square). Total table size ≈ 9 MB, built at startup in < 1 ms.
- **Cube Coordinates**: Every cell has coordinates `(q, r, s)` with `q + r + s = 0`. Direction arithmetic is uniform across all files; `CUBE_TO_INDEX[q+5][r+5][s+5]` maps any cell to its linear index in O(1).
- **Position Evaluation**: Piece-square tables built from cube-coordinate geometry, combined with material counting.
- **Search**: Iterative deepening negamax with alpha-beta pruning. Searches depth 1, 2, … N in sequence; each iteration warms the transposition table so the best move is tried first at the next depth, improving alpha-beta cutoffs at ~20% extra total work.
- **Transposition Table**: 100,000-entry hash table keyed by Zobrist hash; each entry stores evaluation, depth, node type (EXACT / LOWER / UPPER bound), and best move. Cache hits at sufficient depth skip subtree expansion entirely.
- **Move Ordering**: PV move from the TT is tried first; captures are scored by MVV-LVA (Most Valuable Victim − Least Valuable Attacker).
- **Game State**: En passant, checkmate, stalemate, and insufficient material detection. No castling (hex chess has no castling rule) and no opening book.
- **UCI Protocol**: Adapted for hex squares (`a1`–`l11`), file-major FEN, and no castling tokens.

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
