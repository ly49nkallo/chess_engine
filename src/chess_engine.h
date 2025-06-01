#ifndef CHESS_ENGINE_H
#define CHESS_ENGINE_H

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "stdint.h"
#include "stdbool.h"
#include "inttypes.h"
#include "utilities.h"

#define CE_FEN_STARTING_POSITION "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
#define ID_FROM_RANK_FILE(r, f) (r * 8 + f) // Zero-indexed rank and file

/// @brief The structure containing all information about a chess board position
typedef struct ChessBoard {
    // implemented using bitboards b/c im like that fr
    uint64_t bitboards[7]; // indexed by the rank_id enum
    uint8_t piece_list[64]; 
    uint16_t castling_avaliablility; // 4 BITS KQkq : (white king-side, white queen-side, black king-side, black queen-side)
    uint64_t black;
    uint64_t white;
    bool white_turn; // 1: White turn, 0: Black turn
    uint16_t move_counter; // how many total half moves have been played
    uint16_t fifty_move_counter; // how many half move have been played since last capture or pawn move
    uint16_t en_passant; // 0-63 tile id, >=64 no move allowed

    // function pointers
    
} ChessBoard;
void chess_board_init(ChessBoard *board);
void chess_board_destroy(ChessBoard *board);

typedef struct Move {
    uint8_t from;
    uint8_t to;
} Move;

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

typedef struct precomputed_bb {
    uint64_t knight[64];
    uint64_t bishop[64];
    uint64_t rook[64];
    uint64_t queen[64];
    uint64_t king[64];
    uint64_t pawn_white[64];
    uint64_t pawn_black[64];
    int computed;
} Precomputed_BB; // ~3592 bytes. Can be decreased by exploiting symmetry but not worth it for now.
void precomputed_bb_init(Precomputed_BB *pbb);
void precomputed_bb_free(Precomputed_BB *pbb);

/* Methods */

int chess_board_add_piece(ChessBoard *board, const int tile, const int piece_id);
int chess_board_remove_piece(ChessBoard *board, const int tile);
void chess_board_move(ChessBoard *board, int from, int to);
uint64_t chess_board_get_pseudo_legal_moves_BB(ChessBoard *board, const int tile);
int *chess_board_get_pseudo_legal_moves_arr(ChessBoard *board, int tile);

/* Utilities */

void print_board_in_terminal(ChessBoard* board);
int char_to_piece_id(const char piece);
char piece_id_to_char(const int piece);
void generate_board_from_FEN(ChessBoard* board, const char *FEN_string);
void print_board_in_terminal_from_FEN(const char *FEN_string);
void print_bitboard(uint64_t bb);

#endif // CHESS_ENGINE_H
