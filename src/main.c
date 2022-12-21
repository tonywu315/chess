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
    bool player_first = true;

    if (DEBUG_FLAG) {
        signal(SIGINT, handle_signal);
        signal(SIGQUIT, handle_signal);
        signal(SIGSEGV, handle_signal);
    }

    // Parse arguments
    if (argc >= 2) {
        seconds = atoi(argv[1]);
        if (argc >= 3) {
            if (!strcmp(argv[2], "white")) {
                player_first = true;
            } else if (!strcmp(argv[2], "black")) {
                player_first = false;
            } else if (!strcmp(argv[2], "replay")) {
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
            }
        }
    }

    init_attacks();
    init_evaluation();
    init_transposition(512);

    start_board(&board);
    start_game(&board, seconds, player_first);

    if (transposition) {
        free(transposition);
    }

    if (DEBUG_FLAG) {
        save_to_file();
    }

    return 0;
}

static void handle_signal() {
    printf("\nProgram stopped\n");
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

        replay.game_ply = game.ply;
        replay.is_replay = false;

        fwrite(&replay, sizeof(Replay), 1, file);
        fclose(file);
    }
}
