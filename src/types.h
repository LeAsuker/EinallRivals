#ifndef TYPES_H_
#define TYPES_H_

#include "raylib.h"
#include <stdbool.h>

typedef struct Coord {
  int x;
  int y;
} Coord;

typedef struct Skill {
  char name[20];
  int id;
  int damage;
  bool is_magic;
  int cooldown;
  int range;
  Coord *area_of_effect; // Array of relative coordinates defining the AoE
  int aoe_size;          // Number of coordinates in the AoE array
  Texture2D icon;        // Icon representing the skill
} Skill;
struct Terrain;
typedef struct Terrain Terrain;

struct Terrain {
  int id;
  Color color;
  Texture2D sprite;
  bool passable;
  Terrain * deep_version;
  char name[10];
};

// Forward declarations for new character system
struct UnitClass;
struct Character;

typedef struct Faction {
  Color prim_color;
  Color sec_color;
  bool has_turn;
  bool playable;
  char name[10];
  // Each faction now owns a contiguous array of Characters and its count.
  struct Character *characters;
  int character_count;
} Faction;

// Stats struct - used for calculations, aggregates all stat sources
typedef struct Stats {
  int max_health;
  int movement;
  int phys_attack;
  int phys_defense;
  int magic_attack;
  int magic_defense;
  int luck;
  int attack_range;
} Stats;

// Genetics struct - random stat bonuses assigned at character creation (0-5 each)
typedef struct Genetics {
  int max_health;
  int movement;
  int phys_attack;
  int phys_defense;
  int magic_attack;
  int magic_defense;
  int luck;
  int attack_range;
} Genetics;

// Veterancy struct - stat bonuses earned through leveling (starts at 0)
typedef struct Veterancy {
  int max_health;
  int movement;
  int phys_attack;
  int phys_defense;
  int magic_attack;
  int magic_defense;
  int luck;
  int attack_range;
} Veterancy;

// ClassTree - promotion paths for a class
typedef struct ClassTree {
  struct UnitClass *promotions[4]; // Max 4 different promotion options
  int promotion_count;
} ClassTree;

// UnitClass - the class template (formerly ActorTemplate + ClassTree)
typedef struct UnitClass {
  char name[10];
  int max_health;
  int movement;
  int phys_attack;
  int phys_defense;
  int magic_attack;
  int magic_defense;
  int luck;
  int attack_range;
  ClassTree class_tree;
} UnitClass;

// Character - the new actor struct combining class, genetics, and veterancy
typedef struct Character {
  Texture2D sprite;
  Faction *owner;
  bool can_move;
  bool can_act;

  bool level_up_pending;

  int level;
  int next_level_xp;

  int curr_health; // Current health (max derived from stats)

  UnitClass *unit_class;  // Pointer to the class template
  Genetics genetics;      // Random bonuses
  Veterancy veterancy;    // Leveling bonuses

  Skill skills[5];
  int skill_count;

  char name[10]; // Individual character name
} Character;

typedef struct Structure {
  Texture2D sprite;
  bool passable; // can units enter this tile when structure present
  bool lootable; // can be looted
  char name[16];
} Structure;

typedef struct Point {
  int x;
  int y;
  Character *occupant;
  bool in_range;
  bool in_attack_range;
  Structure *structure;
  Terrain terrain;
} Point;

typedef struct {
  Terrain terrain;
  int max_cores; // Maximum number of biome cores
  int max_range; // Maximum spread range
} BiomeConfig;

typedef struct {
  int grid_offset_x;
  int grid_offset_y;
  int grid_cell_size;
  int max_grid_cells_x;
  int max_grid_cells_y;
} GridConfig;

#endif