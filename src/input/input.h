#ifndef INPUT_H_
#define INPUT_H_

/*
 * Module: Input Handling
 * Layer: Input (reads game state, drives mutations)
 * Purpose: Mouse/keyboard processing, button hit-testing, action clicks, movement
 * commands. Translates raw input into game operations.
 * Dependencies: types.h, game/actions.h, game/combat.h, ui/button.h
 * Consumers: main.c (called each frame in the game loop)
 * Rule: May call execution functions (skills, end turn) but must not embed game
 * rules (damage formulas, victory logic).
 * See AGENTS.md and src/CODEMAP.md for architecture context.
 */

#include "game/actions.h"
#include "game/combat.h"
#include "types.h"
#include "ui/button.h"
#include <stdbool.h>

#define ACTION_BUTTON_COUNT 10

// Number of action buttons shown under the map
typedef struct RenderContext RenderContext;

// Input state structure - tracks what actions the player wants to take
typedef struct InputState {
  Point *selected_cell;    // Cell currently under mouse
  Point *focused_cell;     // Cell that has been clicked/selected
  bool left_click;         // True if left mouse button was just pressed
  bool right_click;        // True if right mouse button was just pressed
  bool end_turn_requested; // True if player wants to end turn

  // End-turn button instance (refactored to use Button API)
  Button end_turn_button;

  // Action buttons (10 slots drawn below the map). These are clickable
  // and tracked by the input system.
  Button action_buttons[ACTION_BUTTON_COUNT];
  bool action_clicked[ACTION_BUTTON_COUNT];
  // Track last known screen size to detect window resizes and re-layout
  int last_screen_width;
  int last_screen_height;
  // Currently selected action index (-1 = none). Set when clicking action
  // button.
  int selected_action;
} InputState;

/**
 * @brief Initialize input state fields to sensible defaults.
 * @param state Pointer to an InputState to initialize (must be valid).
 */
void input_init(InputState *state);

/**
 * @brief Position input-owned buttons according to the grid layout.
 *        Call after grid initialization or when the window/grid is resized.
 * @param state InputState owning the buttons to layout.
 * @param grid_config Grid configuration describing offsets and cell sizes.
 */
void input_layout_buttons(InputState *state, GridConfig *grid_config);

/**
 * @brief Process clicks on action buttons and execute skills when applicable.
 * @param state Current input state.
 * @param grid_config Grid configuration for validation and targeting.
 * @param map Map array used for targeting and movement checks.
 */
void input_handle_action_click(InputState *state, GridConfig *grid_config,
                               Point *map);

/**
 * @brief Update input state for the current frame (mouse, clicks, button
 * states).
 * @param state Input state to update.
 * @param grid_config Grid configuration for mapping screen-to-cell coordinates.
 * @param map Map array to query cells under mouse.
 */
void input_update(InputState *state, GridConfig *grid_config, Point *map);

/**
 * @brief Return index of first action button clicked this frame, or -1 if none.
 *        This call consumes the click state (clears the per-button flag).
 * @param state InputState to query.
 * @return Index of clicked action, or -1 if no click.
 */
int input_get_clicked_action(InputState *state);

/**
 * @brief Handle left-click selection logic (select units, focus cells, toggle).
 * @param state Input state containing the click/focus information.
 * @param grid_config Grid configuration for coordinate mapping.
 * @param map Map array to operate on.
 */
void input_handle_selection(InputState *state, GridConfig *grid_config,
                            Point *map);

/**
 * @brief Handle right-click movement logic (attempt to move selected unit).
 * @param state Input state containing selection and click info.
 * @param grid_config Grid configuration for path/range checks.
 * @param map Map array for validating destination and occupancy.
 */
void input_handle_movement(InputState *state, GridConfig *grid_config,
                           Point *map);

/**
 * @brief Handle a left-click on the map according to game rules:
 *        movement, skill targeting, or focus toggles.
 * @param state Input state with click flags (should have left_click true).
 * @param grid_config Grid data for mapping coordinates.
 * @param map Map array to read and update selection/targets.
 */
void input_handle_left_click(InputState *state, GridConfig *grid_config,
                             Point *map);

/**
 * @brief Query whether the mouse is currently hovering the end-turn button.
 * @param ctx Render context used to compute button coordinates.
 * @return true if mouse is over the end-turn button.
 */
bool input_is_mouse_over_end_turn_button(RenderContext *ctx);

/**
 * @brief Check whether the mouse cursor is inside the map grid area.
 * @param grid_config Grid configuration used to map screen-to-cell coordinates.
 * @return true if the cursor is within grid bounds.
 */
bool input_is_mouse_over_map(GridConfig *grid_config);

#endif
