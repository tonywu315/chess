#include "attacks.h"
#include "benchmark.h"
#include "board.h"

#define TEST_POSITIONS 6
#define TEST_DEPTH 5

static const char *positions[TEST_POSITIONS] = {
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
    "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 15",
    "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 1 60",
    "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 2 30",
    "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 0 10",
    "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 4 30"};

static const U64 perft_results[TEST_POSITIONS][TEST_DEPTH] = {
    {20, 400, 8902, 197281, 4865609},     {48, 2039, 97862, 4085603, 193690690},
    {14, 191, 2812, 43238, 674624},       {6, 264, 9467, 422333, 15833292},
    {44, 1486, 62379, 2103487, 89941194}, {46, 2079, 89890, 3894594, 164075551},
};

int main(int argc, char **argv) {
    Board board;
    U64 nodes;
    int position;

    if (argc == 2) {
        position = atoi(argv[1]) - 1;
    } else {
        return 1;
    }

    init_attacks();

    load_fen(&board, positions[position]);
    for (int depth = 1; depth <= TEST_DEPTH; depth++) {
        nodes = 0;
        perft(&board, depth, &nodes);
        if (nodes != perft_results[position][depth - 1]) {
            printf("perft %d: %lld\n", depth, nodes);
            return 1;
        }
    }

    return 0;
}
