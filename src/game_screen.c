#include "game_screen.h"
#include "game_config.h"
#include "bot_player.h"
#include <math.h>
#include <string.h>

/* LOCAL VARIABLES */
static unsigned long frameCount;
static int screenEnded;
static struct BoardTheme *currentBoardTheme;
static ChessBoard *current_board;
static int board_height, board_width;
static Vector2 board_position;
static int whiteSideDown;
static Font boardTextFont;
static Texture2D spritesheet;
/* You can see where I gave up and changed to snake case */
static int sprite_size;
static int hovered_tile_idx;
static int selected_tile_idx;
static int dragging_piece;
static U64 legal_moves_bb;

#define MAX_ARROWS 32
static U64 highlighted_tiles_bb;     // tiles marked by a right click
static Move arrows[MAX_ARROWS];      // arrows drawn by right click + drag
static int arrow_count;
static int right_drag_origin;        // tile where the right button was pressed (-1 if none)

/* Bot players (NULL when the side is human) */
static BotPlayer *s_white_bot;
static BotPlayer *s_black_bot;

/* Move history string for UCI "position ... moves" commands.
 * Each move is appended as "<move> " after it is applied. */
#define MOVE_HISTORY_MAX 4096
static char s_move_history[MOVE_HISTORY_MAX];

/* get_exe_path is defined in bot_registry.c and declared in bot_registry.h */

/* Append a UCI move token to s_move_history. */
static void history_push(const char *move)
{
    strncat(s_move_history, move,
            MOVE_HISTORY_MAX - (int)strlen(s_move_history) - 2);
    strncat(s_move_history, " ",
            MOVE_HISTORY_MAX - (int)strlen(s_move_history) - 1);
}

/* Convert from/to squares to a UCI move string (e.g. "e2e4").
 * Handles queen-promotion automatically for pawns reaching rank 1/8. */
static void squares_to_uci(const ChessBoard *board, int from, int to,
                             char out[8])
{
    out[0] = (char)('a' + (from & 7));
    out[1] = (char)('1' + (from >> 3));
    out[2] = (char)('a' + (to   & 7));
    out[3] = (char)('1' + (to   >> 3));
    out[4] = '\0';
    /* Auto-promote to queen */
    int piece = board->piece_list[from];
    if ((piece & 0b111) == PAWN) {
        int to_rank = to >> 3;
        int color   = piece & 0b11000;
        if ((color == TILE_WHITE && to_rank == 7) ||
            (color == TILE_BLACK && to_rank == 0)) {
            out[4] = 'q';
            out[5] = '\0';
        }
    }
}

/* Apply a UCI move string to the board and record it in history.
 * Returns true on success. */
static bool apply_uci_move(ChessBoard *board, const char *uci_move)
{
    if (!uci_move || uci_move[0] == '0') return false;
    int ffile = uci_move[0] - 'a';
    int frank = uci_move[1] - '1';
    int tfile = uci_move[2] - 'a';
    int trank = uci_move[3] - '1';
    if ((unsigned)ffile > 7 || (unsigned)frank > 7 ||
        (unsigned)tfile > 7 || (unsigned)trank > 7) return false;
    int from = ID_FROM_RANK_FILE(frank, ffile);
    int to   = ID_FROM_RANK_FILE(trank, tfile);
    if (!chess_board_move(board, from, to)) return false;
    /* Handle promotion suffix */
    if (uci_move[4] && uci_move[4] != ' ') {
        int piece = board->piece_list[to];
        if ((piece & 0b111) == PAWN) {
            int rank = 0;
            switch (uci_move[4]) {
                case 'q': case 'Q': rank = QUEEN;  break;
                case 'r': case 'R': rank = ROOK;   break;
                case 'b': case 'B': rank = BISHOP; break;
                case 'n': case 'N': rank = KNIGHT; break;
                default: break;
            }
            if (rank) {
                int color = piece & 0b11000;
                chess_board_remove_piece(board, to);
                chess_board_add_piece(board, to, rank | color);
            }
        }
    }
    history_push(uci_move);
    return true;
}

void _render_piece(Rectangle dest, int piece);

/// @brief Remove all user annotations (highlights and arrows).
static void clear_annotations(void)
{
    highlighted_tiles_bb = 0ULL;
    arrow_count = 0;
}
/// @brief Toggle an arrow between two tiles. Removes it if it already exists,
///        otherwise appends it (up to MAX_ARROWS).
static void toggle_arrow(int from, int to)
{
    for (int i = 0; i < arrow_count; i++) {
        if (arrows[i].from == from && arrows[i].to == to) {
            arrows[i] = arrows[arrow_count - 1];
            arrow_count--;
            return;
        }
    }
    if (arrow_count < MAX_ARROWS) {
        arrows[arrow_count].from = (uint8_t)from;
        arrows[arrow_count].to = (uint8_t)to;
        arrow_count++;
    }
}

/// @brief performed when game screen is transitioned to
void game_screen_init(void) 
{
    screenEnded = 0;
    frameCount = 0L;
    hovered_tile_idx = -1;
    selected_tile_idx = -1;
    highlighted_tiles_bb = 0ULL;
    arrow_count = 0;
    right_drag_origin = -1;
    s_move_history[0] = '\0';
    s_white_bot = NULL;
    s_black_bot = NULL;

    /* Spawn bot players if configured.
     * Both bots receive this engine's own executable as their engine_path
     * so they can spin up a UCI subprocess for move generation. */
    char engine_exe[512];
    get_exe_path(engine_exe, (int)sizeof(engine_exe));

    if (g_game_config.white.type == PLAYER_BOT &&
        g_game_config.white.bot_exe[0]) {
        s_white_bot = bot_player_create(g_game_config.white.bot_exe, engine_exe);
        if (s_white_bot) bot_player_new_game(s_white_bot);
    }
    if (g_game_config.black.type == PLAYER_BOT &&
        g_game_config.black.bot_exe[0]) {
        s_black_bot = bot_player_create(g_game_config.black.bot_exe, engine_exe);
        if (s_black_bot) bot_player_new_game(s_black_bot);
    }
    // setup default board theme
    currentBoardTheme=MemAlloc(sizeof(struct BoardTheme));
    currentBoardTheme->backgroundColor = RAYWHITE;
    currentBoardTheme->boardColorDark = (Color){110, 150, 70, 255};
    currentBoardTheme->boardColorLight = (Color){240, 240, 200, 255};
    currentBoardTheme->selectedColor = YELLOW;
    currentBoardTheme->highlightColor = RED;
    currentBoardTheme->arrowColor = ORANGE;
    currentBoardTheme->pieceFontName = "philosopher_bold";
    currentBoardTheme->legalMoveColor = (Color){0, 255, 0, 100}; // semi-transparent green

    whiteSideDown = 1; //Start with white side down

    board_height = 8 * GetScreenHeight() / 10; // board will be 80% of the height of the screen
    board_width = board_height; // Board is a square
    board_position.x = (float)(GetScreenWidth() / 2 - board_width / 2); // Centred Board Position
    board_position.y = (float)(GetScreenHeight() / 2 - board_height / 2);

    boardTextFont = LoadFontEx("resources/Philosopher-Bold.ttf", 20, 0, 250); // for the annotations on the board

    /* Load Spritesheet */
    char* spritesheet_path;
    Image low_res_spritesheet = LoadImage(LOW_RES_SPRITESHEET_PATH);
    // Image high_res_spritesheet = LoadImage(HIGH_RES_SPRITESHEET_PATH);
    int screen_width = GetScreenWidth();
    int screen_height = GetScreenHeight();
    bool use_high_res = low_res_spritesheet.height / 2 < screen_height / 8; 
    if (use_high_res)
        spritesheet_path = HIGH_RES_SPRITESHEET_PATH;
    else
        spritesheet_path = LOW_RES_SPRITESHEET_PATH;
    spritesheet = LoadTexture(spritesheet_path);
    sprite_size = spritesheet.height / 2;
    printf("INFO: Spritesheet %s Loaded Sucessfully\n", spritesheet_path);

    current_board = MemAlloc(sizeof(ChessBoard));
    if (current_board == NULL) throw_error(__LINE__, __FILE__, "Not enough **WHAM**");
    chess_board_init(current_board);
    generate_board_from_FEN(current_board, CE_FEN_STARTING_POSITION);

    printf("INFO: Loaded Game Screen Successfully\n");
}
/// @brief performed once per frame
void game_screen_update(void) 
{
    /* Determine whose turn it is and whether that side is a bot. */
    bool white_turn = current_board->white_turn;
    BotPlayer *active_bot = white_turn ? s_white_bot : s_black_bot;
    bool human_turn = (active_bot == NULL);

    /* --- Bot turn handling --- */
    if (!human_turn) {
        if (!bot_player_is_thinking(active_bot)) {
            /* Kick off move request (fires once per turn) */
            bot_player_request_move(active_bot,
                                     s_move_history,
                                     NULL,  /* start_fen: always startpos for now */
                                     200);  /* movetime_ms */
        }
        /* Poll for the response without blocking */
        char uci_move[BP_MOVE_MAX];
        if (bot_player_poll_move(active_bot, uci_move)) {
            selected_tile_idx = -1;
            dragging_piece    = 0;
            clear_annotations();
            apply_uci_move(current_board, uci_move);
        }
        frameCount++;
        return; /* skip human input processing on bot turns */
    }

    int mouse_x = GetMouseX();
    int mouse_y = GetMouseY();
    int tile_size = board_width / 8;
    /* Hover tile */
    if (mouse_x > board_position.x && mouse_x < board_position.x + board_width 
    && mouse_y > board_position.y && mouse_y < board_position.y + board_height) {
        hovered_tile_idx = (mouse_x - (int)board_position.x) / tile_size +
                (7 - ((mouse_y - (int)board_position.y) / tile_size)) * 8;
        if (hovered_tile_idx > 63 || hovered_tile_idx < 0)
            ERROR("Hovered tile out of bounds. Got %d", hovered_tile_idx);
    }
    else {
        hovered_tile_idx = -1;
    }
    /* Select tile */
    if (hovered_tile_idx > -1 && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        /* Selecting a piece clears all annotations (highlights and arrows). */
        clear_annotations();
        if ((current_board->piece_list[hovered_tile_idx] & 0b111) != EMPTY) {
            selected_tile_idx = hovered_tile_idx;
            legal_moves_bb = chess_board_legal_moves_BB(current_board, selected_tile_idx);
        }
        else {
            selected_tile_idx = -1;
        }
    }
    /* Highlights (right click) and arrows (right click + drag) */
    if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
        right_drag_origin = hovered_tile_idx;
    }
    if (IsMouseButtonReleased(MOUSE_BUTTON_RIGHT)) {
        if (right_drag_origin > -1 && hovered_tile_idx > -1) {
            if (hovered_tile_idx == right_drag_origin) {
                /* Same tile: toggle a highlight on it. */
                highlighted_tiles_bb ^= (1ULL << hovered_tile_idx);
            }
            else {
                /* Dragged to another tile: toggle an arrow between them. */
                toggle_arrow(right_drag_origin, hovered_tile_idx);
            }
        }
        right_drag_origin = -1;
    }
    /* Drag piece */
    if (selected_tile_idx > -1 && hovered_tile_idx > -1) {
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            dragging_piece = 1;
        }
        if (dragging_piece == 1 
            && IsMouseButtonReleased(MOUSE_BUTTON_LEFT) 
            && hovered_tile_idx != selected_tile_idx) {
            /* Build UCI move string before applying (board state will change) */
            char uci_move[8];
            squares_to_uci(current_board, selected_tile_idx, hovered_tile_idx,
                           uci_move);
            if (chess_board_move(current_board, selected_tile_idx, hovered_tile_idx)) {
                history_push(uci_move);
            }
            dragging_piece = 0;
            selected_tile_idx = -1;
        }
    }
    else if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
        dragging_piece = 0;
    }
    frameCount++;
}
/// @brief render the colored tiles onto the screen
void render_tiles(void) 
{
    int screen_width = GetScreenWidth();
    int screen_height = GetScreenHeight();
    int tileWidth = board_width / 8;
    int i, j;
    for (i = 0; i < 8; i++) { // 1 to 8
        for (j = 0; j < 8; j++) { // A to G
            DrawRectangle(
                (int)(board_position.x) + i * tileWidth, 
                (int)(board_position.y) + j * tileWidth,
                tileWidth, tileWidth,
                (((i + j) & 1) == whiteSideDown) ? currentBoardTheme->boardColorLight 
                                        : currentBoardTheme->boardColorDark
            );
        }
    }
    /* Render white box highlight over hovered tile */
    const float line_width = 1.0f;
    if (hovered_tile_idx > -1) {
        Rectangle r = {board_position.x + (tileWidth * (hovered_tile_idx % 8)), 
            board_position.y + (tileWidth * (7 - (hovered_tile_idx / 8))), 
            (float) tileWidth, (float) tileWidth};
        DrawRectangleLinesEx(r, line_width, WHITE);
    }
    /* Render highlight over selected tile */
    if (selected_tile_idx > -1) {
        Rectangle r = {board_position.x + (tileWidth * (selected_tile_idx % 8)), 
            board_position.y + (tileWidth * (7 - (selected_tile_idx / 8))), 
            (float) tileWidth, (float) tileWidth};
            DrawRectangleRec(r, YELLOW);
    }
}

/// @brief render the legal moves onto the screen. Defer until after pieces are
/// rendered so that pieces are on top of the circles.
void render_legal_moves(void) {
    /* Render small circle over legal moves */
    int screen_width = GetScreenWidth();
    int screen_height = GetScreenHeight();
    int tileWidth = board_width / 8;
    if (selected_tile_idx > -1 && legal_moves_bb) {
        int x, y, i;
        for (i = 0; i < 64; i++) {
            if (legal_moves_bb & (1ULL << i)) {
                x = (int)board_position.x + (tileWidth * (i % 8)) + tileWidth / 2;
                y = (int)board_position.y + (tileWidth * (7 - (i / 8))) + tileWidth / 2;
                Vector2 center = {(float)x, (float)y};
                /* If the move happens to capture a piece, then mark that piece with a larger circle */
                if ((current_board->black | current_board->white) & (1ULL << i)) {
                    DrawRing(center, (float)tileWidth / 4, (float)tileWidth / 3, 0, 360, 30, currentBoardTheme->legalMoveColor);
                }
                else {
                    DrawCircle(x, y, (float)tileWidth / 6, currentBoardTheme->legalMoveColor);
                }
            }
        }
    }
    /* Render larger circle over legal moves the capture a piece */
    if (selected_tile_idx > -1 && legal_moves_bb) {
        int x, y, i;
        for (i = 0; i < 64; i++) {
            if (legal_moves_bb & (1ULL << i) & (current_board->white | current_board->black)) {
                if ((current_board->piece_list[i] & 0b11000) != (current_board->piece_list[selected_tile_idx] & 0b11000)) {
                    x = board_position.x + (tileWidth * (i % 8)) + tileWidth / 2;
                    y = board_position.y + (tileWidth * (7 - (i / 8))) + tileWidth / 2;
                    DrawCircle(x, y, tileWidth / 4, currentBoardTheme->legalMoveColor);
                }
            }
        }
    }

}
/// @brief screen-space center of a tile (0..63)
static Vector2 tile_center(int tile)
{
    int tileWidth = board_width / 8;
    Vector2 c = {
        board_position.x + (float)(tileWidth * (tile % 8)) + (float)tileWidth / 2.0f,
        board_position.y + (float)(tileWidth * (7 - (tile / 8))) + (float)tileWidth / 2.0f
    };
    return c;
}
/// @brief render the tiles highlighted by right clicking
void render_highlights(void)
{
    int tileWidth = board_width / 8;
    Color highlight = currentBoardTheme->highlightColor;
    highlight.a = 160; // semi-transparent overlay
    int i;
    for (i = 0; i < 64; i++) {
        if (highlighted_tiles_bb & (1ULL << i)) {
            Rectangle r = {board_position.x + (float)(tileWidth * (i % 8)),
                board_position.y + (float)(tileWidth * (7 - (i / 8))),
                (float)tileWidth, (float)tileWidth};
            DrawRectangleRec(r, highlight);
        }
    }
}
/// @brief draw a single arrow from one tile to another
static void draw_arrow(int from, int to, Color color)
{
    int tileWidth = board_width / 8;
    Vector2 start = tile_center(from);
    Vector2 end = tile_center(to);
    float thickness = (float)tileWidth / 6.0f;
    float head = (float)tileWidth / 3.0f;

    float dx = end.x - start.x;
    float dy = end.y - start.y;
    float len = sqrtf(dx * dx + dy * dy);
    if (len < 1.0f) return;
    float ux = dx / len;
    float uy = dy / len;

    // Shorten the shaft so it meets the base of the arrow head.
    Vector2 shaft_end = {end.x - ux * head, end.y - uy * head};
    DrawLineEx(start, shaft_end, thickness, color);

    // Arrow head as two lines forming a "V" pointing at the destination tile.
    Vector2 left = {shaft_end.x - uy * head * 0.5f, shaft_end.y + ux * head * 0.5f};
    Vector2 right = {shaft_end.x + uy * head * 0.5f, shaft_end.y - ux * head * 0.5f};
    DrawLineEx(end, left, thickness, color);
    DrawLineEx(end, right, thickness, color);
}
/// @brief render all arrows, including the one currently being dragged
void render_arrows(void)
{
    Color color = currentBoardTheme->arrowColor;
    color.a = 200;
    int i;
    for (i = 0; i < arrow_count; i++) {
        draw_arrow(arrows[i].from, arrows[i].to, color);
    }
    /* Live preview of the arrow being dragged with the right mouse button. */
    if (right_drag_origin > -1 && hovered_tile_idx > -1
        && hovered_tile_idx != right_drag_origin) {
        draw_arrow(right_drag_origin, hovered_tile_idx, color);
    }
}
/// @brief render the side labels (1-8, A-H) onto the screen
void render_labels(void) 
{
    int screen_width = GetScreenWidth();
    int screen_height = GetScreenHeight();
    int tileWidth = board_width / 8;
    const char* ranks = "12345678";
    char* files = "abcdefgh";
    char* symb = MemAlloc(sizeof(char) * 2);
    if (symb == (void *)0)
    {
        printf("ERROR: Could not allocate memory");
        exit(1);
    }
    Vector2 symbolPosition;
    // ranks
    int i;
    for (i = 0; i < 8; i++) {
        *symb = (char)*(ranks + (7 - i));
        *(symb + 1) = '\0'; // terminate string
        Vector2 symbDims = MeasureTextEx(boardTextFont, symb, 20, 5); 
        symbolPosition.x = board_position.x - tileWidth/2 - symbDims.x/2;
        symbolPosition.y = board_position.y + tileWidth/2 - symbDims.y/2 + (tileWidth * i);
        symbolPosition.y += tileWidth/8; // Small Correction Needed For Some Reason ...
        if ((symbolPosition.x <= 0 || symbolPosition.x >= screen_width)
            || (symbolPosition.y <= 0 || symbolPosition.y >= screen_height))
            ERROR("Symbol Position is out of bounds POS:(%f, %f)\n", symbolPosition.x, symbolPosition.y);
            
        DrawTextEx(boardTextFont, symb, symbolPosition, 20, 5, BLACK);
    }
    // files
    for (i = 0; i < 8; i++) {
        *symb = (char)*(files + i);
        *(symb + 1) = '\0'; // terminate string
        Vector2 symbDims = MeasureTextEx(boardTextFont, symb, 20, 5); 
        symbolPosition.x = board_position.x - tileWidth/2 - symbDims.x/2 + (tileWidth * i) + tileWidth;
        symbolPosition.y = board_position.y + board_height + tileWidth/2 - symbDims.y/2 ;
        symbolPosition.y += tileWidth/8; // Small Correction Needed For Some Reason ...
        // printf("DEBUG: Board Position: (%f, %f)", symbolPosition.x, symbolPosition.y);
        if ((symbolPosition.x <= 0 || symbolPosition.x >= screen_width)
            || (symbolPosition.y <= 0 || symbolPosition.y >= screen_height))
            ERROR("Symbol Position is out of bounds POS:(%f, %f)\n", symbolPosition.x, symbolPosition.y);
        DrawTextEx(boardTextFont, symb, symbolPosition, 20, 5, BLACK);
    }

    MemFree(symb);
}
void _render_piece(Rectangle dest, int piece) 
{
    const Vector2 origin = {1, 1}; // a small offest due to a graphical error; should normally be (0, 0)
    const float rotation = 0;
    const Color tint = WHITE;
    int color = ((piece & 0b11000) == TILE_WHITE) ? 0 : 1; // 0: White, 1: Black
    /*
    01, 02, 03, 04, 05, 06
    07, 08, 09, 10, 11, 12
    */
    int idx1 = 0;
    switch ((piece & 0b111)) {
        case PAWN: idx1 = 6; break;
        case KNIGHT: idx1 = 4; break;
        case BISHOP: idx1 = 3; break;
        case ROOK: idx1 = 5; break;
        case QUEEN: idx1 = 2; break;
        case KING: idx1 = 1; break;
    };
    if (idx1) {
        Rectangle source = {
            (idx1 - 1) * sprite_size,
            (color) * sprite_size,
            (float) sprite_size, 
            (float) sprite_size
        };
        DrawTexturePro(spritesheet, source, dest, origin, rotation, tint);
    }
}
/// @brief render the game pieces onto the screen
void render_pieces() 
{
    int screen_width = GetScreenWidth();
    int screen_height = GetScreenHeight();
    int tileWidth = board_width / 8;
    Rectangle source = {0, 0, 200, 200};
    ChessBoard *board = current_board;
    int i, j;
    for (i = 0; i < 8; i ++) { // rank
        for (j = 0; j < 8; j ++) { // file
            Rectangle dest = {
                board_position.x + (float)(j * tileWidth), 
                board_position.y + (float)((7 - i) * tileWidth), 
                (float)tileWidth, (float)tileWidth
            };
            int piece = board->piece_list[ID_FROM_RANK_FILE(i, j)];
            _render_piece(dest, piece);
        }
    }
}
void render_dragged_piece() 
{
    int tileWidth = board_width / 8;
    if (dragging_piece) {
        int mouse_x = GetMouseX();
        int mouse_y = GetMouseY();
        int piece_id = current_board->piece_list[selected_tile_idx];
        Rectangle dest = {
            (float)mouse_x - (tileWidth / 2),
            (float)mouse_y - (tileWidth / 2),
            (float)tileWidth, (float)tileWidth
        };
        _render_piece(dest, piece_id);
    }
}
void render_turn_indicator() 
{
    int tileWidth = board_width / 8;
    bool white_turn = current_board->white_turn;

    /* Choose label: show bot name when a bot is active on this side */
    char turn_text[64];
    BotPlayer *active_bot = white_turn ? s_white_bot : s_black_bot;
    if (active_bot) {
        const char *name = white_turn
            ? g_game_config.white.bot_name
            : g_game_config.black.bot_name;
        if (bot_player_is_thinking(active_bot))
            snprintf(turn_text, sizeof(turn_text), "%s is thinking...", name);
        else
            snprintf(turn_text, sizeof(turn_text), "%s to move", name);
    } else {
        snprintf(turn_text, sizeof(turn_text), "%s to move",
                 white_turn ? "White" : "Black");
    }

    Vector2 text_dims = MeasureTextEx(boardTextFont, turn_text, 20, 5);
    Vector2 position = {
        board_position.x + (board_width - text_dims.x) * 0.5f,
        board_position.y - text_dims.y - (float)tileWidth / 4.0f
    };
    DrawTextEx(boardTextFont, turn_text, position, 20, 5, BLACK);
}
void render_check_indicator() 
{
    // TODO draw a red border around the king in check
    ChessBoard *board = current_board;
    int color_to_move = board->white_turn ? TILE_WHITE : TILE_BLACK;
    if (chess_board_is_in_check(board, color_to_move)) {
        // find king position
        int king_tile = -1;
        int i;
        for (i = 0; i < 64; i++) {
            int piece = board->piece_list[i];
            if ((piece & 0b111) == KING && (piece & 0b11000) == (color_to_move)) {
                king_tile = i;
                break;
            }
        }
        if (king_tile == -1) {
            ERROR("Could not find king on board for color %s", 
                (color_to_move == TILE_WHITE) ? "White" : "Black");
            return;
        }
        int tileWidth = board_width / 8;
        Rectangle r = {board_position.x + (tileWidth * (king_tile % 8)), 
            board_position.y + (tileWidth * (7 - (king_tile / 8))), 
            (float) tileWidth, (float) tileWidth};
            DrawRectangleLinesEx(r, 5.0f, RED);
    }
}   
/// @brief Render logic for game screen. Performed once per frame
void game_screen_draw(void)
{
    int screen_width = GetScreenWidth();
    int screen_height = GetScreenHeight();
    DrawRectangle(0, 0, screen_width, screen_height, RAYWHITE); // background
    render_tiles();
    render_highlights();
    render_labels();
    render_turn_indicator();
    render_check_indicator();
    render_pieces(current_board);
    render_legal_moves();
    render_dragged_piece();
    render_arrows();
}
void game_screen_unload(void) 
{
    bot_player_destroy(s_white_bot);
    bot_player_destroy(s_black_bot);
    s_white_bot = NULL;
    s_black_bot = NULL;
    UnloadFont(boardTextFont);
    MemFree(currentBoardTheme);
    chess_board_destroy(current_board);
    MemFree(current_board);
}
int game_screen_ended(void)
{
    return screenEnded;
}
