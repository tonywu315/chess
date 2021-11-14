#include "board.h"
#include "game.h"

Board board;
Move game_moves[MAX_GAME_LENTH];
int game_position;

/* Test file for code */
int main() {
    start_board();
    start_game(ONE_PLAYER, 6);

    return SUCCESS;
}
