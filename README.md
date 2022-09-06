# Chess Engine

This is a chess engine written in C. The engine searches the game tree using iterative deepening and [alpha-beta pruning](https://en.wikipedia.org/wiki/Alpha%E2%80%93beta_pruning) in order to find the most optimal move.

## Installation

1. Clone this repository: `git https://github.com/tonywu315/chess.git`
2. Enter repository and build executable.
    - On Unix: `make`
    - If [CMake](`https://cmake.org/`) is installed: `cmake -S . -B build && cmake --build build`

## Usage

- Run the executable: `./chess.out [TIME PER MOVE]`
- To enter moves, type the starting square followed by the ending square (e.g. `e2e4`)
- For promotions, append the piece to promote to (`q` = queen, `r` = rook, `b` = bishop, `n` = knight)
