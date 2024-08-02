typedef unsigned long long U64;

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

/* Sliding bitboard shifts */
/* will keep applying shifts and leave line of bits in cardinal directions */
U64 s_slide (U64 b)
{
    U64 cur = b;
    
}
U64 n_slide (U64 b);
U64 e_slide (U64 b);
U64 w_slide (U64 b);

U64 sw_slide (U64 b);
U64 ne_slide (U64 b);
U64 se_slide (U64 b);
U64 nw_slide (U64 b);