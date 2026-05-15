#include "game/game_logic.h"
#include "game/actor.h"
#include "game/combat.h"
#include "game/map.h"
#include "core/utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ============================================================================
// Game State Initialization
// ============================================================================

/**
 * @brief Allocate and initialize a new GameState for the provided factions.
 *
 * @param factions Array of faction prototypes to use in the new state.
 * @param num_factions Number of factions.
 * @return Allocated GameState or NULL on allocation failure.
 */
GameState *game_state_create(Faction *factions, int num_factions) {
  GameState *state = malloc(sizeof(GameState));
  if (state == NULL) {
    fprintf(stderr, "Error: Failed to allocate memory for game state\n");
    return NULL;
  }

  game_state_init(state, factions, num_factions);
  return state;
}

/**
 * @brief Free a GameState previously allocated by game_state_create.
 *
 * Does not free the factions array passed into the state; caller retains
 * ownership of those resources.
 */
void game_state_free(GameState *state) {
  if (state != NULL) {
    free(state);
  }
}

/**
 * @brief Initialize an existing GameState structure with factions.
 *
 * Sets initial phase, turn counters and ensures the first faction has the turn.
 *
 * @param state GameState to initialize (must be preallocated).
 * @param factions Array of factions to reference.
 * @param num_factions Number of factions in the array.
 */
void game_state_init(GameState *state, Faction *factions, int num_factions) {
  state->current_phase = PHASE_PLAYER_TURN;
  state->factions = factions;
  state->num_factions = num_factions;
  // Start with the first faction (index 0)
  state->current_faction_index = 0;
  state->turn_number = 1;
  state->game_over = false;
  state->winner = NULL;

  // Set first faction to have the turn
  if (num_factions > 0) {
    for (int i = 0; i < num_factions; i++) {
      factions[i].has_turn = false;
    }
    factions[0].has_turn = true;
  }

  printf("\n=== GAME START ===\n");
  if (state->current_faction_index < num_factions) {
    printf("Turn %d: %s's turn\n\n", state->turn_number,
           factions[state->current_faction_index].name);
  } else {
    printf("Turn %d: (no playable faction)\n\n", state->turn_number);
  }
}

// ============================================================================
// Turn Management
// ============================================================================

/**
 * @brief Advance the game to the next faction's turn and handle transitions.
 *
 * Updates turn counters, toggles has_turn flags, starts the new faction's
 * turn, and checks victory conditions.
 */
void game_next_turn(GameState *state) {
  // Move to next faction in order
  state->current_faction_index++;
  if (state->current_faction_index >= state->num_factions) {
    state->current_faction_index = 0;
    state->turn_number++;
    printf("\n=== TURN %d ===\n", state->turn_number);
  }

  // Update faction turn flags
  for (int i = 0; i < state->num_factions; i++) {
    state->factions[i].has_turn = (i == state->current_faction_index);
  }

  Faction *current_faction = game_get_current_faction(state);
  printf("\n--- %s's turn ---\n", current_faction->name);

  // Start the new faction's turn
  game_start_faction_turn(state);

  // Check victory conditions
  if (game_check_victory_conditions(state)) {
    state->game_over = true;
    state->current_phase = PHASE_GAME_OVER;

    Faction *winner = game_get_winner(state);
    if (winner != NULL) {
      printf("\n=== VICTORY ===\n");
      printf("%s has won the game!\n", winner->name);
      state->winner = winner;
    }
  }
}

/**
 * @brief End the current faction's turn and advance to the next.
 */
void game_end_current_turn(GameState *state) {
  Faction *current_faction = game_get_current_faction(state);
  printf("Ending turn for %s\n", current_faction->name);

  // End all units' turns for current faction
  game_end_all_unit_turns(current_faction);

  // Move to next turn
  game_next_turn(state);
}

/**
 * @brief Start the current faction's turn by resetting units and updating
 * phase.
 */
void game_start_faction_turn(GameState *state) {
  Faction *current_faction = game_get_current_faction(state);

  // Reset all units for the current faction
  game_reset_faction_units(current_faction);

  // Update phase
  // If the current faction is marked playable, treat it as a player turn
  if (current_faction != NULL && current_faction->playable) {
    state->current_phase = PHASE_PLAYER_TURN;
  } else {
    state->current_phase = PHASE_ENEMY_TURN;
  }
}

// ============================================================================
// AI Processing
// ============================================================================

/**
 * @brief Return the Faction currently active in the GameState or NULL.
 */
Faction *game_get_current_faction(GameState *state) {
  if (state->current_faction_index >= 0 &&
      state->current_faction_index < state->num_factions) {
    return &state->factions[state->current_faction_index];
  }
  return NULL;
}

// ============================================================================
// Unit Turn Management
// ============================================================================

/**
 * @brief Very simple AI turn processor for the current faction.
 *
 * For each alive unit that can perform an action, attempts to attack any
 * reachable enemy, otherwise moves toward the nearest enemy or moves
 * randomly with some probability.
 *
 * On completion the faction's turn is ended.
 */
// ============================================================================
// AI Internal Helpers
// ============================================================================

/**
 * @brief Find the closest enemy of `me` that is within `max_range` of `origin`.
 *
 * @param map Map array.
 * @param total_cells Total number of cells in the map.
 * @param origin Cell to measure distance from.
 * @param me Character whose enemies to search for.
 * @param grid_config Grid configuration for range checks.
 * @param max_range Maximum attack range to consider.
 * @return Pointer to the closest in-range enemy cell, or NULL if none.
 */
static Point *ai_find_closest_enemy_in_range(Point *map, int total_cells,
                                             Point *origin, Character *me,
                                             GridConfig *grid_config,
                                             int max_range) {
  Point *best_target = NULL;
  int best_dist = 999999;
  for (int c = 0; c < total_cells; c++) {
    if (map[c].occupant == NULL)
      continue;
    Character *other = map[c].occupant;
    if (!character_is_enemy(me, other))
      continue;
    if (combat_is_in_range(grid_config, origin, &map[c], max_range)) {
      int d = combat_get_distance(origin, &map[c]);
      if (d < best_dist) {
        best_dist = d;
        best_target = &map[c];
      }
    }
  }
  return best_target;
}

/**
 * @brief Find the closest enemy of `me` on the map regardless of range.
 *
 * @param map Map array.
 * @param total_cells Total number of cells in the map.
 * @param origin Cell to measure distance from.
 * @param me Character whose enemies to search for.
 * @return Pointer to the closest enemy cell, or NULL if none.
 */
static Point *ai_find_closest_enemy(Point *map, int total_cells, Point *origin,
                                    Character *me) {
  Point *closest_enemy = NULL;
  int closest_dist = 999999;
  for (int c = 0; c < total_cells; c++) {
    if (map[c].occupant == NULL)
      continue;
    Character *other = map[c].occupant;
    if (!character_is_enemy(me, other))
      continue;
    int d = combat_get_distance(origin, &map[c]);
    if (d < closest_dist) {
      closest_dist = d;
      closest_enemy = &map[c];
    }
  }
  return closest_enemy;
}

// ============================================================================
// AI Processing
// ============================================================================

/**
 * @brief Very simple AI turn processor for the current faction.
 *
 * For each alive unit that can perform an action, attempts to attack any
 * reachable enemy, otherwise moves toward the nearest enemy or moves
 * randomly with some probability.
 *
 * On completion the faction's turn is ended.
 */
void game_process_ai_turn(GameState *state, Point *map,
                          GridConfig *grid_config) {
  Faction *current = game_get_current_faction(state);
  if (current == NULL)
    return;

  int total_cells = grid_total_cells(grid_config);

  for (int i = 0; i < current->character_count; i++) {
    Character *character = &current->characters[i];
    if (!character_is_alive(character))
      continue;
    if (!character_can_perform_action(character))
      continue;

    // Locate character's cell
    Point *character_cell = NULL;
    for (int c = 0; c < total_cells; c++) {
      if (map[c].occupant == character) {
        character_cell = &map[c];
        break;
      }
    }
    if (character_cell == NULL)
      continue;

    // Search for enemies in attack range; pick the closest
    Stats character_stats = character_get_stats(character);
    int max_range = character_get_max_skill_range(character);
    Point *best_target = ai_find_closest_enemy_in_range(
        map, total_cells, character_cell, character, grid_config, max_range);

    if (best_target != NULL && character->can_act) {
      combat_execute_at_cells(grid_config, map, character_cell, best_target);
      continue;
    }

    // No enemy in immediate attack range
    if (!character->can_move)
      continue;

    // First: try to find a closest enemy that can be reached (move + range)
    Point *closest_enemy =
        ai_find_closest_enemy(map, total_cells, character_cell, character);
    int closest_dist =
        (closest_enemy != NULL)
            ? combat_get_distance(character_cell, closest_enemy)
            : 999999;

    bool moved = false;
    if (closest_enemy != NULL &&
        closest_dist <= (character_stats.movement + max_range)) {
      // Move one step towards the enemy (reduce Manhattan distance)
      int dx = closest_enemy->x - character_cell->x;
      int dy = closest_enemy->y - character_cell->y;
      int sx = (dx > 0) ? 1 : (dx < 0) ? -1 : 0;
      int sy = (dy > 0) ? 1 : (dy < 0) ? -1 : 0;

      // Prefer the axis with larger distance
      int try_x_first = (abs(dx) >= abs(dy));
      int try_order[2][2];
      if (try_x_first) {
        try_order[0][0] = sx;
        try_order[0][1] = 0;
        try_order[1][0] = 0;
        try_order[1][1] = sy;
      } else {
        try_order[0][0] = 0;
        try_order[0][1] = sy;
        try_order[1][0] = sx;
        try_order[1][1] = 0;
      }

      for (int t = 0; t < 2 && !moved; t++) {
        int nx = character_cell->x + try_order[t][0];
        int ny = character_cell->y + try_order[t][1];
        if (!map_is_valid_coords(grid_config, nx, ny))
          continue;
        Point *dest = map_get_cell(map, grid_config, nx, ny);
        if (dest == NULL)
          continue;
        if (!map_can_unit_enter_cell(dest, character))
          continue;
        // Move character
        dest->occupant = character;
        character_cell->occupant = NULL;
        character->can_move = false;
        moved = true;
        // update character_cell to new location so we can attempt an attack
        // after moving
        character_cell = dest;
        break;
      }
    }

    // If we didn't move towards an enemy, fallback to probabilistic random move
    if (!moved) {
      int roll = rand() % 100;
      if (roll < 40) {
        // stay still
        continue;
      }

      int dirs[4][2] = {{0, -1}, {0, 1}, {-1, 0}, {1, 0}};
      // Shuffle directions
      for (int k = 0; k < 4; k++) {
        int r = rand() % 4;
        int tx = dirs[k][0];
        int ty = dirs[k][1];
        dirs[k][0] = dirs[r][0];
        dirs[k][1] = dirs[r][1];
        dirs[r][0] = tx;
        dirs[r][1] = ty;
      }

      for (int d = 0; d < 4; d++) {
        int nx = character_cell->x + dirs[d][0];
        int ny = character_cell->y + dirs[d][1];
        if (!map_is_valid_coords(grid_config, nx, ny))
          continue;
        Point *dest = map_get_cell(map, grid_config, nx, ny);
        if (dest == NULL)
          continue;
        if (!map_can_unit_enter_cell(dest, character))
          continue;
        // Move character
        dest->occupant = character;
        character_cell->occupant = NULL;
        character->can_move = false;
        break;
      }
    }

    // After moving (either toward enemy or random), if character can still act,
    // try to attack any enemy now in range
    if (character->can_act) {
      Point *attack_target = ai_find_closest_enemy_in_range(
          map, total_cells, character_cell, character, grid_config, max_range);
      if (attack_target != NULL) {
        combat_execute_at_cells(grid_config, map, character_cell,
                                attack_target);
      }
    }
  }

  // After AI processes, end the faction's turn
  game_end_current_turn(state);
}
// ============================================================================
// Unit Turn Management
// ============================================================================

/**
 * @brief Reset action flags for all characters in a faction.
 */
void game_reset_faction_units(Faction *faction) {
  if (faction == NULL)
    return;
  character_array_reset_turns(faction->characters, faction->character_count);
}

/**
 * @brief End the turn for every alive unit in the faction.
 */
void game_end_all_unit_turns(Faction *faction) {
  if (faction == NULL)
    return;
  for (int j = 0; j < faction->character_count; j++) {
    Character *character = &faction->characters[j];
    if (character_is_alive(character)) {
      character_end_turn(character);
    }
  }
}

/**
 * @brief Return whether a faction has any actions remaining this turn.
 */
bool game_faction_has_actions_remaining(Faction *faction) {
  if (faction == NULL)
    return false;
  for (int j = 0; j < faction->character_count; j++) {
    Character *character = &faction->characters[j];
    if (character_is_alive(character) &&
        character_can_perform_action(character)) {
      return true;
    }
  }
  return false;
}

// ============================================================================
// Victory Condition Checking
// ============================================================================

/**
 * @brief Evaluate victory conditions and update game state accordingly.
 */
bool game_check_victory_conditions(GameState *state) {
  int factions_alive = 0;

  // Count how many factions still have units
  for (int i = 0; i < state->num_factions; i++) {
    if (!game_is_faction_defeated(&state->factions[i])) {
      factions_alive++;
    }
  }

  // Victory if only one faction remains
  return factions_alive <= 1;
}

/**
 * @brief Check whether a faction has been defeated (no alive units).
 */
bool game_is_faction_defeated(Faction *faction) {
  if (faction == NULL)
    return true;
  int alive_count = faction_count_alive(faction);
  return alive_count == 0;
}

/**
 * @brief Get the faction that has won (if any).
 */
Faction *game_get_winner(GameState *state) {
  // Find the faction that still has units alive
  for (int i = 0; i < state->num_factions; i++) {
    if (!game_is_faction_defeated(&state->factions[i])) {
      return &state->factions[i];
    }
  }
  return NULL; // No winner (shouldn't happen)
}

// ============================================================================
// Game State Queries
// ============================================================================

/**
 * @brief Whether the current phase is the player's turn.
 */
bool game_is_player_turn(GameState *state) {
  return state->current_phase == PHASE_PLAYER_TURN;
}

/**
 * @brief Whether the current phase is an AI/enemy turn.
 */
bool game_is_ai_turn(GameState *state) {
  return state->current_phase == PHASE_ENEMY_TURN;
}

/**
 * @brief Whether the game is marked as over.
 */
bool game_is_over(GameState *state) { return state->game_over; }

/**
 * @brief Human-readable name for a GamePhase value.
 */
const char *game_get_phase_name(GamePhase phase) {
  switch (phase) {
  case PHASE_PLAYER_TURN:
    return "Player Turn";
  case PHASE_ENEMY_TURN:
    return "Enemy Turn";
  case PHASE_TURN_TRANSITION:
    return "Turn Transition";
  case PHASE_GAME_OVER:
    return "Game Over";
  case PHASE_VICTORY:
    return "Victory";
  default:
    return "Unknown";
  }
}

// ============================================================================
// ============================================================================
// Faction / troop utilities
// ============================================================================

/**
 * @brief Count how many characters in a faction are alive.
 */
int faction_count_alive(Faction *faction) {
  if (faction == NULL)
    return 0;
  return character_array_count_alive(faction->characters,
                                     faction->character_count);
}
