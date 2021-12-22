#include "bitboard.h"
#include "move.h"
#include "move_generation.h"

void print_bitboard(Bitboard bitboard) {
    printf("\nBitboard: %" PRIu64 "\n\n", bitboard);
    for (int rank = 7; rank >= 0; rank--) {
        printf("%d ", rank + 1);
        for (int file = 0; file < 8; file++) {
            printf(" %c", get_bit(bitboard, rank * 8 + file) ? '1' : '.');
        }

        printf("\n");
    }

    printf("\n   ");
    for (int i = 0; i < 8; i++) {
        printf("%c ", 'a' + i);
    }
    printf("\n\n");
}

