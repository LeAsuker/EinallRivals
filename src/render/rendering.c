#include "render/rendering.h"
#include "game/actor.h"
#include "input/input.h"
#include "types.h"
#include "ui/button.h"
#include "ui/modal.h"
#include <stddef.h>

// Helper: map genetics value to display symbol and color
static const char *genetic_symbol(int g, Color *out_color) {
  if (out_color)
    *out_color = BLACK;
  switch (g) {
  case 0:
    if (out_color)
      *out_color = RED;
    return "--";
  case 1:
    if (out_color)
      *out_color = RED;
    return "-";
  case 4:
    if (out_color)
      *out_color = GREEN;
    return "+";
  case 5:
    if (out_color)
      *out_color = GREEN;
    return "++";
  default:
    return "";
  }
}

static void draw_stat_with_gen(int x, int y, const char *label, int val,
                               int gen_val, int text_size) {
  DrawText(TextFormat("%s: %d", label, val), x, y, text_size, BLACK);
  Color sym_color;
  const char *sym = genetic_symbol(gen_val, &sym_color);
  if (sym[0] != '\0') {
    DrawText(sym, x + 260, y, text_size, sym_color);
  }
}

// Draw all character stats and genetics markers in one call
// Draw all character stats and genetics markers in one call
// Returns the y position after the last drawn line
static int draw_character_stats_with_gen(int x, int y, Character *occupant,
                                         Stats stats, int text_size) {
  int line_h = text_size + 8;
  DrawText(TextFormat("NAME: %s", occupant->name), x, y, text_size, BLACK);
  y += line_h;
  DrawText(TextFormat("FAC: %s", occupant->owner->name), x, y, text_size,
           BLACK);
  y += line_h;
  DrawText(TextFormat("LVL: %d", occupant->level), x, y, text_size, BLACK);
  y += line_h;

  /* Draw HP with genetics marker to the right */
  DrawText(TextFormat("HP: %d/%d", occupant->curr_health, stats.max_health), x,
           y, text_size, BLACK);
  Color hp_sym_color;
  const char *hp_sym =
      genetic_symbol(occupant->genetics.max_health, &hp_sym_color);
  if (hp_sym[0] != '\0') {
    DrawText(hp_sym, x + 260, y, text_size, hp_sym_color);
  }
  y += line_h;

  draw_stat_with_gen(x, y, "PATK", stats.phys_attack,
                     occupant->genetics.phys_attack, text_size);
  y += line_h;
  draw_stat_with_gen(x, y, "PDEF", stats.phys_defense,
                     occupant->genetics.phys_defense, text_size);
  y += line_h;
  draw_stat_with_gen(x, y, "MATK", stats.magic_attack,
                     occupant->genetics.magic_attack, text_size);
  y += line_h;
  draw_stat_with_gen(x, y, "MDEF", stats.magic_defense,
                     occupant->genetics.magic_defense, text_size);
  y += line_h;
  draw_stat_with_gen(x, y, "LCK", stats.luck, occupant->genetics.luck,
                     text_size);
  y += line_h;
  return y;
}

static void render_map(RenderContext *ctx, Point *map, Point *focused_cell);
static bool cell_is_focused(Point *cell, Point *focused_cell);

static bool cell_is_focused(Point *cell, Point *focused_cell) {
  if (focused_cell == NULL)
    return false;
  return (cell->x == focused_cell->x && cell->y == focused_cell->y);
}
void render_debug_info(RenderContext *ctx, Point *map);
void render_cell_info(RenderContext *ctx, Point *focused_cell);
static void render_ui(RenderContext *ctx, const char *faction_name,
                      Faction *current_faction, Button *end_turn_button);
static void render_map_border(RenderContext *ctx);

void render_init(RenderContext *ctx, GridConfig *grid) {
  NULL_CHECK_VOID(ctx);
  NULL_CHECK_VOID(grid);
  ctx->grid_offset_x = grid->grid_offset_x;
  ctx->grid_offset_y = grid->grid_offset_y;
  ctx->grid_cell_size = grid->grid_cell_size;
  ctx->grid_cells_x = grid->max_grid_cells_x;
  ctx->grid_cells_y = grid->max_grid_cells_y;
}

// New function to render game with full button state
void render_game(RenderContext *ctx, Point *map, Point *focused_cell,
                 Faction *current_faction, InputState *input_state,
                 Button *end_turn_button, Button action_buttons[],
                 int action_count, Modal *modal) {
  NULL_CHECK_VOID(ctx);
  NULL_CHECK_VOID(map);
  BeginDrawing();
  ClearBackground(RAYWHITE);

  render_debug_info(ctx, map);
  render_map(ctx, map, focused_cell);
  render_map_border(ctx);
  render_cell_info(ctx, focused_cell);
  if (current_faction == NULL) {
    Faction _dummy = {.name = "None", .characters = NULL, .character_count = 0};
    current_faction = &_dummy;
  }
  render_ui(ctx, current_faction->name, current_faction, end_turn_button);
  render_actions(ctx, (focused_cell != NULL) ? focused_cell->occupant : NULL,
                 input_state, action_buttons, action_count);

  // Render modal last (on top of everything)
  if (modal) {
    modal_render(modal);
  }

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
                          map_width + border_thickness * 2,
                          map_height + border_thickness * 2, BLACK,
                          border_thickness);
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
        DrawRectangle(x_pos, y_pos, ctx->grid_cell_size, ctx->grid_cell_size,
                      move_tint);
      }
      if (cell->in_attack_range) {
        Color attack_tint = (Color){255, 0, 0, 120};
        DrawRectangle(x_pos, y_pos, ctx->grid_cell_size, ctx->grid_cell_size,
                      attack_tint);
      }
    }

    // Draw selection tint (transparent yellow) if this is the focused cell
    if (focused_cell != NULL && cell->x == focused_cell->x &&
        cell->y == focused_cell->y) {
      Color select_tint = (Color){255, 255, 0, 120};
      DrawRectangle(x_pos, y_pos, ctx->grid_cell_size, ctx->grid_cell_size,
                    select_tint);
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
    DrawRectangleLines(x_pos, y_pos, ctx->grid_cell_size, ctx->grid_cell_size,
                       GRAY);

    // Draw highlights
    if (cell->occupant != NULL) {
      DrawRectangleLines(x_pos, y_pos, ctx->grid_cell_size, ctx->grid_cell_size,
                         cell->occupant->owner->prim_color);
    }
    // range/attack tints are drawn above terrain but below units/structures
  }
}

/**
 * @brief Draw debug overlays such as mouse position and simple runtime
 * information.
 * @param ctx Rendering context (reserved for future use).
 * @param map Optional map pointer for debug inspection (currently unused).
 */
void render_debug_info(RenderContext *ctx, Point *map) {
  int mouse_x = GetMouseX();
  int mouse_y = GetMouseY();
  // Use your safe_mouse functions here
  DrawText(TextFormat("MOUSE: %d %d", mouse_x, mouse_y), 40, 20, 20, DARKGRAY);
}

/**
 * @brief Render a detailed info panel about the focused cell: occupant,
 * terrain, and structure.
 * @param ctx Rendering context used to compute panel position.
 * @param focused_cell Cell to describe (NULL-safe; returns early if NULL).
 */
void render_cell_info(RenderContext *ctx, Point *focused_cell) {
  if (focused_cell == NULL)
    return;

  int info_x =
      ctx->grid_cells_x * ctx->grid_cell_size + ctx->grid_offset_x + 20;
  int info_y = ctx->grid_offset_y;

  if (focused_cell->occupant != NULL) {
    Character *occupant = focused_cell->occupant;
    Stats stats = character_get_stats(occupant);
    int x = info_x + 5;
    int y = info_y + 5;
    int text_size = 24;

    y = draw_character_stats_with_gen(x, y, occupant, stats, text_size);
    // Advance the outer info_y to avoid overlapping the terrain/structure UI
    // below
    info_y = y + 20;
  } else {
    DrawText(TextFormat("OCC: None"), info_x + 5, info_y + 5, 26, BLACK);
  }

  info_y += 300;
  DrawLine(info_x, info_y, info_x + ctx->grid_cell_size * 8, info_y, BLACK);
  DrawText(TextFormat("TRN: %s\nPASS: %s", focused_cell->terrain.name,
                      focused_cell->terrain.passable ? "Yes" : "No"),
           info_x + 5, info_y + 5, 26, BLACK);

  info_y += 100;
  DrawLine(info_x, info_y, info_x + ctx->grid_cell_size * 8, info_y, BLACK);

  if (focused_cell->structure != NULL) {
    Structure *structure = focused_cell->structure;
    DrawText(TextFormat("STRCT: %s\nPASS: %s\nLOOT: %s", structure->name,
                        structure->passable ? "Yes" : "No",
                        structure->lootable ? "Yes" : "No"),
             info_x + 5, info_y + 5, 26, BLACK);
  } else {
    DrawText(TextFormat("STRCT: None"), info_x + 5, info_y + 5, 26, BLACK);
  }

  // Selection highlight is now rendered as a transparent background tint
}

/**
 * @brief Render the right-hand UI panel including the end-turn button and
 * decorative border.
 * @param ctx Rendering context used to compute UI placement and sizing.
 * @param faction_name Name string used for the end-turn button label.
 * @param current_faction Faction providing UI colors (may be NULL for fallback
 * colors).
 * @param end_turn_button Pointer to the End Turn button state. The rect is
 * copied and styled.
 */
static void render_ui(RenderContext *ctx, const char *faction_name,
                      Faction *current_faction, Button *end_turn_button) {

  int info_x =
      ctx->grid_cells_x * ctx->grid_cell_size + ctx->grid_offset_x + 20;
  int info_y = ctx->grid_offset_y;

  int border_thickness = 3;
  DrawThickRectangleLines(info_x, info_y, ctx->grid_cell_size * 8,
                          ctx->grid_cell_size * 17, BLACK, border_thickness);

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
    button_color =
        (end_turn_button != NULL && end_turn_button->pressed) ? GRAY : DARKGRAY;
    border_color = BLACK;
    text_color = border_color;
  }

  // Use the persistent end_turn_button rect, but update visuals based on
  // faction
  Button tmp_btn = *end_turn_button;
  button_set_colors(&tmp_btn, button_color, border_color);
  button_set_border_thickness(&tmp_btn, border_thickness);

  // Set the label so button_draw will render it centered
  const int text_size = 22;
  const char *button_text = TextFormat("End Turn: %s", faction_name);
  button_set_label(&tmp_btn, button_text, text_size, text_color);

  button_draw(&tmp_btn);
}

/**
 * @brief Render the array of action buttons and draw skill icons/overlays where
 * applicable.
 * @param ctx Rendering context for placement calculation.
 * @param character Character providing skill icons (may be NULL).
 * @param input_state Input state used to show selected/dim states.
 * @param action_buttons Array of action buttons to render.
 * @param action_count Number of action buttons.
 */
void render_actions(RenderContext *ctx, Character *character,
                    InputState *input_state, Button action_buttons[],
                    int action_count) {
  NULL_CHECK_VOID(ctx);
  NULL_CHECK_VOID(action_buttons);
  // Render action panels to the bottom of the map (aligned with UI panel)

  int box_w = ctx->grid_cell_size * 2;
  int box_h = ctx->grid_cell_size * 2;

  // Draw the persistent action buttons and icons where applicable
  for (int i = 0; i < action_count; i++) {
    Button *b = &action_buttons[i];
    button_draw(b);

    int bx = b->x;
    int by = b->y;

    if (character != NULL && i < character->skill_count) {
      Skill *s = &character->skills[i];
      if (s->icon.id) {
        Rectangle src =
            (Rectangle){0, 0, (float)s->icon.width, (float)s->icon.height};
        Rectangle dst =
            (Rectangle){(float)bx, (float)by, (float)box_w, (float)box_h};
        Vector2 origin = (Vector2){0.0f, 0.0f};
        DrawTexturePro(s->icon, src, dst, origin, 0.0f, WHITE);
      } else {
        DrawRectangle(bx + 4, by + 4, box_w - 8, box_h - 8,
                      (Color){150, 150, 150, 200});
      }
      // Highlight selected action
      if (input_state != NULL && input_state->selected_action == i) {
        DrawThickRectangleLines(bx, by, box_w, box_h,
                                (Color){255, 255, 100, 255}, 5);
        // Draw skill range below the button
        DrawText(TextFormat("RNG: %d", s->range), bx, by + box_h + 5, 16,
                 BLACK);
      }
      if (input_state != NULL &&
          (!character->can_act || !character->owner->has_turn)) {
        // Dim the icon if character cannot act
        DrawRectangle(bx, by, box_w, box_h, (Color){100, 100, 100, 180});
      }
    }
  }
}