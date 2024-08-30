#include <inttypes.h>
#include <stdio.h>
typedef uint64_t U64;

int main(void) {
    U64 rank_1 = 0x00000000000000ff;
    int i;
    // ranks
    for (i = 0; i < 8; i++) {
        printf("U64 rank_%d = ", i);
        U64 b = rank_1;
        b = (b << (8 * i));
        printf("0x%llx;\n", b);
    }
    // files
    U64 file_A = 0x0101010101010101;
    for (i = 0; i < 8; i++) {
        printf("U64 file_%d = ", i);
        U64 b = file_A;
        b = (b << i);
        printf("0x%llx;\n", b);
    }
    return 0;
}