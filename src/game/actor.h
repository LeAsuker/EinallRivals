#ifndef ACTOR_H_
#define ACTOR_H_

#include "types.h"
#include "raylib.h"
#include <stdbool.h>

// ============================================================================
// Genetics and Veterancy initialization
// ============================================================================

// Initialize genetics with random values (0-5 for each stat)
void genetics_init(Genetics *genetics);

// Initialize veterancy with all zeros
void veterancy_init(Veterancy *veterancy);

// ============================================================================
// Stats calculation
// ============================================================================

// Calculate total stats by summing class, genetics, and veterancy
Stats character_get_stats(Character *character);

// ============================================================================
// Character creation and initialization
// ============================================================================

Character *militia_create(Faction *owner, Texture2D sprite);
void militia_init(Character *character, Faction *owner, Texture2D sprite);
void character_free(Character *character);

// Character state management
void character_reset_turn_flags(Character *character);
void character_end_turn(Character *character);
bool character_can_perform_action(Character *character);
bool character_is_alive(Character *character);

// Character stats and leveling
void character_take_damage(Character *character, int damage);
void character_heal(Character *character, int amount);
void character_gain_experience(Character *character, int xp);
void character_level_up(Character *character);
bool character_has_pending_level_up(Character *character);

// Character queries
bool character_belongs_to_faction(Character *character, Faction *faction);
bool character_is_enemy(Character *char1, Character *char2);
int character_get_health_percentage(Character *character);

// ============================================================================
// Unit class system
// ============================================================================

// Default unit class templates (statically allocated, shared across characters)
extern UnitClass CLASS_MILITIA;
extern UnitClass CLASS_WARG;
extern UnitClass CLASS_SPEARMAN;
extern UnitClass CLASS_SWORDSMAN;

// Get pointer to default class templates
UnitClass *class_get_militia(void);
UnitClass *class_get_warg(void);

// ============================================================================
// Character arrays and groups
// ============================================================================

Character *character_array_create_from_class(int count, Faction *owner, Texture2D sprite, UnitClass *unit_class);
void character_array_free(Character *characters, int count);
void character_array_reset_turns(Character *characters, int count);
int character_array_count_alive(Character *characters, int count);

void character_init_from_class(Character *character, Faction *owner, 
                               Texture2D sprite, UnitClass *unit_class);

// Allocate and initialize a character from a class
Character *character_create_from_class(Faction *owner, Texture2D sprite, UnitClass *unit_class);

#endif
