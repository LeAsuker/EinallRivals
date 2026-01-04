#include "input/input.h"
#include "raylib.h"
#include "core/utils.h"
#include "main.h"
#include "game/map.h"
#include "game/actor.h"
#include "render/rendering.h"
#include "game/actions.h"
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
    
    // Initialize action buttons
    for (int i = 0; i < ACTION_BUTTON_COUNT; i++) {
        button_init(&state->action_buttons[i], 0, 0, 0, 0);
        state->action_clicked[i] = false;
    }
    // Track initial screen size for resize detection
    state->last_screen_width = GetScreenWidth();
    state->last_screen_height = GetScreenHeight();
    state->selected_action = -1;
}

void input_update(InputState *state, GridConfig *grid_config, Point *map) {
    // Get cell under mouse
    state->selected_cell = mouse_to_cell(grid_config, map);
    
    // Check for mouse button presses
    state->left_click = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    state->right_click = IsMouseButtonPressed(MOUSE_BUTTON_RIGHT);

    // If the window size changed, recompute button layout once
    int cur_w = GetScreenWidth();
    int cur_h = GetScreenHeight();
    if (cur_w != state->last_screen_width || cur_h != state->last_screen_height) {
        input_layout_buttons(state, grid_config);
        state->last_screen_width = cur_w;
        state->last_screen_height = cur_h;
    }
    
    // Update end-turn button bounds based on grid layout, then update its
    // Update pressed state and end-turn request based on stored rects
    button_update(&state->end_turn_button, IsMouseButtonDown(MOUSE_BUTTON_LEFT));
    if (state->left_click && button_is_mouse_over(&state->end_turn_button)) {
        state->end_turn_requested = true;
    } else {
        state->end_turn_requested = false;
    }
    
    // Update action buttons pressed/clicked state based on previously laid-out rects
    for (int i = 0; i < ACTION_BUTTON_COUNT; i++) {
        button_update(&state->action_buttons[i], IsMouseButtonDown(MOUSE_BUTTON_LEFT));
        if (state->left_click && button_is_mouse_over(&state->action_buttons[i])) {
            state->action_clicked[i] = true;
        } else {
            state->action_clicked[i] = false;
        }
    }
}

void input_layout_buttons(InputState *state, GridConfig *grid_config) {
    // Position end-turn button
    int button_x = grid_config->max_grid_cells_x * grid_config->grid_cell_size +
                   grid_config->grid_offset_x + 20;
    int button_y = grid_config->grid_offset_y +
                   (grid_config->max_grid_cells_y - 2) * grid_config->grid_cell_size;
    int button_width = grid_config->grid_cell_size * 8;
    int button_height = grid_config->grid_cell_size * 5;
    button_set_rect(&state->end_turn_button, button_x, button_y, button_width, button_height);

    // Layout action buttons (10 slots)
    int actions_x = grid_config->grid_offset_x;
    int actions_y = grid_config->grid_offset_y + grid_config->max_grid_cells_y * grid_config->grid_cell_size + grid_config->grid_cell_size;
    int box_w = grid_config->grid_cell_size * 2;
    int box_h = grid_config->grid_cell_size * 2;

    for (int i = 0; i < ACTION_BUTTON_COUNT; i++) {
        int bx;
        if (i < 5) bx = actions_x + i * box_w;
        else bx = actions_x + 6 * box_w + (i - 5) * box_w;
        int by = actions_y;
        button_set_rect(&state->action_buttons[i], bx, by, box_w, box_h);
        // set default visuals
        state->action_buttons[i].bg_color = (Color){200,200,200,40};
        state->action_buttons[i].border_color = GRAY;
        state->action_buttons[i].border_thickness = 1;
    }
}

int input_get_clicked_action(InputState *state) {
    for (int i = 0; i < ACTION_BUTTON_COUNT; i++) {
        if (state->action_clicked[i]) {
            // Consume click
            state->action_clicked[i] = false;
            return i;
        }
    }
    return -1;
}

void input_handle_action_click(InputState *state, GridConfig *grid_config, Point *map) {
    int clicked_action = input_get_clicked_action(state);
    if (clicked_action < 0) return;

    // Toggle selection: clicking an already-selected action disables it.
    if (state->selected_action == clicked_action) {
        state->selected_action = -1;
    } else {
        state->selected_action = clicked_action;
    }
}

void input_handle_left_click(InputState *state, GridConfig *grid_config, Point *map) {
    // Must have a cell under mouse to do map interactions
    Point *selected = state->selected_cell;
    if (selected == NULL) return;

    Point *focused = state->focused_cell;

    // If clicked the focused cell -> clear focus
    if (focused != NULL && selected->x == focused->x && selected->y == focused->y) {
        state->focused_cell = NULL;
        cell_flag_flush(map, grid_config);
        return;
    }

    // If there is a focused unit, try movement first
    if (focused != NULL && focused->occupant != NULL) {
        Actor *unit = focused->occupant;

        // Movement: if clicked tile is in movement range and is empty
        if (selected->in_range && unit->can_move && unit->owner->has_turn
        && map_can_unit_enter_cell(selected, unit)) {
            printf("Moving unit %s from (%d,%d) to (%d,%d)\n", 
                   unit->name, focused->x, focused->y, selected->x, selected->y);
            selected->occupant = unit;
            focused->occupant = NULL;
            selected->occupant->can_move = false;
            state->focused_cell = selected; // focus moved unit's new cell
            map_clear_range_flags(map, grid_config);
            if (selected->occupant != NULL && selected->occupant->can_act && selected->occupant->owner->has_turn) {
                map_calculate_attack_range(grid_config, map, selected, selected->occupant->attack_range, true);
            }
            return;
        }

        // Action: if an action is selected and clicked tile is in range
        if (state->selected_action >= 0 && state->selected_action < unit->skill_count) {
            Skill *s = &unit->skills[state->selected_action];
            printf("Attempting to use skill %s from (%d,%d) to (%d,%d)\n",
                   s->name, focused->x, focused->y, selected->x, selected->y);
            // check target exists and is enemy and within skill range
            if (s->id == 103) { // Loot skill special case: must target empty cell with structure
                printf("Using loot skill logic\n");
                if (selected->occupant == NULL && selected->structure != NULL) {
                    printf("Executing loot skill on structure %s at (%d,%d)\n",
                           selected->structure->name, selected->x, selected->y);
                    execute_loot_at_cells(grid_config, map, focused, selected, s);
                    state->selected_action = -1; // consume action
                    state->focused_cell = NULL;
                    map_clear_range_flags(map, grid_config);
                    if (selected->occupant != NULL && selected->occupant->can_move && selected->occupant->owner->has_turn) {
                        map_calculate_movement_range(grid_config, map, focused, selected->occupant->movement, true);
                    }
                    return;
                }
            }
            else if (selected->occupant != NULL && actor_is_enemy(unit, selected->occupant)) {
                int dist = abs(focused->x - selected->x) + abs(focused->y - selected->y);
                if (dist <= s->range && unit->can_act) {
                    execute_skill_at_cells(grid_config, map, focused, selected, s);
                    state->selected_action = -1; // consume action
                    state->focused_cell = NULL;
                    map_clear_range_flags(map, grid_config);
                    if (selected->occupant != NULL && selected->occupant->can_move && selected->occupant->owner->has_turn) {
                        map_calculate_movement_range(grid_config, map, focused, unit->movement, true);
                    }
                    return;
                }
            }
        }
    }

    // Default: focus the clicked cell
    handle_cell_selection(grid_config, map, selected, &state->focused_cell);
    state->selected_action = -1; // clear selected action on new focus
}

    // Could add keyboard shortcuts here, e.g.:
    // if (IsKeyPressed(KEY_SPACE)) state->end_turn_requested = true;

void input_handle_selection(InputState *state, GridConfig *grid_config, Point *map) {
    if (!state->left_click) return;
    if (state->selected_cell == NULL) return;
    
    // Don't process selection if clicking on UI elements
    if (state->end_turn_requested) return;
    
    handle_cell_selection(grid_config, map, state->selected_cell, &state->focused_cell);
}

void input_handle_movement(InputState *state, GridConfig *grid_config, Point *map) {
    if (!state->left_click) return;
    if (state->selected_cell == NULL) return;
    if (state->focused_cell == NULL) return;
    
    Point *focused = state->focused_cell;
    Point *selected = state->selected_cell;
    
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
    
    // Always flush flags on left click
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
