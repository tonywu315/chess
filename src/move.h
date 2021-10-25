typedef struct board Board;
typedef struct move Move;

int generate_moves(Board *board, Move *moves);
void move_piece(Board *board, int start, int end);
int player_move_piece(Board *board, int start, int end);