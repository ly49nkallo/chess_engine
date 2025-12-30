#include "game_screen.h"

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

void _render_piece(Rectangle dest, int piece);

/// @brief performed when game screen is transitioned to
void game_screen_init(void) 
{
    screenEnded = 0;
    frameCount = 0L;
    hovered_tile_idx = -1;
    selected_tile_idx = -1;
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
        if ((current_board->piece_list[hovered_tile_idx] & 0b111) != EMPTY) {
            selected_tile_idx = hovered_tile_idx;
            legal_moves_bb = chess_board_pseudo_legal_moves_BB(current_board, selected_tile_idx);
            printf("Allowed Moves for piece [%c] at position %i:\n", 
                piece_id_to_char(current_board->piece_list[selected_tile_idx] & 0b111),
                selected_tile_idx);
            print_bitboard(legal_moves_bb);
        }
        else {
            selected_tile_idx = -1;
        }
    }
    /* Arrows */
    if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
        
    }
    /* Drag piece */
    if (selected_tile_idx > -1 && hovered_tile_idx > -1) {
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            dragging_piece = 1;
        }
        if (dragging_piece == 1 
            && IsMouseButtonReleased(MOUSE_BUTTON_LEFT) 
            && hovered_tile_idx != selected_tile_idx) {
            chess_board_move(current_board, selected_tile_idx, hovered_tile_idx);
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
    const Vector2 origin = {0, 0};
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
/// @brief Render logic for game screen. Performed once per frame
void game_screen_draw(void)
{
    int screen_width = GetScreenWidth();
    int screen_height = GetScreenHeight();
    DrawRectangle(0, 0, screen_width, screen_height, RAYWHITE); // background
    render_tiles();
    render_labels();
    render_pieces(current_board);
    render_legal_moves();
    render_dragged_piece();

}
void game_screen_unload(void) 
{
    UnloadFont(boardTextFont);
    MemFree(currentBoardTheme);
    chess_board_destroy(current_board);
    MemFree(current_board);
}
int game_screen_ended(void)
{
    return screenEnded;
}
