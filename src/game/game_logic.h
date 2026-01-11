#ifndef GAME_LOGIC_H_
#define GAME_LOGIC_H_

#include "types.h"
#include "game/map.h"
#include <stdbool.h>

// Game phase enum - tracks what phase the game is in
/**
 * @brief High-level game phase states used by the turn system.
 */
typedef enum {
    PHASE_PLAYER_TURN,
    PHASE_ENEMY_TURN,
    PHASE_TURN_TRANSITION,
    PHASE_GAME_OVER,
    PHASE_VICTORY
} GamePhase;

// Game state structure - holds all game state information
/**
 * @brief Central game state container tracking factions, turn order and modal state.
 */
typedef struct {
    GamePhase current_phase;
    Faction *factions;
    int num_factions;
    int current_faction_index;
    int turn_number;
    bool game_over;
    Faction *winner;
    struct Modal *modal;  // Pop-up modal system
} GameState;

// Troops are now stored directly on the Faction as `actors` and `actor_count`.

// Game initialization
/**
 * @brief Create a new GameState for the provided factions.
 * @param factions Array of factions owned by the game.
 * @param num_factions Number of factions in the array.
 * @return Newly allocated GameState pointer.
 */
GameState *game_state_create(Faction *factions, int num_factions);

/**
 * @brief Free a GameState and its dependent data.
 * @param state GameState to free (NULL-safe).
 */
void game_state_free(GameState *state);

/**
 * @brief Initialize an existing GameState structure with factions.
 */
void game_state_init(GameState *state, Faction *factions, int num_factions);

// Turn management
/**
 * @brief Advance the game to the next turn (handles turn order and victory checks).
 */
void game_next_turn(GameState *state);

/**
 * @brief End the current faction's turn and advance game state appropriately.
 */
void game_end_current_turn(GameState *state);

/**
 * @brief Start turn logic for the faction whose index is current.
 */
void game_start_faction_turn(GameState *state);

/**
 * @brief Return pointer to the faction whose turn it currently is.
 */
Faction *game_get_current_faction(GameState *state);

// AI processing for non-player factions
/**
 * @brief Perform AI actions for the current faction.
 */
void game_process_ai_turn(GameState *state, Point *map, GridConfig *grid_config);

// Unit turn management
/**
 * @brief Reset action flags for all characters in a faction.
 */
void game_reset_faction_units(Faction *faction);

/**
 * @brief Mark all units in a faction as having ended their turn.
 */
void game_end_all_unit_turns(Faction *faction);

/**
 * @brief Return whether a faction has any actions remaining this turn.
 */
bool game_faction_has_actions_remaining(Faction *faction);

// Victory condition checking
/**
 * @brief Evaluate victory conditions and update game state accordingly.
 */
bool game_check_victory_conditions(GameState *state);

/**
 * @brief Check whether a faction has been defeated (no alive units).
 */
bool game_is_faction_defeated(Faction *faction);

/**
 * @brief Get the faction that has won (if any).
 */
Faction *game_get_winner(GameState *state);

// Game state queries
/**
 * @brief True if the current phase is a player-controlled turn.
 */
bool game_is_player_turn(GameState *state);

/**
 * @brief True if the current phase is an AI-controlled turn.
 */
bool game_is_ai_turn(GameState *state);

/**
 * @brief True if the game is over.
 */
bool game_is_over(GameState *state);

/**
 * @brief Human-readable name for a GamePhase value.
 */
const char *game_get_phase_name(GamePhase phase);

// Troop utilities (operate on faction->actors)
/**
 * @brief Count alive characters for the given faction.
 */
int faction_count_alive(Faction *faction);

#endif
