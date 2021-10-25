typedef struct board Board;

int invalid_square(int square);
int get_rank(int square);
int get_file(int square);
void init_board(struct board *board);
void start_board(struct board *board);
void print_board(struct board *board);