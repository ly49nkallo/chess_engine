
#ifndef CHESS_ENGINE_H
#define CHESS_ENGINE_H

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "stdint.h"
#include "stdbool.h"
#include "inttypes.h"

#define CE_FEN_STARTING_POSITION "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
#define ID_FROM_RANK_FILE(r, f) (r * 8 + f)

/// @brief The structure containing all information about a chess board position
typedef struct ChessBoard {
    // implemented using bitboards b/c im like that fr
    uint64_t* bitboards; // indexed by the rank_id enum
    uint8_t* piece_list; 
    uint16_t castling_avaliablility; // 4 BITS KQkq : (white king-side, white queen-side, black king-side, black queen-side)
    uint64_t black;
    uint64_t white;
    bool white_turn; // 1: White turn, 0: Black turn
    uint16_t move_counter; // how many total half moves have been played
    uint16_t fifty_move_counter; // how many half move have been played since last capture or pawn move
    uint16_t en_passant; // 0-63 tile id, >=64 no move allowed

    // function pointers
    
} ChessBoard;
void chess_board_init(ChessBoard* board);
void chess_board_destroy(ChessBoard* board);

enum rank_id {
    EMPTY = 0, 
    PAWN, 
    KNIGHT, 
    BISHOP, 
    ROOK, 
    QUEEN, 
    KING
};

enum color_id {
    TILE_WHITE = 0b10000, 
    TILE_BLACK = 0b01000
};

typedef struct move_tag {
    uint8_t from_rank;
    uint8_t from_file;
    uint8_t to_rank;
    uint8_t to_file;
    uint8_t castling;
} Move;
/* Utilities */

void print_board_in_terminal(ChessBoard* board);
int char_to_piece_id(const char piece);
char piece_id_to_char(const int piece);
void generate_board_from_FEN(ChessBoard* board, const char* FEN_string);
void print_board_in_terminal_from_FEN(const char* FEN_string);
int chess_board_add_piece(ChessBoard* board, const int tile, const int piece_id);
int chess_board_remove_piece(ChessBoard *board, const int tile, const int piece_id);
uint64_t chess_board_pseudo_legal_get_moves_BB(ChessBoard *board, int tile);
int *chess_board_get_pseudo_legal_moves_arr(ChessBoard *board, int tile);

#endif // CHESS_ENGINE_H
