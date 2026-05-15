#ifndef RENDERING_H_
#define RENDERING_H_

/*
 * Module: Rendering
 * Layer: Presentation (reads game state, draws pixels)
 * Purpose: Draw the map, units, structures, UI chrome, debug overlays, and
 * modals. Pure function of game state: never mutates rules or units.
 * Dependencies: types.h, ui/button.h
 * Consumers: main.c (called each frame), ui/modal.c (modal_render)
 * Rule: Read-only w.r.t. game logic. If you need to change state while drawing,
 * move the logic to input/ or game/.
 * See AGENTS.md and src/CODEMAP.md for architecture context.
 */

#include "raylib.h"
#include "types.h"
#include "ui/button.h"
#include <stddef.h>

typedef struct InputState InputState;
typedef struct Modal Modal;

// Public rendering functions
/**
 * @brief Rendering context describing grid geometry and offsets.
 */
typedef struct RenderContext {
  int grid_offset_x;  /**< Pixel X offset for the top-left of the grid. */
  int grid_offset_y;  /**< Pixel Y offset for the top-left of the grid. */
  int grid_cell_size; /**< Size of one grid cell in pixels. */
  int grid_cells_x;   /**< Number of cells horizontally. */
  int grid_cells_y;   /**< Number of cells vertically. */
} RenderContext;

struct InputState;

/**
 * @brief Initialize a RenderContext from a GridConfig.
 * @param ctx Pointer to RenderContext to initialize (must be non-NULL).
 * @param grid Grid configuration to read geometry from.
 */
void render_init(RenderContext *ctx, GridConfig *grid);

/**
 * @brief Draw debug overlays such as mouse position and other runtime info.
 * @param ctx Rendering context (NULL-safe guards may be present).
 * @param map Pointer to map data (used for debug info if needed).
 */
void render_debug_info(RenderContext *ctx, Point *map);

/**
 * @brief Render an info panel describing a focused cell (terrain, occupant,
 * structure).
 * @param ctx Rendering context.
 * @param focused_cell Cell to describe (NULL-safe).
 */
void render_cell_info(RenderContext *ctx, Point *focused_cell);

/**
 * @brief Render the full game frame: map, UI, cell info, and optional modal
 * overlay.
 * @param ctx Rendering context.
 * @param map Map cell array to render.
 * @param focused_cell Currently focused cell (can be NULL).
 * @param current_faction Pointer to current player's faction (can be NULL for
 * dummy display).
 * @param input_state Current input state for UI reactions.
 * @param end_turn_button End-turn button state for rendering.
 * @param action_buttons Array of action buttons to render.
 * @param action_count Number of action buttons.
 * @param modal Optional modal to render on top of the scene.
 */
void render_game(RenderContext *ctx, Point *map, Point *focused_cell,
                 Faction *current_faction, InputState *input_state,
                 Button *end_turn_button, Button action_buttons[],
                 int action_count, Modal *modal);

/**
 * @brief Render action buttons/icons for the given character.
 * @param ctx Rendering context.
 * @param character Character providing skills/icons (may be NULL).
 * @param input_state Input state used to show selected/dimmed states.
 * @param action_buttons Array of buttons to draw.
 * @param action_count Number of action buttons.
 */
void render_actions(RenderContext *ctx, Character *character,
                    InputState *input_state, Button action_buttons[],
                    int action_count);
// Note: combat forecast rendering moved to `game/combat.*` UI helpers.
// Drawing helpers: draw thicker lines/rect outlines by repeating
// DrawLine/DrawRectangleLines

/**
 * @brief Draw a thick line by repeating DrawLine with vertical offsets.
 * @param startPosX Start X pixel coordinate.
 * @param startPosY Start Y pixel coordinate.
 * @param endPosX End X pixel coordinate.
 * @param endPosY End Y pixel coordinate.
 * @param color Line color.
 * @param thickness Thickness in pixels (number of repeated lines).
 */
static inline void DrawThickLine(int startPosX, int startPosY, int endPosX,
                                 int endPosY, Color color, int thickness) {
  for (int i = 0; i < thickness; i++) {
    DrawLine(startPosX, startPosY + i, endPosX, endPosY + i, color);
  }
}

/**
 * @brief Draw a rectangle outline with the specified thickness.
 * @param posX Top-left X pixel coordinate.
 * @param posY Top-left Y pixel coordinate.
 * @param width Width of the rectangle in pixels.
 * @param height Height of the rectangle in pixels.
 * @param color Outline color.
 * @param thickness Outline thickness in pixels.
 */
static inline void DrawThickRectangleLines(int posX, int posY, int width,
                                           int height, Color color,
                                           int thickness) {
  for (int i = 0; i < thickness; i++) {
    DrawRectangleLines(posX - i, posY - i, width + i * 2, height + i * 2,
                       color);
  }
}
#endif
