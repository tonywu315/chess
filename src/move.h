#ifndef MOVE_H
#define MOVE_H

struct move;

void move_piece(struct move move);
void unmove_piece(struct move move);
int player_move_piece(struct move move);

#endif