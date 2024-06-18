#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../src/chess_engine.h"

int main(int argc, char** argv) {
    ChessBoard b;
    chess_board_init(&b);
    print_board_in_terminal(&b);
    print_board_in_terminal_from_FEN(CE_FEN_STARTING_POSITION);
    chess_board_destroy(&b);
    printf("%d\n", 1 == 1);
}
