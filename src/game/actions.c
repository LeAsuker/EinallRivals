#include "game/actions.h"
#include "game/actor.h"
#include "game/combat.h"
#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief Free any dynamic memory held by a Skill instance.
 *
 * This will free the area_of_effect array if present and reset related
 * fields to safe defaults. Passing NULL is a no-op.
 *
 * @param skill Skill instance to cleanup (may be NULL).
 */
void skill_free(Skill *skill) {
  if (skill == NULL)
    return;
  if (skill->area_of_effect) {
    free(skill->area_of_effect);
    skill->area_of_effect = NULL;
    skill->aoe_size = 0;
  }
}

Skill spear_strike = {.name = "Spear Strike",
                      .id = 101,
                      .damage = -1, // Damage to be set dynamically
                      .is_magic = false,
                      .cooldown = 0,
                      .range = 1,
                      .area_of_effect = NULL,
                      .aoe_size = 0,
                      .icon = {0}};

/**
 * @brief Copy the predefined Spear Strike skill into `dest_skill`.
 *
 * @param dest_skill Destination Skill struct pointer (must be non-NULL).
 */
void action_copy_spear_strike(Skill *dest_skill) {
  if (dest_skill == NULL)
    return;
  memcpy(dest_skill, &spear_strike, sizeof(Skill));
  return;
}

Skill bite = {.name = "Bite",
              .id = 102,
              .damage = -1, // Damage to be set dynamically
              .is_magic = false,
              .cooldown = 0,
              .range = 1,
              .area_of_effect = NULL,
              .aoe_size = 0,
              .icon = {0}};

Skill Loot = {.name = "Loot",
              .id = 103,
              .damage = 0, // No damage
              .is_magic = false,
              .cooldown = 0,
              .range = 1,
              .area_of_effect = NULL,
              .aoe_size = 0,
              .icon = {0}};

/**
 * @brief Copy the predefined Loot skill into `dest_skill`.
 *
 * @param dest_skill Destination Skill struct pointer (must be non-NULL).
 */
void action_copy_loot(Skill *dest_skill) {
  if (dest_skill == NULL)
    return;
  memcpy(dest_skill, &Loot, sizeof(Skill));
  return;
}

/**
 * @brief Copy the predefined Bite skill into `dest_skill`.
 *
 * @param dest_skill Destination Skill struct pointer (must be non-NULL).
 */
void action_copy_bite(Skill *dest_skill) {
  if (dest_skill == NULL)
    return;
  memcpy(dest_skill, &bite, sizeof(Skill));
  return;
}

/**
 * @brief Load action-related icon textures. Must be called after the
 * window/renderer has been initialized.
 */
void actions_load_icons(void) {
  // Load spear strike icon
  spear_strike.icon =
      LoadTexture("../../resources/actions/spear_strike_icon.png");
  // If there were other action icons, load them similarly, e.g.:
  // bite.icon = LoadTexture("../../resources/actions/bite_icon.png");
}

/**
 * @brief Unload any textures loaded by actions_load_icons.
 *
 * Safe to call multiple times; checks texture ids before unloading.
 */
void actions_unload_icons(void) {
  if (spear_strike.icon.id)
    UnloadTexture(spear_strike.icon);
  if (bite.icon.id)
    UnloadTexture(bite.icon);
}

/**
 * @brief Set the effective damage on `skill` based on `owner` stats.
 *
 * If `skill->is_magic` is true, magic attack stat is used; otherwise
 * physical attack stat is used. Passing NULL is a no-op.
 *
 * @param skill Skill to adjust (must be non-NULL).
 * @param owner Character whose stats determine the damage.
 */
void action_set_damage(Skill *skill, Character *owner) {
  if (skill == NULL)
    return;
  if (owner == NULL)
    return;

  Stats stats = character_get_stats(owner);
  if (skill->is_magic) {
    // Example: magic damage scales with magic attack
    skill->damage = stats.magic_attack;
    return;
  }
  skill->damage = stats.phys_attack;
}

/**
 * @brief Add a copy of `skill` to a character's skill slots.
 *
 * Adjusts the skill damage based on the character's stats before adding.
 * No-op if `character` or `skill` is NULL or if the skill slots are full.
 *
 * @param character Character to receive the skill.
 * @param skill Skill prototype to add.
 */
void action_add_skill_to_character(Character *character, Skill *skill) {
  if (character == NULL || skill == NULL)
    return;
  if (character->skill_count >= 5)
    return; // Max skills reached
  action_set_damage(skill, character);
  character->skills[character->skill_count] = *skill;
  character->skill_count++;
}

/**
 * @brief Execute an offensive skill from an attacker cell onto a defender cell.
 *
 * Performs range checks, damage application, logs the action, grants
 * experience, and clears the defender from the map if killed. Preconditions
 * such as non-NULL pointers and occupant presence are verified; otherwise
 * the function returns without effect.
 *
 * @param grid_config Grid configuration (unused but kept for future checks).
 * @param map Pointer to map array (unused in current implementation).
 * @param attacker_cell Cell containing the attacker Character.
 * @param defender_cell Cell containing the defender Character.
 * @param skill Skill to execute (damage may be adjusted by action_set_damage).
 */
void execute_skill_at_cells(GridConfig *grid_config, Point *map,
                            Point *attacker_cell, Point *defender_cell,
                            Skill *skill) {
  if (attacker_cell == NULL || defender_cell == NULL || skill == NULL)
    return;
  if (attacker_cell->occupant == NULL || defender_cell->occupant == NULL)
    return;

  Character *attacker = attacker_cell->occupant;
  Character *defender = defender_cell->occupant;

  // Attacker must be able to act and target must be enemy
  if (!attacker->can_act)
    return;
  if (!character_is_enemy(attacker, defender))
    return;

  // Check range
  int dx = abs(attacker_cell->x - defender_cell->x);
  int dy = abs(attacker_cell->y - defender_cell->y);
  int dist = dx + dy;
  if (dist > skill->range)
    return;

  // Ensure skill damage is set
  if (skill->damage < 0)
    action_set_damage(skill, attacker);

  int damage = skill->damage;
  // Apply damage
  defender->curr_health -= damage;
  if (defender->curr_health < 0)
    defender->curr_health = 0;

  Stats defender_stats = character_get_stats(defender);
  printf("%s uses %s on %s for %d damage! (%s: %d/%d HP)\n", attacker->name,
         skill->name, defender->name, damage, defender->name,
         defender->curr_health, defender_stats.max_health);

  // Grant experience
  bool killed = false;
  if (defender->curr_health == 0) {
    killed = true;
    printf("%s has been defeated!\n", defender->name);
  }
  combat_grant_experience(attacker, defender, killed);

  // Update units and map
  if (killed) {
    // remove dead unit from map
    defender_cell->occupant = NULL;
  }

  // Attacker used their action
  attacker->can_act = false;
}

/**
 * @brief Execute a loot action from `looter_cell` against a lootable structure
 * cell.
 *
 * Validates input parameters and ensures the looter can act and the target
 * structure is lootable and within range. Grants example XP and marks the
 * structure as already looted. This function currently performs no item
 * transfer logic; extend as needed.
 *
 * @param grid_config Grid configuration (currently unused).
 * @param map Map array (currently unused).
 * @param looter_cell Cell containing the looter Character.
 * @param lootable Cell containing a lootable structure.
 * @param skill Skill used for looting (used for range and logging).
 */
void execute_loot_at_cells(GridConfig *grid_config, Point *map,
                           Point *looter_cell, Point *lootable, Skill *skill) {
  if (looter_cell == NULL || lootable == NULL || skill == NULL)
    return;
  if (looter_cell->occupant == NULL)
    return;

  Character *looter = looter_cell->occupant;
  // Looter must be able to act
  if (!looter->can_act)
    return;
  if (lootable->structure == NULL || !lootable->structure->lootable)
    return; // Must be a lootable structure to loot

  // Check range
  int dx = abs(looter_cell->x - lootable->x);
  int dy = abs(looter_cell->y - lootable->y);
  int dist = dx + dy;
  if (dist > skill->range)
    return;

  // Perform loot action
  printf("%s loots %s at (%d, %d)!\n", looter->name, skill->name, lootable->x,
         lootable->y);

  // Here you could add logic to give resources/items to the looter
  // Looter used their action
  character_gain_experience(looter, 100); // Example: gain some XP for looting
  if (looter->level_up_pending) {
    character_level_up(looter);
  }
  looter->can_act = false;
  lootable->structure->lootable = false; // Mark structure as looted
}