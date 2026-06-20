#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include "../src/chess_engine.h"

///@file test.c
///@brief Tests for the move generator (bitboard and array) and king check logic.

static int tests_run = 0;
static int tests_failed = 0;

#define ASSERT_U64(desc, got, exp) do {                              \
    tests_run++;                                                     \
    U64 _g = (got), _e = (exp);                                      \
    if (_g != _e) {                                                  \
        tests_failed++;                                              \
        printf("FAIL: %s\n  got:\n", desc); print_bitboard(_g);      \
        printf("  expected:\n"); print_bitboard(_e);                 \
    } else {                                                         \
        printf("PASS: %s\n", desc);                                  \
    }                                                                \
} while (0)

#define ASSERT_TRUE(desc, cond) do {                                 \
    tests_run++;                                                     \
    if (!(cond)) { tests_failed++; printf("FAIL: %s\n", desc); }     \
    else { printf("PASS: %s\n", desc); }                            \
} while (0)

/// Build a bitboard from a -1 terminated list of tile indices.
static U64 mask_arr(const int *tiles)
{
    U64 b = 0ULL;
    for (; *tiles >= 0; ++tiles) b |= (1ULL << *tiles);
    return b;
}

/// Compare a -1 terminated array against an expected -1 terminated array.
static int arr_equals(const int *got, const int *exp)
{
    int i = 0;
    while (got[i] >= 0 && exp[i] >= 0) {
        if (got[i] != exp[i]) return 0;
        i++;
    }
    return got[i] == exp[i]; // both must hit the -1 terminator together
}

static void setup(ChessBoard *board, const char *fen)
{
    chess_board_init(board);
    generate_board_from_FEN(board, fen);
}

static void test_pawn_pushes(void)
{
    ChessBoard board;
    setup(&board, CE_FEN_STARTING_POSITION);
    /* e2 = rank1 file4 = 12, double push must reach e4 (28). */
    int e2_exp[] = {20, 28, -1};
    ASSERT_U64("white pawn e2 double push",
        chess_board_pseudo_legal_moves_BB(&board, 12), mask_arr(e2_exp));
    /* h2 = 15, the H-file pawn must not wrap; pushes to h3/h4. */
    int h2_exp[] = {23, 31, -1};
    ASSERT_U64("white H-file pawn h2 (no wrap)",
        chess_board_pseudo_legal_moves_BB(&board, 15), mask_arr(h2_exp));
    /* a2 = 8, the A-file pawn must not wrap; pushes to a3/a4. */
    int a2_exp[] = {16, 24, -1};
    ASSERT_U64("white A-file pawn a2 (no wrap)",
        chess_board_pseudo_legal_moves_BB(&board, 8), mask_arr(a2_exp));
}

static void test_pawn_captures(void)
{
    ChessBoard board;
    /* White pawn e2 with black pawns on d3 (19) and f3 (21). */
    setup(&board, "8/8/8/8/8/3p1p2/4P3/8 w - - 0 1");
    int exp[] = {19, 20, 21, 28, -1};
    ASSERT_U64("white pawn e2 captures d3/f3 plus pushes",
        chess_board_pseudo_legal_moves_BB(&board, 12), mask_arr(exp));
}

static void test_knight(void)
{
    ChessBoard board;
    setup(&board, CE_FEN_STARTING_POSITION);
    /* b1 knight = 1; d2 (11) is blocked by an own pawn, leaving a3/c3. */
    int exp[] = {16, 18, -1};
    ASSERT_U64("white knight b1 excludes own-occupied d2",
        chess_board_pseudo_legal_moves_BB(&board, 1), mask_arr(exp));
}

static void test_slider(void)
{
    ChessBoard board;
    /* Lone white rook on d4 (27) on an otherwise empty board. */
    setup(&board, "8/8/8/8/3R4/8/8/8 w - - 0 1");
    int exp[] = {
        3, 11, 19,            /* down the d-file */
        24, 25, 26, 28, 29, 30, 31, /* along rank 4 */
        35, 43, 51, 59,       /* up the d-file */
        -1
    };
    ASSERT_U64("white rook d4 on empty board",
        chess_board_pseudo_legal_moves_BB(&board, 27), mask_arr(exp));
}

static void test_check_detection(void)
{
    ChessBoard board;
    /* Black rook e2 gives check to white king e1; black king is safe. */
    setup(&board, "4k3/8/8/8/8/8/4r3/4K3 w - - 0 1");
    ASSERT_TRUE("white king is in check", chess_board_is_in_check(&board, TILE_WHITE));
    ASSERT_TRUE("black king is not in check", !chess_board_is_in_check(&board, TILE_BLACK));
}

static void test_legal_check_evasion(void)
{
    ChessBoard board;
    /* White king e1 in check from rook e2: may step to d1/f1 or capture on e2. */
    setup(&board, "4k3/8/8/8/8/8/4r3/4K3 w - - 0 1");
    int exp[] = {3, 5, 12, -1};
    ASSERT_U64("white king legal check evasions",
        chess_board_legal_moves_BB(&board, 4), mask_arr(exp));
}

static void test_castling(void)
{
    ChessBoard board;
    /* Both sides have full castling rights and clear lanes. */
    setup(&board, "r3k2r/8/8/8/8/8/8/R3K2R w - - 0 1");
    U64 king_moves = chess_board_legal_moves_BB(&board, 4);
    ASSERT_TRUE("white may castle king-side (g1)", king_moves & (1ULL << 6));
    ASSERT_TRUE("white may castle queen-side (c1)", king_moves & (1ULL << 2));

    /* Perform the king-side castle and verify rook relocation. */
    ASSERT_TRUE("king-side castle move accepted", chess_board_move(&board, 4, 6));
    ASSERT_TRUE("king now on g1", board.piece_list[6] == (KING | TILE_WHITE));
    ASSERT_TRUE("rook now on f1", board.piece_list[5] == (ROOK | TILE_WHITE));
    ASSERT_TRUE("e1 vacated", board.piece_list[4] == 0);
    ASSERT_TRUE("h1 vacated", board.piece_list[7] == 0);
}

static void test_en_passant(void)
{
    ChessBoard board;
    setup(&board, CE_FEN_STARTING_POSITION);
    ASSERT_TRUE("1. e4", chess_board_move(&board, 12, 28));
    ASSERT_TRUE("1... a6", chess_board_move(&board, 48, 40));
    ASSERT_TRUE("2. e5", chess_board_move(&board, 28, 36));
    ASSERT_TRUE("2... d5 (double push)", chess_board_move(&board, 51, 35));
    /* e5 pawn (36) should now see the en passant target d6 (43). */
    ASSERT_TRUE("en passant target available",
        chess_board_pseudo_legal_moves_BB(&board, 36) & (1ULL << 43));
    ASSERT_TRUE("3. exd6 e.p. accepted", chess_board_move(&board, 36, 43));
    ASSERT_TRUE("white pawn now on d6", board.piece_list[43] == (PAWN | TILE_WHITE));
    ASSERT_TRUE("captured black pawn removed from d5", board.piece_list[35] == 0);
    ASSERT_TRUE("e5 vacated", board.piece_list[36] == 0);
}

static void test_array_generator(void)
{
    ChessBoard board;
    /* Array form must agree with the bitboard form (ascending, -1 terminated). */
    setup(&board, "8/8/8/8/8/3p1p2/4P3/8 w - - 0 1");
    int *got = chess_board_pseudo_legal_moves_arr(&board, 12);
    int exp[] = {19, 20, 21, 28, -1};
    ASSERT_TRUE("pseudo-legal array matches bitboard", arr_equals(got, exp));
    free(got);

    setup(&board, "4k3/8/8/8/8/8/4r3/4K3 w - - 0 1");
    int *legal = chess_board_legal_moves_arr(&board, 4);
    int legal_exp[] = {3, 5, 12, -1};
    ASSERT_TRUE("legal array matches bitboard", arr_equals(legal, legal_exp));
    free(legal);
}

int main(void)
{
    test_pawn_pushes();
    test_pawn_captures();
    test_knight();
    test_slider();
    test_check_detection();
    test_legal_check_evasion();
    test_castling();
    test_en_passant();
    test_array_generator();

    printf("\n==== %d/%d tests passed ====\n", tests_run - tests_failed, tests_run);
    return (tests_failed == 0) ? 0 : 1;
}
