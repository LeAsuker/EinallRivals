#include "game/game_logic.h"
#include "game/actor.h"
#include "game/map.h"
#include "game/combat.h"
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
        printf("Turn %d: %s's turn\n\n", state->turn_number, factions[state->current_faction_index].name);
    } else {
        printf("Turn %d: (no playable faction)\n\n", state->turn_number);
    }
}

// ============================================================================
// Turn Management
// ============================================================================

void game_next_turn(GameState *state, TroopGroup *troop_groups, int num_groups) {
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

void game_process_ai_turn(GameState *state, TroopGroup *troop_groups, int num_groups, Point *map, GridConfig *grid_config) {
    Faction *current = game_get_current_faction(state);
    if (current == NULL) return;

    // Find troop group for current faction
    TroopGroup *group = NULL;
    for (int i = 0; i < num_groups; i++) {
        if (troop_groups[i].faction == current) {
            group = &troop_groups[i];
            break;
        }
    }
    if (group == NULL) return;

    int total_cells = grid_config->max_grid_cells_x * grid_config->max_grid_cells_y;

    for (int i = 0; i < group->count; i++) {
        Actor *actor = &group->troops[i];
        if (!actor_is_alive(actor)) continue;
        if (!actor_can_perform_action(actor)) continue;

        // Locate actor's cell
        Point *actor_cell = NULL;
        for (int c = 0; c < total_cells; c++) {
            if (map[c].occupant == actor) {
                actor_cell = &map[c];
                break;
            }
        }
        if (actor_cell == NULL) continue;

        // Search for enemies in attack range; pick the closest
        Point *best_target = NULL;
        int best_dist = 999999;
        for (int c = 0; c < total_cells; c++) {
            if (map[c].occupant == NULL) continue;
            Actor *other = map[c].occupant;
            if (!actor_is_enemy(actor, other)) continue;
            if (combat_is_in_range(grid_config, actor_cell, &map[c], actor->range)) {
                int d = combat_get_distance(actor_cell, &map[c]);
                if (d < best_dist) {
                    best_dist = d;
                    best_target = &map[c];
                }
            }
        }

        if (best_target != NULL && actor->can_act) {
            combat_execute_at_cells(grid_config, map, actor_cell, best_target);
            // continue to next actor
            continue;
        }

        // No enemy in range: 40% chance to not move, else move one tile randomly
        int roll = rand() % 100;
        if (roll < 40 || !actor->can_move) {
            // do nothing
            continue;
        }

        // Try to move by one tile in a random direction (up/down/left/right)
        int dirs[4][2] = {{0,-1},{0,1},{-1,0},{1,0}};
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
            int nx = actor_cell->x + dirs[d][0];
            int ny = actor_cell->y + dirs[d][1];
            if (!map_is_valid_coords(grid_config, nx, ny)) continue;
            Point *dest = map_get_cell(map, grid_config, nx, ny);
            if (dest == NULL) continue;
            if (!map_can_unit_enter_cell(dest, actor)) continue;
            // Move actor
            dest->occupant = actor;
            actor_cell->occupant = NULL;
            actor->can_move = false;
            break;
        }
    }

    // After AI processes, end the faction's turn
    game_end_current_turn(state, troop_groups, num_groups);
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
