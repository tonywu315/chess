#include "benchmark.h"
#include "board.h"

Board board;
Move game_moves[MAX_GAME_LENTH];
int root_pos;
int search_pos;

int main() {
    U64 nodes;
    const U64 perft_results[5] = {20, 400, 8902, 197281, 4865609};

    start_board();

    for (int i = 1; i <= 5; i++) {
        nodes = 0;
        perft(i, &nodes);
        if (nodes != perft_results[i - 1]) {
            printf("perft %d: %lld\n", i, nodes);
            return FAILURE;
        }
    }

    return SUCCESS;
}
