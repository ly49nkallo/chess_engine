#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../src/chess_engine.h"
#include <inttypes.h>

int main(void) {
    ChessBoard board;
    chess_board_init(&board);
    generate_board_from_FEN(&board, CE_FEN_STARTING_POSITION);
    print_bitboard(board.bitboards[PAWN - 1]);
    return 0;
}
