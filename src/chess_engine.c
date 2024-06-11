#include "utilities.h"
#include "chess_engine.h"

///@file chess_engine.c
///@brief Code that handles the logic and execution of a legal game of chess

/// @brief prints a ChessBoard in the terminal
/// @param board : A pointer to an empty chess board
void chess_board_init(ChessBoard* board)
{
    board->bitboards = malloc(sizeof(uint64_t) * 6);
    board->piece_list = malloc(sizeof(uint16_t) * 64);
    board->castling_avaliablility = 0;
    board->black = 0;
    board->white = 0;
    board->white_turn = true;
    board->move_counter = 0;
    board->fifty_move_counter = 0;
}
void chess_board_destroy(ChessBoard* board)
{
    free(board->bitboards);
    free(board->piece_list);
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
    if (board->white_turn)
        printf("White to play\n\n");
    else
        printf("Black to play\n\n");

    for (i = 7; i >= 0; i--) // for each rank
    {
        printf("|");
        for (j = 0; j <= 7; j++) // for each file
        {
            int index = get_id_from_rank_file(i, j);
            int piece = board->piece_list[index];
            char symb = piece_id_to_char(piece);
            printf("%c", symb);
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
/// @brief generates the CCRRR piece ID given it's FEN character representation
/// @param piece the FEN character representation
/// @return (int) the piece ID
int create_piece_id(const char piece) // From FEN version of piece ID
{
    int color_mask;
    char _piece = piece;
    if ((int)_piece > (int)'Z') // is lowercase >>> black
        color_mask = 0b01000;
    else {
        color_mask = 0b10000;
        _piece += 32;
    }
    int rank_id;
    switch(_piece){
        case 'p': rank_id = PAWN;   break;
        case 'n': rank_id = KNIGHT; break;
        case 'b': rank_id = BISHOP; break;
        case 'r': rank_id = ROOK;   break;
        case 'q': rank_id = QUEEN;  break;
        case 'k': rank_id = KING;   break;
        default: printf("ERROR: Unknown piece %c\n", _piece); exit(1);
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
    printf("ERROR: Failed to match piece with id: %d\n", piece);
    exit(1);
}

/// @brief generate board from FEN string
/// @param board pointer to empty chessboard
/// @param FEN_string the FEN string
/// @return if function was succesful
void generate_board_from_FEN(ChessBoard* board, const char* FEN_string) 
{
    //TODO
    unsigned int i = 0; // count FEN string index
    unsigned int r = 7; // current rank (zero-indexed)
    unsigned int j = r * 8; // count board index starting at A8
    unsigned int state = 0;
    while (FEN_string[i] != '\0')
    {
        if (state == 0) { // piece arrangement
            printf("%c", FEN_string[i]);
            if (atoi(&FEN_string[i])) {// is numeric
                j += atoi(&FEN_string[i]);
            }
            else if (FEN_string[i] == '/') {
                if (r == 0)
                    throw_error(__LINE__, __FILE__, "ERROR: FEN decode failed. FEN: %s\n",  FEN_string);
                r--;
                j = r * 8;
            }
            else if (FEN_string[i] == ' ') {
                state ++;
                i ++;
                continue;
            }
            else {
                    if (j > r * 8 + 7)
                        throw_error(__LINE__, __FILE__, "FEN decode failed. Tried to index %d on rank %d. FEN: %s\n", j, r, FEN_string);
                    int piece_id = create_piece_id(FEN_string[i]);
                    board->piece_list[j] = piece_id;
                    board->bitboards[piece_id & 0b111] += (1 << j);
                    uint64_t* color_mask = ((piece_id & 0b11000) == 0b10000) ? &(board->white) :&(board->black);
                    *color_mask += (1 << j);
                    j ++;
            }
        }
        else if (state == 1) { // turn to play
            printf("State 1");
            if (FEN_string[i] == 'w') {
                printf("w\n");
            }
            else if (FEN_string[i] == 'b') {
                printf("b\n");
            }
            else {
                throw_error(__LINE__, __FILE__, "Turn-to-play decode failed. Got '%c'. FEN: %s\n", FEN_string[i], FEN_string);
            }
        }
        i ++;
    }
}

/// @brief prints the board corresponding to the FEN string in the terminal window
/// @param FEN_string the FEN string
void print_board_in_terminal_from_FEN(const char* FEN_string)
{
    ChessBoard* board = malloc(sizeof(ChessBoard));
    if (board == (void*) 0) {printf("ERROR: Failed to allocate memory"); exit(1);}
    generate_board_from_FEN(board, FEN_string);
    print_board_in_terminal(board);
    free(board);
}
