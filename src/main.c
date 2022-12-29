#include "attacks.h"
#include "benchmark.h"
#include "board.h"
#include "evaluation.h"
#include "game.h"
#include "transposition.h"
#include "uci.h"

static void handle_signal();
static void save_to_file();

int main(void) {
    Board board;

    if (DEBUG_FLAG) {
        signal(SIGINT, handle_signal);
        signal(SIGQUIT, handle_signal);
        signal(SIGSEGV, handle_signal);
    }

    start_uci(&board);

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

        replay.game_ply = game.ply;
        replay.is_replay = false;

        fwrite(&replay, sizeof(Replay), 1, file);
        fclose(file);
    }
}
