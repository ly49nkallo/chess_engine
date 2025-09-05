#ifndef CHESS_ENGINE_H
#define CHESS_ENGINE_H

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "stdint.h"
#include "stdbool.h"
#include "stdint.h"
#include "inttypes.h"
#include "utilities.h"

#define CE_FEN_STARTING_POSITION "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
#define ID_FROM_RANK_FILE(r, f) (r * 8 + f) // Zero-indexed rank and file
typedef uint64_t U64;

/// @brief The structure containing all information about a chess board position
typedef struct ChessBoard {
    // implemented using bitboards b/c im like that fr
    uint64_t bitboards[7]; // indexed by the rank_id enum
    uint8_t piece_list[64]; 
    uint16_t castling_avaliablility; // 4 BITS KQkq : (white king-side, white queen-side, black king-side, black queen-side)
    U64 black;
    U64 white;
    bool white_turn; // 1: White turn, 0: Black turn
    uint16_t move_counter; // how many total half moves have been played
    uint16_t fifty_move_counter; // how many half move have been played since last capture or pawn move
    uint16_t en_passant; // Last 4 bits: 0-63 tile id, 
    // First bit = White Castle left, Second bit = White Castle right
    // Third bit = Black Castle left, Fourth bit = Black Castle right
    // function pointers (submit to the dark side)
    
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
    U64 knight[64];
    U64 bishop[64];
    U64 rook[64];
    U64 queen[64];
    U64 king[64];
    U64 pawn_white[64];
    U64 pawn_black[64];
    int computed;
} Precomputed_BB; 
// 8 (bytes/U64) * 64 (unique tiles) * 7 (symmetric move types)
// ~= 3592 bytes. Can be decreased by exploiting symmetry but not worth it for now.
void precomputed_bb_init(Precomputed_BB *pbb);
void precomputed_bb_free(Precomputed_BB *pbb);

/* Methods */

int chess_board_add_piece(ChessBoard *board, const int tile, const int piece_id);
int chess_board_remove_piece(ChessBoard *board, const int tile);
void chess_board_move(ChessBoard *board, int from, int to);
U64 chess_board_pseudo_legal_moves_BB(ChessBoard *board, int tile);
int *chess_board_get_pseudo_legal_moves_arr(ChessBoard *board, int tile);

/* Utilities */

void print_board_in_terminal(ChessBoard* board);
int char_to_piece_id(const char piece);
char piece_id_to_char(const int piece);
void generate_board_from_FEN(ChessBoard* board, const char *FEN_string);
void print_board_in_terminal_from_FEN(const char *FEN_string);
void print_bitboard(U64 bb);

#endif 
