#ifndef INPUT_H_
#define INPUT_H_

#include "types.h"
#include "game/combat.h"
#include "render/rendering.h"
#include "ui/button.h"
#include <stdbool.h>

// Number of action buttons shown under the map
#define ACTION_BUTTON_COUNT 10

// Input state structure - tracks what actions the player wants to take
typedef struct {
    Point *selected_cell;      // Cell currently under mouse
    Point *focused_cell;       // Cell that has been clicked/selected
    bool left_click;           // True if left mouse button was just pressed
    bool right_click;          // True if right mouse button was just pressed
    bool end_turn_requested;   // True if player wants to end turn

    // End-turn button instance (refactored to use Button API)
    Button end_turn_button;
    
    // Action buttons (10 slots drawn below the map). These are clickable
    // and tracked by the input system.
    Button action_buttons[ACTION_BUTTON_COUNT];
    bool action_clicked[ACTION_BUTTON_COUNT];
    // Track last known screen size to detect window resizes and re-layout
    int last_screen_width;
    int last_screen_height;
} InputState;

// Initialize input state
void input_init(InputState *state);

// Layout input-owned buttons based on the provided grid configuration.
// Call once after grid/layout is initialized, and again if the grid
// or UI layout changes (e.g., window resize).
void input_layout_buttons(InputState *state, GridConfig *grid_config);
// Process action clicks and execute skills when applicable. This moves
// clicked-action handling out of main.c and into the input subsystem.
void input_handle_action_click(InputState *state, GridConfig *grid_config, Point *map);

// Update input state based on current frame's input
// Returns the cell currently under the mouse cursor
void input_update(InputState *state, GridConfig *grid_config, Point *map);

// Return index of the first action button that was clicked this frame,
// or -1 if none. This consumes the click state (clears the flag).
int input_get_clicked_action(InputState *state);

// Handle left click selection logic
void input_handle_selection(InputState *state, GridConfig *grid_config, Point *map);

// Handle right click movement logic
void input_handle_movement(InputState *state, GridConfig *grid_config, Point *map);

// Check if mouse is over the end turn button
bool input_is_mouse_over_end_turn_button(RenderContext *ctx);

// Check if the mouse is over the map area (returns true if the cursor
// is inside the grid bounds, false otherwise)
bool input_is_mouse_over_map(GridConfig *grid_config);

#endif
