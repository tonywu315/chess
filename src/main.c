#include "attacks.h"
#include "benchmark.h"
#include "bitboard.h"
#include "evaluation.h"
#include "game.h"
#include "transposition.h"

static void handle_signal();
static void save_to_file();

int main(int argc, char **argv) {
    Board board;
    int seconds = 10;

    if (DEBUG_FLAG) {
        signal(SIGINT, handle_signal);
    }

    init_attacks();
    init_evaluation();
    init_transposition(512);

    start_board(&board);

    if (argc >= 2) {
        seconds = atoi(argv[1]);
        if (argc >= 3) {
            if (!strcmp(argv[2], "replay")) {
                FILE *file = fopen(REPLAY_FILE, "rb");
                if (!file) {
                    perror("error opening file");
                    exit(1);
                }
                if (!fread(&replay, sizeof(Replay), 1, file)) {
                    perror("error reading file");
                    exit(1);
                }
                replay.is_replay = true;
            } else if (!strcmp(argv[2], "benchmark")) {
                benchmark(&board, 6);
            }
        }
    }

    // load_fen(&board, "r3k2r/pb2qpbp/1pn1pnp1/2PpP3/2B2B2/2N2N2/PPPQ1PPP/R3K2R
    // w KQkq - 0 1"); load_fen(&board, "8/k7/3p4/p2P1p2/P2P1P2/8/8/K7 w - - ");
    // load_fen(&board, "8/2P5/1K1r4/8/8/8/8/k7 w - - 0 1");
    start_game(&board, true, seconds);

    if (transposition) {
        free(transposition);
    }

    if (DEBUG_FLAG) {
        save_to_file();
    }

    return 0;
}

static void handle_signal() {
    printf("\n");
    save_to_file();
    exit(0);
}

static void save_to_file() {
    if (!replay.is_replay) {
        FILE *file = fopen(REPLAY_FILE, "wb");
        if (!file) {
            perror("error opening file");
            exit(1);
        }

        replay.game_ply = game_ply;
        replay.is_replay = false;

        fwrite(&replay, sizeof(Replay), 1, file);
        fclose(file);
    }
}
