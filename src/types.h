#ifndef TYPES_H_
#define TYPES_H_

#include "raylib.h"
#include <stdbool.h>
#include <stdio.h>

/**
 * @brief Simple 2D integer coordinate used by area-of-effect definitions.
 */
typedef struct Coord {
  int x; /**< X offset */
  int y; /**< Y offset */
} Coord;

/**
 * @brief Skill definition describing an action a Character can perform.
 */
typedef struct Skill {
  char name[20];         /**< Human-readable name */
  int id;                /**< Internal skill identifier */
  int damage;            /**< Base damage value */
  bool is_magic;         /**< Flag indicating magic vs physical */
  int cooldown;          /**< Cooldown in turns (0 = usable every turn) */
  int range;             /**< Targeting range in tiles */
  Coord *area_of_effect; /**< Array of relative coordinates defining the AoE */
  int aoe_size;          /**< Number of coordinates in the AoE array */
  Texture2D icon;        /**< Icon texture used in UI */
} Skill;

struct Terrain;
typedef struct Terrain Terrain;

/**
 * @brief Terrain data for a map cell.
 */
struct Terrain {
  int id;                /**< Terrain identifier */
  Color color;           /**< Fallback color used when no sprite present */
  Texture2D sprite;      /**< Visual sprite for the terrain */
  bool passable;         /**< Whether units can enter this terrain tile */
  Terrain *deep_version; /**< Pointer to an alternate terrain variant (e.g.,
                            dug/deep) */
  char name[10];         /**< Short terrain name */
};

// Forward declarations for character/class types
struct UnitClass;
struct Character;
struct Modal;

/**
 * @brief Faction owning characters and providing color theming.
 */
typedef struct Faction {
  Color prim_color; /**< Primary faction color */
  Color sec_color;  /**< Secondary faction color */
  bool has_turn;    /**< Whether the faction currently has control */
  bool playable;    /**< Whether this faction is player-controlled */
  char name[10];    /**< Short faction name */
  struct Character *characters; /**< Contiguous array of characters owned */
  int character_count;          /**< Number of characters in the array */
} Faction;

/**
 * @brief Aggregated stats used for computations (derived from
 * class/genetics/veterancy).
 */
typedef struct Stats {
  int max_health;
  int movement;
  int phys_attack;
  int phys_defense;
  int magic_attack;
  int magic_defense;
  int luck;
} Stats;

/**
 * @brief Genetics: small random bonuses assigned when a character is created.
 */
typedef struct Genetics {
  int max_health;
  int movement;
  int phys_attack;
  int phys_defense;
  int magic_attack;
  int magic_defense;
  int luck;
} Genetics;

/**
 * @brief Veterancy: permanent stat increases gained when leveling up.
 */
typedef struct Veterancy {
  int max_health;
  int movement;
  int phys_attack;
  int phys_defense;
  int magic_attack;
  int magic_defense;
  int luck;
} Veterancy;

/**
 * @brief Promotion tree describing possible class upgrades.
 */
typedef struct ClassTree {
  struct UnitClass *promotions[4]; /**< Promotion options (up to 4) */
  int promotion_count;             /**< Number of available promotions */
} ClassTree;

/**
 * @brief UnitClass: template defining base stats for a class.
 */
typedef struct UnitClass {
  char name[10];
  int max_health;
  int movement;
  int phys_attack;
  int phys_defense;
  int magic_attack;
  int magic_defense;
  int luck;
  ClassTree class_tree;
} UnitClass;

/**
 * @brief Character instance combining a UnitClass template with genetics and
 * veterancy.
 */
typedef struct Character {
  Texture2D sprite; /**< Visual sprite */
  Faction *owner;   /**< Owning faction */
  bool can_move;    /**< Movement available this turn */
  bool can_act;     /**< Action available this turn */

  bool level_up_pending; /**< Flag for deferred manual level-up */

  int level;         /**< Current level */
  int next_level_xp; /**< XP required for next level */

  int curr_health; /**< Current HP (max derived from stats) */

  UnitClass *unit_class; /**< Pointer to class template */
  Genetics genetics;     /**< Random stat modifiers */
  Veterancy veterancy;   /**< Permanent level bonuses */

  Skill skills[5]; /**< Skill slots */
  int skill_count; /**< Number of skills in use */

  char name[10]; /**< Character name */
} Character;

/**
 * @brief Small structure placed on map cells (e.g., lair, hut).
 */
typedef struct Structure {
  Texture2D sprite;
  bool passable; /**< Can units enter the tile when the structure is present? */
  bool lootable; /**< Can be looted by a skill? */
  char name[16];
} Structure;

/**
 * @brief Map cell representing position, occupant, tints and terrain.
 */
typedef struct Point {
  int x;
  int y;
  Character *occupant;
  bool in_range;        /**< Movement highlight flag */
  bool in_attack_range; /**< Attack highlight flag */
  Structure *structure;
  Terrain terrain;
} Point;

/**
 * @brief Biome configuration used when generating terrain spreads.
 */
typedef struct {
  Terrain terrain;
  int max_cores; // Maximum number of biome cores
  int max_range; // Maximum spread range
} BiomeConfig;

/**
 * @brief Grid configuration containing offsets and cell sizes.
 */
typedef struct {
  int grid_offset_x;
  int grid_offset_y;
  int grid_cell_size;
  int max_grid_cells_x;
  int max_grid_cells_y;
} GridConfig;

// Global debug flag (defined in main.c)
extern bool DEBUG_LOG;

/**
 * @brief Null-check helper macros. Use NULL_CHECK_VOID(ptr) in void functions,
 *        and NULL_CHECK_RET(ptr, retval) for functions that return a value.
 */
#define NULL_CHECK_VOID(p)                                                     \
  do {                                                                         \
    if ((p) == NULL) {                                                         \
      if (DEBUG_LOG)                                                           \
        fprintf(stderr, "[ERR] Null pointer '%s' in %s\n", #p, __func__);      \
      return;                                                                  \
    }                                                                          \
  } while (0)
#define NULL_CHECK_RET(p, r)                                                   \
  do {                                                                         \
    if ((p) == NULL) {                                                         \
      if (DEBUG_LOG)                                                           \
        fprintf(stderr, "[ERR] Null pointer '%s' in %s\n", #p, __func__);      \
      return (r);                                                              \
    }                                                                          \
  } while (0)

#endif