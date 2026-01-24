#include "chess_engine.h"
#include "bitboard_utils.h"

///@file chess_engine.c
///@brief Code that handles the logic and execution of a legal game of chess

/// @brief constructor for new ChessBoard object
/// @param board
void chess_board_init(ChessBoard *board)
{
    printf("sizeof board->bitboards = %i\n", (int) (sizeof (board->bitboards)));
    memset(board->bitboards, 0, sizeof (board->bitboards));
    memset(board->piece_list, 0, sizeof (board->piece_list));
    board->castling_avaliablility = 0;
    board->black = 0;
    board->white = 0;
    board->white_turn = true;
    board->move_counter = 0;
    board->fifty_move_counter = 0;
    board->en_passant = 0xF000; // left and right castle are allowed
}
/// @brief destructor for ChessBoard object
/// @param board
void chess_board_destroy(ChessBoard *board)
{
    free(board->bitboards);
    free(board->piece_list);
}
/// @brief constructor for precomputed bitboard
/// @param pbb
void precomputed_bb_init(Precomputed_BB *pbb)
{
    if (pbb->computed) WARNING("Recomputing already populated precomputed bitboards", 0);
    memset(pbb->bishop, 0, sizeof(uint64_t) * 64);
    memset(pbb->knight, 0, sizeof(uint64_t) * 64);
    memset(pbb->rook, 0, sizeof(uint64_t) * 64);
    memset(pbb->queen, 0, sizeof(uint64_t) * 64);
    memset(pbb->king, 0, sizeof(uint64_t) * 64);
    memset(pbb->pawn_white, 0, sizeof(uint64_t) * 64);
    memset(pbb->pawn_black, 0, sizeof(uint64_t) * 64);
    pbb->computed = 0;
}
/// @brief destructor for precomputed bitboard
/// @param pbb
void precomputed_bb_free(Precomputed_BB *pbb)
{
    free(pbb);
}
/// @brief precompute the bitboards
/// @param pbb 
void precomputed_bb_compute(Precomputed_BB *pbb) ///@note TODO
{
    int i; // piece tile is on
    int j; //
    int k;
    // bishops
    // @TODO
    for (i = 0; i < 64; i ++) {
        uint64_t bb = 0ULL;
        // east slide
        bb += e_one(bb);
        pbb->bishop[i] = bb;
    }
}
/// @brief Add a new piece to a chess to the chess board and update bitboards and piece list accordingly
/// @param board
/// @param tile
/// @param piece_id integer representation of a piece CCRRR
/// @return 1 if successful, 0 otherwise
int chess_board_add_piece(ChessBoard *board, const int tile, const int piece_id)
{
    if (tile < 0 || tile > 63)
        ERROR("Rank/File out of bounds. Rank: %d, File %d", tile / 8, tile % 8);
    // Check if the tile is already occupied
    if ((board->white | board->black) & (1LL << tile))
    {
        WARNING("Tile is already occupied %d", tile);
        return 0;
    }
    // Add to bitboards
    int piece_color = piece_id & 0b11000;
    if ((piece_color) == TILE_WHITE)
    {
        if (board->white & (1ULL << tile))
            ERROR("Bitboard index already occupied", 0);
        board->white += (1ULL << tile);
    }
    else if ((piece_color) == TILE_BLACK)
    {
        if (board->white & (1ULL << tile))
            ERROR("Bitboard index already occupied", 0);
        board->black += (1ULL << tile);
    }
    else
        ERROR("Invalid piece color %d", (piece_color));
    int piece_rank = (piece_id & 0b111);
    board->bitboards[piece_rank - 1] += (1ULL << tile);

    // Add to piece list
    board->piece_list[tile] = (uint8_t)piece_id;
    return 1;
}

/// @brief Remove a piece to a chess to the chess board and update bitboards and piece list accordingly
/// @param board
/// @param tile
/// @param piece_id integer representation of a piece CCRRR
/// @return 1 if successful, 0 otherwise
int chess_board_remove_piece(ChessBoard *board, const int tile)
{
    int piece_id = board->piece_list[tile];
    U64 l = (1ULL << tile);
    if (tile < 0 || tile > 63)
        ERROR("Rank/File out of bounds. Rank: %d, File %d", tile / 8, tile % 8);
    // Check if the tile is occupied
    if (!(((board->white) | (board->black)) & l)) {
        WARNING("Failed to detect piece in tile", 0);
        return 0;
    }
    // Remove from bitboards
    if ((piece_id & 0b11000) == TILE_WHITE) {
        if (!(board->white & l)) {
            WARNING("Bitboard white and piece white disagree", 0);
            return 0;
        }
        board->white -= l;
    }
    else if ((piece_id & 0b11000) == TILE_BLACK) {
        if (!(board->black & l)) {
            WARNING("Bitboard black and piece black disagree", 0);
            return 0;
        }
        board->black -= l; // perhaps -= is the fastest operation to do this, but be careful for bugs from carry bit
    }
    else
        ERROR("Piece %d", piece_id);
    int piece_rank = (piece_id & 0b111);
    // If piece does not occupy the area of the board
    if (!(board->bitboards[piece_rank - 1] & l))
    {
        WARNING("Bitboard piece mask disagrees with piece list %llu, %llu", board->bitboards[piece_rank - 1], l);
        return 0;
    }

    board->bitboards[piece_rank - 1] -= l;

    // remove from piece list
    board->piece_list[tile] = 0;
    return 1;
}

/// @brief Get valid moves for a piece (Bitboard representation).
///        This function does not account for checks on the king so it is 'pseudo' legal.
/// @param board
/// @param tile The tile of the piece we should calculate moves for
/// @return Bitboard representing the valid moves
U64 _castle_moves_for_piece(const ChessBoard *board, const int tile, const int color, const int rank)
{
    const U64 occupied = (board->white | board->black);
    U64 bb = 0ULL;

    // Castling availability is stored in en_passant high bits per existing code.
    // Pseudo-legal only: does not verify check or attack on transit squares.
    if (color == TILE_WHITE) {
        // Only allow castling from the starting king square.
        if (rank == KING && tile == 4) {
            if (board->en_passant & (1U << 15)) { // white queen side
                if (!(occupied & ((1ULL << 1) | (1ULL << 2) | (1ULL << 3)))
                    && board->piece_list[0] == (ROOK | TILE_WHITE)) {
                    bb |= (1ULL << 2);
                }
            }
            if (board->en_passant & (1U << 14)) { // white king side
                if (!(occupied & ((1ULL << 5) | (1ULL << 6)))
                    && board->piece_list[7] == (ROOK | TILE_WHITE)) {
                    bb |= (1ULL << 6);
                }
            }
        }
    } else if (color == TILE_BLACK) {
        if (rank == KING && tile == 60) {
            if (board->en_passant & (1U << 13)) { // black queen side
                if (!(occupied & ((1ULL << 57) | (1ULL << 58) | (1ULL << 59)))
                    && board->piece_list[56] == (ROOK | TILE_BLACK)) {
                    bb |= (1ULL << 58);
                }
            }
            if (board->en_passant & (1U << 12)) { // black king side
                if (!(occupied & ((1ULL << 61) | (1ULL << 62)))
                    && board->piece_list[63] == (ROOK | TILE_BLACK)) {
                    bb |= (1ULL << 62);
                }
            }
        }
    }

    return bb;
}

U64 chess_board_pseudo_legal_moves_BB(ChessBoard *board, const int tile)
{
    U64 tile_mask = (1ULL << tile);
    int piece_id = board->piece_list[tile];
    int rank = (piece_id & 0b00111);
    int color = (piece_id & 0b11000);
    U64 bb = 0ULL;
    switch(rank) {
        case EMPTY:
            ERROR("get moves for empty tile (id: %d)", tile);
            return bb; // empty bitboard
        case PAWN: 
            if (color == TILE_WHITE) {
                /* Move */
                // if tile in front is not occupied
                if (!(n_one(tile_mask) & (board->black | board->white))) {
                    // single pawn push
                    bb += n_one(tile_mask);
                    // double pawn push (only possible when pawn on second rank)
                    if ((tile_mask & rank_2) && !(n_one(n_one(tile_mask)) & (board->black | board->white))) {
                        bb += n_one(n_one(tile_mask));
                    }
                }
                {
                    // En passant target is encoded in lower 6 bits of board->en_passant
                    uint16_t ep = (uint16_t)(board->en_passant & 0x3F);
                    U64 ep_mask = ep ? (1ULL << ep) : 0ULL;
                    bb |= (ne_one(tile_mask) | nw_one(tile_mask)) & ep_mask;
                }
                // Capture
                if (ne_one(tile_mask) & board->black) {
                    bb += ne_one(tile_mask);
                }
                if (nw_one(tile_mask) & board->black) {
                    bb += nw_one(tile_mask);
                }
            }
            if (color == TILE_BLACK) {
                /* Move */
                // if tile in front is not occupied
                if (!(s_one(tile_mask) & (board->black | board->white))) {
                    // single pawn push
                    bb += s_one(tile_mask);
                    // double pawn push (only possible when pawn on second rank)
                    if ((tile_mask & rank_7) && !(s_one(s_one(tile_mask)) & (board->black | board->white))) {
                        bb += s_one(s_one(tile_mask));
                    }
                }
                {
                    // En passant target is encoded in lower 6 bits of board->en_passant
                    uint16_t ep = (uint16_t)(board->en_passant & 0x3F);
                    U64 ep_mask = ep ? (1ULL << ep) : 0ULL;
                    bb |= (se_one(tile_mask) | sw_one(tile_mask)) & ep_mask;
                }
                // Capture
                if (se_one(tile_mask) & board->white) {
                    bb += se_one(tile_mask);
                }
                if (sw_one(tile_mask) & board->white) {
                    bb += sw_one(tile_mask);
                }
            }
            break;
        /* *********************************************** */
        case KNIGHT:
            // move in an L shape (2 + 1)
            // TODO make extensible for more pieces or fairy chess
            bb |= ne_one(n_one(tile_mask)) | ne_one(e_one(tile_mask))
                | se_one(s_one(tile_mask)) | se_one(e_one(tile_mask))
                | sw_one(s_one(tile_mask)) | sw_one(w_one(tile_mask))
                | nw_one(n_one(tile_mask)) | nw_one(w_one(tile_mask)); // holy shit thank you copilot
            // capture
            break;
        /* *********************************************** */
                        /* Sliding Moves */
        /* *********************************************** */
        case BISHOP:
            // move / capture
            {int dir;
                for (dir = 0; dir < 8; dir = dir + 2) {
                    bb |= s_slide_with_obstacle(tile_mask, (board->white | board->black), dir, true);
                }
            }
            break;
        /* *********************************************** */
        case ROOK:
            // move / capture
            {int dir;
                for (dir = 1; dir < 8; dir = dir + 2) {
                    bb |= s_slide_with_obstacle(tile_mask, (board->white | board->black), dir, true);
                }
            }
            break;
        /* *********************************************** */
        case QUEEN:
            // move / capture
            {int dir;
                for (dir = 0; dir < 8; ++dir) {
                    bb |= s_slide_with_obstacle(tile_mask, (board->white | board->black), dir, true);
                }
            }
            break;
        /* *********************************************** */
        case KING:
            // move / capture
            {int dir;
                for (dir = 0; dir < 8; ++dir) {
                    U64 o = (board->white | board->black);
                    bb |= shift_one(tile_mask, dir);
                }
            }
            bb |= _castle_moves_for_piece(board, tile, color, rank);
            break;
    }
    print_bitboard(bb);
    return bb;
}

// Fast check for whether a square is attacked by the given side.
static bool is_square_attacked(const ChessBoard *board, const int square, const int attacker_color)
{
    const U64 occ = (board->white | board->black);
    const U64 attackers = (attacker_color == TILE_WHITE) ? board->white : board->black;
    const U64 mask = 1ULL << square;
    const U64 pawns = board->bitboards[PAWN - 1] & attackers;
    const U64 knights = board->bitboards[KNIGHT - 1] & attackers;
    const U64 kings = board->bitboards[KING - 1] & attackers;
    // Offsets match shift[] ordering in bitboard_utils (NE,E,SE,S,SW,W,NW,N).
    const int dir_offset[8] = { 9, 1, -7, -8, -9, -1, 7, 8 };

    // Pawn attacks are directional relative to the attacker.
    if (attacker_color == TILE_WHITE) {
        if ((sw_one(mask) | se_one(mask)) & pawns) return true;
    } else {
        if ((nw_one(mask) | ne_one(mask)) & pawns) return true;
    }

    {
        // Knight jumps from the target square.
        U64 knight_attacks = ne_one(n_one(mask)) | ne_one(e_one(mask))
            | se_one(s_one(mask)) | se_one(e_one(mask))
            | sw_one(s_one(mask)) | sw_one(w_one(mask))
            | nw_one(n_one(mask)) | nw_one(w_one(mask));
        if (knight_attacks & knights) return true;
    }

    {
        // Adjacent king attacks (prevents illegal king adjacency).
        U64 king_attacks = 0ULL;
        for (int dir = 0; dir < 8; ++dir) {
            king_attacks |= shift_one(mask, dir);
        }
        if (king_attacks & kings) return true;
    }

    // Ray-trace sliders; stop at first blocker and check its rank.
    for (int dir = 0; dir < 8; ++dir) {
        U64 cursor = mask;
        int idx = square;
        while (1) {
            cursor = shift_one(cursor, dir);
            if (!cursor) break;
            idx += dir_offset[dir];
            if (cursor & occ) {
                int piece = board->piece_list[idx];
                if ((piece & 0b11000) == attacker_color) {
                    int rank = piece & 0b00111;
                    // Even dirs are diagonals; odd dirs are orthogonals.
                    if ((dir & 1) == 0) {
                        if (rank == BISHOP || rank == QUEEN) return true;
                    } else {
                        if (rank == ROOK || rank == QUEEN) return true;
                    }
                }
                break;
            }
        }
    }

    return false;
}
/// @brief Check if the king of the given color is in check on the given board
/// @param board 
/// @param color 
/// @return true if in check, false otherwise
static bool in_check(ChessBoard *board, const int color)
{
    return false; // @TODO
}
U64 chess_board_legal_moves_BB(ChessBoard *board, const int tile)
{
    U64 pseudo_legal_moves = chess_board_pseudo_legal_moves_BB(board, tile);
    U64 legal_moves = 0ULL;
    const int piece = board->piece_list[tile];
    const int rank = (piece & 0b00111);
    const int color = (piece & 0b11000);
    const int opponent_color = (color == TILE_WHITE) ? TILE_BLACK : TILE_WHITE;
    const U64 occ = (board->white | board->black);
    const uint16_t ep_target = (uint16_t)(board->en_passant & 0x3F);
    int king_square = -1;

    {
        // Cache the current king square for non-king moves.
        U64 king_bb = board->bitboards[KING - 1] & ((color == TILE_WHITE) ? board->white : board->black);
        for (int i = 0; i < 64; ++i) {
            if (king_bb & (1ULL << i)) { king_square = i; break; }
        }
        if (king_square < 0) return 0ULL;
    }

    // Simulate each move and keep only those that preserve king safety.
    for (int to = 0; to < 64; ++to) {
        U64 to_mask = 1ULL << to;
        if (!(pseudo_legal_moves & to_mask)) continue;

        ChessBoard tmp = *board;
        // Shallow copy is enough; we only update occupancy and piece maps.
        U64 from_mask = 1ULL << tile;

        // Remove the moving piece from its source square.
        tmp.piece_list[tile] = 0;
        tmp.bitboards[rank - 1] &= ~from_mask;
        if (color == TILE_WHITE) tmp.white &= ~from_mask;
        else tmp.black &= ~from_mask;

        // Apply capture or en passant on the temp board.
        if (occ & to_mask) {
            int captured_piece = tmp.piece_list[to];
            if (captured_piece) {
                int cap_rank = (captured_piece & 0b00111);
                U64 cap_mask = to_mask;
                tmp.bitboards[cap_rank - 1] &= ~cap_mask;
                if ((captured_piece & 0b11000) == TILE_WHITE) tmp.white &= ~cap_mask;
                else tmp.black &= ~cap_mask;
                tmp.piece_list[to] = 0;
            }
        } else if (rank == PAWN && ep_target && to == ep_target) {
            // En passant capture removes the pawn behind the target square.
            int capture_tile = (color == TILE_WHITE) ? (to - 8) : (to + 8);
            U64 cap_mask = 1ULL << capture_tile;
            int captured_piece = tmp.piece_list[capture_tile];
            if (captured_piece) {
                int cap_rank = (captured_piece & 0b00111);
                tmp.bitboards[cap_rank - 1] &= ~cap_mask;
                if ((captured_piece & 0b11000) == TILE_WHITE) tmp.white &= ~cap_mask;
                else tmp.black &= ~cap_mask;
                tmp.piece_list[capture_tile] = 0;
            }
        }

        // Place the moving piece on its destination.
        tmp.piece_list[to] = piece;
        tmp.bitboards[rank - 1] |= to_mask;
        if (color == TILE_WHITE) tmp.white |= to_mask;
        else tmp.black |= to_mask;

        // Also move the rook for castling so the attack test sees correct blockers.
        if (rank == KING &&
            ((tile == 4 && (to == 6 || to == 2)) || (tile == 60 && (to == 62 || to == 58)))) {
            int rook_from = 0;
            int rook_to = 0;
            if (color == TILE_WHITE) {
                if (to == 6) { rook_from = 7; rook_to = 5; }
                else { rook_from = 0; rook_to = 3; }
            } else {
                if (to == 62) { rook_from = 63; rook_to = 61; }
                else { rook_from = 56; rook_to = 59; }
            }
            int rook_piece = tmp.piece_list[rook_from];
            U64 rook_from_mask = 1ULL << rook_from;
            U64 rook_to_mask = 1ULL << rook_to;
            if (rook_piece) {
                tmp.piece_list[rook_from] = 0;
                tmp.piece_list[rook_to] = rook_piece;
                tmp.bitboards[ROOK - 1] &= ~rook_from_mask;
                tmp.bitboards[ROOK - 1] |= rook_to_mask;
                if (color == TILE_WHITE) {
                    tmp.white = (tmp.white & ~rook_from_mask) | rook_to_mask;
                } else {
                    tmp.black = (tmp.black & ~rook_from_mask) | rook_to_mask;
                }
            }
        }

        // King square changes only if the king moved.
        int king_sq = (rank == KING) ? to : king_square;
        if (!is_square_attacked(&tmp, king_sq, opponent_color)) {
            legal_moves |= to_mask;
        }
    }

    return legal_moves;
}

/// @brief Get valid moves for a piece (Index array representation)
/// @param board
/// @param tile
/// @return Array of integers representing the valid moves on the board
int *chess_board_get_pseudo_legal_moves_arr(ChessBoard *board, const int tile)
{
    // techinically less efficient than bitboard representation because of linear time lookup

    int piece_id = board->piece_list[tile];
    int rank = (piece_id & 0b00111);
    int color = (piece_id & 0b11000);
    int *ret = malloc(64 * sizeof(int));
    switch (rank)
    {
    }
    throw_not_implemented_error(__LINE__, __FILE__);
    return (void *)0;
}
/// @brief Move a piece from one tile to another, enforcing turn order and pseudo-legal moves.
/// @param board
/// @param from Source tile index (0-63)
/// @param to Destination tile index (0-63)
/// @return 1 if successful, 0 otherwise
// @note Captures opponent pieces on the destination square.
int chess_board_move(ChessBoard *board, const int from, const int to)
{
    if (from == to) {
        WARNING("Tried to move piece to same tile: %d", from);
        return 0; // nothing to be done
    }
    int piece = board->piece_list[from];
    int color = piece & 0b11000;
    int rank = piece & 0b00111;
    uint16_t ep_target = (uint16_t)(board->en_passant & 0x3F);
    int is_castle = (rank == KING)
        && ((from == 4 && (to == 6 || to == 2)) || (from == 60 && (to == 62 || to == 58)));
    U64 targets_bb = (board->white | board->black) & (~(1ULL << from)); // all pieces except the one moving
    INFO("Move piece %c from tile %d to tile %d", piece_id_to_char(piece), from, to);

    // Validate move and enforce pseudo-legal moves, as well as turn order
    // Check turn order
    if (!(board->white_turn && color == TILE_WHITE) 
        && !(!board->white_turn && color == TILE_BLACK)) {
        WARNING("Cannot move piece not on turn. It's %s's turn but piece is %s", 
            (board->white_turn) ? "White" : "Black",
            (color == TILE_WHITE) ? "White" : "Black");
        return 0;
    }
    // Check the move is a member of the legal moves
    if (!(chess_board_legal_moves_BB(board, from) & (1ULL << to))) {
        WARNING("Move is not legal. Piece %c at %d cannot move to %d", 
            piece_id_to_char(piece), from, to);
        return 0;
    }
    // Check for moving onto own piece
    if (color == TILE_WHITE) {
        if (board->white & (1ULL << to)) {
            WARNING("Cannot move onto own piece at tile %d", to);
            return 0;
        }
    } else if (color == TILE_BLACK) {
        if (board->black & (1ULL << to)) {
            WARNING("Cannot move onto own piece at tile %d", to);
            return 0;
        }
    }

    // Check for captures
    if (color == TILE_WHITE)
    {
        // If white captures black
        if (board->black & (1ULL << to)) {
            if (!chess_board_remove_piece(board, to))
                ERROR("Unable to remove piece from tile %d", to);
        }
    }
    else if (color == TILE_BLACK)
    {
        // If black captures white
        if (board->white & (1ULL << to)) {
            if (!chess_board_remove_piece(board, to))
                ERROR("Unable to remove piece from tile %d", to);
        }
    }
    if (rank == PAWN && ep_target && to == ep_target
        && !((board->white | board->black) & (1ULL << to))) {
        // En passant capture removes the pawn behind the target square.
        int capture_tile = (color == TILE_WHITE) ? (to - 8) : (to + 8);
        if (!chess_board_remove_piece(board, capture_tile))
            ERROR("Unable to remove piece from tile %d", capture_tile);
    }
    // Update castling rights when king or rook moves.
    if (rank == KING) {
        if (color == TILE_WHITE)
            board->en_passant &= ~((1U << 15) | (1U << 14));
        else
            board->en_passant &= ~((1U << 13) | (1U << 12));
    } else if (rank == ROOK) {
        if (color == TILE_WHITE) {
            if (from == 0) board->en_passant &= ~(1U << 15);
            if (from == 7) board->en_passant &= ~(1U << 14);
        } else {
            if (from == 56) board->en_passant &= ~(1U << 13);
            if (from == 63) board->en_passant &= ~(1U << 12);
        }
    }
    // Attempts to actually move the piece
    if (!chess_board_remove_piece(board, from)) ERROR("Unable to remove piece from tile %d", from);
    if (!chess_board_add_piece(board, to, piece)) ERROR("Unable to add piece to tile %d", to);
    if (is_castle) {
        // Move the rook to complete the castling move.
        int rook_from = 0;
        int rook_to = 0;
        if (color == TILE_WHITE) {
            if (to == 6) { rook_from = 7; rook_to = 5; }
            else { rook_from = 0; rook_to = 3; }
        } else {
            if (to == 62) { rook_from = 63; rook_to = 61; }
            else { rook_from = 56; rook_to = 59; }
        }
        int rook_piece = board->piece_list[rook_from];
        if (!chess_board_remove_piece(board, rook_from))
            ERROR("Unable to remove piece from tile %d", rook_from);
        if (!chess_board_add_piece(board, rook_to, rook_piece))
            ERROR("Unable to add piece to tile %d", rook_to);
    }
    {
        uint16_t new_ep = 0;
        if (rank == PAWN) {
            if (color == TILE_WHITE) {
                if (to - from == 16) new_ep = (uint16_t)(from + 8);
            } else {
                if (from - to == 16) new_ep = (uint16_t)(from - 8);
            }
        }
        board->en_passant = (board->en_passant & 0xF000) | new_ep;
    }
    board->white_turn = !board->white_turn;
    board->move_counter += 1;
    if ((piece & 0b111) == PAWN || (targets_bb & (1ULL << to))) {
        board->fifty_move_counter = 0;
    }
    else {
        board->fifty_move_counter += 1;
    }
    return 1;
}

//////////////////////////////////////////
/*              UTILITIES               */
//////////////////////////////////////////

/// @brief prints a ChessBoard in the terminal
/// @param board : A pointer to an empty chess board
void print_board_in_terminal(ChessBoard *board)
{
    /* [White / Black] to play
       8 |r|n|b|q|k|b|n|r| [L][R]
       7 |p|p|p|p|p|p|p|p| move c:[n]
       6 | | | | | | | | |
       5 | | | | | | | | |
       4 | | | | | | | | |
       3 | | | | | | | | |
       2 |P|P|P|P|P|P|P|P| 50 move c:[n]
       1 |R|N|B|Q|K|B|N|R| [L][R]
          A B C D E F G H
    */
    int i,j;
    if (!board->white_turn)
        printf("\nWhite to play\n");
    else
        printf("\nBlack to play\n");

    for (i = 7; i >= 0; i--) // for each rank
    {
        printf("%d |", i + 1);
        for (j = 0; j < 8; j++) // for each file
        {
            int index = ID_FROM_RANK_FILE(i, j);
            int piece = board->piece_list[index];
            char symb = piece_id_to_char(piece);
            printf("%c", symb);
            printf("|");
        }
        if (i == 0) {
            if (board->en_passant & (1 << 15)) printf(" [L]");
            if (board->en_passant & (1 << 14)) printf(" [R]");
        }
        if (i == 1) {
            printf(" 50 move c:[%d]", board->fifty_move_counter);
        }
        if (i == 6) {
            printf(" move c:[%d]", board->move_counter);
        }   
        if (i == 7) {
            if (board->en_passant & (1 << 15)) printf(" [L]");
            if (board->en_passant & (1 << 14)) printf(" [R]");
        }
        printf("\n");
    }
    printf("   a b c d e f g h \n");
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
    else
    {
        color_mask = 0b10000;
        _piece += 32;
    }
    int rank_id;
    switch (_piece)
    {
    case 'p':
        rank_id = PAWN;
        break;
    case 'n':
        rank_id = KNIGHT;
        break;
    case 'b':
        rank_id = BISHOP;
        break;
    case 'r':
        rank_id = ROOK;
        break;
    case 'q':
        rank_id = QUEEN;
        break;
    case 'k':
        rank_id = KING;
        break;
    default:
        ERROR("Unknown piece %c", _piece);
    }
    return rank_id | color_mask;
}

/// @brief takes the integer denoting the piece and returns the FEN symbol
/// @param piece integer representing the piece's identifier
/// @return character symbol corresponding to FEN notation
char piece_id_to_char(const int piece)
{
    if ((piece & 0b111) == EMPTY)
        return ' ';
    char c = 0;
    switch (piece & 0b111)
    {
    case PAWN:
        c = 'P';
        break;
    case KNIGHT:
        c = 'N';
        break;
    case BISHOP:
        c = 'B';
        break;
    case ROOK:
        c = 'R';
        break;
    case QUEEN:
        c = 'Q';
        break;
    case KING:
        c = 'K';
        break;
    }
    if (c == 0)
        ERROR("ERROR: Failed to match piece with id: %d\n", piece);

    return c + (((piece & 0b11000) == TILE_BLACK) ? 32 : 0); // switch for white and black
}

/// @brief generate board from FEN string
/// @param board pointer to empty chessboard
/// @param FEN_string the FEN string
void generate_board_from_FEN(ChessBoard *board, const char *FEN_string)
{
    // TODO
    unsigned int i = 0;     // count FEN string index
    unsigned int r = 7;     // current rank (zero-indexed)
    unsigned int j = r * 8; // count board index starting at A8
    unsigned int state = 0;
    while (FEN_string[i] != '\0') {
        if (state == 0) { // piece arrangement
            if (atoi(&FEN_string[i])) { // is numeric
                j += atoi(&FEN_string[i]);
            }
            else if (FEN_string[i] == '/') {
                if (r == 0)
                    ERROR("ERROR: FEN decode failed. FEN: %s\n", FEN_string);
                r--;
                j = r * 8;
            }
            else if (FEN_string[i] == ' ') {
                state++;
                i++;
                continue;
            }
            else {
                if (j > r * 8 + 7)
                    ERROR("FEN decode failed. Tried to index %d on rank %d. FEN: %s\n", j, r, FEN_string);
                int piece_id = char_to_piece_id(FEN_string[i]);
                chess_board_add_piece(board, j, piece_id);
                j++;
            }
        }
        else if (state == 1) { // turn to play
            if (FEN_string[i] == 'w') {
                board->white_turn = 1;
                state++;
            }
            else if (FEN_string[i] == 'b') {
                board->white_turn = 0;
                state++;
            }
            else
                ERROR("Turn-to-play decode failed. Got '%c'. FEN: %s\n", FEN_string[i], FEN_string);
        }
        i++;
    }
}

/// @brief prints the board corresponding to the FEN string in the terminal window
/// @param FEN_string the FEN string
void print_board_in_terminal_from_FEN(const char *FEN_string)
{
    ChessBoard board;
    chess_board_init(&board);
    generate_board_from_FEN(&board, FEN_string);
    print_board_in_terminal(&board);
    chess_board_destroy(&board);
}

/// @brief Pretty print a bitboard to the terminal in 8x8 square corresponding to chess board
/// @param bb the bitboard (64 bits)
void print_bitboard(uint64_t bb) // Maybe depreciate
{
    int i = 7; int j = 0;
    char row[9];
    while(i >= 0) {
        while(j < 8) {
            char c = (((bb >> (i * 8 + j)) & 1) == 1) ? '#' : '.';
            row[j] = c;
            j++;
        }
        row[8] = '\0';
        j = 0;
        puts(row); // already prints newline character
        i--;
    }
    printf("   a b c d e f g h \n");
}
/// @brief Returns True if ChessBoard is a valid potential gamestate, False if otherwise
/// @param board 
bool verify_chessboard(ChessBoard *board) {
    ERROR("Not Implemented!", 0);
    return false; // placeholder
}
