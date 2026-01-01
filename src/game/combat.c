#include "combat.h"
#include "actor.h"
#include <stdlib.h>
#include <stdio.h>

// Combat constants
#define BASE_HIT_CHANCE 80
#define BASE_CRIT_CHANCE 5
#define KILL_EXP_BONUS 20
#define DAMAGE_EXP_BASE 10

// Forward declarations for internal functions
static int calculate_hit_chance(Actor *attacker, Actor *defender);
static int calculate_crit_chance(Actor *attacker, Actor *defender);
static bool roll_hit(int hit_chance);
static bool roll_crit(int crit_chance);
static void apply_combat_damage(Actor *actor, int damage);

// ============================================================================
// Combat Execution
// ============================================================================

CombatResult combat_execute(Actor *attacker, Actor *defender) {
    CombatResult result = {0};
    result.attacker = attacker;
    result.defender = defender;
    result.attacker_damage_dealt = 0;
    result.defender_damage_dealt = 0;
    result.attacker_died = false;
    result.defender_died = false;
    result.was_critical = false;
    
    // Calculate if defender can counter
    result.defender_can_counter = (defender->range >= 1); // Simplified check
    
    // Attacker's attack
    int hit_chance = calculate_hit_chance(attacker, defender);
    if (roll_hit(hit_chance)) {
        int damage = combat_calculate_damage(attacker, defender, false);
        
        // Check for critical hit
        int crit_chance = calculate_crit_chance(attacker, defender);
        if (roll_crit(crit_chance)) {
            damage = damage * 3; // Critical hits do triple damage
            result.was_critical = true;
            printf("Critical hit! ");
        }
        
        result.attacker_damage_dealt = damage;
        apply_combat_damage(defender, damage);
        
        printf("%s attacks %s for %d damage! (%s: %d/%d HP)\n",
               attacker->name, defender->name, damage,
               defender->name, defender->curr_health, defender->max_health);
        
        // Check if defender died
        if (!actor_is_alive(defender)) {
            result.defender_died = true;
            printf("%s has been defeated!\n", defender->name);
            
            // Grant experience for kill
            combat_grant_experience(attacker, defender, true);
            
            // Attacker can act again after killing
            attacker->can_act = false;
            return result;
        }
    } else {
        printf("%s's attack missed!\n", attacker->name);
    }
    
    // Defender's counter-attack (if alive and in range)
    if (actor_is_alive(defender) && result.defender_can_counter) {
        hit_chance = calculate_hit_chance(defender, attacker);
        if (roll_hit(hit_chance)) {
            int damage = combat_calculate_damage(defender, attacker, false);
            
            result.defender_damage_dealt = damage;
            apply_combat_damage(attacker, damage);
            
            printf("%s counters for %d damage! (%s: %d/%d HP)\n",
                   defender->name, damage,
                   attacker->name, attacker->curr_health, attacker->max_health);
            
            // Check if attacker died from counter
            if (!actor_is_alive(attacker)) {
                result.attacker_died = true;
                printf("%s has been defeated by counter-attack!\n", attacker->name);
                return result;
            }
        } else {
            printf("%s's counter-attack missed!\n", defender->name);
        }
    }
    
    // Grant experience if no one died
    if (!result.defender_died) {
        combat_grant_experience(attacker, defender, false);
    }
    
    // Attacker has used their action
    attacker->can_act = false;
    
    return result;
}

CombatResult combat_execute_at_cells(GridConfig *grid_config, Point *map,
                                     Point *attacker_cell, Point *defender_cell) {
    CombatResult result = {0};
    
    // Validate cells have occupants
    if (attacker_cell->occupant == NULL || defender_cell->occupant == NULL) {
        fprintf(stderr, "Error: Cannot execute combat with empty cells\n");
        return result;
    }
    
    Actor *attacker = attacker_cell->occupant;
    Actor *defender = defender_cell->occupant;
    
    // Validate combat is possible
    if (!combat_can_attack(grid_config, map, attacker_cell, defender_cell)) {
        fprintf(stderr, "Error: Combat not possible between these cells\n");
        return result;
    }
    
    // Execute combat
    result = combat_execute(attacker, defender);
    
    // Remove dead units from map
    if (result.defender_died) {
        defender_cell->occupant = NULL;
    }
    if (result.attacker_died) {
        attacker_cell->occupant = NULL;
    }
    
    return result;
}

// ============================================================================
// Combat Prediction
// ============================================================================

CombatForecast combat_forecast(Actor *attacker, Actor *defender) {
    CombatForecast forecast = {0};
    
    // Calculate damages
    forecast.attacker_damage = combat_calculate_damage(attacker, defender, false);
    forecast.defender_damage = combat_calculate_damage(defender, attacker, false);
    
    // Calculate health after combat
    forecast.attacker_health_after = attacker->curr_health - forecast.defender_damage;
    forecast.defender_health_after = defender->curr_health - forecast.attacker_damage;
    
    // Clamp to 0
    if (forecast.attacker_health_after < 0) forecast.attacker_health_after = 0;
    if (forecast.defender_health_after < 0) forecast.defender_health_after = 0;
    
    // Determine if anyone dies
    forecast.attacker_kills_defender = (forecast.defender_health_after == 0);
    forecast.defender_kills_attacker = (forecast.attacker_health_after == 0);
    
    // Check if defender can counter
    forecast.defender_can_counter = (defender->range >= 1);
    
    // Calculate chances
    forecast.hit_chance = calculate_hit_chance(attacker, defender);
    forecast.crit_chance = calculate_crit_chance(attacker, defender);
    
    return forecast;
}

bool combat_can_attack(GridConfig *grid_config, Point *map,
                      Point *attacker_cell, Point *defender_cell) {
    // Check cells have occupants
    if (attacker_cell->occupant == NULL || defender_cell->occupant == NULL) {
        return false;
    }
    
    Actor *attacker = attacker_cell->occupant;
    Actor *defender = defender_cell->occupant;
    
    // Check attacker can act
    if (!attacker->can_act) {
        return false;
    }
    
    // Check they are enemies
    if (!actor_is_enemy(attacker, defender)) {
        return false;
    }
    
    // Check if defender is in range
    int distance = combat_get_distance(attacker_cell, defender_cell);
    if (distance > attacker->range) {
        return false;
    }
    
    return true;
}

// ============================================================================
// Damage Calculation
// ============================================================================

int combat_calculate_damage(Actor *attacker, Actor *defender, bool is_magic) {
    if (is_magic) {
        return combat_calculate_magical_damage(attacker, defender);
    } else {
        return combat_calculate_physical_damage(attacker, defender);
    }
}

int combat_calculate_physical_damage(Actor *attacker, Actor *defender) {
    int base_damage = attacker->attack - defender->defense;
    
    // Minimum damage is 1
    if (base_damage < 1) {
        base_damage = 1;
    }
    
    return base_damage;
}

int combat_calculate_magical_damage(Actor *attacker, Actor *defender) {
    int base_damage = attacker->magic_attack - defender->magic_defense;
    
    // Minimum damage is 1
    if (base_damage < 1) {
        base_damage = 1;
    }
    
    return base_damage;
}

// ============================================================================
// Combat Queries
// ============================================================================

bool combat_is_in_range(GridConfig *grid_config, Point *cell1, Point *cell2, int range) {
    (void)grid_config; // Unused, but kept for API consistency
    int distance = combat_get_distance(cell1, cell2);
    return distance <= range;
}

int combat_get_distance(Point *cell1, Point *cell2) {
    // Manhattan distance
    int dx = abs(cell1->x - cell2->x);
    int dy = abs(cell1->y - cell2->y);
    return dx + dy;
}

bool combat_can_counter_attack(Actor *attacker, Actor *defender, int distance) {
    // Defender can counter if their range reaches the attacker
    return defender->range >= distance;
}

// ============================================================================
// Experience and Rewards
// ============================================================================

int combat_calculate_experience(Actor *attacker, Actor *defender, bool killed) {
    (void)attacker; // Unused, but kept for future level-based XP calculation
    
    int base_xp = DAMAGE_EXP_BASE;
    
    // More XP for fighting higher level enemies
    base_xp += defender->level * 5;
    
    // Bonus XP for killing
    if (killed) {
        base_xp += KILL_EXP_BONUS;
    }
    
    return base_xp;
}

void combat_grant_experience(Actor *attacker, Actor *defender, bool killed) {
    int xp = combat_calculate_experience(attacker, defender, killed);
    
    printf("%s gained %d experience!\n", attacker->name, xp);
    actor_gain_experience(attacker, xp);
}

// ============================================================================
// Internal Helper Functions
// ============================================================================

static int calculate_hit_chance(Actor *attacker, Actor *defender) {
    (void)attacker;  // Future: Could use attacker->skill stat
    (void)defender;  // Future: Could use defender->speed/luck for evasion
    
    // For now, use base hit chance
    return BASE_HIT_CHANCE;
}

static int calculate_crit_chance(Actor *attacker, Actor *defender) {
    (void)attacker;  // Future: Could use attacker->luck/skill
    (void)defender;  // Future: Could use defender->luck to reduce crit chance
    
    // For now, use base crit chance
    return BASE_CRIT_CHANCE;
}

static bool roll_hit(int hit_chance) {
    int roll = rand() % 100;
    return roll < hit_chance;
}

static bool roll_crit(int crit_chance) {
    int roll = rand() % 100;
    return roll < crit_chance;
}

static void apply_combat_damage(Actor *actor, int damage) {
    actor_take_damage(actor, damage);
}
