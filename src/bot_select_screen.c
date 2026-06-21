/*
 * bot_select_screen.c -- GUI for selecting White / Black player types.
 *
 * Layout (800 x 600):
 *
 *   +----------------------------------------------------------------+
 *   ¦              Choose Players                                     ¦
 *   ¦                                                                 ¦
 *   ¦   +--------- White ---------+  +--------- Black ---------+    ¦
 *   ¦   ¦  ? Human                ¦  ¦  ? Human                ¦    ¦
 *   ¦   ¦  ? bogo-bot             ¦  ¦  ? bogo-bot             ¦    ¦
 *   ¦   ¦  ? ...                  ¦  ¦  ? ...                  ¦    ¦
 *   ¦   +-------------------------+  +-------------------------+    ¦
 *   ¦                                                                 ¦
 *   ¦                  [ Start Game ]                                 ¦
 *   +----------------------------------------------------------------+
 */

#include "bot_select_screen.h"
#include "game_config.h"
#include "bot_registry.h"
#include "screens.h"
#include "raylib.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/* ------------------------------------------------------------------ */
/* Local state                                                          */
/* ------------------------------------------------------------------ */

static int            s_ended;
static Font           s_font;
static BotRegistry   *s_registry;

/* Index into the option list (0 = Human, 1..n = bot) for each side */
static int s_white_sel;
static int s_black_sel;

/* Total number of options per column: 1 (Human) + registry count */
#define MAX_OPTIONS (1 + BOT_REGISTRY_MAX)
static int s_option_count; /* 1 + s_registry->count */

/* ------------------------------------------------------------------ */
/* Layout constants (derived from 800x600 window)                      */
/* ------------------------------------------------------------------ */
#define TITLE_FONT_SIZE   28
#define LABEL_FONT_SIZE   20
#define OPTION_FONT_SIZE  18
#define OPTION_HEIGHT     32
#define RADIO_RADIUS       7
#define COL_W            260
#define COL_H_MAX        320
#define COL_Y             90
#define WHITE_COL_X       80
#define BLACK_COL_X      460
#define BTN_W            160
#define BTN_H             40

/* ------------------------------------------------------------------ */
/* Helpers                                                              */
/* ------------------------------------------------------------------ */

static bool point_in_rect(Vector2 pt, Rectangle r)
{
    return pt.x >= r.x && pt.x <= r.x + r.width
        && pt.y >= r.y && pt.y <= r.y + r.height;
}

/* Draw one radio-button row.  Returns true if it was clicked. */
static bool draw_option_row(Vector2 mouse, bool mouse_released,
                             float x, float y, float row_w,
                             bool selected, const char *label,
                             Color text_color)
{
    float cx = x + RADIO_RADIUS + 4;
    float cy = y + OPTION_HEIGHT / 2.0f;
    Rectangle row_rect = { x, y, row_w, OPTION_HEIGHT };
    bool hovered = point_in_rect(mouse, row_rect);

    /* Radio circle */
    Color ring_color = hovered ? DARKGRAY : GRAY;
    DrawCircleV((Vector2){cx, cy}, (float)RADIO_RADIUS, ring_color);
    if (selected)
        DrawCircleV((Vector2){cx, cy}, (float)(RADIO_RADIUS - 3), DARKBLUE);
    else
        DrawCircleV((Vector2){cx, cy}, (float)(RADIO_RADIUS - 2), RAYWHITE);

    /* Label */
    Vector2 text_pos = { cx + RADIO_RADIUS + 6, cy - OPTION_FONT_SIZE / 2.0f };
    DrawTextEx(s_font, label, text_pos, OPTION_FONT_SIZE, 2, text_color);

    return hovered && mouse_released;
}

/* Draw a column (side panel) with its options.
 * Returns the new selection index if a click occurred, or -1. */
static int draw_column(Vector2 mouse, bool mouse_released,
                        float col_x, const char *title,
                        int cur_sel)
{
    int new_sel = -1;
    float content_h = (float)(s_option_count * OPTION_HEIGHT);
    float col_h = content_h + 50.0f; /* extra padding */

    /* Panel background */
    Rectangle panel = { col_x - 10, (float)COL_Y - 30,
                         (float)COL_W + 20, col_h + 10 };
    DrawRectangleRec(panel, (Color){220, 220, 220, 255});
    DrawRectangleLinesEx(panel, 1.5f, DARKGRAY);

    /* Column title */
    Vector2 title_dims = MeasureTextEx(s_font, title, LABEL_FONT_SIZE, 2);
    Vector2 title_pos = {
        col_x + ((float)COL_W - title_dims.x) * 0.5f,
        (float)COL_Y - 20.0f
    };
    DrawTextEx(s_font, title, title_pos, LABEL_FONT_SIZE, 2, BLACK);

    /* Options */
    float row_y = (float)COL_Y + 10.0f;
    for (int i = 0; i < s_option_count; i++) {
        const char *label = (i == 0) ? "Human"
                                      : s_registry->bots[i - 1].name;
        bool clicked = draw_option_row(mouse, mouse_released,
                                        col_x, row_y, (float)COL_W,
                                        (cur_sel == i), label, BLACK);
        if (clicked) new_sel = i;
        row_y += OPTION_HEIGHT;
    }

    return new_sel;
}

/* ------------------------------------------------------------------ */
/* Screen lifecycle                                                     */
/* ------------------------------------------------------------------ */

void bot_select_screen_init(void)
{
    s_ended = 0;
    s_white_sel = 0; /* default: Human */
    s_black_sel = 0;

    s_font = LoadFontEx("resources/Philosopher-Bold.ttf", 28, 0, 250);

    s_registry = bot_registry_create();
    s_option_count = 1 + (s_registry ? s_registry->count : 0);

    game_config_reset();
}

void bot_select_screen_update(void)
{
    /* All interaction is handled in draw for simplicity (immediate-mode style) */
    (void)0;
}

void bot_select_screen_draw(void)
{
    int sw = GetScreenWidth();
    int sh = GetScreenHeight();
    DrawRectangle(0, 0, sw, sh, RAYWHITE);

    Vector2 mouse = GetMousePosition();
    bool released = IsMouseButtonReleased(MOUSE_BUTTON_LEFT);

    /* Title */
    const char *title = "Choose Players";
    Vector2 title_dims = MeasureTextEx(s_font, title, TITLE_FONT_SIZE, 2);
    Vector2 title_pos = {
        (sw - title_dims.x) * 0.5f,
        20.0f
    };
    DrawTextEx(s_font, title, title_pos, TITLE_FONT_SIZE, 2, BLACK);

    /* White column */
    int w_click = draw_column(mouse, released,
                               (float)WHITE_COL_X, "White", s_white_sel);
    if (w_click >= 0) s_white_sel = w_click;

    /* Black column */
    int b_click = draw_column(mouse, released,
                               (float)BLACK_COL_X, "Black", s_black_sel);
    if (b_click >= 0) s_black_sel = b_click;

    /* Start Game button */
    float btn_x = (sw - BTN_W) * 0.5f;
    float btn_y = (float)sh - BTN_H - 30.0f;
    Rectangle btn_rect = { btn_x, btn_y, (float)BTN_W, (float)BTN_H };
    bool btn_hovered = point_in_rect(mouse, btn_rect);
    Color btn_bg    = btn_hovered ? DARKBLUE  : (Color){50, 100, 180, 255};
    Color btn_text  = RAYWHITE;
    DrawRectangleRec(btn_rect, btn_bg);
    DrawRectangleLinesEx(btn_rect, 1.5f, DARKGRAY);
    const char *btn_label = "Start Game";
    Vector2 btn_dims = MeasureTextEx(s_font, btn_label, LABEL_FONT_SIZE, 2);
    Vector2 btn_text_pos = {
        btn_x + ((float)BTN_W - btn_dims.x) * 0.5f,
        btn_y + ((float)BTN_H - btn_dims.y) * 0.5f
    };
    DrawTextEx(s_font, btn_label, btn_text_pos, LABEL_FONT_SIZE, 2, btn_text);

    if (btn_hovered && released) {
        /* Commit the selection into g_game_config */
        game_config_reset();

        if (s_white_sel == 0) {
            g_game_config.white.type = PLAYER_HUMAN;
        } else {
            int idx = s_white_sel - 1;
            g_game_config.white.type = PLAYER_BOT;
            strncpy(g_game_config.white.bot_exe,
                    s_registry->bots[idx].exe_path,
                    BOT_PATH_MAX - 1);
            strncpy(g_game_config.white.bot_name,
                    s_registry->bots[idx].name,
                    BOT_NAME_MAX - 1);
        }

        if (s_black_sel == 0) {
            g_game_config.black.type = PLAYER_HUMAN;
        } else {
            int idx = s_black_sel - 1;
            g_game_config.black.type = PLAYER_BOT;
            strncpy(g_game_config.black.bot_exe,
                    s_registry->bots[idx].exe_path,
                    BOT_PATH_MAX - 1);
            strncpy(g_game_config.black.bot_name,
                    s_registry->bots[idx].name,
                    BOT_NAME_MAX - 1);
        }

        s_ended = (int)GAME;
    }
}

void bot_select_screen_unload(void)
{
    UnloadFont(s_font);
    bot_registry_destroy(s_registry);
    s_registry = NULL;
}

int bot_select_screen_ended(void)
{
    return s_ended;
}
