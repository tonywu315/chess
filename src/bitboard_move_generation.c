#include "bitboard_move_generation.h"
#include "bitboard.h"

typedef struct magics {
    Bitboard *attacks;
    Bitboard mask;
    Bitboard magic;
    U8 shift;
} Magic;

const Bitboard MASK_FILE_A = UINT64_C(0xFEFEFEFEFEFEFEFE);
const Bitboard MASK_FILE_H = UINT64_C(0x7F7F7F7F7F7F7F7F);
const Bitboard MASK_FILE_AB = UINT64_C(0xFCFCFCFCFCFCFCFC);
const Bitboard MASK_FILE_HG = UINT64_C(0x3F3F3F3F3F3F3F3F);

const Bitboard FILE_A = UINT64_C(0x0101010101010101);
const Bitboard FILE_H = UINT64_C(0x8080808080808080);
const Bitboard EDGES = UINT64_C(0x007E7E7E7E7E7E00);
const Bitboard EMPTY = UINT64_C(0);

Bitboard pawn_attacks[2][64];
Bitboard knight_attacks[64];
Bitboard king_attacks[64];

/* Fancy Magic Bitboards */
Magic rook_magics[64];
Magic bishop_magics[64];
Bitboard rook_attacks[102400];
Bitboard bishop_attacks[5248];

static Bitboard init_pawn_attacks(U8 square, U8 player);
static Bitboard init_knight_attacks(U8 square);
static Bitboard init_king_attacks(U8 square);

static void init_magics(U8 piece);
static Bitboard get_rook_mask(U8 square);
static Bitboard get_bishop_mask(U8 square);
static Bitboard get_slider_attack(U8 square, Bitboard occupancy, U8 piece);
static inline Bitboard get_rook_attacks(U8 square, Bitboard occupancy);
static inline Bitboard get_bishop_attacks(U8 square, Bitboard occupancy);
static Bitboard random();

void init_attacks() {
    for (U8 square = 0; square < 64; square++) {
        pawn_attacks[WHITE - 1][square] = init_pawn_attacks(square, WHITE);
        pawn_attacks[BLACK - 1][square] = init_pawn_attacks(square, BLACK);
        knight_attacks[square] = init_knight_attacks(square);
        king_attacks[square] = init_king_attacks(square);
    }

    init_magics(ROOK);
    init_magics(BISHOP);
}

int is_attacked(Board board, U8 square, U8 player) {
    int shift = player == WHITE ? -1 : 5;
    return (pawn_attacks[2 - player][square] & board.pieces[PAWN + shift]) ||
           (knight_attacks[square] & board.pieces[KNIGHT + shift]) ||
           (king_attacks[square] & board.pieces[KING + shift]) ||
           (get_rook_attacks(square, board.occupancies[2]) &
            (board.pieces[ROOK + shift] | board.pieces[QUEEN + shift])) ||
           (get_bishop_attacks(square, board.occupancies[2]) &
            (board.pieces[BISHOP + shift] | board.pieces[QUEEN + shift]));
}

static Bitboard init_pawn_attacks(U8 square, U8 player) {
    Bitboard pawn = create_bit(square);

    if (player == WHITE) {
        return ((pawn << 9) & MASK_FILE_A) | ((pawn << 7) & MASK_FILE_H);
    } else {
        return ((pawn >> 7) & MASK_FILE_A) | ((pawn >> 9) & MASK_FILE_H);
    }
}

static Bitboard init_knight_attacks(U8 square) {
    Bitboard knight = create_bit(square);

    set_bit(&knight, square);

    Bitboard left1 = (knight >> 1) & MASK_FILE_H;
    Bitboard left2 = (knight >> 2) & MASK_FILE_HG;
    Bitboard right1 = (knight << 1) & MASK_FILE_A;
    Bitboard right2 = (knight << 2) & MASK_FILE_AB;
    Bitboard height1 = left1 | right1;
    Bitboard height2 = left2 | right2;

    return (height1 << 16) | (height1 >> 16) | (height2 << 8) | (height2 >> 8);
}

static Bitboard init_king_attacks(U8 square) {
    Bitboard king = create_bit(square);
    Bitboard attacks =
        ((king << 1) & MASK_FILE_A) | ((king >> 1) & MASK_FILE_H);

    king |= attacks;
    attacks |= (king << 8) | (king >> 8);

    return attacks;
}

static void init_magics(U8 piece) {
    Bitboard occupancy[4096], attacks[4096], bitboard = EMPTY;
    int repeated[4096] = {0}, attacks_index = 0, found = 0;

    for (U8 square = A1; square <= H8; square++) {
        Magic magic;

        if (piece == ROOK) {
            magic.attacks = &rook_attacks[attacks_index];
            magic.mask = get_rook_mask(square);
        } else {
            magic.attacks = &bishop_attacks[attacks_index];
            magic.mask = get_bishop_mask(square);
        }

        magic.shift = 64 - get_population(magic.mask);

        int count = 0;
        do {
            occupancy[count] = bitboard;
            attacks[count++] = get_slider_attack(square, bitboard, piece);
            bitboard = (bitboard - magic.mask) & magic.mask;
        } while (bitboard);

        for (int i = 0; i < count;) {
            magic.magic = random();
            if (get_population((magic.magic * magic.mask) >> 56) < 6) {
                continue;
            }

            found += 1;
            for (i = 0; i < count; i++) {
                unsigned int index =
                    (int)(((occupancy[i] & magic.mask) * magic.magic) >>
                          magic.shift);

                if (repeated[index] < found) {
                    repeated[index] = found;
                    magic.attacks[index] = attacks[i];
                } else if (magic.attacks[index] != attacks[i]) {
                    break;
                }
            }
        }

        attacks_index += 1 << (64 - magic.shift);
        if (piece == ROOK) {
            rook_magics[square] = magic;
        } else {
            bishop_magics[square] = magic;
        }
    }
}

static Bitboard get_rook_mask(U8 square) {
    const Bitboard files = UINT64_C(0x0001010101010100);
    const Bitboard ranks = UINT64_C(0x7E);

    Bitboard attacks = (files << (square % 8)) | (ranks << (8 * (square / 8)));
    clear_bit(&attacks, square);

    return attacks;
}

static Bitboard get_bishop_mask(U8 square) {
    Bitboard attacks = EMPTY;
    int rank = square / 8, file = square % 8, r, f;

    for (r = rank + 1, f = file + 1; r <= 6 && f <= 6; r++, f++) {
        set_bit(&attacks, r * 8 + f);
    }
    for (r = rank + 1, f = file - 1; r <= 6 && f >= 1; r++, f--) {
        set_bit(&attacks, r * 8 + f);
    }
    for (r = rank - 1, f = file + 1; r >= 1 && f <= 6; r--, f++) {
        set_bit(&attacks, r * 8 + f);
    }
    for (r = rank - 1, f = file - 1; r >= 1 && f >= 1; r--, f--) {
        set_bit(&attacks, r * 8 + f);
    }

    return attacks;
}

static Bitboard get_slider_attack(U8 square, Bitboard occupancy, U8 piece) {
    Bitboard attacks = EMPTY;
    int rook_direction[4] = {UP, DOWN, LEFT, RIGHT};
    int bishop_direction[4] = {UPRIGHT, UPLEFT, DOWNRIGHT, DOWNLEFT};

    for (int i = 0; i < 4; i++) {
        int direction = piece == ROOK ? rook_direction[i] : bishop_direction[i];
        U8 current = square;

        while (!get_bit(occupancy, current) && in_bounds(current, direction)) {
            set_bit(&attacks, current + direction);
            current += direction;
        }
    }

    return attacks;
}

static inline Bitboard get_rook_attacks(U8 square, Bitboard occupancy) {
    Magic m = rook_magics[square];
    return m.attacks[((occupancy & m.mask) * m.magic) >> m.shift];
}

static inline Bitboard get_bishop_attacks(U8 square, Bitboard occupancy) {
    Magic m = bishop_magics[square];
    return m.attacks[((occupancy & m.mask) * m.magic) >> m.shift];
}

static Bitboard random() {
    /* Fastest seed in first 5000 seeds */
    static Bitboard seed = UINT64_C(4302);
    Bitboard rand = ~EMPTY;

    for (int i = 0; i < 3; i++) {
        seed ^= seed >> 12;
        seed ^= seed << 25;
        seed ^= seed >> 27;
        rand &= seed * UINT64_C(2685821657736338717);
    }

    return rand;
}

// U8 pop_lsb(Bitboard *bitboard) {
//     U8 lsb = get_lsb(*bitboard);
//     *bitboard &= *bitboard - 1;
//     return lsb;
// }

// U8 get_lsb(Bitboard bitboard) {
//     static const U8 lsb_table[64] = {
//         63, 30, 3,  32, 59, 14, 11, 33, 60, 24, 50, 9,  55, 19, 21, 34,
//         61, 29, 2,  53, 51, 23, 41, 18, 56, 28, 1,  43, 46, 27, 0,  35,
//         62, 31, 58, 4,  5,  49, 54, 6,  15, 52, 12, 40, 7,  42, 45, 16,
//         25, 57, 48, 13, 10, 39, 8,  44, 20, 47, 38, 22, 17, 37, 36, 26};

//     bitboard ^= bitboard - 1;
//     return lsb_table[((int)bitboard ^ (bitboard >> 32)) * 0x78291ACF >> 26];
// }
