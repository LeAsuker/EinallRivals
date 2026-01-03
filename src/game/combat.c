#include "game/combat.h"
#include "game/actor.h"
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
    // New rule: battle skills are single-sided actions used during the actor's turn.
    // They hit deterministically and do raw physical attack equal to attacker's phys_attack.
    result.attacker_damage_dealt = attacker->phys_attack;
    result.defender_damage_dealt = 0;
    result.defender_can_counter = false;

    apply_combat_damage(defender, result.attacker_damage_dealt);

    printf("%s uses a battle skill on %s for %d damage! (%s: %d/%d HP)\n",
           attacker->name, defender->name, result.attacker_damage_dealt,
           defender->name, defender->curr_health, defender->max_health);

    // Check if defender died
    if (!actor_is_alive(defender)) {
        result.defender_died = true;
        printf("%s has been defeated!\n", defender->name);
        combat_grant_experience(attacker, defender, true);
        // Attacker used their action
        attacker->can_act = false;
        return result;
    }

    // Grant experience for the engagement (non-kill)
    combat_grant_experience(attacker, defender, false);

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
    // Single-sided skill forecast: attacker deals raw phys_attack, no counter.
    forecast.attacker_damage = attacker->phys_attack;
    forecast.defender_damage = 0;

    forecast.attacker_health_after = attacker->curr_health; // unchanged
    forecast.defender_health_after = defender->curr_health - forecast.attacker_damage;
    if (forecast.defender_health_after < 0) forecast.defender_health_after = 0;

    forecast.attacker_kills_defender = (forecast.defender_health_after == 0);
    forecast.defender_kills_attacker = false;
    forecast.defender_can_counter = false;

    // Deterministic skill: always hits, no crit by default
    forecast.hit_chance = 100;
    forecast.crit_chance = 0;

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
    if (distance > attacker->attack_range) {
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
    int base_damage = attacker->phys_attack - defender->phys_defense;
    
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
    return defender->attack_range >= distance;
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
