#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../src/chess_engine.h"
#include <inttypes.h>

typedef unsigned long long U64;
U64 gen_shift_a(U64, int);
U64 gen_shift_b(U64, int);


int main(int argc, char** argv) {
    int i;
    U64 j;
    int err = 0;
    const int width = 10;
    for (j = -width; j < width; j++) {
        for (i = -width; i < width; i ++) {
            U64 a = gen_shift_a(j, i);
            U64 b = gen_shift_b(j, i);
            err ++;
            if (err > 2 * 4 * width * width) {
                printf("ERROR");
                exit(1);
            }
            printf("a: %llu, b: %llu, eq? %d", a, b, (int) (a == b));
        }
    }
}


U64 gen_shift_a(U64 x, int s) 
{
   char left  =   (char) s;
   char right = -((char)(s >> 8) & left);
   return (x >> right) << (right + left); 
}

U64 gen_shift_b(U64 x, int s) 
{
    return (x > 0) ? (x << s) : (x >> s);
}