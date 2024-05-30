#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

int main(int argc, char** argv) {
    if (argc != 2) {
        printf("Usage: test.exe <text>");
        exit(-1);
    }
    char* text = argv[1];
    int textlen = strlen(text);
    printf("Text: %s\n", text);
    printf("Text length: %d\n", textlen);
    return 0;
}