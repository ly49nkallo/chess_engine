#include <stdint.h>
typedef uint64_t U64;

const U64 notAFile = 0xfefefefefefefefe; // ~0x0101010101010101
const U64 notHFile = 0x7f7f7f7f7f7f7f7f; // ~0x8080808080808080

/* Single step bitboard shift */

U64 s_one (U64 b) {return  b >> 8;}
U64 n_one (U64 b) {return  b << 8;}
U64 e_one (U64 b) {return (b << 1) & notAFile;}
U64 w_one (U64 b) {return (b >> 1) & notHFile;}

U64 sw_one (U64 b) {return (b >> 9) & notHFile;}
U64 ne_one (U64 b) {return (b << 9) & notAFile;}
U64 se_one (U64 b) {return (b >> 7) & notAFile;}
U64 nw_one (U64 b) {return (b << 7) & notHFile;}

const U64 rank_0 = 0x00000000000000ff;
const U64 rank_1 = 0x000000000000ff00;
const U64 rank_2 = 0x0000000000ff0000;
const U64 rank_3 = 0x00000000ff000000;
const U64 rank_4 = 0x000000ff00000000;
const U64 rank_5 = 0x0000ff0000000000;
const U64 rank_6 = 0x00ff000000000000;
const U64 rank_7 = 0xff00000000000000;
const U64 file_0 = 0x0101010101010101;
const U64 file_1 = 0x0202020202020202;
const U64 file_2 = 0x0404040404040404;
const U64 file_3 = 0x0808080808080808;
const U64 file_4 = 0x1010101010101010;
const U64 file_5 = 0x2020202020202020;
const U64 file_6 = 0x4040404040404040;
const U64 file_7 = 0x8080808080808080;

/* Sliding bitboard shifts */
/* will keep applying shifts and leave line of bits in cardinal directions */
U64 s_slide (U64 b)
{
    U64 cursor = b;
}
U64 n_slide (U64 b);
U64 e_slide (U64 b);
U64 w_slide (U64 b);

U64 sw_slide (U64 b);
U64 ne_slide (U64 b);
U64 se_slide (U64 b);
U64 nw_slide (U64 b);

U64 rotate_left (U64 b, int s) {return _rotl64(b, s);}
U64 rotate_right (U64 b, int s) {return _rotr64(b, s);}
U64 generic_rotation(U64 b, int s) { // Custom
    return (s > 0) ? _rotl64(b, s) : _rotr64(b, s);
}

/* Alternatively */
// U64 rotateLeft (U64 x, int s) {return (x << s) | (x >> (64-s));}
// U64 rotateRight(U64 x, int s) {return (x >> s) | (x << (64-s));}

U64 gen_shift(U64 b, int s) {return (s > 0)? (b << s) : (b >> -s);}
/* Alternatively */
// U64 genShift(U64 x, int s) {
//    char left  =   (char) s;
//    char right = -((char)(s >> 8) & left);
//    return (x >> right) << (right + left);
// }

int shift[8] = {9, 1, -7, -8, -9, -1, 7, 8};
const U64 avoid_wrap[8] =
{
    0xfefefefefefefe00, // Dir 1: not A file and not rank 1
    0xfefefefefefefefe, // Dir 2: not A file
    0x00fefefefefefefe, // Dir 3: not A file and not rank 8
    0x00ffffffffffffff, // Dir 4: not rank 8
    0x007f7f7f7f7f7f7f, // Dir 5: not rank 8 and not G file
    0x7f7f7f7f7f7f7f7f, // Dir 6: not G file
    0x7f7f7f7f7f7f7f00, // Dir 7: not rank 1 and not G file
    0xffffffffffffff00, // Dir 8: not rank 1
};

U64 shift_one (U64 b, int dir8) {
   return rotate_left(b, shift[dir8]) & avoid_wrap[dir8];
}

U64 single_bit_set(int pos) {return 1ULL << (pos);}

