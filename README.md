# Chess Engine

This is a UCI chess engine written in C. The engine searches the game tree using iterative deepening and [alpha-beta pruning](https://en.wikipedia.org/wiki/Alpha%E2%80%93beta_pruning) in order to find the most optimal move.

## Installation

1. Clone this repository: `git https://github.com/tonywu315/chess.git`
2. Enter repository and build executable.
    - On Unix: `make`
    - If [CMake](`https://cmake.org/`) is installed: `cmake -S . -B build && cmake --build build`

## UCI Protocol

The Universal Chess Interface (UCI) is a communication protocol that enables chess engines to communicate with user interfaces. This chess engine is compatible with all GUIs that support UCI. Here is a list of supported UCI options:

- **Hash**

    This is the size of the hash table in megabytes.
