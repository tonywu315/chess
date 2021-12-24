#ifndef BITBOARD_H
#define BITBOARD_H

#include "constants.h"

void init_board(Board *board);
void start_board(Board *board);
void print_board(Board board, int score);
void print_bitboard(Bitboard bitboard);
void load_fen(Board *board, const char *fen);
char *get_coordinates(U8 square);

#endif