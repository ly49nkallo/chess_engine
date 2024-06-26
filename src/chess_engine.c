#include "utilities.h"
#include "chess_engine.h"


///@file chess_engine.c
///@brief Code that handles the logic and execution of a legal game of chess

/// @brief constructor for new ChessBoard object
/// @param board 
void chess_board_init(ChessBoard* board)
{
    board->bitboards = malloc((sizeof board->bitboards) * 6);
    memset(board->bitboards, 0, (sizeof board->bitboards) * 6);
    board->piece_list = malloc((sizeof board->piece_list) * 64);
    memset(board->piece_list, 0, (sizeof board->piece_list) * 64);
    board->castling_avaliablility = 0;
    board->black = 0;
    board->white = 0;
    board->white_turn = true;
    board->move_counter = 0;
    board->fifty_move_counter = 0;
}
/// @brief destructor for ChessBoard object
/// @param board 
void chess_board_destroy(ChessBoard* board)
{
    free(board->bitboards);
    free(board->piece_list);
}

/// @brief prints a ChessBoard in the terminal
/// @param board : A pointer to an empty chess board
void print_board_in_terminal(ChessBoard* board)
{
    /* [White / Black] to play
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
        printf("%d |", i);
        for (j = 0; j <= 7; j++) // for each file
        {
            int index = ID_FROM_RANK_FILE(i, j);
            int piece = board->piece_list[index];
            char symb = piece_id_to_char(piece);
            printf("%c", symb);
            printf("|");
        }
        printf("\n");
    }
    printf("   a b c d e f g h ");
}

/// @brief generates the CCRRR piece ID given it's FEN character representation
/// @param piece the FEN character representation
/// @return (int) the piece ID
int char_to_piece_id(const char piece) // From FEN version of piece ID
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
    if ((piece & 0b111) == EMPTY) return ' ';
    char c = 0;
    switch(piece & 0b111) { 
        case PAWN:   c = 'P'; break;
        case KNIGHT: c = 'N'; break;
        case BISHOP: c = 'B'; break;
        case ROOK:   c = 'R'; break;
        case QUEEN:  c = 'Q'; break;
        case KING:   c = 'K'; break;
    }
    if (c == 0) 
        throw_error(__LINE__, __FILE__, "ERROR: Failed to match piece with id: %d\n", piece);

    return c + (((piece & 0b11000) == TILE_BLACK) ? 32 : 0); // switch for white and black
}

/// @brief generate board from FEN string
/// @param board pointer to empty chessboard
/// @param FEN_string the FEN string
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
                    int piece_id = char_to_piece_id(FEN_string[i]);
                    board->piece_list[j] = piece_id;
                    board->bitboards[piece_id & 0b111] += (1LL << j);
                    uint64_t* color_mask = ((piece_id & 0b11000) == 0b10000) ? &(board->white) :&(board->black);
                    *color_mask += (1LL << j);
                    j ++;
            }
        }
        else if (state == 1) { // turn to play
            if (FEN_string[i] == 'w') {
                state ++;
            }
            else if (FEN_string[i] == 'b') {
                state ++;
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
    ChessBoard board;
    chess_board_init(&board);
    generate_board_from_FEN(&board, FEN_string);
    print_board_in_terminal(&board);
    chess_board_destroy(&board);
}

/// @brief Add a new piece to a chess to the chess board and update bitboards and piece list accordingly
/// @param board 
/// @param rank
/// @param file
/// @param piece_id integer representation of a piece CCRRR
void chess_board_add_piece(ChessBoard* board, const int rank, const int file, const int piece_id) {
    if (rank < 0 || rank > 7 || file < 0 || file > 7)
        throw_error(__LINE__, __FILE__, "Rank/File out of bounds. Rank: %d, File %d", rank, file);
    // Add to bitboards
    if ((piece_id & 0b11000) == TILE_WHITE) {
        board->white += (1LL << ID_FROM_RANK_FILE(rank, file));
    }
    else {
        board->black += (1LL << ID_FROM_RANK_FILE(rank, file));
    }
    int piece_rank = (piece_id & 0b111);
    board->bitboards[piece_rank] += (1 << ID_FROM_RANK_FILE(rank, file));

    // Add to piece list
    board->piece_list[ID_FROM_RANK_FILE(rank, file)] = piece_id;
}
/// @brief moves the piece residing at the from-tile to the to-tile
/// @param from_rank 
/// @param from_file 
/// @param to_rank 
/// @param to_file 
/// @return 1 if the move is a valid move and 0 if move is invalid
int chess_board_move(const int from, const int to) {
    return 0;
}

int chess_board_is_valid(int from_rank, int from_file, int to_rank, int to_file) {
    return 0;
}