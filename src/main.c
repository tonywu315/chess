#include "bitboard.h"
#include "bitboard_move_generation.h"
#include "game.h"

Board board;
Move game_moves[MAX_GAME_LENTH];
int game_position;

/* Start game */
int main(void) {    
    init_attacks();

    return SUCCESS;
}

