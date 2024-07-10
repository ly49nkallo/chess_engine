#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../src/chess_engine.h"
#include <inttypes.h>

int main() {
    print_board_in_terminal_from_FEN(CE_FEN_STARTING_POSITION);
    WARNING("HI %d\n", 12);
}
