#include "board.h"
#include "game.h"
#include "benchmark.h"

Board board;
Move game_moves[MAX_GAME_LENTH];
int game_position;

/* Start game */
int main() {
    start_board();
    start_singleplayer(WHITE, 6);

    return SUCCESS;
}
