#include "input.h"
#include "raylib.h"
#include "utils.h"
#include "main.h"
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
}

void input_update(InputState *state, GridConfig *grid_config, Point *map) {
    // Get cell under mouse
    state->selected_cell = mouse_to_cell(grid_config, map);
    
    // Check for mouse button presses
    state->left_click = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    state->right_click = IsMouseButtonPressed(MOUSE_BUTTON_RIGHT);
    
    // Check for end turn button (you can add keyboard shortcut here too)
    if (state->left_click && input_is_mouse_over_end_turn_button(grid_config)) {
        state->end_turn_requested = true;
    } else {
        state->end_turn_requested = false;
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
    
    // Check if movement is valid
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
    cell_flag_flush(map, grid_config);
}

bool input_is_mouse_over_end_turn_button(GridConfig *grid_config) {
    // End turn button position (matching your original code)
    int button_x = grid_config->max_grid_cells_x * grid_config->grid_cell_size + 
                   grid_config->grid_offset_x + 20;
    int button_y = grid_config->grid_offset_y + 
                   (grid_config->max_grid_cells_y - 3) * grid_config->grid_cell_size;
    
    // Button dimensions (approximate - adjust based on your UI)
    int button_width = 200;
    int button_height = 30;
    
    int mouse_x = GetMouseX();
    int mouse_y = GetMouseY();
    
    return (mouse_x >= button_x && mouse_x <= button_x + button_width &&
            mouse_y >= button_y && mouse_y <= button_y + button_height);
}

// ============================================================================
// Internal helper functions
// ============================================================================

static Point *mouse_to_cell(GridConfig *grid_config, Point *map) {
    int x = (safe_mouse_x(grid_config) - grid_config->grid_offset_x) / 
            grid_config->grid_cell_size;
    int y = (safe_mouse_y(grid_config) - grid_config->grid_offset_y) / 
            grid_config->grid_cell_size;
    
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
                range_calc(grid_config, map, selected_cell, 
                          occupant->movement, true);
            }
            
            // Show attack range if unit can still act
            if (occupant->can_act) {
                attack_range_calc(grid_config, map, selected_cell, 
                                 occupant->range, true);
            }
        }
    }
}