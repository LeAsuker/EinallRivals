#ifndef SKILL_DATA_H_
#define SKILL_DATA_H_

#include "types.h"

// Forward declaration for ActionIcons (defined in actions.h)
struct ActionIcons;
typedef struct ActionIcons ActionIcons;

#define SKILL_ID_LOOT 103

/**
 * @brief Populate @p dest_skill with the predefined Spear Strike data.
 *
 * The icon field is patched from @p icons if non-NULL.
 *
 * @param dest_skill Destination Skill struct to initialize.
 * @param icons      Loaded action icons (may be NULL for no icon).
 */
void action_copy_spear_strike(Skill *dest_skill, const ActionIcons *icons);

/**
 * @brief Populate @p dest_skill with the predefined Bite skill data.
 *
 * The icon field is patched from @p icons if non-NULL.
 *
 * @param dest_skill Destination Skill struct to initialize.
 * @param icons      Loaded action icons (may be NULL for no icon).
 */
void action_copy_bite(Skill *dest_skill, const ActionIcons *icons);

/**
 * @brief Populate @p dest_skill with the predefined Loot skill data.
 * @param dest_skill Destination Skill struct to initialize.
 */
void action_copy_loot(Skill *dest_skill);

#endif
