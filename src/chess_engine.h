
#ifndef CHESS_ENGINE_H
#define CHESS_ENGINE_H

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "stdint.h"
#include "stdbool.h"
#include "inttypes.h"

// #include "raylib.h"

#define FEN_STARTING_POSITION "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

typedef struct ChessBoard {
    // implemented using bitboards b/c im like that fr
    uint64_t* bitmasks; // indexed by the rank_id enum
    uint16_t* piece_list; 
    uint16_t castling_avaliablility; // 4 BITS KQkq : (white king-side, white queen-side, black king-side, black queen-side)
    uint64_t black;
    uint64_t white;
    bool white_turn; // 1: White turn, 0: Black turn
    uint16_t move_counter; // how many total half moves have been played
    uint16_t fifty_move_counter; // how many half move have been played since last capture or pawn move
    //TODO En Passant

    // function pointers
    
} ChessBoard;
void chess_board_init(ChessBoard* board);

enum rank_id {EMPTY = 0, PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING}; // KING = 6

// Utilities //

void print_board_in_terminal(ChessBoard* board);
int get_id_from_rank_file(const int rank, const int file);
int create_piece_id(const char piece);
char piece_id_to_char(const int piece);
void generate_board_from_FEN(ChessBoard* board, const char* FEN_string);
void print_board_in_terminal_from_FEN(const char* FEN_string);

#endif