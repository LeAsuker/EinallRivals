#ifndef INPUT_H_
#define INPUT_H_

#include "types.h"
#include "game/combat.h"
#include <stdbool.h>

// Input state structure - tracks what actions the player wants to take
typedef struct {
    Point *selected_cell;      // Cell currently under mouse
    Point *focused_cell;       // Cell that has been clicked/selected
    bool left_click;           // True if left mouse button was just pressed
    bool right_click;          // True if right mouse button was just pressed
    bool end_turn_requested;   // True if player wants to end turn
} InputState;

// Initialize input state
void input_init(InputState *state);

// Update input state based on current frame's input
// Returns the cell currently under the mouse cursor
void input_update(InputState *state, GridConfig *grid_config, Point *map);

// Handle left click selection logic
void input_handle_selection(InputState *state, GridConfig *grid_config, Point *map);

// Handle right click movement logic
void input_handle_movement(InputState *state, GridConfig *grid_config, Point *map);

// Check if mouse is over the end turn button
bool input_is_mouse_over_end_turn_button(GridConfig *grid_config);

#endif
