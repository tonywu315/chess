#define SUCCESS 0
#define FAILURE 1
#define MAXIMUM_MOVES 256

typedef struct {
    char start;
    char end;
} Move;

enum directions {
    UPRIGHT = 17,
    UP = 16,
    UPLEFT = 15,
    RIGHT = 1,
    LEFT = -1,
    DOWNRIGHT = -15,
    DOWN = -16,
    DOWNLEFT = -17,
};
