#include "chess_engine.h"

static void uci_print_id(void)
{
    // Identify engine for UCI handshake.
    printf("id name chess_engine\n");
    printf("id author unknown\n");
    printf("uciok\n");
    fflush(stdout);
}

static void uci_print_ready(void)
{
    // Respond to isready for synchronization.
    printf("readyok\n");
    fflush(stdout);
}

static void uci_handle_position(ChessBoard *board, const char *line)
{
    // Parse "position" command, load the board, then apply optional move list.
    const char *p = line + 8; // skip "position"
    while (*p == ' ') p++;

    if (strncmp(p, "startpos", 8) == 0) {
        // Standard initial position.
        chess_board_init(board);
        generate_board_from_FEN(board, CE_FEN_STARTING_POSITION);
        p += 8;
    } else if (strncmp(p, "fen", 3) == 0) {
        // Parse explicit FEN (first 6 fields) and load it.
        char fen[128];
        char *f = fen;
        int fields = 0;
        p += 3;
        while (*p == ' ') p++;
        // Copy exactly 6 FEN fields into a single string, stopping at EOL.
        while (*p && fields < 6) {
            while (*p == ' ') p++;
            if (*p == '\0' || *p == '\n' || *p == '\r') break;
            while (*p && *p != ' ' && *p != '\n' && *p != '\r') {
                if (f < fen + (int)sizeof(fen) - 1) *f++ = *p;
                p++;
            }
            if (f < fen + (int)sizeof(fen) - 1) *f++ = ' ';
            fields++;
        }
        if (f != fen) f--;
        *f = '\0';
        chess_board_init(board);
        generate_board_from_FEN(board, fen);
    } else {
        return;
    }

    // Apply subsequent moves, if any.
    while (*p == ' ') p++;
    if (strncmp(p, "moves", 5) != 0) return;
    p += 5;
    while (*p) {
        char m[8];
        int len = 0;
        while (*p == ' ') p++;
        if (*p == '\0' || *p == '\n' || *p == '\r') break;
        // Read one UCI move token (e.g. "e2e4" or "e7e8q").
        while (*p && *p != ' ' && *p != '\n' && *p != '\r') {
            if (len < (int)sizeof(m) - 1) m[len++] = *p;
            p++;
        }
        m[len] = '\0';
        if (len < 4) continue;
        {
            // Convert UCI move to 0..63 tile indexes and play it.
            int ffile = m[0] - 'a';
            int frank = m[1] - '1';
            int tfile = m[2] - 'a';
            int trank = m[3] - '1';
            if ((unsigned)ffile > 7 || (unsigned)frank > 7 ||
                (unsigned)tfile > 7 || (unsigned)trank > 7) {
                continue;
            }
            int from = ID_FROM_RANK_FILE(frank, ffile);
            int to = ID_FROM_RANK_FILE(trank, tfile);
            if (!chess_board_move(board, from, to)) continue;
            if (len >= 5) {
                // Apply promotion if present.
                int piece = board->piece_list[to];
                if ((piece & 0b111) == PAWN) {
                    int rank = 0;
                    // UCI promotions are indicated by a trailing piece letter.
                    switch (m[4]) {
                    case 'q': case 'Q': rank = QUEEN; break;
                    case 'r': case 'R': rank = ROOK; break;
                    case 'b': case 'B': rank = BISHOP; break;
                    case 'n': case 'N': rank = KNIGHT; break;
                    default: rank = 0; break;
                    }
                    if (rank) {
                        int color = piece & 0b11000;
                        chess_board_remove_piece(board, to);
                        chess_board_add_piece(board, to, rank | color);
                    }
                }
            }
        }
    }
}

static void uci_handle_go(ChessBoard *board, const char *line)
{
    // TODO: parse search limits (depth, movetime, wtime/btime, etc).
    // Quick move selection: find the first legal move and return it.
    const int color = board->white_turn ? TILE_WHITE : TILE_BLACK;
    U64 pieces = board->white_turn ? board->white : board->black;
    int from = -1;
    int to = -1;

    // Walk all pieces for the side to move and pick the first legal move. // Why?
    while (pieces) {
        int sq = 0;
#ifdef USING_INTRINSICS
        unsigned long idx = 0;
        _BitScanForward64(&idx, pieces);
        sq = (int)idx;
#else
        {
            U64 tmp = pieces;
            while ((tmp & 1ULL) == 0ULL) {
                tmp >>= 1;
                sq++;
            }
        }
#endif
        pieces &= pieces - 1;
        if ((board->piece_list[sq] & 0b11000) != color) continue;
        U64 moves = chess_board_legal_moves_BB(board, sq);
        if (moves) {
#ifdef USING_INTRINSICS
            unsigned long tidx = 0;
            _BitScanForward64(&tidx, moves);
            to = (int)tidx;
#else
            {
                int t = 0;
                U64 tmp = moves;
                while ((tmp & 1ULL) == 0ULL) {
                    tmp >>= 1;
                    t++;
                }
                to = t;
            }
#endif
            from = sq;
            break;
        }
    }

    if (from < 0 || to < 0) {
        // No legal move available (checkmate/stalemate).
        printf("bestmove 0000\n");
        fflush(stdout);
        return;
    }

    // Convert from/to squares into UCI notation.
    char uci[6];
    uci[0] = (char)('a' + (from & 7));
    uci[1] = (char)('1' + (from >> 3));
    uci[2] = (char)('a' + (to & 7));
    uci[3] = (char)('1' + (to >> 3));
    uci[4] = '\0';

    // Auto-queen for promotions.
    if ((board->piece_list[from] & 0b00111) == PAWN) {
        int to_rank = to >> 3;
        if ((color == TILE_WHITE && to_rank == 7) || (color == TILE_BLACK && to_rank == 0)) {
            uci[4] = 'q';
            uci[5] = '\0';
        }
    }

    printf("bestmove %s\n", uci);
    fflush(stdout);
    (void)line;
}

static void uci_handle_stop(void)
{
    // TODO: signal any ongoing search to stop.
}

static void uci_handle_ucinewgame(ChessBoard *board)
{
    // TODO: reset engine state (hash, history, counters).
    (void)board;
}

void uci_loop(ChessBoard *board)
{
    char line[512];
    while (fgets(line, sizeof(line), stdin)) {
        // Prefix checks keep parsing simple and match standard UCI usage.
        if (strncmp(line, "uci", 3) == 0) {
            uci_print_id();
        } else if (strncmp(line, "isready", 7) == 0) {
            uci_print_ready();
        } else if (strncmp(line, "ucinewgame", 10) == 0) {
            uci_handle_ucinewgame(board);
        } else if (strncmp(line, "position", 8) == 0) {
            uci_handle_position(board, line);
        } else if (strncmp(line, "go", 2) == 0) {
            uci_handle_go(board, line);
        } else if (strncmp(line, "stop", 4) == 0) {
            uci_handle_stop();
        } else if (strncmp(line, "quit", 4) == 0) {
            break;
        }
    }
}