#ifndef RENDERING_H_
#define RENDERING_H_

#include "raylib.h"
#include "types.h"  // We'll create this for shared types
#include <stddef.h>

// we use gridconfig for rendering

// Public rendering functions
typedef struct {
    int grid_offset_x;
    int grid_offset_y;
    int grid_cell_size;
    int grid_cells_x;
    int grid_cells_y;
} RenderContext;

void render_init(RenderContext *ctx, GridConfig * grid);
void render_game(RenderContext *ctx, Point *map, Point *focused_cell, const char *current_faction);
void render_debug_info(RenderContext *ctx, Point *map);
void render_cell_info(RenderContext *ctx, Point *focused_cell);
void render_combat_forecast(Point *attacker_cell, Point *defender_cell);
#endif