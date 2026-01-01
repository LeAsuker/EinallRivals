#include "game/game_logic.h"
#include "game/actor.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// ============================================================================
// Game State Initialization
// ============================================================================

GameState *game_state_create(Faction *factions, int num_factions) {
    GameState *state = malloc(sizeof(GameState));
    if (state == NULL) {
        fprintf(stderr, "Error: Failed to allocate memory for game state\n");
        return NULL;
    }
    
    game_state_init(state, factions, num_factions);
    return state;
}

void game_state_free(GameState *state) {
    if (state != NULL) {
        free(state);
    }
}

void game_state_init(GameState *state, Faction *factions, int num_factions) {
    state->current_phase = PHASE_PLAYER_TURN;
    state->factions = factions;
    state->num_factions = num_factions;
    state->current_faction_index = 0;
    state->turn_number = 1;
    state->game_over = false;
    state->winner = NULL;
    
    // Set first faction to have the turn
    if (num_factions > 0) {
        factions[0].has_turn = true;
        for (int i = 1; i < num_factions; i++) {
            factions[i].has_turn = false;
        }
    }
    
    printf("\n=== GAME START ===\n");
    printf("Turn %d: %s's turn\n\n", state->turn_number, factions[0].name);
}

// ============================================================================
// Turn Management
// ============================================================================

void game_next_turn(GameState *state, TroopGroup *troop_groups, int num_groups) {
    // Move to next faction
    state->current_faction_index++;
    
    // If we've cycled through all factions, start a new round
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
    game_start_faction_turn(state, troop_groups, num_groups);
    
    // Check victory conditions
    if (game_check_victory_conditions(state, troop_groups, num_groups)) {
        state->game_over = true;
        state->current_phase = PHASE_GAME_OVER;
        
        Faction *winner = game_get_winner(state, troop_groups, num_groups);
        if (winner != NULL) {
            printf("\n=== VICTORY ===\n");
            printf("%s has won the game!\n", winner->name);
            state->winner = winner;
        }
    }
}

void game_end_current_turn(GameState *state, TroopGroup *troop_groups, int num_groups) {
    Faction *current_faction = game_get_current_faction(state);
    printf("Ending turn for %s\n", current_faction->name);
    
    // End all units' turns for current faction
    game_end_all_unit_turns(troop_groups, num_groups, current_faction);
    
    // Move to next turn
    game_next_turn(state, troop_groups, num_groups);
}

void game_start_faction_turn(GameState *state, TroopGroup *troop_groups, int num_groups) {
    Faction *current_faction = game_get_current_faction(state);
    
    // Reset all units for the current faction
    game_reset_faction_units(troop_groups, num_groups, current_faction);
    
    // Update phase
    if (state->current_faction_index == 0) {
        state->current_phase = PHASE_PLAYER_TURN;
    } else {
        state->current_phase = PHASE_ENEMY_TURN;
    }
}

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

void game_reset_faction_units(TroopGroup *troop_groups, int num_groups, Faction *faction) {
    for (int i = 0; i < num_groups; i++) {
        if (troop_groups[i].faction == faction) {
            actor_array_reset_turns(troop_groups[i].troops, troop_groups[i].count);
        }
    }
}

void game_end_all_unit_turns(TroopGroup *troop_groups, int num_groups, Faction *faction) {
    for (int i = 0; i < num_groups; i++) {
        if (troop_groups[i].faction == faction) {
            for (int j = 0; j < troop_groups[i].count; j++) {
                Actor *actor = &troop_groups[i].troops[j];
                if (actor_is_alive(actor)) {
                    actor_end_turn(actor);
                }
            }
        }
    }
}

bool game_faction_has_actions_remaining(TroopGroup *troop_groups, int num_groups, Faction *faction) {
    for (int i = 0; i < num_groups; i++) {
        if (troop_groups[i].faction == faction) {
            for (int j = 0; j < troop_groups[i].count; j++) {
                Actor *actor = &troop_groups[i].troops[j];
                if (actor_is_alive(actor) && actor_can_perform_action(actor)) {
                    return true;
                }
            }
        }
    }
    return false;
}

// ============================================================================
// Victory Condition Checking
// ============================================================================

bool game_check_victory_conditions(GameState *state, TroopGroup *troop_groups, int num_groups) {
    int factions_alive = 0;
    
    // Count how many factions still have units
    for (int i = 0; i < state->num_factions; i++) {
        if (!game_is_faction_defeated(troop_groups, num_groups, &state->factions[i])) {
            factions_alive++;
        }
    }
    
    // Victory if only one faction remains
    return factions_alive <= 1;
}

bool game_is_faction_defeated(TroopGroup *troop_groups, int num_groups, Faction *faction) {
    for (int i = 0; i < num_groups; i++) {
        if (troop_groups[i].faction == faction) {
            int alive_count = troop_group_count_alive(&troop_groups[i]);
            if (alive_count > 0) {
                return false; // Faction still has units alive
            }
        }
    }
    return true; // Faction has no units alive
}

Faction *game_get_winner(GameState *state, TroopGroup *troop_groups, int num_groups) {
    // Find the faction that still has units alive
    for (int i = 0; i < state->num_factions; i++) {
        if (!game_is_faction_defeated(troop_groups, num_groups, &state->factions[i])) {
            return &state->factions[i];
        }
    }
    return NULL; // No winner (shouldn't happen)
}

// ============================================================================
// Game State Queries
// ============================================================================

bool game_is_player_turn(GameState *state) {
    return state->current_phase == PHASE_PLAYER_TURN;
}

bool game_is_ai_turn(GameState *state) {
    return state->current_phase == PHASE_ENEMY_TURN;
}

bool game_is_over(GameState *state) {
    return state->game_over;
}

const char *game_get_phase_name(GamePhase phase) {
    switch (phase) {
        case PHASE_PLAYER_TURN: return "Player Turn";
        case PHASE_ENEMY_TURN: return "Enemy Turn";
        case PHASE_TURN_TRANSITION: return "Turn Transition";
        case PHASE_GAME_OVER: return "Game Over";
        case PHASE_VICTORY: return "Victory";
        default: return "Unknown";
    }
}

// ============================================================================
// Troop Group Utilities
// ============================================================================

TroopGroup troop_group_create(Actor *troops, int count, Faction *faction) {
    TroopGroup group;
    group.troops = troops;
    group.count = count;
    group.faction = faction;
    return group;
}

int troop_group_count_alive(TroopGroup *group) {
    return actor_array_count_alive(group->troops, group->count);
}
