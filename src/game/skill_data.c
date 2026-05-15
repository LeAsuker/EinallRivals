#include "game/skill_data.h"
#include "game/actions.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ============================================================================
// Default Skill Definitions
// ============================================================================

static const Skill spear_strike = {.name = "Spear Strike",
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
 * If @p icons is non-NULL, the spear_strike icon texture is patched in.
 *
 * @param dest_skill Destination Skill struct pointer (must be non-NULL).
 * @param icons      Loaded action icons (may be NULL).
 */
void action_copy_spear_strike(Skill *dest_skill, const ActionIcons *icons) {
  NULL_CHECK_VOID(dest_skill);
  memcpy(dest_skill, &spear_strike, sizeof(Skill));
  if (icons != NULL) {
    dest_skill->icon = icons->spear_strike;
  }
  return;
}

static const Skill bite = {.name = "Bite",
              .id = 102,
              .damage = -1, // Damage to be set dynamically
              .is_magic = false,
              .cooldown = 0,
              .range = 1,
              .area_of_effect = NULL,
              .aoe_size = 0,
              .icon = {0}};

static const Skill Loot = {.name = "Loot",
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
  NULL_CHECK_VOID(dest_skill);
  memcpy(dest_skill, &Loot, sizeof(Skill));
  return;
}

/**
 * @brief Copy the predefined Bite skill into `dest_skill`.
 *
 * If @p icons is non-NULL, the bite icon texture is patched in.
 *
 * @param dest_skill Destination Skill struct pointer (must be non-NULL).
 * @param icons      Loaded action icons (may be NULL).
 */
void action_copy_bite(Skill *dest_skill, const ActionIcons *icons) {
  NULL_CHECK_VOID(dest_skill);
  memcpy(dest_skill, &bite, sizeof(Skill));
  if (icons != NULL) {
    dest_skill->icon = icons->bite;
  }
  return;
}
