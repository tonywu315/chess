#ifndef BOARD_H
#define BOARD_H

#include "constants.h"

int invalid_square(U8 square);
int get_rank(U8 square);
int get_file(U8 square);
void init_board();
void start_board();
void print_board(int score);
Move san_to_move(const char *san, U8 player);
char *move_to_san(const Move *move, U8 player);
void load_fen(const char *fen);

#endif