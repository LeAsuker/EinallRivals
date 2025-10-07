#include "rendering.h"
#include "types.h"
#include <stddef.h>

static void render_map(RenderContext *ctx, Point *map);
void render_debug_info(RenderContext *ctx, Point *map);
void render_cell_info(RenderContext *ctx, Point *focused_cell);
static void render_ui(RenderContext *ctx, const char *faction_name);

void render_init(RenderContext *ctx, GridConfig* grid) {
    ctx->grid_offset_x = grid->grid_offset_x;
    ctx->grid_offset_y = grid->grid_offset_y;
    ctx->grid_cell_size = grid->grid_cell_size;
    ctx->grid_cells_x = grid->max_grid_cells_x;
    ctx->grid_cells_y = grid->max_grid_cells_y;
}

void render_game(RenderContext *ctx, Point *map, Point *focused_cell, const char *current_faction) {
    BeginDrawing();
    ClearBackground(RAYWHITE);
    
    render_debug_info(ctx, map);
    render_map(ctx, map);
    render_cell_info(ctx, focused_cell);
    render_ui(ctx, current_faction);
    
    EndDrawing();
}

// Private helper function (not in header, only used internally)
static void render_map(RenderContext *ctx, Point *map) {
    int total_cells = ctx->grid_cells_x * ctx->grid_cells_y;
    
    for (int i = 0; i < total_cells; i++) {
        Point *cell = &map[i];
        int x_pos = ctx->grid_offset_x + cell->x * ctx->grid_cell_size;
        int y_pos = ctx->grid_offset_y + cell->y * ctx->grid_cell_size;
        
        // Draw terrain
        DrawRectangle(x_pos, y_pos, ctx->grid_cell_size, ctx->grid_cell_size, 
                     cell->terrain.color);
        DrawTexture(cell->terrain.sprite, x_pos, y_pos, WHITE);
        
        // Draw occupant
        if (cell->occupant != NULL) {
            DrawTexture(cell->occupant->sprite, x_pos, y_pos, WHITE);
        }
        
        // Draw grid lines
        DrawRectangleLines(x_pos, y_pos, ctx->grid_cell_size, ctx->grid_cell_size, GRAY);
        
        // Draw highlights
        if (cell->occupant != NULL) {
            DrawRectangleLines(x_pos, y_pos, ctx->grid_cell_size, ctx->grid_cell_size,
                             cell->occupant->owner->prim_color);
        }
        if (cell->in_range) {
            DrawRectangleLines(x_pos, y_pos, ctx->grid_cell_size, ctx->grid_cell_size, BLUE);
        }
        if (cell->in_attack_range) {
            DrawRectangleLines(x_pos, y_pos, ctx->grid_cell_size, ctx->grid_cell_size, RED);
        }
    }
}

void render_debug_info(RenderContext *ctx, Point *map) {
    int mouse_x = GetMouseX();
    int mouse_y = GetMouseY();
    // Use your safe_mouse functions here
    DrawText(TextFormat("MOUSE: %d %d", mouse_x, mouse_y), 40, 20, 20, DARKGRAY);
}

void render_cell_info(RenderContext *ctx, Point *focused_cell) {
    if (focused_cell == NULL) return;
    
    int info_x = ctx->grid_cells_x * ctx->grid_cell_size + ctx->grid_offset_x + 20;
    int info_y = ctx->grid_offset_y;
    
    if (focused_cell->occupant != NULL) {
        Actor *occupant = focused_cell->occupant;
        DrawText(TextFormat("NAME: %s\nFAC: %s\nLVL: %d\nHP: %d/%d",
                           occupant->name, occupant->owner->name, 
                           occupant->level, occupant->curr_health, occupant->max_health),
                info_x, info_y, 20, BLACK);
    } else {
        DrawText(TextFormat("TRN: %s", focused_cell->terrain.name),
                info_x, info_y, 20, BLACK);
    }
    
    // Draw selection highlight
    DrawRectangleLines(
        focused_cell->x * ctx->grid_cell_size + ctx->grid_offset_x,
        focused_cell->y * ctx->grid_cell_size + ctx->grid_offset_y,
        ctx->grid_cell_size, ctx->grid_cell_size, YELLOW);
}

static void render_ui(RenderContext *ctx, const char *faction_name) {
    int ui_x = ctx->grid_cells_x * ctx->grid_cell_size + ctx->grid_offset_x + 20;
    int ui_y = ctx->grid_offset_y + (ctx->grid_cells_y - 3) * ctx->grid_cell_size;
    
    DrawText(TextFormat("End Turn: %s", faction_name), ui_x, ui_y, 20, BLACK);
}