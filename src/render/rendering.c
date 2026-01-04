#include "render/rendering.h"
#include "input/input.h"
#include "types.h"
#include <stddef.h>
#include "ui/button.h"

static void render_map(RenderContext *ctx, Point *map, Point *focused_cell);
static bool cell_is_focused(Point *cell, Point *focused_cell);

static bool cell_is_focused(Point *cell, Point *focused_cell) {
    if (focused_cell == NULL) return false;
    return (cell->x == focused_cell->x && cell->y == focused_cell->y);
}
void render_debug_info(RenderContext *ctx, Point *map);
void render_cell_info(RenderContext *ctx, Point *focused_cell);
static void render_ui(RenderContext *ctx, const char *faction_name, Faction *current_faction, Button *end_turn_button);
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
                     Faction *current_faction, InputState *input_state, Button *end_turn_button, Button action_buttons[], int action_count) {
    BeginDrawing();
    ClearBackground(RAYWHITE);
    
    render_debug_info(ctx, map);
    render_map(ctx, map, focused_cell);
    render_map_border(ctx);
    render_cell_info(ctx, focused_cell);
    render_ui(ctx, current_faction->name, current_faction, end_turn_button);
    render_actions(ctx, (focused_cell != NULL) ? focused_cell->occupant : NULL, input_state, action_buttons, action_count);
    
    EndDrawing();
}


static void render_map_border(RenderContext *ctx) {
    int border_thickness = 3;
    int map_x = ctx->grid_offset_x;
    int map_y = ctx->grid_offset_y;
    int map_width = ctx->grid_cells_x * ctx->grid_cell_size;
    int map_height = ctx->grid_cells_y * ctx->grid_cell_size;
    
    // Draw thick black border around the map
    DrawThickRectangleLines(map_x - border_thickness, map_y - border_thickness,
                            map_width + border_thickness * 2, map_height + border_thickness * 2,
                            BLACK, border_thickness);
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
                        Faction *current_faction, Button *end_turn_button) {

    int info_x = ctx->grid_cells_x * ctx->grid_cell_size + ctx->grid_offset_x + 20;
    int info_y = ctx->grid_offset_y;

    int border_thickness = 3;
    DrawThickRectangleLines(info_x, info_y, ctx->grid_cell_size * 8, ctx->grid_cell_size * 17, BLACK, border_thickness);

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
        if (end_turn_button != NULL && end_turn_button->pressed) {
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
        button_color = (end_turn_button != NULL && end_turn_button->pressed) ? GRAY : DARKGRAY;
        border_color = BLACK;
        text_color = border_color;
    }
    
    // Use the persistent end_turn_button rect, but update visuals based on faction
    Button tmp_btn = *end_turn_button;
    button_set_colors(&tmp_btn, button_color, border_color);
    button_set_border_thickness(&tmp_btn, border_thickness);

    // Set the label so button_draw will render it centered
    const int text_size = 22;
    const char *button_text = TextFormat("End Turn: %s", faction_name);
    button_set_label(&tmp_btn, button_text, text_size, text_color);
    
    button_draw(&tmp_btn);
}

void render_actions(RenderContext *ctx, Actor *actor, InputState *input_state, Button action_buttons[], int action_count) {
    // Render action panels to the bottom of the map (aligned with UI panel)
    
    int box_w = ctx->grid_cell_size * 2;
    int box_h = ctx->grid_cell_size * 2;

    // Draw the persistent action buttons and icons where applicable
    for (int i = 0; i < action_count; i++) {
        Button *b = &action_buttons[i];
        button_draw(b);

        int bx = b->x;
        int by = b->y;

        if (actor != NULL && i < actor->skill_count) {
            Skill *s = &actor->skills[i];
            if (s->icon.id) {
                Rectangle src = (Rectangle){0, 0, (float)s->icon.width, (float)s->icon.height};
                Rectangle dst = (Rectangle){(float)bx, (float)by, (float)box_w, (float)box_h};
                Vector2 origin = (Vector2){0.0f, 0.0f};
                DrawTexturePro(s->icon, src, dst, origin, 0.0f, WHITE);
            } else {
                DrawRectangle(bx + 4, by + 4, box_w - 8, box_h - 8, (Color){150,150,150,200});
            }
            // Highlight selected action
            if (input_state != NULL && input_state->selected_action == i) {
                DrawThickRectangleLines(bx, by, box_w, box_h, (Color){255, 255, 100, 255}, 5);
            }
            if (input_state != NULL && !actor->can_act) {
                // Dim the icon if actor cannot act
                DrawRectangle(bx, by, box_w, box_h, (Color){100, 100, 100, 180});
            }
        }
    }

}