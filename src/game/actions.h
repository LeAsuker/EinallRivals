#ifndef ACTIONS_H_
#define ACTIONS_H_

#include "raylib.h"
#include "types.h"
#include <stdbool.h>

/**
 * @brief Populate @p dest_skill with the predefined Spear Strike data.
 * @param dest_skill Destination Skill struct to initialize.
 */
void action_copy_spear_strike(Skill *dest_skill);

/**
 * @brief Populate @p dest_skill with the predefined Bite skill data.
 * @param dest_skill Destination Skill struct to initialize.
 */
void action_copy_bite(Skill *dest_skill);

/**
 * @brief Populate @p dest_skill with the predefined Loot skill data.
 * @param dest_skill Destination Skill struct to initialize.
 */
void action_copy_loot(Skill *dest_skill);

/**
 * @brief Free any resources owned by a Skill (e.g., AoE arrays).
 * @param skill Skill to free (NULL-safe).
 */
void skill_free(Skill *skill);

/**
 * @brief Set the damage of a Skill based on an owning Character's stats.
 * @param skill Skill to modify.
 * @param owner Character that owns the skill (used for scaling).
 */
void action_set_damage(Skill *skill, Character *owner);

/**
 * @brief Add a Skill to a Character's skill list (if slots available).
 * @param character Character to receive the skill.
 * @param skill Skill to add (copied into character's slot).
 */
void action_add_skill_to_character(Character *character, Skill *skill);

/**
 * @brief Load shared icons/textures used by action UI elements.
 */
void actions_load_icons(void);

/**
 * @brief Unload any textures loaded by actions_load_icons.
 */
void actions_unload_icons(void);

/**
 * @brief Execute a combat skill from attacker_cell onto defender_cell.
 * This applies damage, plays animations/effects, and grants experience.
 * @param grid_config Grid configuration for positional calculations.
 * @param map Map cell array.
 * @param attacker_cell Cell containing the attacker.
 * @param defender_cell Target cell for the skill.
 * @param skill Skill being used.
 */
void execute_skill_at_cells(GridConfig *grid_config, Point *map, Point *attacker_cell, Point *defender_cell, Skill *skill);

/**
 * @brief Execute a loot skill: attempt to loot a structure on the target cell.
 * @param grid_config Grid configuration for positional calculations.
 * @param map Map cell array.
 * @param looter_cell Cell containing the looter.
 * @param lootable Target cell that may contain a lootable structure.
 * @param skill Loot skill being used.
 */
void execute_loot_at_cells(GridConfig *grid_config, Point *map, Point *looter_cell, Point *lootable, Skill *skill);
#endif