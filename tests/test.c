#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../src/chess_engine.h"

int main(int argc, char** argv) {
    ChessBoard* board = malloc(sizeof(ChessBoard));
    chess_board_init(board);
    print_board_in_terminal(board);
    generate_board_from_FEN(board, CE_FEN_STARTING_POSITION);
    print_board_in_terminal(board);
    chess_board_destroy(board);
    return 0;
}
