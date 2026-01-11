#ifndef COMBAT_H_
#define COMBAT_H_

#include "types.h"
#include "game/map.h"
#include <stdbool.h>

/**
 * @brief Outcome details for a single combat engagement.
 */
typedef struct {
    Character *attacker;
    Character *defender;
    int attacker_damage_dealt;
    int defender_damage_dealt;
    bool attacker_died;
    bool defender_died;
    bool was_critical;
    bool defender_can_counter;
} CombatResult;

/**
 * @brief Deterministic forecast of a combat engagement used for UI display.
 */
typedef struct {
    int attacker_damage;
    int defender_damage;
    int attacker_health_after;
    int defender_health_after;
    bool attacker_kills_defender;
    bool defender_kills_attacker;
    bool defender_can_counter;
    int hit_chance;
    int crit_chance;
} CombatForecast;

// Combat execution
CombatResult combat_execute(Character *attacker, Character *defender);
CombatResult combat_execute_at_cells(GridConfig *grid_config, Point *map, 
                                     Point *attacker_cell, Point *defender_cell);

// Combat prediction (for UI display)
CombatForecast combat_forecast(Character *attacker, Character *defender);
bool combat_can_attack(GridConfig *grid_config, Point *map, 
                      Point *attacker_cell, Point *defender_cell);

// Damage calculation
int combat_calculate_damage(Character *attacker, Character *defender, bool is_magic);
int combat_calculate_physical_damage(Character *attacker, Character *defender);
int combat_calculate_magical_damage(Character *attacker, Character *defender);

// Combat queries
bool combat_is_in_range(GridConfig *grid_config, Point *cell1, Point *cell2, int range);
int combat_get_distance(Point *cell1, Point *cell2);
bool combat_can_counter_attack(Character *attacker, Character *defender, int distance);

// Experience and rewards
int combat_calculate_experience(Character *attacker, Character *defender, bool killed);
void combat_grant_experience(Character *attacker, Character *defender, bool killed);

#endif
