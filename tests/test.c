#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

int main(int argc, char** argv) {
    char* digits = "0123456789";

    for (int i = 0; i < 10; i++)
        printf("%c\n", digits[i]);

    for (int i = 0; i < 10; i++)
        printf("%c\n", i[digits]);
    return 0;
}
