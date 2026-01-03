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

typedef struct Faction {
  Color prim_color;
  Color sec_color;
  bool has_turn;
  bool playable;
  char name[10];
  // Each faction now owns a contiguous array of Actors and its count.
  struct Actor *actors;
  int actor_count;
} Faction;

typedef struct Actor {
  Texture2D sprite;
  Faction *owner;
  bool can_move;
  bool can_act;

  bool level_up_pending;

  int level;
  int next_level_xp;

  int max_health;
  int curr_health;

  int movement;

  int phys_attack;
  int phys_defense;

  int magic_defense;
  int magic_attack;
  int luck;
  int attack_range;

  Skill skills[5];
  int skill_count;

  char name[10];
} Actor;

typedef struct Structure {
  Texture2D sprite;
  bool passable; // can units enter this tile when structure present
  bool lootable; // can be looted
  char name[16];
} Structure;

typedef struct Point {
  int x;
  int y;
  Actor *occupant;
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