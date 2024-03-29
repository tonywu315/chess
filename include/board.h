#ifndef BOARD_H
#define BOARD_H

#include "types.h"

void init_board(Board *board);
void print_board(const Board *board, int score, bool game_over);
void print_bitboard(Bitboard bitboard);
bool load_fen(Board *board, const char *fen);
char *get_coordinates(int square);

#endif
