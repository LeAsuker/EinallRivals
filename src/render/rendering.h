#ifndef RENDERING_H_
#define RENDERING_H_

#include "raylib.h"
#include "types.h"
#include <stddef.h>
#include "ui/button.h"

typedef struct InputState InputState;
typedef struct Modal Modal;

// Public rendering functions
typedef struct RenderContext {
    int grid_offset_x;
    int grid_offset_y;
    int grid_cell_size;
    int grid_cells_x;
    int grid_cells_y;
} RenderContext;

struct InputState;

void render_init(RenderContext *ctx, GridConfig * grid);
void render_debug_info(RenderContext *ctx, Point *map);
void render_cell_info(RenderContext *ctx, Point *focused_cell);
void render_combat_forecast(Point *attacker_cell, Point *defender_cell);
void render_game(RenderContext *ctx, Point *map, Point *focused_cell, 
                     Faction *current_faction, InputState *input_state, Button *end_turn_button, Button action_buttons[], int action_count, Modal *modal);
void render_actions(RenderContext *ctx, Character *character, InputState *input_state, Button action_buttons[], int action_count);
#
// Drawing helpers: draw thicker lines/rect outlines by repeating DrawLine/DrawRectangleLines
static inline void DrawThickLine(int startPosX, int startPosY, int endPosX, int endPosY, Color color, int thickness) {
    for (int i = 0; i < thickness; i++) {
        DrawLine(startPosX, startPosY + i, endPosX, endPosY + i, color);
    }
}

static inline void DrawThickRectangleLines(int posX, int posY, int width, int height, Color color, int thickness) {
    for (int i = 0; i < thickness; i++) {
        DrawRectangleLines(posX - i, posY - i, width + i * 2, height + i * 2, color);
    }
}
#
#endif
