#include "move_generation.h"
#include "bitboard.h"

typedef struct magics {
    Bitboard *attacks;
    Bitboard mask;
    Bitboard magic;
    int shift;
} Magic;

static const Bitboard MASK_FILE_A = UINT64_C(0xFEFEFEFEFEFEFEFE);
static const Bitboard MASK_FILE_H = UINT64_C(0x7F7F7F7F7F7F7F7F);
static const Bitboard MASK_FILE_AB = UINT64_C(0xFCFCFCFCFCFCFCFC);
static const Bitboard MASK_FILE_HG = UINT64_C(0x3F3F3F3F3F3F3F3F);
static const Bitboard EMPTY = UINT64_C(0);

/* Lookup tables and Fancy Magic Bitboards */
Bitboard pawn_attacks[2][64];
Bitboard knight_attacks[64];
Bitboard king_attacks[64];
Magic rook_magics[64];
Magic bishop_magics[64];
Bitboard rook_attacks[102400];
Bitboard bishop_attacks[5248];

static Bitboard init_pawn_attacks(int square, int player);
static Bitboard init_knight_attacks(int square);
static Bitboard init_king_attacks(int square);

static void init_magics(int piece);
static Bitboard get_rook_mask(int square);
static Bitboard get_bishop_mask(int square);
static Bitboard get_slider_attack(int square, Bitboard occupancy, int piece);
static inline Bitboard get_rook_attacks(int square, Bitboard occupancy);
static inline Bitboard get_bishop_attacks(int square, Bitboard occupancy);
static Bitboard sparse_random();

void init_attacks() {
    for (int square = 0; square < 64; square++) {
        pawn_attacks[WHITE][square] = init_pawn_attacks(square, WHITE);
        pawn_attacks[BLACK][square] = init_pawn_attacks(square, BLACK);
        knight_attacks[square] = init_knight_attacks(square);
        king_attacks[square] = init_king_attacks(square);
    }

    init_magics(ROOK);
    init_magics(BISHOP);
}

int is_attacked(Board board, int square, int player) {
    int shift = player == WHITE ? 0 : 6;
    return (pawn_attacks[2 - player][square] & board.pieces[PAWN + shift]) ||
           (knight_attacks[square] & board.pieces[KNIGHT + shift]) ||
           (king_attacks[square] & board.pieces[KING + shift]) ||
           (get_rook_attacks(square, board.occupancies[2]) &
            (board.pieces[ROOK + shift] | board.pieces[QUEEN + shift])) ||
           (get_bishop_attacks(square, board.occupancies[2]) &
            (board.pieces[BISHOP + shift] | board.pieces[QUEEN + shift]));
}

static Bitboard init_pawn_attacks(int square, int player) {
    Bitboard pawn = create_bit(square);

    if (player == WHITE) {
        return ((pawn << 9) & MASK_FILE_A) | ((pawn << 7) & MASK_FILE_H);
    } else {
        return ((pawn >> 7) & MASK_FILE_A) | ((pawn >> 9) & MASK_FILE_H);
    }
}

static Bitboard init_knight_attacks(int square) {
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

static Bitboard init_king_attacks(int square) {
    Bitboard king = create_bit(square);
    Bitboard attacks =
        ((king << 1) & MASK_FILE_A) | ((king >> 1) & MASK_FILE_H);

    king |= attacks;
    attacks |= (king << 8) | (king >> 8);

    return attacks;
}

static void init_magics(int piece) {
    Bitboard occupancy[4096], attacks[4096], bitboard = EMPTY;
    int repeated[4096] = {0}, attacks_index = 0, found = 0;

    for (int square = A1; square <= H8; square++) {
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
            magic.magic = sparse_random();
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

static Bitboard get_rook_mask(int square) {
    const Bitboard files = UINT64_C(0x0001010101010100);
    const Bitboard ranks = UINT64_C(0x7E);

    Bitboard attacks = (files << (square % 8)) | (ranks << (8 * (square / 8)));
    clear_bit(&attacks, square);

    return attacks;
}

static Bitboard get_bishop_mask(int square) {
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

static Bitboard get_slider_attack(int square, Bitboard occupancy, int piece) {
    Bitboard attacks = EMPTY;
    int rook_direction[4] = {UP, DOWN, LEFT, RIGHT};
    int bishop_direction[4] = {UPRIGHT, UPLEFT, DOWNRIGHT, DOWNLEFT};

    for (int i = 0; i < 4; i++) {
        int direction = piece == ROOK ? rook_direction[i] : bishop_direction[i];
        int current = square;

        while (!get_bit(occupancy, current) && in_bounds(current, direction)) {
            set_bit(&attacks, current + direction);
            current += direction;
        }
    }

    return attacks;
}

static inline Bitboard get_rook_attacks(int square, Bitboard occupancy) {
    Magic m = rook_magics[square];
    return m.attacks[((occupancy & m.mask) * m.magic) >> m.shift];
}

static inline Bitboard get_bishop_attacks(int square, Bitboard occupancy) {
    Magic m = bishop_magics[square];
    return m.attacks[((occupancy & m.mask) * m.magic) >> m.shift];
}

static inline Bitboard sparse_random() {
    /* Fastest seed in first 100,000 seeds */
    static Bitboard seed = UINT64_C(0xAE793F42471A8799);
    Bitboard rand = ~EMPTY;

    for (int i = 0; i < 3; i++) {
        seed ^= seed >> 12;
        seed ^= seed << 25;
        seed ^= seed >> 27;
        rand &= seed * UINT64_C(0x2545F4914F6CDD1D);
    }

    return rand;
}
