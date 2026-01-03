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


// New function to render game with full button state
void render_game(RenderContext *ctx, Point *map, Point *focused_cell, 
                     Faction *current_faction, bool button_pressed) {
    BeginDrawing();
    ClearBackground(RAYWHITE);
    
    render_debug_info(ctx, map);
    render_map(ctx, map, focused_cell);
    render_map_border(ctx);
    render_cell_info(ctx, focused_cell);
    render_ui(ctx, current_faction->name, current_faction, button_pressed);
    render_actions(ctx, (focused_cell != NULL) ? focused_cell->occupant : NULL);
    
    EndDrawing();
}


static void render_map_border(RenderContext *ctx) {
    int border_thickness = 3;
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
    

    for (int i = 0; i < border_thickness; i++) {
    DrawRectangleLines(map_x - border_thickness + i, map_y - border_thickness + i, 
                        map_width + (border_thickness - i) * 2, map_height + (border_thickness - i) * 2, 
                        BLACK);
    }
}

// Private helper function (not in header, only used internally)
static void render_map(RenderContext *ctx, Point *map, Point *focused_cell) {
    int total_cells = ctx->grid_cells_x * ctx->grid_cells_y;
    
    for (int i = 0; i < total_cells; i++) {
        Point *cell = &map[i];
        int x_pos = ctx->grid_offset_x + cell->x * ctx->grid_cell_size;
        int y_pos = ctx->grid_offset_y + cell->y * ctx->grid_cell_size;
        
        // Draw terrain
        // Possibly add default error texture if terrain sprite is NULL
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
        DrawText(TextFormat("NAME: %s\nFAC: %s\nLVL: %d\nHP: %d/%d\nPATK: %d\nPDEF: %d\nMATK: %d\nMDEF: %d\nLCK: %d\nRNG: %d",
                           occupant->name, occupant->owner->name, 
                           occupant->level, occupant->curr_health, occupant->max_health,
                           occupant->phys_attack, occupant->phys_defense,
                           occupant->magic_attack, occupant->magic_defense,
                           occupant->luck, occupant->attack_range),
                info_x + 5, info_y + 5, 26, BLACK);
    } else {
        DrawText(TextFormat("OCC: None"),
                info_x + 5, info_y + 5, 26, BLACK);
    }

    info_y += 300;
    DrawLine(info_x, info_y, info_x + ctx->grid_cell_size * 8, info_y, BLACK);
    DrawText(TextFormat("TRN: %s\nPASS: %s",
                       focused_cell->terrain.name,
                       focused_cell->terrain.passable ? "Yes" : "No"),
                info_x + 5, info_y + 5, 26, BLACK);

    info_y += 100;
    DrawLine(info_x, info_y, info_x + ctx->grid_cell_size * 8, info_y, BLACK);

    if (focused_cell->structure != NULL) {
        Structure *structure = focused_cell->structure;
        DrawText(TextFormat("STRCT: %s\nPASS: %s\nLOOT: %s",
                           structure->name, 
                           structure->passable ? "Yes" : "No",
                           structure->lootable ? "Yes" : "No"),
                info_x + 5, info_y + 5, 26, BLACK);
    } else {
        DrawText(TextFormat("STRCT: None"),
                info_x + 5, info_y + 5, 26, BLACK);
    }
    
    // Selection highlight is now rendered as a transparent background tint
}

static void render_ui(RenderContext *ctx, const char *faction_name,
                        Faction *current_faction, bool button_pressed) {

    int info_x = ctx->grid_cells_x * ctx->grid_cell_size + ctx->grid_offset_x + 20;
    int info_y = ctx->grid_offset_y;

    int border_thickness = 3;
    for (int i = 0; i < border_thickness; i++) {
        DrawRectangleLines(info_x - i, info_y - i, 
                          ctx->grid_cell_size * 8 + i * 2, ctx->grid_cell_size * 17 + i * 2, 
                          BLACK);
    }

    int ui_x = ctx->grid_cells_x * ctx->grid_cell_size + ctx->grid_offset_x + 20;
    int ui_y = ctx->grid_offset_y + (ctx->grid_cells_y - 2) * ctx->grid_cell_size;
    
    int button_width = ctx->grid_cell_size * 8;
    int button_height = ctx->grid_cell_size * 5;
    border_thickness = 3;
    
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
    const int text_size = 22;
    const char *button_text = TextFormat("End Turn: %s", faction_name);
    int text_width = MeasureText(button_text, text_size);
    int text_x = ui_x + (button_width - text_width) / 2;
    int text_y = ui_y + (button_height - text_size) / 2;
    
    DrawText(button_text, text_x, text_y, text_size, text_color);
}

void render_actions(RenderContext *ctx, Actor *actor) {
    // Render action panels to the right of the map (aligned with UI panel)
    int actions_x = ctx->grid_offset_x;
    int actions_y = ctx->grid_offset_y + ctx->grid_cells_y * ctx->grid_cell_size + ctx->grid_cell_size;

    int box_w = ctx->grid_cell_size * 2;
    int box_h = ctx->grid_cell_size * 2;

    // Draw background lightly so boxes are visible even if overlapping window edges
    Color bg = (Color){200, 200, 200, 40};
    for (int i = 0; i < 5; i++) {
        DrawRectangle(actions_x + i * box_w, actions_y, box_w, box_h, bg);
        DrawRectangleLines(actions_x + i * box_w, actions_y, box_w, box_h, BLACK);
    }

    for (int i = 0; i < 5; i++) {
        DrawRectangle(actions_x + 6* box_w + i * box_w, actions_y, box_w, box_h, bg);
        DrawRectangleLines(actions_x + 6* box_w + i * box_w, actions_y, box_w, box_h, BLACK);
    }

}