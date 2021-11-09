# Chess Engine

This is a simple chess engine written in C. The engine uses a [0x88](https://en.wikipedia.org/wiki/0x88) chess board representation and searches the game tree with [alpha-beta pruning](https://en.wikipedia.org/wiki/Alpha%E2%80%93beta_pruning). 

## Installation

1. Clone this repository: `git https://github.com/tonywu315/chess.git`

2. Enter repository and build executable: `cmake -S . -B build && cmake --build build`

## Usage

- Run the executable: `./chess.out`

- To enter moves, type the starting square followed by the ending square (e.g. `e2e4`)

- For promotions, append the piece to promote to (`q` = queen, `r` = rook, `b` = bishop, `n` = knight)

## TODO

- Tranposition table
- Iterative deepening
- Aspiration windows
- Quiescence Search
- Null move pruning
- Move ordering
- Opening book and endgame tablebase
- Improve evaluation
- Implement Universal Chess Interface
- Create a simple GUI
- Switch to bitboard representation
- Multithreading
- Create neural network for evaluation
