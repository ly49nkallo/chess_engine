#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../src/chess_engine.h"
#include "../src/rmem.h"
#include <inttypes.h>
typedef uint64_t U64;


int main(void) {
    /* Test if legal moves BB lines up with legal moves piece list */
    char *FEN1 = CE_FEN_STARTING_POSITION;
    char *FEN2 = "rnbqkbnr/8/pppppppp/8/8/PPPPPPPP/8/RNBQKBNR w KQkq - 0 1";

    ChessBoard cb1;
    generate_board_from_FEN(&cb1, FEN1);
    ChessBoard cb2;
    generate_board_from_FEN(&cb2, FEN2);

    int idx;
    for (idx = 0; idx < 8 * 8; idx++) {
        if (cb1.piece_list[idx] != EMPTY) {
            U64 bb = chess_board_get_pseudo_legal_moves_BB(&cb1, idx);
            int *piece_list = chess_board_get_pseudo_legal_moves_arr(&cb1, idx);
            for (int j = 0; j < 64; j++) {
                
            }
        }
    }

}
