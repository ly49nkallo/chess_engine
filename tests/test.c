#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../src/chess_engine.h"
#include <inttypes.h>
typedef uint64_t U64;
int main(void) {
    U64 avoid_wrap[8] =
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
    int i;
    for (i = 0; i < 8; ++i) {
        print_bitboard(avoid_wrap[i]);
        printf("\n");
    }
}
