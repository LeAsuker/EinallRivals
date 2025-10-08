#ifndef GAME_LOGIC_H_
#define GAME_LOGIC_H_

#include "types.h"
#include "map.h"
#include <stdbool.h>

// Game phase enum - tracks what phase the game is in
typedef enum {
    PHASE_PLAYER_TURN,
    PHASE_ENEMY_TURN,
    PHASE_TURN_TRANSITION,
    PHASE_GAME_OVER,
    PHASE_VICTORY
} GamePhase;

// Game state structure - holds all game state information
typedef struct {
    GamePhase current_phase;
    Faction *factions;
    int num_factions;
    int current_faction_index;
    int turn_number;
    bool game_over;
    Faction *winner;
} GameState;

// Troop management structure
typedef struct {
    Actor *troops;
    int count;
    Faction *faction;
} TroopGroup;

// Game initialization
GameState *game_state_create(Faction *factions, int num_factions);
void game_state_free(GameState *state);
void game_state_init(GameState *state, Faction *factions, int num_factions);

// Turn management
void game_next_turn(GameState *state, TroopGroup *troop_groups, int num_groups);
void game_end_current_turn(GameState *state, TroopGroup *troop_groups, int num_groups);
void game_start_faction_turn(GameState *state, TroopGroup *troop_groups, int num_groups);
Faction *game_get_current_faction(GameState *state);

// Unit turn management
void game_reset_faction_units(TroopGroup *troop_groups, int num_groups, Faction *faction);
void game_end_all_unit_turns(TroopGroup *troop_groups, int num_groups, Faction *faction);
bool game_faction_has_actions_remaining(TroopGroup *troop_groups, int num_groups, Faction *faction);

// Victory condition checking
bool game_check_victory_conditions(GameState *state, TroopGroup *troop_groups, int num_groups);
bool game_is_faction_defeated(TroopGroup *troop_groups, int num_groups, Faction *faction);
Faction *game_get_winner(GameState *state, TroopGroup *troop_groups, int num_groups);

// Game state queries
bool game_is_player_turn(GameState *state);
bool game_is_ai_turn(GameState *state);
bool game_is_over(GameState *state);
const char *game_get_phase_name(GamePhase phase);

// Troop group utilities
TroopGroup troop_group_create(Actor *troops, int count, Faction *faction);
int troop_group_count_alive(TroopGroup *group);

#endif