#ifndef BOARD_H
#define BOARD_H

#include "constants.h"

int invalid_square(Fast square);
int get_rank(Fast square);
int get_file(Fast square);
void init_board();
void start_board();
void print_board();

#endif