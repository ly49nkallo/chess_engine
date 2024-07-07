#include "game_screen.h"

/* LOCAL VARIABLES */
static long frameCount;
static int screenEnded;
static struct BoardTheme* currentBoardTheme;
static ChessBoard* currentBoard;
static int board_height, board_width;
static Vector2 board_position;
static int whiteSideDown;
static Font boardTextFont;
static Texture2D spritesheet;
static int sprite_size;
static int hovered_tile_idx;
static int selected_tile_idx;


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

    whiteSideDown = 1; //Start with white side down

    board_height = 8 * GetScreenHeight() / 10; // board will be 80% of the height of the screen
    board_width = board_height; // Board is a square
    board_position.x = (float)(GetScreenWidth() / 2 - board_width / 2); // Centred Board Position
    board_position.y = (float)(GetScreenHeight() / 2 - board_height / 2);

    boardTextFont = LoadFontEx("resources/Philosopher-Bold.ttf", 20, 0, 250); // for the annotations on the board

    /* Load Spritesheet */
    char* spritesheet_path;
    Image low_res_spritesheet = LoadImage(LOW_RES_SPRITESHEET_PATH);
    Image high_res_spritesheet = LoadImage(HIGH_RES_SPRITESHEET_PATH);
    int screen_width = GetScreenWidth();
    int screen_height = GetScreenHeight();
    if (low_res_spritesheet.height / 2 < screen_height / 8)
        spritesheet_path = HIGH_RES_SPRITESHEET_PATH;
    else
        spritesheet_path = LOW_RES_SPRITESHEET_PATH;
    spritesheet = LoadTexture(spritesheet_path);
    sprite_size = spritesheet.height / 2;
    printf("INFO: Spritesheet %s Loaded Sucessfully\n", spritesheet_path);

    currentBoard = MemAlloc(sizeof(ChessBoard));
    if (currentBoard == NULL) throw_error(__LINE__, __FILE__, "Not enough RAM");
    chess_board_init(currentBoard);
    generate_board_from_FEN(currentBoard, CE_FEN_STARTING_POSITION);

    printf("INFO: Loaded Game Screen Successfully\n");
}
/// @brief performed once per frame
void game_screen_update(void) 
{
    /* Hover tile */
    int mouse_x = GetMouseX();
    int mouse_y = GetMouseY();
    int tile_size = board_width / 8;
    if (mouse_x > board_position.x && mouse_x < board_position.x + board_width 
    && mouse_y > board_position.y && mouse_y < board_position.y + board_height) {
        hovered_tile_idx = (mouse_x - (int)board_position.x) / tile_size +
                (7 - ((mouse_y - (int)board_position.y) / tile_size)) * 8;
        if (hovered_tile_idx > 63 || hovered_tile_idx < 0)
            throw_error(__LINE__, __FILE__, "Hovered tile out of bounds. Got %d", hovered_tile_idx);
    }
    else {
        hovered_tile_idx = -1;
    }
    /* Select tile */
    if (hovered_tile_idx > -1 && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        if ((currentBoard->piece_list[hovered_tile_idx] & 0b111) != EMPTY) {
            selected_tile_idx = hovered_tile_idx;        
        }
        else {
            selected_tile_idx = -1;
        }
    }
    /* Arrows */
    if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {

    }
    frameCount++;
}
/// @brief render the colored tiles onto the screen
void render_tiles(void) 
{
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();
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
/// @brief render the side labels (1-8, A-H) onto the screen
void render_labels(void) 
{
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();
    int tileWidth = board_width / 8;
    char* ranks = "12345678";
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
        if ((symbolPosition.x <= 0 || symbolPosition.x >= screenWidth)
            || (symbolPosition.y <= 0 || symbolPosition.y >= screenHeight))
        {
            printf("ERROR: Symbol Position is out of bounds POS:(%f, %f)\n", symbolPosition.x, symbolPosition.y);
            exit(1);
        }
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
        if ((symbolPosition.x <= 0 || symbolPosition.x >= screenWidth)
            || (symbolPosition.y <= 0 || symbolPosition.y >= screenHeight))
        {
            throw_error(__LINE__, __FILE__, "ERROR: Symbol Position is out of bounds POS:(%f, %f)\n", symbolPosition.x, symbolPosition.y);
        }
        DrawTextEx(boardTextFont, symb, symbolPosition, 20, 5, BLACK);
    }

    MemFree(symb);
}
/// @brief render the game pieces onto the screen
/// @param board 
void render_pieces(ChessBoard* board) 
{
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();
    int tileWidth = board_width / 8;
    Rectangle source = {0, 0, 200, 200};
    const Vector2 origin = {0, 0};
    const float rotation = 0;
    const Color tint = WHITE;
    int i, j;
    for (i = 0; i < 8; i ++) { // rank
        for (j = 0; j < 8; j ++) { // file
            Rectangle dest = {
                board_position.x + (float)(j * tileWidth), 
                board_position.y + (float)(i * tileWidth), 
                (float)tileWidth, (float)tileWidth
            };
            int piece = board->piece_list[ID_FROM_RANK_FILE(i, j)];
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
                int idx2 = idx1 + (6 * color);
                Rectangle source = {
                    (idx1 - 1) * sprite_size, 
                    (1 - color) * sprite_size, 
                    sprite_size, 
                    sprite_size
                };
                DrawTexturePro(spritesheet, source, dest, origin, rotation, tint);
            }
        }
    }
}
/// @brief Render logic for game screen. Performed once per frame
void game_screen_draw(void)
{
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();
    DrawRectangle(0, 0, screenWidth, screenHeight, RAYWHITE); // background
    render_tiles();
    render_labels();
    render_pieces(currentBoard);

}
void game_screen_unload(void) 
{
    UnloadFont(boardTextFont);
    MemFree(currentBoardTheme);
    chess_board_destroy(currentBoard);
    MemFree(currentBoard);
}
int game_screen_ended(void)
{
    return screenEnded;
}
