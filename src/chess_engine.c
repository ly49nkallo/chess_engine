#include "utilities.h"
#include "chess_engine.h"
///@file chess_engine.c
///@brief Code that handles the logic and execution of a legal game of chess

/// @brief prints a ChessBoard in the terminal
/// @param board : A pointer to an empty chess board

void chess_board_init(ChessBoard* board)
{
    board->bitmasks = MemAlloc(sizeof(uint64_t) * 6);
    board->piece_list = MemAlloc(sizeof(uint16_t) * 8 * 4);
    board->castling_avaliablility = 0;
    board->black = 0;
    board->white = 0;
    board->white_turn = true;
    board->move_counter = 0;
    board->fifty_move_counter = 0;
}
void chess_board_destroy(ChessBoard* board)
{
    MemFree(board->bitmasks);
    MemFree(board->piece_list);
}

void print_board_in_terminal(ChessBoard* board)
{
    /* 
       8 |r|n|b|q|k|b|n|r| 
       7 |p|p|p|p|p|p|p|p|
       6 | | | | | | | | |
       5 | | | | | | | | | 
       4 | | | | | | | | | 
       3 | | | | | | | | | 
       2 |P|P|P|P|P|P|P|P|
       1 |R|N|B|Q|K|B|N|R|
          A B C D E F G H
    */
    int i,j;
    int piece;
    abort();
    for (i = 0; i < 8; i++) // for each line
    {
        printf("|");
        for (j = 0; j < 8; j++) 
        {
            printf("|");
        }
    }
}

/// @brief get the corresponding index of a tile given rank and file
/// @param rank zero indexed rank
/// @param file zero indexed file
/// @return index of tile
int get_id_from_rank_file(const int rank, const int file)
{
    return rank * 8 + file;
}
/// @brief 
/// @param piece 
/// @return 
int create_piece_id(const char piece) // From FEN version of piece ID
{
    int color_mask;
    char _piece = piece;
    if ((int)_piece > (int)'Z') // is lowercase >>> black
        color_mask = 0b01000;
    else {
        color_mask = 0b10000;
        _piece -= 32;
    }
    int rank_id;
    switch(_piece){
        case 'p': rank_id = PAWN;   break;
        case 'n': rank_id = KNIGHT; break;
        case 'b': rank_id = BISHOP; break;
        case 'r': rank_id = ROOK;   break;
        case 'q': rank_id = QUEEN;  break;
        case 'k': rank_id = KING;   break;
        default: printf("Unknown piece %c", _piece); exit(1);
    }
    return rank_id | color_mask;
}


/// @brief takes the integer denoting the piece and returns the FEN symbol
/// @param piece integer representing the piece's identifier
/// @return symbol corresponding to FEN notation
char piece_id_to_char(const int piece)
{
    if (piece & 0b10000) //white
    {
        switch(piece & 0b111) {
        case PAWN:   return 'P'; break;
        case KNIGHT: return 'N'; break;
        case BISHOP: return 'B'; break;
        case ROOK:   return 'R'; break;
        case QUEEN:  return 'Q'; break;
        case KING:   return 'K'; break;
        }
    }
    else
    {
        switch(piece & 0b111) {
        case PAWN:   return 'p'; break;
        case KNIGHT: return 'n'; break;
        case BISHOP: return 'b'; break;
        case ROOK:   return 'r'; break;
        case QUEEN:  return 'q'; break;
        case KING:   return 'k'; break;
        }
    }
    printf("Failed to match piece with id: %d", piece);
    exit(1);
}

/// @brief generate board from FEN string
/// @param board pointer to empty chessboard
/// @param FEN_string the FEN string
/// @return if function was succesful
void generate_board_from_FEN(ChessBoard* board, const char* FEN_string) 
{
    //TODO
    int i = 0; // count FEN string index
    int r = 8; // current rank
    int j = r * 8 - 7; // count board index starting at A8
    int state = 0;
    while (FEN_string[i] != '\0' && r > 0)
    {
        if (state == 0) {
            if (atoi(&FEN_string[i])) {// is numeric
                j += atoi(&FEN_string[i]);
            }
            else if (FEN_string[i] == '/') {
                r--;
                j = r * 8 - 7;
            }
            else if (FEN_string[i] == ' ') {
                state ++;
            }
            else {
                //TODO
            }
        }
        i++;
    }
}

/// @brief prints the board corresponding to the FEN string in the terminal window
/// @param FEN_string the FEN string
void print_board_in_terminal_from_FEN(const char* FEN_string)
{
    ChessBoard* board = MemAlloc(sizeof(ChessBoard));
    if (board == (void*) 0) {printf("ERROR: Failed to allocate memory"); exit(1);}
    generate_board_from_FEN(board, FEN_string);
    print_board_in_terminal(board);
    MemFree(board);
}