#ifndef ACTOR_H_
#define ACTOR_H_

/*
 * Module: Actor (Character) System
 * Layer: Game Domain (src/game/)
 * Purpose: Character lifecycle, stats calculation, genetics, veterancy,
 * leveling, and class templates.
 * Dependencies: types.h
 * Consumers: main.c, game/faction_init.c, game/combat.c, game/game_logic.c
 * See AGENTS.md and src/CODEMAP.md for architecture context.
 */

#include "game/class_data.h"
#include "raylib.h"
#include "types.h"
#include <stdbool.h>

// Forward declaration for ActionIcons (defined in actions.h)
struct ActionIcons;
typedef struct ActionIcons ActionIcons;

// ============================================================================
// Genetics and Veterancy initialization
// ============================================================================

/**
 * @brief Initialize genetics with random values for each stat (0-5).
 * @param genetics Pointer to a Genetics struct to initialize.
 */
void genetics_init(Genetics *genetics);

/**
 * @brief Initialize veterancy values to zero for all stats.
 * @param veterancy Pointer to a Veterancy struct to initialize.
 */
void veterancy_init(Veterancy *veterancy);

// ============================================================================
// Stats calculation
// ============================================================================

/**
 * @brief Calculate the total stats for a character by summing class base
 *        stats with genetics and veterancy bonuses.
 * @param character Pointer to the character whose stats to calculate.
 * @return A Stats struct with computed values (zeros if character is NULL).
 */
Stats character_get_stats(Character *character);

// ============================================================================
// Character creation and initialization
// ============================================================================

/**
 * @brief Allocate and initialize a default militia character.
 * @param owner The owning faction for the character.
 * @param sprite Sprite texture for the character.
 * @param icons  Loaded action icons (may be NULL).
 * @return Pointer to a newly allocated Character, or NULL on allocation
 * failure.
 */
Character *militia_create(Faction *owner, Texture2D sprite,
                          const ActionIcons *icons);

/**
 * @brief Initialize an existing Character struct as a militia.
 * @param character The Character to initialize.
 * @param owner The owning faction for the character.
 * @param sprite Sprite texture for the character.
 * @param icons  Loaded action icons (may be NULL).
 */
void militia_init(Character *character, Faction *owner, Texture2D sprite,
                 const ActionIcons *icons);

/**
 * @brief Free resources owned by a Character and deallocate it.
 * @param character The Character to free (NULL-safe).
 */
void character_free(Character *character);

// Character state management

/**
 * @brief Reset per-turn action flags so the character can act again.
 * @param character The character to reset (NULL-safe).
 */
void character_reset_turn_flags(Character *character);

/**
 * @brief End the character's turn, disabling movement and actions.
 * @param character The character to end the turn for (NULL-safe).
 */
void character_end_turn(Character *character);

/**
 * @brief Check whether a character can perform any action this turn.
 * @param character The character to query.
 * @return true if the character can move or act, false otherwise.
 */
bool character_can_perform_action(Character *character);

/**
 * @brief Check whether a character is alive (health > 0).
 * @param character The character to query.
 * @return true if alive, false otherwise.
 */
bool character_is_alive(Character *character);

// Character stats and leveling

/**
 * @brief Apply damage to a character and handle death when health reaches 0.
 * @param character The character taking damage.
 * @param damage Amount of damage to apply (positive integer).
 */
void character_take_damage(Character *character, int damage);

/**
 * @brief Heal a character by a given amount (clamped to max health).
 * @param character The character to heal.
 * @param amount Amount of health to restore.
 */
void character_heal(Character *character, int amount);

/**
 * @brief Reduce the XP-to-next-level by xp and mark level-up if threshold
 * reached.
 * @param character The character gaining experience.
 * @param xp Amount of experience gained.
 */
void character_gain_experience(Character *character, int xp);

/**
 * @brief Perform a level-up: increase veterancy bonuses, heal, and set
 * next-level XP.
 * @param character The character leveling up.
 */
void character_level_up(Character *character);

/**
 * @brief Query whether the character has a pending manual level-up available.
 * @param character The character to query.
 * @return true if a level-up is pending, false otherwise.
 */
bool character_has_pending_level_up(Character *character);

// Character queries

/**
 * @brief Check if a character belongs to a given faction.
 * @param character The character to test.
 * @param faction The faction to compare against.
 * @return true if the character belongs to the faction, false otherwise.
 */
bool character_belongs_to_faction(Character *character, Faction *faction);

/**
 * @brief Check whether two characters are enemies (belong to different
 * factions).
 * @param char1 First character.
 * @param char2 Second character.
 * @return true if the characters belong to different factions.
 */
bool character_is_enemy(Character *char1, Character *char2);

/**
 * @brief Get the character's current health as a percentage of max health.
 * @param character The character to query.
 * @return Health percentage (0-100); 0 if character is NULL or max health is 0.
 */
int character_get_health_percentage(Character *character);

/**
 * @brief Get the maximum range among the character's skills.
 * @param character The character to inspect.
 * @return Maximum skill range (0 if no skills or character NULL).
 */
int character_get_max_skill_range(Character *character);

// ============================================================================
// Character arrays and groups
// ============================================================================

/**
 * @brief Allocate and initialize an array of characters from a UnitClass.
 * @param count Number of characters to allocate.
 * @param owner Owning faction for all characters.
 * @param sprite Sprite to assign to each character.
 * @param unit_class UnitClass template to base characters on.
 * @param icons  Loaded action icons (may be NULL).
 * @return Pointer to an allocated Character array or NULL on failure.
 */
Character *character_array_create_from_class(int count, Faction *owner,
                                             Texture2D sprite,
                                             const UnitClass *unit_class,
                                             const ActionIcons *icons);

/**
 * @brief Free an array of characters and their per-skill resources.
 * @param characters Array pointer (NULL-safe).
 * @param count Number of characters in the array.
 */
void character_array_free(Character *characters, int count);

/**
 * @brief Reset per-turn flags for each alive character in an array.
 * @param characters Array of characters.
 * @param count Number of characters.
 */
void character_array_reset_turns(Character *characters, int count);

/**
 * @brief Count how many characters in an array are alive.
 * @param characters Array of characters.
 * @param count Number of characters.
 * @return Number of alive characters.
 */
int character_array_count_alive(Character *characters, int count);

/**
 * @brief Initialize a character struct from a given UnitClass.
 * @param character Character to initialize.
 * @param owner Owning faction.
 * @param sprite Sprite to use.
 * @param unit_class The class template to initialize from.
 * @param icons  Loaded action icons (may be NULL).
 */
void character_init_from_class(Character *character, Faction *owner,
                               Texture2D sprite, const UnitClass *unit_class,
                               const ActionIcons *icons);

/**
 * @brief Allocate and initialize a character given a UnitClass template.
 * @param owner Owning faction.
 * @param sprite Sprite to use.
 * @param unit_class Unit class to base the character on.
 * @param icons  Loaded action icons (may be NULL).
 * @return Newly allocated Character or NULL on allocation failure.
 */
Character *character_create_from_class(Faction *owner, Texture2D sprite,
                                       const UnitClass *unit_class,
                                       const ActionIcons *icons);

/**
 * @brief Promote a character to a new class, updating its sprite and name.
 * @param character The character to promote.
 * @param new_class The new UnitClass template.
 * @param new_sprite Sprite texture for the promoted class.
 * @param icons Loaded action icons (may be NULL).
 */
void character_promote(Character *character, const UnitClass *new_class,
                       Texture2D new_sprite, const ActionIcons *icons);

#endif
