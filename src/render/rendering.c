#include "render/rendering.h"
#include "types.h"
#include <stddef.h>

static void render_map(RenderContext *ctx, Point *map, Point *focused_cell);
static bool cell_is_focused(Point *cell, Point *focused_cell);

static bool cell_is_focused(Point *cell, Point *focused_cell) {
    if (focused_cell == NULL) return false;
    return (cell->x == focused_cell->x && cell->y == focused_cell->y);
}
void render_debug_info(RenderContext *ctx, Point *map);
void render_cell_info(RenderContext *ctx, Point *focused_cell);
static void render_ui(RenderContext *ctx, const char *faction_name, Faction *current_faction, bool button_pressed);
static void render_map_border(RenderContext *ctx);

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
    render_map(ctx, map, focused_cell);
    render_map_border(ctx);
    render_cell_info(ctx, focused_cell);
    render_ui(ctx, current_faction, NULL, false);
    
    EndDrawing();
}

// New function to render game with full button state
void render_game_full(RenderContext *ctx, Point *map, Point *focused_cell, 
                     Faction *current_faction, bool button_pressed) {
    BeginDrawing();
    ClearBackground(RAYWHITE);
    
    render_debug_info(ctx, map);
    render_map(ctx, map, focused_cell);
    render_map_border(ctx);
    render_cell_info(ctx, focused_cell);
    render_ui(ctx, current_faction->name, current_faction, button_pressed);
    
    EndDrawing();
}


static void render_map_border(RenderContext *ctx) {
    int border_thickness = 4;
    int map_x = ctx->grid_offset_x;
    int map_y = ctx->grid_offset_y;
    int map_width = ctx->grid_cells_x * ctx->grid_cell_size;
    int map_height = ctx->grid_cells_y * ctx->grid_cell_size;
    
    // Draw thick black border around the map
    DrawRectangleLines(map_x - border_thickness, 
                      map_y - border_thickness,
                      map_width + border_thickness * 2,
                      map_height + border_thickness * 2,
                      BLACK);
    
    // Draw inner border for extra thickness
    DrawRectangleLines(map_x - border_thickness + 1, 
                      map_y - border_thickness + 1,
                      map_width + (border_thickness - 1) * 2,
                      map_height + (border_thickness - 1) * 2,
                      BLACK);
    
    DrawRectangleLines(map_x - border_thickness + 2, 
                      map_y - border_thickness + 2,
                      map_width + (border_thickness - 2) * 2,
                      map_height + (border_thickness - 2) * 2,
                      BLACK);
    
    DrawRectangleLines(map_x - border_thickness + 3, 
                      map_y - border_thickness + 3,
                      map_width + (border_thickness - 3) * 2,
                      map_height + (border_thickness - 3) * 2,
                      BLACK);
}

// Private helper function (not in header, only used internally)
static void render_map(RenderContext *ctx, Point *map, Point *focused_cell) {
    int total_cells = ctx->grid_cells_x * ctx->grid_cells_y;
    
    for (int i = 0; i < total_cells; i++) {
        Point *cell = &map[i];
        int x_pos = ctx->grid_offset_x + cell->x * ctx->grid_cell_size;
        int y_pos = ctx->grid_offset_y + cell->y * ctx->grid_cell_size;
        
        // Draw terrain
        DrawRectangle(x_pos, y_pos, ctx->grid_cell_size, ctx->grid_cell_size, 
                     cell->terrain.color);
        DrawTexture(cell->terrain.sprite, x_pos, y_pos, WHITE);
        
        // Draw structure (if present) and occupant after tint so they remain on top

        // Apply move/attack tints over the terrain (transparent fill)
        if (!cell_is_focused(cell, focused_cell)) {
            if (cell->in_range) {
                Color move_tint = (Color){0, 0, 255, 120};
                DrawRectangle(x_pos, y_pos, ctx->grid_cell_size, ctx->grid_cell_size, move_tint);
            }
            if (cell->in_attack_range) {
                Color attack_tint = (Color){255, 0, 0, 120};
                DrawRectangle(x_pos, y_pos, ctx->grid_cell_size, ctx->grid_cell_size, attack_tint);
            }
        }

        // Draw selection tint (transparent yellow) if this is the focused cell
        if (focused_cell != NULL && cell->x == focused_cell->x && cell->y == focused_cell->y) {
            Color select_tint = (Color){255, 255, 0, 120};
            DrawRectangle(x_pos, y_pos, ctx->grid_cell_size, ctx->grid_cell_size, select_tint);
        }

        // Draw structure (if present)
        if (cell->structure != NULL) {
            DrawTexture(cell->structure->sprite, x_pos, y_pos, WHITE);
        }
        
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
        // range/attack tints are drawn above terrain but below units/structures
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
    
    // Selection highlight is now rendered as a transparent background tint
}

static void render_ui(RenderContext *ctx, const char *faction_name,
                        Faction *current_faction, bool button_pressed) {
    int ui_x = ctx->grid_cells_x * ctx->grid_cell_size + ctx->grid_offset_x + 20;
    int ui_y = ctx->grid_offset_y + (ctx->grid_cells_y - 3) * ctx->grid_cell_size;
    
    int button_width = 200;
    int button_height = 40;
    int border_thickness = 3;
    
    // Determine button colors
    Color button_color;
    Color border_color;
    Color text_color;
    
    if (current_faction != NULL) {
        if (button_pressed) {
            // When pressed, use secondary color for background and primary for border
            button_color = current_faction->sec_color;
            border_color = current_faction->prim_color;
            text_color = border_color;
        } else {
            // Normal state: primary color for background, darker version for border
            button_color = current_faction->prim_color;
            border_color = current_faction->sec_color;
            text_color = border_color;
        }
    } else {
        // Fallback colors if faction is NULL
        button_color = button_pressed ? GRAY : DARKGRAY;
        border_color = BLACK;
        text_color = border_color;
    }
    
    // Draw button background
    DrawRectangle(ui_x, ui_y, button_width, button_height, button_color);
    
    // Draw button border (thick)
    for (int i = 0; i < border_thickness; i++) {
        DrawRectangleLines(ui_x - i, ui_y - i, 
                          button_width + i * 2, button_height + i * 2, 
                          border_color);
    }
    
    // Draw button text
    const char *button_text = TextFormat("End Turn: %s", faction_name);
    int text_width = MeasureText(button_text, 18);
    int text_x = ui_x + (button_width - text_width) / 2;
    int text_y = ui_y + (button_height - 18) / 2;
    
    DrawText(button_text, text_x, text_y, 18, text_color);
}
/*
void render_combat_forecast(Point *attacker_cell, Point *defender_cell) {
    if (attacker_cell->occupant == NULL || defender_cell->occupant == NULL) {
        return;
    }
    
    CombatForecast forecast = combat_forecast(attacker_cell->occupant, 
                                              defender_cell->occupant);
    
    // Display forecast
    DrawText(TextFormat("Attacker: %d damage -> %d HP",
                       forecast.attacker_damage,
                       forecast.defender_health_after),
             10, 100, 20, RED);
    
    DrawText(TextFormat("Defender: %d damage -> %d HP",
                       forecast.defender_damage,
                       forecast.attacker_health_after),
             10, 130, 20, BLUE);
    
    DrawText(TextFormat("Hit: %d%% Crit: %d%%",
                       forecast.hit_chance,
                       forecast.crit_chance),
             10, 160, 20, BLACK);
}
*/
