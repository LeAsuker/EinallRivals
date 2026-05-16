#include "game/actor.h"
#include "game/actions.h"
#include "game/archetype.h"
#include "game/class_data.h"
#include "game/item.h"
#include "game/skill_data.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ============================================================================
// Genetics and Veterancy Initialization
// ============================================================================

/**
 * @brief Initialize genetics with random small bonuses (0-5) for each stat.
 * @param genetics Pointer to Genetics to initialize (NULL-safe).
 */
void genetics_init(Genetics *genetics) {
  NULL_CHECK_VOID(genetics);
  genetics->max_health = rand() % 6; // 0-5
  genetics->movement = rand() % 6;
  genetics->phys_attack = rand() % 6;
  genetics->phys_defense = rand() % 6;
  genetics->magic_attack = rand() % 6;
  genetics->magic_defense = rand() % 6;
  genetics->luck = rand() % 6;
}

/**
 * @brief Zero out veterancy (permanent stat bonuses from leveling).
 * @param veterancy Pointer to Veterancy to initialize (NULL-safe).
 */
void veterancy_init(Veterancy *veterancy) {
  NULL_CHECK_VOID(veterancy);
  veterancy->max_health = 0;
  veterancy->movement = 0;
  veterancy->phys_attack = 0;
  veterancy->phys_defense = 0;
  veterancy->magic_attack = 0;
  veterancy->magic_defense = 0;
  veterancy->luck = 0;
}

// ============================================================================
// Stats Calculation
// ============================================================================

/**
 * @brief Compute aggregate stats for a character by summing class, genetics,
 * and veterancy.
 * @param character Character to compute stats for.
 * @return Stats struct with computed values; zeros if character is NULL.
 */
Stats character_get_stats(Character *character) {
  Stats stats = {0};
  NULL_CHECK_RET(character, stats);

  const UnitClass *cls = character->unit_class;
  Genetics *gen = &character->genetics;
  Veterancy *vet = &character->veterancy;

  if (cls != NULL) {
    stats.max_health = cls->max_health + gen->max_health + vet->max_health;
    stats.movement = cls->movement + gen->movement + vet->movement;
    stats.phys_attack = cls->phys_attack + gen->phys_attack + vet->phys_attack;
    stats.phys_defense =
        cls->phys_defense + gen->phys_defense + vet->phys_defense;
    stats.magic_attack =
        cls->magic_attack + gen->magic_attack + vet->magic_attack;
    stats.magic_defense =
        cls->magic_defense + gen->magic_defense + vet->magic_defense;
    stats.luck = cls->luck + gen->luck + vet->luck;
  }

  // Add equipped item stat bonuses
  for (int i = 0; i < character->equipment_count; i++) {
    Item *item = character->equipment[i].item;
    if (item != NULL) {
      stats.max_health += item->stats.max_health;
      stats.movement += item->stats.movement;
      stats.phys_attack += item->stats.phys_attack;
      stats.phys_defense += item->stats.phys_defense;
      stats.magic_attack += item->stats.magic_attack;
      stats.magic_defense += item->stats.magic_defense;
      stats.luck += item->stats.luck;
    }
  }

  return stats;
}

static void character_give_starting_items(Character *character);

// ============================================================================
// Character base initialization (shared across all class-specific inits)
// ============================================================================

/**
 * @brief Initialize the common fields of a Character from a UnitClass template
 *        and an Archetype.
 *
 * Sets sprite, owner, action flags, level, genetics, veterancy, health, name,
 * archetype, and equipment slots. Does NOT add any skills; callers must do that.
 *
 * @param character Character struct to initialize.
 * @param owner Owning faction.
 * @param sprite Sprite texture.
 * @param unit_class UnitClass template to use.
 * @param archetype Archetype that determines equipment slot layout.
 */
static void character_init_base(Character *character, Faction *owner,
                                Texture2D sprite, const UnitClass *unit_class,
                                Archetype archetype) {
  fprintf(stderr, "[DEBUG] character_init_base start\n");
  NULL_CHECK_VOID(character);
  NULL_CHECK_VOID(owner);
  NULL_CHECK_VOID(unit_class);
  character->sprite = sprite;
  character->owner = owner;
  character->can_move = true;
  character->can_act = true;
  character->level = 1;
  character->next_level_xp = 100;
  character->level_up_pending = false;
  character->unit_class = unit_class;
  character->archetype = archetype;
  genetics_init(&character->genetics);
  veterancy_init(&character->veterancy);

  // Initialize equipment slots BEFORE calling character_get_stats,
  // so the stats function sees a safe equipment_count.
  const ArchetypeSlotConfig *config = archetype_get_slot_config(archetype);
  if (config != NULL) {
    character->equipment_count = config->slot_count;
    for (int i = 0; i < MAX_EQUIP_SLOTS; i++) {
      if (i < config->slot_count) {
        character->equipment[i].type = config->slots[i];
      } else {
        character->equipment[i].type = EQUIP_SLOT_NONE;
      }
      character->equipment[i].item = NULL;
    }
  } else {
    character->equipment_count = 0;
    for (int i = 0; i < MAX_EQUIP_SLOTS; i++) {
      character->equipment[i].type = EQUIP_SLOT_NONE;
      character->equipment[i].item = NULL;
    }
  }

  // Initialize inventory slots
  for (int i = 0; i < INVENTORY_SIZE; i++) {
    character->inventory.items[i] = NULL;
  }

  Stats stats = character_get_stats(character);
  character->curr_health = stats.max_health;
  strcpy(character->name, unit_class->name);
  character->skill_count = 0;

  // Give starting items so the inventory system is usable from the start
  character_give_starting_items(character);
  fprintf(stderr, "[DEBUG] character_init_base done\n");
}

// ============================================================================
// Starting Items Helper
// ============================================================================

static void character_give_starting_items(Character *character) {
  fprintf(stderr, "[DEBUG] character_give_starting_items start\n");
  NULL_CHECK_VOID(character);
  Stats potion_stats = {1, 0, 0, 0, 0, 0, 0};
  fprintf(stderr, "[DEBUG] creating potion\n");
  Item *potion =
      item_create(100, "Small Potion", ITEM_TYPE_TRINKET, potion_stats);
  fprintf(stderr, "[DEBUG] potion=%p\n", (void *)potion);
  if (potion != NULL)
    character_add_item_to_inventory(character, potion);

  if (character->archetype == ARCHETYPE_WARRIOR) {
    fprintf(stderr, "[DEBUG] creating sword\n");
    Stats sword_stats = {0, 0, 1, 0, 0, 0, 0};
    Item *sword =
        item_create(101, "Rusty Sword", ITEM_TYPE_MELEE_WEAPON, sword_stats);
    fprintf(stderr, "[DEBUG] sword=%p\n", (void *)sword);
    if (sword != NULL)
      character_add_item_to_inventory(character, sword);

    fprintf(stderr, "[DEBUG] creating armor\n");
    Stats armor_stats = {2, 0, 0, 1, 0, 0, 0};
    Item *armor =
        item_create(102, "Leather Armor", ITEM_TYPE_ARMOR, armor_stats);
    fprintf(stderr, "[DEBUG] armor=%p\n", (void *)armor);
    if (armor != NULL)
      character_add_item_to_inventory(character, armor);
  }
  fprintf(stderr, "[DEBUG] character_give_starting_items done\n");
}

// ============================================================================
// Character Creation and Initialization
// ============================================================================

/**
 * @brief Allocate and initialize a militia character.
 * @param owner Owning faction for the character.
 * @param sprite Sprite texture for the character.
 * @return Pointer to newly allocated Character or NULL on allocation failure.
 */
Character *militia_create(Faction *owner, Texture2D sprite,
                          const ActionIcons *icons) {
  Character *character = malloc(sizeof(Character));
  if (character == NULL) {
    fprintf(stderr, "Error: Failed to allocate memory for character\n");
    return NULL;
  }

  militia_init(character, owner, sprite, icons);
  return character;
}

/**
 * @brief Allocate and initialize a character from an Archetype.
 *
 * The base UnitClass is resolved automatically from the archetype.
 *
 * @param owner Owning faction for the character.
 * @param sprite Sprite texture for the character.
 * @param archetype Archetype that determines base class and equipment slots.
 * @return Newly allocated Character or NULL on allocation failure.
 */
Character *character_create_from_class(Faction *owner, Texture2D sprite,
                                       Archetype archetype,
                                       const ActionIcons *icons) {
  Character *character = malloc(sizeof(Character));
  if (character == NULL) {
    fprintf(stderr, "Error: Failed to allocate memory for character\n");
    return NULL;
  }
  const UnitClass *base_class = archetype_get_base_class(archetype);
  if (base_class == NULL) {
    free(character);
    return NULL;
  }
  character_init_from_class(character, owner, sprite, base_class, archetype,
                            icons);
  return character;
}

/**
 * @brief Initialize a Character as a militia: set class, random genetics, and
 * default skills.
 * @param character Character to initialize (NULL-safe via NULL_CHECK_VOID).
 * @param owner Owning faction for the character.
 * @param sprite Sprite texture for the character.
 */
void militia_init(Character *character, Faction *owner, Texture2D sprite,
                  const ActionIcons *icons) {
  fprintf(stderr, "[DEBUG] militia_init start\n");
  character_init_base(character, owner, sprite, &CLASS_MILITIA,
                      ARCHETYPE_WARRIOR);
  fprintf(stderr, "[DEBUG] militia_init: adding skills\n");
  // Add militia-specific skills
  Skill tmp;
  action_copy_loot(&tmp);
  action_add_skill_to_character(character, &tmp);
  action_copy_spear_strike(&tmp, icons);
  action_add_skill_to_character(character, &tmp);
  fprintf(stderr, "[DEBUG] militia_init done\n");
}

/**
 * @brief Initialize a Character as a warg: set class, random genetics, and
 * default skills.
 * @param character Character to initialize (NULL-safe via NULL_CHECK_VOID).
 * @param owner Owning faction for the character.
 * @param sprite Sprite texture for the character.
 */
void warg_init(Character *character, Faction *owner, Texture2D sprite,
               const ActionIcons *icons) {
  character_init_base(character, owner, sprite, &CLASS_WARG, ARCHETYPE_WARG);
  // Add warg-specific skills
  Skill tmp;
  action_copy_bite(&tmp, icons);
  action_add_skill_to_character(character, &tmp);
}

/**
 * @brief Initialize a Character based on a given UnitClass and Archetype.
 *
 * If the UnitClass name matches a known template (Militia, Warg), the
 * corresponding class-specific init helper is used.
 *
 * @param character Character struct to initialize (NULL-safe via NULL_CHECK_VOID).
 * @param owner Owning faction for the character.
 * @param sprite Sprite texture to assign.
 * @param unit_class UnitClass template to initialize from.
 * @param archetype Archetype that determines equipment slots.
 */
void character_init_from_class(Character *character, Faction *owner,
                               Texture2D sprite, const UnitClass *unit_class,
                               Archetype archetype,
                               const ActionIcons *icons) {
  NULL_CHECK_VOID(character);
  NULL_CHECK_VOID(owner);
  NULL_CHECK_VOID(unit_class);
  if (strcmp(unit_class->name, "Militia") == 0) {
    militia_init(character, owner, sprite, icons); // Add default militia skills
  } else if (strcmp(unit_class->name, "Warg") == 0) {
    warg_init(character, owner, sprite, icons); // Add default warg skills
  } else {
    character_init_base(character, owner, sprite, unit_class, archetype);
    // Generic classes start with no skills
  }
}

// ============================================================================
// Character Resource Cleanup
// ============================================================================

/**
 * @brief Free all heap-allocated resources owned by a character (skills,
 *        equipped items, inventory items) without freeing the struct itself.
 * @param character Character whose resources should be released.
 */
static void character_cleanup_resources(Character *character) {
  if (character == NULL) {
    return;
  }
  // Free any per-skill allocations
  for (int i = 0; i < character->skill_count; i++) {
    skill_free(&character->skills[i]);
  }
  character->skill_count = 0;
  // Free equipped items
  for (int i = 0; i < character->equipment_count; i++) {
    if (character->equipment[i].item != NULL) {
      item_free(character->equipment[i].item);
      character->equipment[i].item = NULL;
    }
  }
  character->equipment_count = 0;
  // Free inventory items
  for (int i = 0; i < INVENTORY_SIZE; i++) {
    if (character->inventory.items[i] != NULL) {
      item_free(character->inventory.items[i]);
      character->inventory.items[i] = NULL;
    }
  }
}

// ============================================================================
// Character Creation and Initialization
// ============================================================================

/**
 * @brief Free resources owned by a Character and deallocate it (NULL-safe).
 * @param character Character to free.
 */
void character_free(Character *character) {
  NULL_CHECK_VOID(character);
  character_cleanup_resources(character);
  free(character);
}

// ============================================================================
// Character State Management
// ============================================================================

/**
 * @brief Reset per-turn flags so the character can move and act again.
 * @param character Character to reset (NULL-safe).
 */
void character_reset_turn_flags(Character *character) {
  NULL_CHECK_VOID(character);
  character->can_move = true;
  character->can_act = true;
}

/**
 * @brief Mark the character as having finished its turn (cannot move or act).
 * @param character Character to update (NULL-safe).
 */
void character_end_turn(Character *character) {
  NULL_CHECK_VOID(character);
  character->can_move = false;
  character->can_act = false;
}

/**
 * @brief Query whether the character can perform any action this turn.
 * @param character Character to query.
 * @return true if the character can move or act, false otherwise.
 */
bool character_can_perform_action(Character *character) {
  return character->can_move || character->can_act;
}

/**
 * @brief Check if character is alive (current health > 0).
 * @param character Character to inspect.
 * @return true if alive.
 */
bool character_is_alive(Character *character) {
  return character->curr_health > 0;
}

// ============================================================================
// Character Stats and Leveling
// ============================================================================

/**
 * @brief Apply damage to a character and clamp health to zero; print defeat
 * message on death.
 * @param character Character receiving damage (NULL-safe via NULL_CHECK_VOID).
 * @param damage Amount of damage to apply.
 */
void character_take_damage(Character *character, int damage) {
  NULL_CHECK_VOID(character);
  character->curr_health -= damage;

  // Clamp health to 0 minimum
  if (character->curr_health < 0) {
    character->curr_health = 0;
  }

  // Log death
  if (!character_is_alive(character)) {
    printf("%s has been defeated!\n", character->name);
  }
}

/**
 * @brief Heal a character by amount, clamped to its computed max health.
 * @param character Character to heal (NULL-safe).
 * @param amount Heal amount.
 */
void character_heal(Character *character, int amount) {
  NULL_CHECK_VOID(character);
  Stats stats = character_get_stats(character);
  character->curr_health += amount;

  // Clamp health to max
  if (character->curr_health > stats.max_health) {
    character->curr_health = stats.max_health;
  }
}

/**
 * @brief Apply experience to the character and mark a pending level-up if
 * threshold reached.
 * @param character Character gaining experience (NULL-safe).
 * @param xp Experience points to add.
 */
void character_gain_experience(Character *character, int xp) {
  NULL_CHECK_VOID(character);
  character->next_level_xp -= xp;

  // If we've reached or passed required XP, mark a pending level-up
  if (character->next_level_xp <= 0) {
    character->next_level_xp = 0;
    character->level_up_pending = true;
  }
}

/**
 * @brief Perform an immediate level-up: increase veterancy bonuses, heal, and
 * set next-level XP.
 * @param character Character performing the level-up (NULL-safe).
 */
void character_level_up(Character *character) {
  NULL_CHECK_VOID(character);
  character->level++;

  // Increase veterancy stats (permanent bonuses from leveling)
  character->veterancy.max_health += 3;
  character->veterancy.phys_attack += 1;
  character->veterancy.phys_defense += 1;
  character->veterancy.magic_attack += 1;
  character->veterancy.magic_defense += 1;

  // Full heal on level up
  Stats stats = character_get_stats(character);
  character->curr_health = stats.max_health;

  // Experience needed for next level (exponential growth)
  character->next_level_xp = 100 * character->level;
  // Clear pending flag when level-up is performed manually
  character->level_up_pending = false;
  printf("%s leveled up to level %d!\n", character->name, character->level);
}

// ============================================================================
// Character Queries
// ============================================================================

/**
 * @brief Promote a character to a new class, updating its sprite and name.
 * @param character The character to promote.
 * @param new_class The new UnitClass template.
 * @param new_sprite Sprite texture for the promoted class.
 * @param icons Loaded action icons (may be NULL).
 */
void character_promote(Character *character, const UnitClass *new_class,
                       Texture2D new_sprite, const ActionIcons *icons) {
  NULL_CHECK_VOID(character);
  NULL_CHECK_VOID(new_class);
  character->unit_class = new_class;
  character->sprite = new_sprite;
  strcpy(character->name, new_class->name);
  (void)icons; // Reserved for future skill reassignment on promotion
}

/**
 * @brief Check whether a character belongs to the provided faction.
 * @param character Character to test.
 * @param faction Faction to compare against.
 * @return true if the character's owner equals the faction.
 */
bool character_belongs_to_faction(Character *character, Faction *faction) {
  NULL_CHECK_RET(character, false);
  NULL_CHECK_RET(faction, false);
  return character->owner == faction;
}

/**
 * @brief Check whether two characters are enemies (different owners).
 * @param char1 First character.
 * @param char2 Second character.
 * @return true if owners differ.
 */
bool character_is_enemy(Character *char1, Character *char2) {
  NULL_CHECK_RET(char1, false);
  NULL_CHECK_RET(char2, false);
  return char1->owner != char2->owner;
}

/**
 * @brief Compute current health as a percentage of max health.
 * @param character The character to query.
 * @return Integer percentage (0-100), 0 if invalid input.
 */
int character_get_health_percentage(Character *character) {
  NULL_CHECK_RET(character, 0);
  Stats stats = character_get_stats(character);
  if (stats.max_health == 0)
    return 0;
  return (character->curr_health * 100) / stats.max_health;
}

/**
 * @brief Get the maximum range among the character's skills.
 * @param character The character to inspect.
 * @return Maximum skill range, or 0 if none.
 */
int character_get_max_skill_range(Character *character) {
  if (character == NULL)
    return 0;
  int max_range = 0;
  for (int i = 0; i < character->skill_count; i++) {
    if (character->skills[i].range > max_range) {
      max_range = character->skills[i].range;
    }
  }
  return max_range;
}

/**
 * @brief Check if the character has a pending level-up that must be applied.
 * @param character Character to query.
 * @return true if a level-up is pending.
 */
bool character_has_pending_level_up(Character *character) {
  return character->level_up_pending;
}

// ============================================================================
// Character Arrays and Groups
// ============================================================================

/**
 * @brief Allocate and initialize an array of characters from an Archetype.
 *
 * The base UnitClass is resolved automatically from the archetype.
 *
 * @param count Number of characters to create.
 * @param owner Owning faction for all characters.
 * @param sprite Sprite to assign to each character.
 * @param archetype Archetype that determines base class and equipment slots.
 * @return Pointer to allocated array or NULL on failure.
 */
Character *character_array_create_from_class(int count, Faction *owner,
                                             Texture2D sprite,
                                             Archetype archetype,
                                             const ActionIcons *icons) {
  NULL_CHECK_RET(owner, NULL);
  const UnitClass *base_class = archetype_get_base_class(archetype);
  NULL_CHECK_RET(base_class, NULL);
  Character *characters = malloc(sizeof(Character) * count);

  if (characters == NULL) {
    fprintf(stderr, "Error: Failed to allocate memory for character array\n");
    return NULL;
  }

  // Initialize all characters
  for (int i = 0; i < count; i++) {
    fprintf(stderr, "[DEBUG] character_init_from_class start i=%d\n", i);
    // Init since we have an array, use init function directly
    character_init_from_class(&characters[i], owner, sprite, base_class,
                              archetype, icons);
    fprintf(stderr, "[DEBUG] character_init_from_class done i=%d\n", i);
  }

  fprintf(stderr, "[DEBUG] character_array_create_from_class returning %p\n",
          (void *)characters);
  return characters;
}

/**
 * @brief Free an array of characters and their per-skill resources.
 * @param characters Array pointer (NULL-safe).
 * @param count Number of characters in the array.
 */
void character_array_free(Character *characters, int count) {
  if (characters != NULL) {
    for (int i = 0; i < count; i++) {
      character_cleanup_resources(&characters[i]);
    }
    free(characters);
  }
}

/**
 * @brief Reset per-turn flags for each alive character.
 * @param characters Array of characters.
 * @param count Number of characters in the array.
 */
void character_array_reset_turns(Character *characters, int count) {
  for (int i = 0; i < count; i++) {
    if (character_is_alive(&characters[i])) {
      character_reset_turn_flags(&characters[i]);
    }
  }
}

/**
 * @brief Count how many characters in the array are alive.
 * @param characters Array of characters.
 * @param count Number of characters.
 * @return Number of alive characters.
 */
int character_array_count_alive(Character *characters, int count) {
  int alive_count = 0;

  for (int i = 0; i < count; i++) {
    if (character_is_alive(&characters[i])) {
      alive_count++;
    }
  }

  return alive_count;
}
