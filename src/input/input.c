#include "input/input.h"
#include "raylib.h"
#include "core/utils.h"
#include "main.h"
#include "game/map.h"
#include "game/actor.h"
#include "render/rendering.h"
#include <stdio.h>
#include <stdlib.h>

// Forward declarations for internal helper functions
static Point *mouse_to_cell(GridConfig *grid_config, Point *map);
static void handle_cell_selection(GridConfig *grid_config, Point *map, 
                                   Point *selected_cell, Point **focused_cell);

void input_init(InputState *state) {
    state->selected_cell = NULL;
    state->focused_cell = NULL;
    state->left_click = false;
    state->right_click = false;
    state->end_turn_requested = false;

    // Initialize end-turn button with a sensible default rect. The exact
    // position/size will be updated every frame in input_update based on
    // the current GridConfig so we don't need layout data here.
    button_init(&state->end_turn_button, 0, 0, 0, 0);
}

void input_update(InputState *state, GridConfig *grid_config, Point *map) {
    // Get cell under mouse
    state->selected_cell = mouse_to_cell(grid_config, map);
    
    // Check for mouse button presses
    state->left_click = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    state->right_click = IsMouseButtonPressed(MOUSE_BUTTON_RIGHT);
    
    // Update end-turn button bounds based on grid layout, then update its
    // pressed state and derive end_turn_requested from click events.
    {
        // Compute the same rectangle that the UI uses for the end-turn button
        int button_x = grid_config->max_grid_cells_x * grid_config->grid_cell_size +
                       grid_config->grid_offset_x + 20;
        int button_y = grid_config->grid_offset_y +
                       (grid_config->max_grid_cells_y - 2) * grid_config->grid_cell_size;
        int button_width = grid_config->grid_cell_size * 8;
        int button_height = grid_config->grid_cell_size * 5;

        button_set_rect(&state->end_turn_button, button_x, button_y, button_width, button_height);

        // Update pressed state (true while holding the mouse down over the button)
        button_update(&state->end_turn_button, IsMouseButtonDown(MOUSE_BUTTON_LEFT));

        // Register an end turn request when the left mouse button was *pressed* this frame
        if (state->left_click && button_is_mouse_over(&state->end_turn_button)) {
            state->end_turn_requested = true;
        } else {
            state->end_turn_requested = false;
        }
    }
    
    // Could add keyboard shortcuts here, e.g.:
    // if (IsKeyPressed(KEY_SPACE)) state->end_turn_requested = true;
}

void input_handle_selection(InputState *state, GridConfig *grid_config, Point *map) {
    if (!state->left_click) return;
    if (state->selected_cell == NULL) return;
    
    // Don't process selection if clicking on UI elements
    if (state->end_turn_requested) return;
    
    handle_cell_selection(grid_config, map, state->selected_cell, &state->focused_cell);
}

void input_handle_movement(InputState *state, GridConfig *grid_config, Point *map) {
    if (!state->right_click) return;
    if (state->selected_cell == NULL) return;
    if (state->focused_cell == NULL) return;
    
    Point *focused = state->focused_cell;
    Point *selected = state->selected_cell;
    
    // Check if right-clicking on an enemy (attack)
    if (focused->occupant != NULL &&
        selected->occupant != NULL &&
        actor_is_enemy(focused->occupant, selected->occupant) &&
        focused->occupant->owner->has_turn &&
        focused->occupant->can_act) {
        
        // Check if enemy is in attack range
        if (combat_can_attack(grid_config, map, focused, selected)) {
            printf("\n=== COMBAT ===\n");
            CombatResult result = combat_execute_at_cells(grid_config, map, focused, selected);
            printf("=== END COMBAT ===\n\n");
            
            // Clear focus after combat
            state->focused_cell = NULL;
            map_clear_range_flags(map, grid_config);
            return;
        }
    }
    
    // Check if movement is valid (existing code)
    if (focused->occupant != NULL &&
        selected->in_range &&
        selected->occupant == NULL &&
        focused->occupant->can_move &&
        focused->occupant->owner->has_turn) {
        
        // Perform movement
        selected->occupant = focused->occupant;
        focused->occupant = NULL;
        selected->occupant->can_move = false;
        
        // Clear focus after moving
        state->focused_cell = NULL;
    }
    
    // Always flush flags on right click
    map_clear_range_flags(map, grid_config);
}

bool input_is_mouse_over_end_turn_button(RenderContext *ctx) {
    // Create a temporary button using the same layout rules and query it.
    Button tmp;
    button_init(&tmp,
                ctx->grid_cells_x * ctx->grid_cell_size + ctx->grid_offset_x + 20,
                ctx->grid_offset_y + (ctx->grid_cells_y - 2) * ctx->grid_cell_size,
                ctx->grid_cell_size * 8,
                ctx->grid_cell_size * 5);

    return button_is_mouse_over(&tmp);
}

// ============================================================================
// Internal helper functions
// ============================================================================

bool input_is_mouse_over_map(GridConfig *grid_config) {
    int mouse_x = GetMouseX();
    int mouse_y = GetMouseY();

    int left = grid_config->grid_offset_x;
    int top = grid_config->grid_offset_y;
    int right = left + grid_config->grid_cell_size * grid_config->max_grid_cells_x;
    int bottom = top + grid_config->grid_cell_size * grid_config->max_grid_cells_y;

    return (mouse_x >= left && mouse_x < right &&
            mouse_y >= top && mouse_y < bottom);
}

static Point *mouse_to_cell(GridConfig *grid_config, Point *map) {
    // If the mouse is outside the map area, don't compute a cell index.
    // This prevents accidental selection/focus changes when interacting
    // with UI elements outside the grid (including the end turn button
    // or any future interactive UI elements).
    if (!input_is_mouse_over_map(grid_config)) return NULL;

    int x = (GetMouseX() - grid_config->grid_offset_x) /
            grid_config->grid_cell_size;
    int y = (GetMouseY() - grid_config->grid_offset_y) /
            grid_config->grid_cell_size;

    if (x < 0 || x >= grid_config->max_grid_cells_x ||
        y < 0 || y >= grid_config->max_grid_cells_y) {
        return NULL;
    }

    return map + grid_config->max_grid_cells_x * y + x;
}

static void handle_cell_selection(GridConfig *grid_config, Point *map, 
                                   Point *selected_cell, Point **focused_cell) {
    // Flush previous range indicators
    cell_flag_flush(map, grid_config);
    
    // Update focused cell
    *focused_cell = selected_cell;
    
    // If there's an occupant, show their ranges
    if (selected_cell->occupant != NULL) {
        Actor *occupant = selected_cell->occupant;
        
        // Only show ranges if it's their turn
        if (occupant->owner->has_turn) {
            // Show movement range if unit can still move
            if (occupant->can_move) {
                map_calculate_movement_range(grid_config, map,
                                   selected_cell, occupant->movement, true);
            }
            
            // Show attack range if unit can still act
            if (occupant->can_act) {
                map_calculate_attack_range(grid_config, map,
                                selected_cell, occupant->attack_range, true);
            }
        }
    }
}
