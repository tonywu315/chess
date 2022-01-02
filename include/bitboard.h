#ifndef BITBOARD_H
#define BITBOARD_H

#include "types.h"

void init_board(Board *board);
void start_board(Board *board);
void print_board(const Board *board, int score);
void print_bitboard(Bitboard bitboard);
void load_fen(Board *board, const char *fen);
char *get_coordinates(int square);

#endif
