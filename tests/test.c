#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../src/chess_engine.h"

int main(int argc, char** argv) {
    unsigned int i = 0;
    unsigned int r = 7;
    unsigned int j = r * 8; // 56
    for (i = 0; i < 64; i ++)
    {
        if (i % 8 == 0 && i != 0) {
            r--;
            j = r * 8;
            printf("\n");
        }
        else {
            j++;
        }
        printf("%d", j);
        printf(" ");
    }

    ChessBoard* board = malloc(sizeof(ChessBoard));
    chess_board_init(board);
    generate_board_from_FEN(board, FEN_STARTING_POSITION);
    print_board_in_terminal(board);
    return 0;
}
