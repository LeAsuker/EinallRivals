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
static int calculate_hit_chance(Character *attacker, Character *defender);
static int calculate_crit_chance(Character *attacker, Character *defender);
static bool roll_hit(int hit_chance);
static bool roll_crit(int crit_chance);
static void apply_combat_damage(Character *character, int damage);

// ============================================================================
// Combat Execution
// ============================================================================

/**
 * @brief Execute a single combat engagement between two characters.
 *
 * This simplified combat model treats battle skills as single-sided
 * deterministic actions that deal the attacker's physical attack as damage.
 * The function applies damage, logs events, grants experience, and toggles
 * the attacker's action availability.
 *
 * @param attacker Character performing the attack.
 * @param defender Character receiving the attack.
 * @return CombatResult describing damage, deaths, and other outcomes.
 */
CombatResult combat_execute(Character *attacker, Character *defender) {
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
    Stats attacker_stats = character_get_stats(attacker);
    Stats defender_stats = character_get_stats(defender);
    result.attacker_damage_dealt = attacker_stats.phys_attack;
    result.defender_damage_dealt = 0;
    result.defender_can_counter = false;

    apply_combat_damage(defender, result.attacker_damage_dealt);

    printf("%s uses a battle skill on %s for %d damage! (%s: %d/%d HP)\n",
           attacker->name, defender->name, result.attacker_damage_dealt,
           defender->name, defender->curr_health, defender_stats.max_health);

    // Check if defender died
    if (!character_is_alive(defender)) {
        result.defender_died = true;
        printf("%s has been defeated!\n", defender->name);
        combat_grant_experience(attacker, defender, true);
        // Attacker used their action
        attacker->can_act = false;
        return result;
    }

    // Grant experience for the engagement (non-kill)
    combat_grant_experience(attacker, defender, false);

    if (attacker->level_up_pending) {
        character_level_up(attacker);
    }


    // Attacker has used their action
    attacker->can_act = false;

    return result;
}

/**
 * @brief Execute combat between occupants of two map cells.
 *
 * Validates occupancy and that combat is possible via combat_can_attack,
 * then delegates to combat_execute and cleans up map occupants of dead units.
 *
 * @param grid_config Grid configuration.
 * @param map Map array.
 * @param attacker_cell Cell containing the attacker.
 * @param defender_cell Cell containing the defender.
 * @return CombatResult of the engagement.
 */
CombatResult combat_execute_at_cells(GridConfig *grid_config, Point *map,
                                     Point *attacker_cell, Point *defender_cell) {
    CombatResult result = {0};
    
    // Validate cells have occupants
    if (attacker_cell->occupant == NULL || defender_cell->occupant == NULL) {
        fprintf(stderr, "Error: Cannot execute combat with empty cells\n");
        return result;
    }
    
    Character *attacker = attacker_cell->occupant;
    Character *defender = defender_cell->occupant;
    
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

/**
 * @brief Produce a deterministic forecast for a single-sided skill attack.
 *
 * Returns expected damage and post-engagement HP values without randomness.
 *
 * @param attacker Attacking character.
 * @param defender Defending character.
 * @return CombatForecast summarizing expected outcomes.
 */
CombatForecast combat_forecast(Character *attacker, Character *defender) {
    CombatForecast forecast = {0};
    // Single-sided skill forecast: attacker deals raw phys_attack, no counter.
    Stats attacker_stats = character_get_stats(attacker);
    forecast.attacker_damage = attacker_stats.phys_attack;
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

/**
 * @brief Determine whether an attacker cell can perform an attack on a defender cell.
 *
 * Validates occupancy, that the attacker is able to act, that the two
 * characters are enemies, and that the defender is within the maximum
 * attack range of the attacker.
 *
 * @param grid_config Grid configuration (unused currently).
 * @param map Map array (unused currently).
 * @param attacker_cell Cell containing the attacker.
 * @param defender_cell Cell containing the defender.
 * @return true if an attack can be initiated; false otherwise.
 */
bool combat_can_attack(GridConfig *grid_config, Point *map,
                      Point *attacker_cell, Point *defender_cell) {
    // Check cells have occupants
    if (attacker_cell->occupant == NULL || defender_cell->occupant == NULL) {
        return false;
    }
    
    Character *attacker = attacker_cell->occupant;
    Character *defender = defender_cell->occupant;
    
    // Check attacker can act
    if (!attacker->can_act) {
        return false;
    }
    
    // Check they are enemies
    if (!character_is_enemy(attacker, defender)) {
        return false;
    }
    
    // Check if defender is in range using attacker's maximum skill range
    int max_range = character_get_max_skill_range(attacker);
    int distance = combat_get_distance(attacker_cell, defender_cell);
    if (distance > max_range) {
        return false;
    }
    
    return true;
}

// ============================================================================
// Damage Calculation
// ============================================================================

/**
 * @brief Compute damage for an interaction, delegating to physical or magical formulas.
 *
 * @param attacker Attacking character.
 * @param defender Defending character.
 * @param is_magic True to use magic damage; false for physical.
 * @return Damage amount (minimum 1).
 */
int combat_calculate_damage(Character *attacker, Character *defender, bool is_magic) {
    if (is_magic) {
        return combat_calculate_magical_damage(attacker, defender);
    } else {
        return combat_calculate_physical_damage(attacker, defender);
    }
}

/**
 * @brief Calculate physical damage as attacker's phys_attack minus defender phys_defense.
 * Ensures a minimum of 1 damage.
 */
int combat_calculate_physical_damage(Character *attacker, Character *defender) {
    Stats attacker_stats = character_get_stats(attacker);
    Stats defender_stats = character_get_stats(defender);
    int base_damage = attacker_stats.phys_attack - defender_stats.phys_defense;
    
    // Minimum damage is 1
    if (base_damage < 1) {
        base_damage = 1;
    }
    
    return base_damage;
}

/**
 * @brief Calculate magical damage as attacker's magic_attack minus defender magic_defense.
 * Ensures a minimum of 1 damage.
 */
int combat_calculate_magical_damage(Character *attacker, Character *defender) {
    Stats attacker_stats = character_get_stats(attacker);
    Stats defender_stats = character_get_stats(defender);
    int base_damage = attacker_stats.magic_attack - defender_stats.magic_defense;
    
    // Minimum damage is 1
    if (base_damage < 1) {
        base_damage = 1;
    }
    
    return base_damage;
}
// ============================================================================
// Combat Queries
// ============================================================================

/**
 * @brief Check whether two cells are within a given range.
 *
 * @param grid_config Grid configuration (kept for compatibility; unused).
 * @param cell1 First cell.
 * @param cell2 Second cell.
 * @param range Range limit.
 * @return true if the Manhattan distance <= range.
 */
bool combat_is_in_range(GridConfig *grid_config, Point *cell1, Point *cell2, int range) {
    (void)grid_config; // Unused, but kept for API consistency
    int distance = combat_get_distance(cell1, cell2);
    return distance <= range;
}

/**
 * @brief Compute Manhattan distance between two cells.
 */
int combat_get_distance(Point *cell1, Point *cell2) {
    // Manhattan distance
    int dx = abs(cell1->x - cell2->x);
    int dy = abs(cell1->y - cell2->y);
    return dx + dy;
}

/**
 * @brief Whether defender can counter-attack an attacker at the given distance.
 *
 * Uses the defender's maximum skill range to determine counter capability.
 */
bool combat_can_counter_attack(Character *attacker, Character *defender, int distance) {
    // Defender can counter if their maximum skill range reaches the attacker
    int max_range = character_get_max_skill_range(defender);
    return max_range >= distance;
}
// ============================================================================
// Experience and Rewards
// ============================================================================

/**
 * @brief Calculate experience reward for defeating or damaging an enemy.
 *
 * Currently uses a simple formula: base damage XP plus bonus based on
 * defender level and a kill bonus when applicable.
 */
int combat_calculate_experience(Character *attacker, Character *defender, bool killed) {
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

/**
 * @brief Grant calculated experience to the attacker and print a log message.
 */
void combat_grant_experience(Character *attacker, Character *defender, bool killed) {
    int xp = combat_calculate_experience(attacker, defender, killed);
    
    printf("%s gained %d experience!\n", attacker->name, xp);
    character_gain_experience(attacker, xp);
}

// ============================================================================
// Internal Helper Functions
// ============================================================================

/**
 * @brief Calculate hit chance for an attacker against a defender.
 */
static int calculate_hit_chance(Character *attacker, Character *defender) {
    (void)attacker;  // Future: Could use attacker->skill stat
    (void)defender;  // Future: Could use defender->speed/luck for evasion
    
    // For now, use base hit chance
    return BASE_HIT_CHANCE;
}

/**
 * @brief Calculate critical hit chance for an attacker against a defender.
 */
static int calculate_crit_chance(Character *attacker, Character *defender) {
    (void)attacker;  // Future: Could use attacker->luck/skill
    (void)defender;  // Future: Could use defender->luck to reduce crit chance
    
    // For now, use base crit chance
    return BASE_CRIT_CHANCE;
}

/**
 * @brief Roll for a hit given a percentage chance (0-99).
 */
static bool roll_hit(int hit_chance) {
    int roll = rand() % 100;
    return roll < hit_chance;
}

/**
 * @brief Roll for a critical hit given percentage chance (0-99).
 */
static bool roll_crit(int crit_chance) {
    int roll = rand() % 100;
    return roll < crit_chance;
}

/**
 * @brief Apply damage to a character using the public character_take_damage helper.
 */
static void apply_combat_damage(Character *character, int damage) {
    character_take_damage(character, damage);
}
