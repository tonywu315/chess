#ifndef MOVE_GENERATION_H
#define MOVE_GENERATION_H

struct move;

struct move create_move(char start, char end, char flag);
int is_attacked(int square, char player);
int generate_moves(struct move *moves);

#endif