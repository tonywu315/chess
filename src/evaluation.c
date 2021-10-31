#include "evaluation.h"

static const int values[7] = {0, 100, 300, 300, 500, 900, 9000};

/* Very simple evaluation function for pieces */
int eval() {
    int centipawn = 0;

    for (Fast i = A1; i <= H8; i++) {
        if (board.colors[i]) {
            centipawn += (3 - (2 * board.colors[i])) * values[board.pieces[i]];
        }
    }

    return centipawn;
}